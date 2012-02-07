/* tc-xgate.c -- Assembler code for Freescale XGATE
 Copyright 2009, 2010, 2011, 2012
 Free Software Foundation, Inc.
 Contributed by Sean Keys <skeys@ipdatasys.com>

 This file is part of GAS, the GNU Assembler.

 GAS is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 GAS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GAS; see the file COPYING.  If not, write to
 the Free Software Foundation, 51 Franklin Street - Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "opcode/xgate.h"
#include "dwarf2dbg.h"
#include "elf/xgate.h"
#include "stdio.h"

const char comment_chars[] = ";!";
const char line_comment_chars[] = "#*";
const char line_separator_chars[] = "";
const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

#define SIXTEENTH_BIT	0x8000
#define N_BITS_IN_WORD	16	/* number of bits in a word */

//#define STATE_CONDITIONAL_BRANCH		(1)
#define STATE_PC_RELATIVE				(2)
#define REGISTER_P(ptr)		(ptr == 'r')
#define INCREMENT			01
#define DECREMENT			02
#define MAXREGISTER			07
#define MINREGISTER			00

#define OPTION_MMCU 'm'


/* This macro has no side-effects.  */
#define ENCODE_RELAX(what,length) (((what) << 2) + (length))

/* what this is */
struct xgate_opcode_handle
{
  unsigned char number_of_modes;
  char *name;
  struct xgate_opcode *opc0;
  struct xgate_opcode *opc1;
  struct xgate_opcode *opc2;
  struct xgate_opcode *opc3;
  struct xgate_opcode *opc4;
};

/*  LOCAL FUNCTIONS */
static char * xgate_parse_exp (char *s, expressionS * op);
static inline char *skip_whitespace (char *);
static void get_default_target (void);	/* Pseudo op to indicate a relax group.  */
static char *extract_word (char *, char *, int);
static char *xgate_new_instruction (int size);	/* number of bytes */
unsigned short
xgate_apply_operand (unsigned short new_mask,
			unsigned short *oper_bits_availiable,
			unsigned short mask, unsigned char n_bits);
void xgate_operands (struct xgate_opcode *opcode, char **line);

static unsigned int xgate_operand (struct xgate_opcode *opcode,
				      int *bit_width, int where,
				      char **op_con, char **line);
static struct xgate_opcode * xgate_find_match (struct
						    xgate_opcode_handle
						    *opcode_handle,
						    unsigned char
						    numberOfModes,
						    unsigned int sh_format);
void append_str (char *in, char c);
static int cmp_opcode (struct xgate_opcode *, struct xgate_opcode *);
unsigned int xgate_detect_format (char *line_in);

/* LOCAL DATA */
static struct hash_control *xgate_hash;
static unsigned int prev = 0;	/* Previous opcode.  */
static unsigned char fixup_required = 0;
static unsigned char macroClipping = 0;	/* used to enable clipping of 16 bit operands into 8 bit constraints */
static char oper_check;
static char flag_print_insn_syntax = 0;
static char flag_print_opcodes = 0;

/* This table describes how you change sizes for the various types of variable
 size expressions.  This version only supports two kinds.  */

/* The fields are:
 How far Forward this mode will reach.
 How far Backward this mode will reach.
 How many bytes this mode will add to the size of the frag.
 Which mode to go to if the offset won't fit in this one.  */

relax_typeS md_relax_table[] = {
  {1, 1, 0, 0},			/* First entries aren't used.  */
  {1, 1, 0, 0},			/* For no good reason except.  */
  {1, 1, 0, 0},			/* that the VAX doesn't either.  */
  {1, 1, 0, 0},
  /* XGATE 9 and 10 bit pc rel todo complete and test */
//    {(511), (-512), 0, ENCODE_RELAX (STATE_PC_RELATIVE, STATE_WORD)},
//    {(1023), (-1024), 0, ENCODE_RELAX (STATE_PC_RELATIVE, STATE_WORD)},
  {0, 0, 0, 0}
};

/* XGATE's registers all are 16-bit general purpose.  They are numbered according to the specifications.  */
typedef enum register_id
{
  REG_NONE = -1,
  REG_R0 = 0,
  REG_R1 = 1,
  REG_R2 = 2,
  REG_R3 = 3,
  REG_R4 = 4,
  REG_R5 = 5,
  REG_R6 = 6,
  REG_R7 = 7,
  REG_PC = 8,
  REG_CCR = 9
} register_id;

/* This table describes all the machine specific pseudo-ops the assembler
 has to support.  The fields are: pseudo-op name without dot function to
  call to execute this pseudo-op Integer arg to pass to the function.  */
const pseudo_typeS md_pseudo_table[] = {
  /* The following pseudo-ops are supported for MRI compatibility.  */
  {0, 0, 0}
};

/* statics */
static int current_architecture;
static int xgate_nb_opcode_defs = 0;
static const char *default_cpu;
static int elf_flags = E_XGATE_F64; /* ELF flags to set in the output file header.  */

const char *md_shortopts = "m:";

struct option md_longopts[] =  {
#define OPTION_PRINT_INSN_SYNTAX  (OPTION_MD_BASE + 0)
        { "print-insn-syntax", no_argument, NULL, OPTION_PRINT_INSN_SYNTAX },

#define OPTION_PRINT_OPCODES  (OPTION_MD_BASE + 1)
        { "print-opcodes", no_argument, NULL, OPTION_PRINT_OPCODES },

#define OPTION_GENERATE_EXAMPLE  (OPTION_MD_BASE + 2)
        { "generate-example", no_argument, NULL, OPTION_GENERATE_EXAMPLE },

#define OPTION_MSHORT  (OPTION_MD_BASE + 3)
        { "mshort", no_argument, NULL, OPTION_MSHORT },

#define OPTION_MLONG  (OPTION_MD_BASE + 4)
        { "mlong", no_argument, NULL, OPTION_MLONG },

#define OPTION_MSHORT_DOUBLE  (OPTION_MD_BASE + 5)
        { "mshort-double", no_argument, NULL, OPTION_MSHORT_DOUBLE },

#define OPTION_MLONG_DOUBLE  (OPTION_MD_BASE + 6)
        { "mlong-double", no_argument, NULL, OPTION_MLONG_DOUBLE },

        { NULL, no_argument, NULL, 0 }
};
size_t md_longopts_size = sizeof(md_longopts);

char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, TRUE);
}

int
md_parse_option(int c, char *arg)
{
  switch (c)
    {
  case OPTION_MMCU:
    if (strcasecmp(arg, "v1") == 0)
      current_architecture = XGATE_V1;
    else if (strcasecmp(arg, "v2") == 0)
      current_architecture = XGATE_V2;
    else if (strcasecmp(arg, "v3") == 0)
      current_architecture = XGATE_V3;
    else
      as_bad(_(" architecture variant invalid"));
    break;

  case OPTION_PRINT_INSN_SYNTAX:
    flag_print_insn_syntax = 1;
    break;

  case OPTION_PRINT_OPCODES:
    flag_print_opcodes = 1;
    break;

  case OPTION_GENERATE_EXAMPLE:
    flag_print_opcodes = 2;
    break;

  case OPTION_MSHORT:
    elf_flags &= ~E_XGATE_I32;
    break;

  case OPTION_MLONG:
    elf_flags |= E_XGATE_I32;
    break;

  case OPTION_MSHORT_DOUBLE:
    elf_flags &= ~E_XGATE_F64;
    break;

  case OPTION_MLONG_DOUBLE:
    elf_flags |= E_XGATE_F64;
    break;

  default:
    return 0;
    }
  return 1;
}

const char *
xgate_arch_format (void)
{
  get_default_target ();
  if (current_architecture & cpuxgate)
    {
      return "elf32-xgate";
    }
  else
    {
      return "error";
    }
}

static void
get_default_target (void)
{
  const bfd_target *target;
  bfd abfd;
  if (current_architecture != 0)
    return;
  default_cpu = "unknown";
  target = bfd_find_target (0, &abfd);
  if (target && target->name)
    {
      if (strcmp (target->name, "elf32-xgate") == 0)
	{
	  current_architecture = cpuxgate;
	  default_cpu = "XGATE V1";
	  return;
	}
      else
	{
	  as_bad (_("Default target `%s' is not supported."), target->name);
	}
    }
}

void
md_begin (void)
{
  struct xgate_opcode *xgate_opcode_ptr = NULL;
  struct xgate_opcode *xgate_op_table = NULL;
  struct xgate_opcode_handle *op_handles = 0;
  char prev_op_name[9];
  char handle_enum = 0;
  unsigned int number_of_handle_rows = 0;
  int i, j = 0;
  /* create a local copy of our opcode table including and an extra line for NULL termination */
  xgate_op_table =
    (struct xgate_opcode *) xmalloc ((xgate_num_opcodes + 1) *
					sizeof (struct xgate_opcode));
  memset (xgate_op_table, 0,
	  sizeof (struct xgate_opcode) * (xgate_num_opcodes + 1));
  for (xgate_opcode_ptr = (struct xgate_opcode*) xgate_opcodes, i = 0;
       i < xgate_num_opcodes; i++)
    {
      xgate_op_table[i] = xgate_opcode_ptr[i];
    }
  qsort(xgate_op_table, xgate_num_opcodes,
      sizeof(struct xgate_opcode), (int
      (*)(const void *, const void *)) cmp_opcode);
  /* Calculate number of handles since this will be smaller than the raw number of opcodes in the table */
  for (xgate_opcode_ptr = xgate_op_table; xgate_opcode_ptr->name;
       xgate_opcode_ptr++)
    {
      if (strcmp (prev_op_name, xgate_opcode_ptr->name))
	{
	  number_of_handle_rows++;
	}
      strcpy (prev_op_name, xgate_opcode_ptr->name);
    }
  op_handles =
    (struct xgate_opcode_handle *)
    xmalloc (sizeof (struct xgate_opcode_handle) *
	     (number_of_handle_rows + 1));
  /* insert opcode names into hash table, aliasing duplicates */
  /* TODO do this dynamically with xmalloc and a loop */
  xgate_hash = hash_new ();	/* create a new has control table */
  for (xgate_opcode_ptr = xgate_op_table, i = 0, j = 0; i
       < xgate_num_opcodes; i++, xgate_opcode_ptr++)
    {
      if (strcmp (prev_op_name, xgate_opcode_ptr->name))
	{
	  handle_enum = 1;
	  if (i)
	    {
	      j++;
	    }
	  op_handles[j].name = xgate_opcode_ptr->name;
	  op_handles[j].opc0 = xgate_opcode_ptr;
	}
      else
	{
	  handle_enum++;
	  switch (handle_enum)
	    {
	    case 2:
	      op_handles[j].opc1 = xgate_opcode_ptr;
	      break;
	    case 3:
	      op_handles[j].opc2 = xgate_opcode_ptr;
	      break;
	    case 4:
	      op_handles[j].opc3 = xgate_opcode_ptr;
	      break;
	    case 5:
	      op_handles[j].opc4 = xgate_opcode_ptr;
	      break;
	    default:
	      as_bad (_(":error adding operand handle"));
	      break;
	    }
	}
      op_handles[j].number_of_modes = handle_enum;
      strcpy (prev_op_name, op_handles[j].name);
    }
  while (op_handles->name)
    {
      hash_insert (xgate_hash, op_handles->name, (char *) op_handles);
      op_handles++;
    }
}

void
xgate_init_after_args (void)
{
}

void
md_show_usage(FILE * stream)
{
  get_default_target();
  fprintf(
      stream,
      _("\
      Freescale XGATE co-processor options:\n\
       -mshort                 use 16-bit int ABI (default)\n\
       -mlong                  use 32-bit int ABI\n\
       -mshort-double          use 32-bit double ABI\n\
       -mlong-double           use 64-bit double ABI (default)\n\
      --mxgate                 specify the processor variant[default %s]\n\
      --print-insn-syntax     print the syntax of instruction in case of error\n\
      --print-opcodes         print the list of instructions with syntax\n\
      --generate-example      generate an example of each instruction"),
      default_cpu);
}

enum bfd_architecture
xgate_arch (void)
{
  get_default_target ();
  return bfd_arch_xgate;
}

int
xgate_mach (void)
{
  return 0;
}

void
xgate_print_statistics (FILE * file)
{
  //  int i;
  struct xgate_opcode *opc;
  hash_print_statistics (file, "opcode table", xgate_hash);
  opc = (struct xgate_opcode*)xgate_opcodes;
  if (opc == 0 || xgate_nb_opcode_defs == 0)
    return;
  /* Dump the opcode statistics table.  */
  //  fprintf (file, _("Name   # Modes  Min ops  Max ops  Modes mask  # Used\n"));
  //  for (i = 0; i < xgate_nb_opcode_defs; i++, opc++)
  //   {
  //     fprintf (file, "%-7.7s  %5d  %7d  %7d  0x%08lx  %7d\n",
  //           opc->opcode->name,
  //           opc->nb_modes,
  //           opc->min_operands, opc->max_operands, opc->format, opc->used);
  //    }
}

const char *
xgate_listing_header (void)
{
  if (current_architecture & cpuxgate)
    return "XGATE GAS ";
  else
    return "ERROR MC9S12X GAS ";
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

/* GAS will call this function for each section at the end of the assembly,
   to permit the CPU backend to adjust the alignment of a section.  */

valueT
md_section_align (asection * seg, valueT addr)
{
  int align = bfd_get_section_alignment (stdoutput, seg);
  return ((addr + (1 << align) - 1) & (-1 << align));
}

void
md_assemble(char *input_line)
{
  struct xgate_opcode *opcode = 0;
  struct xgate_opcode *macro_opcode = 0;
  struct xgate_opcode_handle *opcode_handle = 0;
  char *saved_input_line = input_line; /* caller expects it to be returned as it was passed */
  char op_name[9] = { 0 };
  char handle_enum_alias = 0;
  unsigned int sh_format = 0;
  char *p = 0;
  fixup_required = 0;
  oper_check = 0; /* set error flags */
  input_line = extract_word(input_line, op_name, sizeof(op_name));
  /* check to make sure we are not reading a bogus line */
  if (!op_name[0])
    as_bad(_("opcode missing or not found on input line"));
  if (!(opcode_handle = (struct xgate_opcode_handle *) hash_find(
      xgate_hash, op_name)))
    {
      as_bad(_("opcode not found in hash"));
    }
  else
    {
      /* detect operand format so we can pull the proper opcode bin */
      handle_enum_alias = opcode_handle->number_of_modes;
      sh_format = xgate_detect_format(input_line);
      if (handle_enum_alias > 1)
        {
          opcode = xgate_find_match(opcode_handle,
              opcode_handle->number_of_modes, sh_format);
        }
      else
        {
          /* todo i suspect that this code should not be here if xgate_find_match was more thorough ? */
          opcode = opcode_handle->opc0;
        }
      if (!opcode)
        {
          as_bad(_(":error matching operands to opcode"));
          //TODO print list of possibilities
        }
      else if (opcode->size == 2)
        { /* if size is one word assemble that native insn */
          xgate_operands(opcode, &input_line);
        }
      else
        { /* if insns is a simplified instruction expand it out */
          macroClipping = 1;
          unsigned int i;
          /* skip past our ';' separator */
          for (i = strlen(opcode->constraints), p = opcode->constraints;  i > 0; i--, p++)
            {
              if (*p == ';')
                {
                  p++;
                  break;
                }
            }
          input_line = skip_whitespace(input_line);
          char *macro_inline = input_line;
          /* loop though the macro's opcode list and apply operands to each real opcode*/
          for (i = 0; *p && i < (opcode->size / 2); i++)
            { /* loop though macro operand list */
              input_line = macro_inline; /* rewind */
              p = extract_word(p, op_name, 10);
              if (!(opcode_handle = (struct xgate_opcode_handle *) hash_find(
                  xgate_hash, op_name)))
                {
                  as_bad(_(": processing macro, real opcode handle not found in hash"));
                  break;
                }
              else
                {
                  sh_format = xgate_detect_format(input_line);
                  macro_opcode = xgate_find_match(opcode_handle,
                      opcode_handle->number_of_modes, sh_format);
                  xgate_operands(macro_opcode, &input_line);
                }
            }
        }
    }
  macroClipping = 0;
  input_line = saved_input_line;
}
/* Force truly undefined symbols to their maximum size, and generally set up
 the frag list to be relaxed.  */
int
md_estimate_size_before_relax (fragS *fragp, asection *seg)
{
  /* If symbol is undefined or located in a different section,
     select the largest supported relocation.  */
  relax_substateT subtype;
  relax_substateT rlx_state[] = {0, 2};

  for (subtype = 0; subtype < ARRAY_SIZE (rlx_state); subtype += 2)
    {
      if (fragp->fr_subtype == rlx_state[subtype]
          && (!S_IS_DEFINED (fragp->fr_symbol)
              || seg != S_GET_SEGMENT (fragp->fr_symbol)))
        {
          fragp->fr_subtype = rlx_state[subtype + 1];
          break;
        }
    }

  if (fragp->fr_subtype >= ARRAY_SIZE (md_relax_table))
    abort ();

  return md_relax_table[fragp->fr_subtype].rlx_length;
}

//long
//xgate_relax_frag (segT seg ATTRIBUTE_UNUSED, fragS * fragP,
//		     long stretch ATTRIBUTE_UNUSED)
//{
//  //TODO
//}

/* Relocation, relaxation and frag conversions.  */

/* PC-relative offsets are relative to the start of the
 next instruction.  That is, the address of the offset, plus its
 size, since the offset is always the last part of the insn.  */
long
md_pcrel_from (fixS * fixP)
{
  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

/* If while processing a fixup, a reloc really needs to be created
 then it is done here.  */
arelent *
tc_gen_reloc(asection * section ATTRIBUTE_UNUSED, fixS * fixp)
{
  arelent *reloc;
  reloc = (arelent *) xmalloc(sizeof(arelent));
  reloc->sym_ptr_ptr = (asymbol **) xmalloc(sizeof(asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym(fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  if (fixp->fx_r_type == 0)
    reloc->howto = bfd_reloc_type_lookup(stdoutput, BFD_RELOC_16);
  else
    {
      reloc->howto = bfd_reloc_type_lookup(stdoutput, fixp->fx_r_type);
    }
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where(fixp->fx_file, fixp->fx_line, _
      ("Relocation %d is not supported by object file format."),
          (int) fixp->fx_r_type);
      return NULL;
    }
  /* Since we use Rel instead of Rela, encode the vtable entry to be
   used in the relocation's section offset.  */
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    reloc->address = fixp->fx_offset;
  reloc->addend = 0;
  return reloc;
}

/* Patch the instruction with the resolved operand.  Elf relocation
 info will also be generated to take care of linker/loader fixups.
 The XGATE addresses only 16-bit addresses.The BFD_RELOC_32 is necessary
 for the support of --gstabs. */
void
md_apply_fix(fixS * fixP, valueT * valP, segT seg ATTRIBUTE_UNUSED)
{
  char *where;
  long value = *valP;
  int opcode = 0;
  ldiv_t result;
  /* if the fixup is done mark it done so no further symbol resolution will take place */
  if (fixP->fx_addsy == (symbolS *) NULL)
    {
      fixP->fx_done = 1;
    }
  /* We don't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != (symbolS *) NULL)
    as_bad_where(fixP->fx_file, fixP->fx_line, _("Expression too complex."));
  where = fixP->fx_frag->fr_literal + fixP->fx_where;
  opcode = bfd_getl16(where);
  int mask = 0;

  switch (fixP->fx_r_type)
    {
  case R_XGATE_PCREL_9:
    if (value < -512 || value > 511)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value %ld too large for 9-bit PC-relative branch."), value);
    result = ldiv(value, 2); /* from bytes to words */
    value = result.quot;
    if (result.rem)
      as_bad_where(fixP->fx_file, fixP->fx_line, _
      ("Value %ld not aligned by 2 for 9-bit PC-relative branch."), value);
    mask = 0x1FF; /* Clip into 8-bit field FIXME I'm sure there is a more proper place for this */
    value &= mask;
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case R_XGATE_PCREL_10:
    if (value < -1024 || value > 1023)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value %ld too large for 10-bit PC-relative branch."), value);
    result = ldiv(value, 2); /* from bytes to words */
    value = result.quot;
    if (result.rem)
      as_bad_where(fixP->fx_file, fixP->fx_line, _
      ("Value %ld not aligned by 2 for 10-bit PC-relative branch."), value);
    mask = 0x3FF; /* Clip into 9-bit field FIXME I'm sure there is a more proper place for this */
    value &= mask;
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_XGATE_IMM8_HI:
    if (value < -65537 || value > 65535)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 16-bit range."));
    value >>= 8;
    value &= 0x00ff;
    bfd_putb16((bfd_vma) value | opcode, (void *) where);
    break;
  case BFD_RELOC_XGATE_IMM8_LO:
    if (value < -65537 || value > 65535)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 16-bit range."));
    value &= 0x00ff;
    bfd_putb16((bfd_vma) value | opcode, (void *) where);
    break;
  case BFD_RELOC_XGATE_IMM3:
    if (value < 0 || value > 7)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 3-bit range."));
    value <<= 8; /* make big endian */
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_XGATE_IMM4:
    if (value < 0 || value > 15)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 4-bit range."));
    value <<= 4; /* align the operand bits */
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_XGATE_IMM5:
    if (value < 0 || value > 31)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 5-bit range."));
    value <<= 5; /* align the operand bits */
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_32:
    break;
  default:
    as_fatal(_("Line %d: unknown relocation type: 0x%x."), fixP->fx_line,
        fixP->fx_r_type);
    break;
    }
}

/* See whether we need to force a relocation into the output file.  */
int
tc_xgate_force_relocation (fixS * fixP)
{
  //printf("\n in force_relocation");
  //if (fixP->fx_r_type == BFD_RELOC_XGATE_RL_GROUP)
  //  return 1;
  return generic_force_reloc (fixP);
}

/* Here we decide which fixups can be adjusted to make them relative
 to the beginning of the section instead of the symbol.  Basically
 we need to make sure that the linker relaxation is done
 correctly, so in some cases we force the original symbol to be
 used.  */
int
tc_xgate_fix_adjustable (fixS * fixP)
{
  switch (fixP->fx_r_type)
    {
/*       For the linker relaxation to work correctly, these relocs
         need to be on the symbol itself. */

    case BFD_RELOC_16:
    case BFD_RELOC_XGATE_RL_JUMP:
    case BFD_RELOC_XGATE_RL_GROUP:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_32:
      return 0;
    default:
      return 1;
    }
}

void
md_convert_frag (bfd * abfd ATTRIBUTE_UNUSED, asection * sec ATTRIBUTE_UNUSED,
		 fragS * fragP)
{
  int temp = fragP->fr_address;
  temp++;
  return;
}

/* Set the ELF specific flags.  */
void
xgate_elf_final_processing (void)
{
  elf_flags |= EF_XGATE_MACH;
  elf_elfheader (stdoutput)->e_flags &= ~EF_XGATE_ABI;
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}

static inline char *
skip_whitespace (char *s)
{
  while (*s == ' ' || *s == '\t' || *s == '(' || *s == ')')
    {
      s++;
    }
  return s;
}

/* extract a word (continuous alpha-numeric chars) from the input line */
static char *
extract_word (char *from, char *to, int limit)
{
  char *op_end;
  int size = 0;
  /* Drop leading whitespace.  */
  from = skip_whitespace (from);
  *to = 0;
  /* Find the op code end.  */
  for (op_end = from; *op_end != 0 && is_part_of_name (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
	break;
    }
  to[size] = 0;
  return op_end;
}

static char *
xgate_new_instruction (int size)
{
  char *f = frag_more (size);
  dwarf2_emit_insn (size);
  return f;
}

unsigned short
xgate_apply_operand(unsigned short new_mask,
    unsigned short *availiable_mask_bits, unsigned short mask,
    unsigned char n_bits)
{
  unsigned short n_shifts;
  unsigned int n_drop_bits;
  /* shift until you find an available operand bit "1" and record the number of shifts */
  for (n_shifts = 0; !(*availiable_mask_bits & SIXTEENTH_BIT) && n_shifts < 16;
      n_shifts++)
    {
      *availiable_mask_bits <<= 1;
    }
  /* shift for the number of bits your operand requires while bits are available */
  for (n_drop_bits = n_bits;
      n_drop_bits && (*availiable_mask_bits & SIXTEENTH_BIT); --n_drop_bits)
    {
      *availiable_mask_bits <<= 1;

    }
  if (n_drop_bits)
    as_bad(_(":operand has too many bits"));
  *availiable_mask_bits >>= n_shifts + n_bits;
  if ((n_drop_bits == 0) && (*availiable_mask_bits == 0))
    {
      oper_check = 1; /* flag operand check as good */
    }
  new_mask <<= N_BITS_IN_WORD - (n_shifts + n_bits);
  mask |= new_mask;
  return mask;
}

/* Parse ordinary expression.  */
static char *
xgate_parse_exp (char *s, expressionS * op)
{
  input_line_pointer = s;
  expression (op);
  if (op->X_op == O_absent)
    as_bad (_("missing operand"));
  return input_line_pointer;
}

/* for testing comment out to prevent defined but not used warning

static unsigned int xgate_get_constant(char *str, int max)
{
  expressionS ex;

  str = skip_whitespace(str);
  input_line_pointer = str;
  expression (& ex);

  if (ex.X_op != O_constant)
    as_bad(_("constant value required"));

  if (ex.X_add_number > max || ex.X_add_number < 0)
    as_bad(_("number must be positive and less than %d"), max + 1);

  return ex.X_add_number;
}
*/

void
append_str (char *in, char c)
{
  unsigned char length = (unsigned char) strlen (in);
  in[length] = c;
  in[++length] = 0;		/* terminate */
  return;
}

static int
cmp_opcode (struct xgate_opcode *op1, struct xgate_opcode *op2)
{
  return strcmp (op1->name, op2->name);
}

/* Parse instruction operands */
void
xgate_operands(struct xgate_opcode *opcode, char **line)
{
  char *frag = xgate_new_instruction(opcode->size);
  int where = frag - frag_now->fr_literal;
  char *op = opcode->constraints;
  unsigned int bin = (int) opcode->bin_opcode;
  char *str = *line;
  /* TODO Structure this */
  unsigned short oper_mask = 0;
  int operand_bit_length = 0;
  unsigned int operand = 0;
  char n_operand_bits = 0;
  char first_can_equal_second = 0;
  int i = 0;
  char c = 0;

  /* generate available operand bits mask */
  for (i = 0; (c = opcode->format[i]); i++)
    {
      if (ISDIGIT (c) || (c == 's'))
        {
          oper_mask <<= 1;
        }
      else
        {
          oper_mask <<= 1;
          oper_mask += 1;
          n_operand_bits++;
        }
    }
  /* Opcode has operands.  */
  /* Parse first operand.  */
  if (*op)
    {
      if (*op == '=')
        {
          first_can_equal_second = 1;
          ++op;
        }
      operand = xgate_operand(opcode, &operand_bit_length, where, &op, &str);
      ++op;
      bin = xgate_apply_operand(operand, &oper_mask, bin, operand_bit_length);
      /* Parse second operand.  */
      if (*op)
        {
          if (*op == ',')
            ++op;
          str = skip_whitespace(str);
           if (*str++ != ',')
            {
              if (first_can_equal_second)
                {
                  bin = xgate_apply_operand(operand, &oper_mask, bin, operand_bit_length);
                  ++op;
                }
              else
                {
                  as_bad(_("`,' required before second operand"));
                }
            }
          else
            {
              str = skip_whitespace(str);
              operand = xgate_operand(opcode, &operand_bit_length, where, &op,
                  &str);
              ++op;
            }
           bin = xgate_apply_operand(operand, &oper_mask, bin, operand_bit_length);
        }
      /* parse the third register */
      if (*op)
        {
          if (*op == ',')
            ++op;
          str = skip_whitespace(str);
          if (*str++ != ',')
            as_bad(_("`,' required before third operand"));
          str = skip_whitespace(str);
          operand = xgate_operand(opcode, &operand_bit_length, where, &op, &str);
          bin = xgate_apply_operand(operand, &oper_mask, bin, operand_bit_length);
        }
    }
  if (opcode->size == 2 && fixup_required)
    {
      bfd_putl16(bin, frag);
    }
  else if ((opcode->sh_format & XGATE_PCREL))
    {
      bfd_putl16(opcode->bin_opcode, frag); /* write our data to a frag for further processing */
    }
  else
    { /* apply operand mask(s)to bin opcode and write the output */
      number_to_chars_bigendian(frag, bin, opcode->size); /* since we are done write this frag in xgate BE format */
    }
  prev = bin;
  *line = str;
  return;
}

static unsigned int
xgate_operand (struct xgate_opcode *opcode, int *bit_width, int where,
		  char **op_con, char **line)
{
  expressionS op_expr;
  char *op_constraint = *op_con;
  unsigned int op_mask = 0;
  char *str = skip_whitespace (*line);
  char r_name[20] = { 0 };
  unsigned int pp_fix = 0;
  unsigned short max_size = 0;
//  char reg_expected = 0;
  int i;
  *bit_width = 0;
  /* reset */
  switch (*op_constraint)
    {
      /* TODO should be able to combine with with case R */
    case '+':			/* indexed register operand +/- or plain r  */
      pp_fix = 0; /* default to neither inc or dec */
      //TODO maybe use a loop so the order is not important
      *bit_width = 5;
      str = skip_whitespace (str);
      while (*str != ' ' && *str != '\t')
	{
	  if (*str == '-')
	    pp_fix = DECREMENT;
	  else if (*str == '+')
	    pp_fix = INCREMENT;
	  else if (*str == 'r' || *str == 'R')
	    {
//	      reg_expected = 1;
	      str = extract_word (str, r_name, sizeof (r_name));
	      if (ISDIGIT (r_name[1]))
		{
		  if (r_name[2] == '\0' && (r_name[1] - '0' < 8))
		    op_mask = r_name[1] - '0';
		  if (r_name[2] != '\0' && (r_name[1] - '0' > 7))
		    as_bad (_(": expected register name r0-r7 read %s"),
			    r_name);
		  //else{ //if (r_name[1] != '0'
		  //      && ISDIGIT (r_name[2])
		  //      && r_name[3] == '\0')
		  //      op_mask = (r_name[1] - '0') * 10 + r_name[2] - '0';
		  /* TODO fix so it checks for range 0-7 */
		  continue;
		  as_bad (_
			  (": not digit expected register name r0-r7 read %s"),
			  r_name);
		}
	    }
	  str++;
	}
      op_mask <<= 2;
      op_mask |= pp_fix;
      break;
    case 'r':	/* register operand.  */
      if (*str == 'r' || *str == 'R')
	{
	  *bit_width = 3;
	  str = extract_word (str, r_name, sizeof (r_name));
	  op_mask = 0xff;
	  if (ISDIGIT (r_name[1]))
	    {
	      if (r_name[2] == '\0')
		op_mask = r_name[1] - '0';
	      else if (r_name[1] != '0' && ISDIGIT (r_name[2]) && r_name[3]
		       == '\0')
		op_mask = (r_name[1] - '0') * 10 + r_name[2] - '0';
	      if (op_mask > MAXREGISTER)
		as_bad (_(": expected register name r0-r7 read %s "), r_name);
	    }
	}
      else
	{
	  as_bad (_(": expected register name r0-r7 read %s "), r_name);
	}
      break;
    case 'i':	/* immediate value or expression expected */
      (*op_con)++; /* advance the original format pointer */
      op_constraint++;
      if (ISDIGIT (*op_constraint))
	{
	  *bit_width = (int) *op_constraint - '0';
	}
      else if (*op_constraint == 'a')
	{
	  *bit_width = 0x0A;
	}
      else if (*op_constraint == 'f')
	{
	  *bit_width = 0x0F;
	}
      //as_bad(_(":expected numerical value after i constraint"));
      /* todo test not treating the # as constant value required, instead just treat it as *immediate value
       * http://tigcc.ticalc.org/doc/gnuasm.html#SEC31
       */
      if (*str == '#')
	str++;
      str = xgate_parse_exp (str, &op_expr);
      if (op_expr.X_op == O_constant)
	{
	  if (!ISDIGIT (*op_constraint))
	    as_bad (_
		    (":expected bit length with constraint type i(# immediate) read %c"),
		    *op_constraint);
	  //op_mask = xgate_get_constant(str, 0xFFFF);
	  op_mask = op_expr.X_add_number;
	  if ((opcode->name[strlen (opcode->name) - 1] == 'l')
	      && macroClipping)
	    {
	      op_mask &= 0x00FF;
	    }
	  else if ((opcode->name[strlen (opcode->name) - 1]) == 'h'
		   && macroClipping)
	    {
	      op_mask >>= 8;
	    }
	  // make sure it fits
	  for (i = *bit_width; i; i--)
	    {
	      max_size <<= 1;
	      max_size += 1;
	    }
	  if (op_mask > max_size)
	    as_bad (_(":operand value(%d) too big for constraint"), op_mask);
	}
      else
	{
	  fixS *fixp = 0;
	  fixup_required = 1;
	  if (*op_constraint == '8')
	    {
	      if ((opcode->name[strlen (opcode->name) - 1] == 'l')
		  && macroClipping)
		{
		  fixp = fix_new_exp (frag_now, where, 2, &op_expr, FALSE, BFD_RELOC_XGATE_IMM8_LO);
		  fixp->fx_pcrel_adjust = 0;
		}
	      if ((opcode->name[strlen (opcode->name) - 1]) == 'h'
		  && macroClipping)
		{
		  fixp = fix_new_exp (frag_now, where, 2, &op_expr, FALSE, BFD_RELOC_XGATE_IMM8_HI);
		  fixp->fx_pcrel_adjust = 0;
		}
	      if (!fixp)
		{
		  as_bad (_(":unknown relocation"));
		}
	    }
	  else if (*op_constraint == '5')
	    {
	      fixp = fix_new_exp (frag_now, where, 2, &op_expr, FALSE, BFD_RELOC_XGATE_IMM5);	/* BFD_RELOC_XGATE_IMM5 forced type into bfd-in-2 around line 2367 R_XGATE_HI8 */
	      fixp->fx_pcrel_adjust = 0;
	    }
	  else if (*op_constraint == '4')
	    {
	      fixp = fix_new_exp (frag_now, where, 2, &op_expr, FALSE, BFD_RELOC_XGATE_IMM4);	/* BFD_RELOC_XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_XGATE_HI8 */
	      fixp->fx_pcrel_adjust = 0;
	    }
	  else if (*op_constraint == '3')
	    {
	      fixp = fix_new_exp (frag_now, where, 2, &op_expr, FALSE, BFD_RELOC_XGATE_IMM3);	/* BFD_RELOC_XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_XGATE_HI8 */
	      fixp->fx_pcrel_adjust = 0;
	    }
	  else
	    {
	      as_bad (_(":unknown relocation constraint size"));
	    }
	}
      break;
    case 'c':   /* CCR register expected */
      if (*str == 'c' || *str == 'C')
	{
	  *bit_width = 0;
	  str = extract_word (str, r_name, sizeof (r_name));
	  if (!(strcmp (r_name, "ccr") || strcmp (r_name, "CCR")))
	    as_bad (_(": expected register name ccr read %s "), r_name);
	}
      else
	{
	  as_bad (_(": expected character c or C  read %c"), *str);
	}
      break;
    case 'p':   /* PC register expected */
      if (*str == 'p' || *str == 'P')
	{
	  *bit_width = 0;
	  str = extract_word (str, r_name, sizeof (r_name));
	  if (!(strcmp (r_name, "pc") || strcmp (r_name, "PC")))
	    as_bad (_(": expected register name pc read %s "), r_name);
	}
      else
	{
	  as_bad (_(": expected character p or P read %c "), *str);
	}
      break;
    case 'b':   /* branch expected */
      str = xgate_parse_exp (str, &op_expr);
      (*op_con)++;
      op_constraint++;
      if (op_expr.X_op != O_register)
	{
	  if (*op_constraint == '9')
	    {   /* mode == M68XG_OP_REL9 */
	      fixS *fixp;
	      fixp = fix_new_exp (frag_now, where, 2, &op_expr, TRUE, R_XGATE_PCREL_9);	/* forced type into bfd-in-2 around line 2367 */
	      fixp->fx_pcrel_adjust = 1;
	    }
	  else if (*op_constraint == 'a')
	    {	/* mode == M68XG_OP_REL10 */
	      fixS *fixp;
	      fixp = fix_new_exp (frag_now, where, 2, &op_expr, TRUE, R_XGATE_PCREL_10);	/* forced type into bfd-in-2 around line 2367 */
	      fixp->fx_pcrel_adjust = 1;
	    }
	}
      else
	{
	  as_fatal (_("Operand `%x' not recognized in fixup8."),
		    op_expr.X_op);
	}
      break;
    case '?':
      break;
    default:
      as_bad (_("unknown constraint `%c'"), *op_constraint);
      break;
    }
  *line = str;
  return op_mask;
}

unsigned int
xgate_detect_format (char *line_in)
{
  char num_operands = 0;
  char *str = skip_whitespace (line_in);
  int i = 0;
  int j = 0;
  char c = 0;
  unsigned int stripped_length = 0;
  char sh_format[10] = { 0 };	/* shorthand format */
  char operands_stripped[3][20] = { {0} };
  /* if the length is zero this is an inherent instruction */
  if (strlen (str) == 0)
    {
      return XGATE_INH;
    }
  for (i = 0, j = 0, num_operands = 1; (c = TOLOWER (*str)) != 0; str++)
    {
      if (c == ' ' || c == '\t' || c == '(' || c == ')' || c == '-'
	  || c == '+')
	{
	  continue;
	}
      if (c == ',')
	{
	  j++;
	  num_operands++;
	  i = 0;
	  continue;
	}
      if (i > 20)
	{			/* we should never need more than 20 chars to figure out what it is */
	  continue;
	}
      operands_stripped[j][i++] = c;
    }
  /* process our substrings to see what we have */
  for (i = 0, j = 0; num_operands > i; i++)
    {
      stripped_length = strlen (&operands_stripped[i][0]);
      /* add separator if we have more than one operand */
      if (i > 0)
	{
	  sh_format[j++] = ',';
	}
      /* try to process by length first */
      if (stripped_length > 3)
	{
	  sh_format[j++] = 'i';
	}
      else if (stripped_length == 1)
	{
	  sh_format[j++] = 'i';
	}
      else if (stripped_length == 2)
	{
	  if (operands_stripped[i][0] == 'r'
	      && ISDIGIT (operands_stripped[i][1]))
	    {
	      sh_format[j++] = 'r';
	    }
	  else if (operands_stripped[i][0] == 'p'
		   && operands_stripped[i][1] == 'c')
	    {
	      sh_format[j++] = 'p';
	    }
	  else
	    {
	      sh_format[j++] = 'i';
	    }
	}
      else if (stripped_length == 3)
	{
	  if (operands_stripped[i][0] == 'c'
	      && (operands_stripped[i][1] == 'c'
		  && operands_stripped[i][2] == 'r'))
	    {
	      sh_format[j++] = 'c';
	    }
	  else if (operands_stripped[i][0] == '#')
	    {
	      sh_format[j++] = 'i';
	    }
	  else
	    {
	      sh_format[j++] = 'i';
	    }
	}
      else
	{			//default to immediate
	  sh_format[j++] = 'i';
	}
    }
  /* strings TODO maybe structure this */
  char *i_string = { "i" };
  char *r_string = { "r" };
  char *r_r_string = { "r,r" };
  char *r_r_r_string = { "r,r,r" };
  char *r_i_string = { "r,i" };
  char *r_c_string = { "r,c" };
  char *c_r_string = { "c,r" };
  char *r_p_string = { "r,p" };
  char *r_r_i_string = { "r,r,i" };
  /* TODO Clean this */
  if (!strcmp (i_string, sh_format) && num_operands == 1)
    return XGATE_I;
  if (!strcmp (r_i_string, sh_format) && num_operands == 2)
    return XGATE_R_I;
  if (!strcmp (r_r_r_string, sh_format) && num_operands == 3)
    return XGATE_R_R_R;
  if (!strcmp (r_r_string, sh_format) && num_operands == 2)
    return XGATE_R_R;
  if (!strcmp (r_string, sh_format) && num_operands == 1)
    return XGATE_R;
  if (!strcmp (r_c_string, sh_format) && num_operands == 2)
    return XGATE_R_C;
  if (!strcmp (c_r_string, sh_format) && num_operands == 2)
    return XGATE_C_R;
  if (!strcmp (r_p_string, sh_format) && num_operands == 2)
    return XGATE_R_P;
  if (!strcmp (r_r_i_string, sh_format) && num_operands == 3)
    {
      return XGATE_R_R_I;
    }
  as_bad ((":Error unable to detect operand format"));
  return 0;
}

static struct xgate_opcode *
xgate_find_match (struct xgate_opcode_handle *opcode_handle,
		     unsigned char numberOfModes, unsigned int sh_format)
{
  struct xgate_opcode *opcode = 0;
  /* TODO make this into a loop */
  while (numberOfModes)
    {
      switch (numberOfModes)
	{
	case 1:
	  if (opcode_handle->opc0->sh_format & sh_format)
	    opcode = opcode_handle->opc0;
	  break;
	case 2:
	  if (opcode_handle->opc1->sh_format & sh_format)
	    opcode = opcode_handle->opc1;
	  break;
	case 3:
	  if (opcode_handle->opc2->sh_format & sh_format)
	    opcode = opcode_handle->opc2;
	  break;
	case 4:
	  if (opcode_handle->opc3->sh_format & sh_format)
	    opcode = opcode_handle->opc3;
	  break;
	case 5:
	  if (opcode_handle->opc2->sh_format & sh_format)
	    opcode = opcode_handle->opc2;
	  break;
	default:
	  as_bad ((":Error too many operand to opcode combinations"));
	  break;
	}
      numberOfModes--;
    }
  return opcode;
}
