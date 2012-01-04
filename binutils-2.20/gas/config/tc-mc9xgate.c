/* tc-mc9xgate.c -- Assembler code for the Freescale XGATE
 Copyright 1999, 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010
 Free Software Foundation, Inc.
 Contributed by Sean Keys <info@powerefi.com>

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
#include "opcode/mc9xgate.h"
#include "dwarf2dbg.h"
#include "elf/mc9xgate.h"
#include "stdio.h" /* TODO only here for testing */

const char comment_chars[] = ";!";
const char line_comment_chars[] = "#*";
const char line_separator_chars[] = "";

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

#define SIXTEENTH_BIT	0x8000
#define N_BITS_IN_WORD	16	/* number of bits in a word */

#define STATE_CONDITIONAL_BRANCH		(1)
#define STATE_PC_RELATIVE				(2)
#define STATE_INDEXED_OFFSET            (3)
#define STATE_INDEXED_PCREL             (4)
#define STATE_XBCC_BRANCH               (5)
#define STATE_CONDITIONAL_BRANCH_6812	(6)

#define STATE_BYTE			(0)
#define STATE_BITS5                     (0)
#define STATE_WORD			(1)
#define STATE_BITS9                     (1)
#define STATE_LONG			(2)
#define STATE_BITS16                    (2)
#define STATE_UNDF			(3)	/* Symbol undefined in pass1 */

#define REGISTER_P(ptr)		(ptr == 'r')

#define INCREMENT			01
#define DECREMENT			02

#define MAXREGISTER			07
#define MINREGISTER			00
#define NINE_BITS			511
#define TEN_BITS			1023

//struct mc9xgate_parsed_op{
//	unsigned short bin_operand_mask;
//	unsigned int  shorthand_format; /* prime values used as flags */
//	char	num_modes;
// address
//};
// bfd_reloc_code_real_type
/* This macro has no side-effects.  */
#define ENCODE_RELAX(what,length) (((what) << 2) + (length))
#define RELAX_STATE(s) ((s) >> 2)
#define RELAX_LENGTH(s) ((s) & 3)

#define IS_OPCODE(C1,C2)        (((C1) & 0x0FF) == ((C2) & 0x0FF))

struct mc9xgate_opcode_handle
{
  unsigned char number_of_modes;
  char *name;
  struct mc9xgate_opcode *opc0;
  struct mc9xgate_opcode *opc1;
  struct mc9xgate_opcode *opc2;
  struct mc9xgate_opcode *opc3;
  struct mc9xgate_opcode *opc4;
};

struct macroOperands
{
  char *oper1;
  char *oper2;
  char *oper3;
};

/*  LOCAL FUNCTIONS */
static void
s_mc9xgate_mode(int); /* Pseudo op to control the ELF flags.  */
static inline char *
skip_whitespace(char *);
static void
get_default_target(void); /* Pseudo op to indicate a relax group.  */
static void
s_mc9xgate_relax(int);
static void
s_mc9xgate_mode(int);
static void
s_mc9xgate_mark_symbol(int);/* Pseudo op to control the ELF flags.  */
static char*
extract_word(char *, char *, int);
static char *
mc9xgate_new_instruction(int size); /* number of bytes */
unsigned short
mc9xgate_apply_operand(unsigned short new_mask,
    unsigned short *oper_bits_availiable, unsigned short mask,
    unsigned char n_bits);
static unsigned int
mc9xgate_operands(struct mc9xgate_opcode *opcode, char **line);
static unsigned int
mc9xgate_operand(struct mc9xgate_opcode *opcode, int *bit_width, int where,
    char **op_con, char **line);
static struct mc9xgate_opcode*
mc9xgate_find_match(struct mc9xgate_opcode_handle *opcode_handle,
    unsigned char numberOfModes, unsigned int sh_format);

void
mc9xgate_get_macro_masks(unsigned int *operMask0, unsigned int *operMask1,
    char *string);

unsigned short
mc9xgate_get_operands_old(char *input, struct mc9xgate_opcode *opcode);
static unsigned int
mc9xgate_get_constant(char *input, int max);
void
append_str(char *in, char c);
static int
cmp_opcode(struct mc9xgate_opcode *, struct mc9xgate_opcode *);

unsigned int
mc9xgate_detect_format(char *line_in);

//char get_address_mode(char *);
/* Mark the symbols with STO_M68HC12_FAR to indicate the functions
 are using 'rtc' for returning.  It is necessary to use 'call'
 to invoke them.  This is also used by the debugger to correctly
 find the stack frame.  */
/* END LOCAL FUNCTIONS */

/* LOCAL DATA */
static struct hash_control *mc9xgate_hash;
static unsigned int prev = 0; /* Previous opcode.  */
static unsigned char fixup_required = 0;
//static char parse_error;
//static char error_message[ sizeof( char) ];
static unsigned char macroClipping = 0; /* used to enable clipping of 16 bit operands into 8 bit constraints */
static char oper_check;

static unsigned int oper1 = 0;
static unsigned int oper2 = 0;
static unsigned int oper3 = 0;
static int oper1_present = 0;
static int oper1_bit_length = 0;
static int oper2_present = 0;
static int oper2_bit_length = 0;
static int oper3_present = 0;
static int oper3_bit_length = 0;

//static struct mc9xgate_opcode *mc9xgate_op_table = 0;


typedef struct mc9xgate_operand
{
  expressionS exp0;
  expressionS exp1;
  unsigned char reg;
} operand;

/* This table describes how you change sizes for the various types of variable
 size expressions.  This version only supports two kinds.  */

/* The fields are:
 How far Forward this mode will reach.
 How far Backward this mode will reach.
 How many bytes this mode will add to the size of the frag.
 Which mode to go to if the offset won't fit in this one.  */

relax_typeS md_relax_table[] =
  {
    { 1, 1, 0, 0 }, /* First entries aren't used.  */
    { 1, 1, 0, 0 }, /* For no good reason except.  */
    { 1, 1, 0, 0 }, /* that the VAX doesn't either.  */
    { 1, 1, 0, 0 },

  /* Relax for bcc <L>.
   These insns are translated into b!cc +3 jmp L.  */
    { (127), (-128), 0, ENCODE_RELAX (STATE_CONDITIONAL_BRANCH, STATE_WORD) },
    { 0, 0, 3, 0 },
    { 1, 1, 0, 0 },
    { 1, 1, 0, 0 },

  /* Relax for bsr <L> and bra <L>.
   These insns are translated into jsr and jmp.  */
    { (127), (-128), 0, ENCODE_RELAX (STATE_PC_RELATIVE, STATE_WORD) },
    { 0, 0, 1, 0 },
    { 1, 1, 0, 0 },
    { 1, 1, 0, 0 },

  /* Relax for indexed offset: 5-bits, 9-bits, 16-bits.  */
    { (15), (-16), 0, ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS9) },
    { (255), (-256), 1, ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS16) },
    { 0, 0, 2, 0 },
    { 1, 1, 0, 0 },

  /* Relax for PC relative offset: 5-bits, 9-bits, 16-bits.
   For the 9-bit case, there will be a -1 correction to take into
   account the new byte that's why the range is -255..256.  */
    { (15), (-16), 0, ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS9) },
    { (256), (-255), 1, ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS16) },
    { 0, 0, 2, 0 },
    { 1, 1, 0, 0 },

  /* Relax for dbeq/ibeq/tbeq r,<L>:
   These insns are translated into db!cc +3 jmp L.  */
    { (255), (-256), 0, ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_WORD) },
    { 0, 0, 3, 0 },
    { 1, 1, 0, 0 },
    { 1, 1, 0, 0 },

  /* Relax for bcc <L> on 68HC12.
   These insns are translated into lbcc <L>.  */
    { (127), (-128), 0,
        ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812, STATE_WORD) },
    { 0, 0, 2, 0 },
    { 1, 1, 0, 0 },
    { 1, 1, 0, 0 },

  };

/* mc9Xgate registers all are 16-bit.  They are numbered according to the mc9xgate.  */
typedef enum register_id
{
  REG_NONE = -1,
  REG_A = 0, /* is tied to the value “$0000”
   Selecting R0 as destination register will discard the result of the instruction. Only
   the Condition Code Register will be updated  */
  REG_B = 1, /* variable pointer, It is preloaded with the initial variable pointer
   of the channel’s service request vector  */
  REG_C = 2,
  REG_D = 3,
  REG_E = 4,
  REG_F = 5,
  REG_G = 6,
  REG_H = 7,
  REG_CCR = 8, //was 8 set to 55 for testing
  REG_PC = 9
} register_id;

/* ELF flags to set in the output file header.  */
//static int elf_flags = E_MC9XGATE_F64;

/* These are the machine dependent pseudo-ops.  These are included so
 the assembler can work on the output from the SUN C compiler, which
 generates these.  */

/* This table describes all the machine specific pseudo-ops the assembler
 has to support.  The fields are:
 pseudo-op name without dot
 function to call to execute this pseudo-op
 Integer arg to pass to the function.  */
const pseudo_typeS md_pseudo_table[] =
  {
  /* The following pseudo-ops are supported for MRI compatibility.  */
    { "fcb", cons, 1 },
    { "fdb", cons, 2 },
    { "fcc", stringer, 8 + 1 },
    { "rmb", s_space, 0 },

  /* Motorola ALIS.  */
    { "xrefb", s_ignore, 0 }, /* Same as xref  */

  /* Gcc driven relaxation.  */
    { "relax", s_mc9xgate_relax, 0 },

  /* .mode instruction (ala SH).  */
    { "mode", s_mc9xgate_mode, 0 },

  /* .far instruction.  */
    { "far", s_mc9xgate_mark_symbol, STO_MC9XGATE_FAR },

  /* .interrupt instruction.  */
    { "interrupt", s_mc9xgate_mark_symbol, STO_MC9XGATE_INTERRUPT },

    { 0, 0, 0 } };

/* statics */
static int current_architecture;
static int mc9xgate_nb_opcode_defs = 0;
static const char *default_cpu;
//static struct hash_control *mc9xgate_hash;
//static struct mc9xgate_opcode_def *mc9xgate_opcode_defs = 0;
static unsigned int numberOfCalls = 0;
/* ELF flags to set in the output file header.  */
static int elf_flags = E_MC9XGATE_F64;

/* Declaired to keep the current make happy with alot of baggage from the old port  TODO clean up*/
const char *md_shortopts = "Sm:";
struct option md_longopts[] =
  {

#define OPTION_FORCE_LONG_BRANCH (OPTION_MD_BASE)
        { "force-long-branches", no_argument, NULL, OPTION_FORCE_LONG_BRANCH },
        { "force-long-branchs", no_argument, NULL, OPTION_FORCE_LONG_BRANCH }, /* Misspelt version kept for backwards compatibility.  */

#define OPTION_SHORT_BRANCHES     (OPTION_MD_BASE + 1)
        { "short-branches", no_argument, NULL, OPTION_SHORT_BRANCHES },
        { "short-branchs", no_argument, NULL, OPTION_SHORT_BRANCHES }, /* Misspelt version kept for backwards compatibility.  */

#define OPTION_STRICT_DIRECT_MODE  (OPTION_MD_BASE + 2)
        { "strict-direct-mode", no_argument, NULL, OPTION_STRICT_DIRECT_MODE },

#define OPTION_PRINT_INSN_SYNTAX  (OPTION_MD_BASE + 3)
        { "print-insn-syntax", no_argument, NULL, OPTION_PRINT_INSN_SYNTAX },

#define OPTION_PRINT_OPCODES  (OPTION_MD_BASE + 4)
        { "print-opcodes", no_argument, NULL, OPTION_PRINT_OPCODES },

#define OPTION_GENERATE_EXAMPLE  (OPTION_MD_BASE + 5)
        { "generate-example", no_argument, NULL, OPTION_GENERATE_EXAMPLE },

#define OPTION_MSHORT  (OPTION_MD_BASE + 6)
        { "mshort", no_argument, NULL, OPTION_MSHORT },

#define OPTION_MLONG  (OPTION_MD_BASE + 7)
        { "mlong", no_argument, NULL, OPTION_MLONG },

#define OPTION_MSHORT_DOUBLE  (OPTION_MD_BASE + 8)
        { "mshort-double", no_argument, NULL, OPTION_MSHORT_DOUBLE },

#define OPTION_MLONG_DOUBLE  (OPTION_MD_BASE + 9)
        { "mlong-double", no_argument, NULL, OPTION_MLONG_DOUBLE },

        { NULL, no_argument, NULL, 0 } };

size_t md_longopts_size = sizeof(md_longopts);

char *
md_atof(int type, char *litP, int *sizeP)
{
  return ieee_md_atof(type, litP, sizeP, TRUE);
}

int
md_parse_option(int c, char *arg)
{
  c = 0; //for testing
  *arg = 't'; //for testing
  return 1;
}

const char *
mc9xgate_arch_format(void)
{
  get_default_target();
  if (current_architecture & cpumc9xgate)
    {
      return "elf32-mc9xgate";
    }
  else
    {
      //printf("\n ERROR in mc9xgate_arch_format ---unknown----\n");  //for debugging
      return "error";
    }
}

static void
get_default_target(void)
{
  const bfd_target *target;
  bfd abfd;

  if (current_architecture != 0)
    return;

  default_cpu = "unknown";
  target = bfd_find_target(0, &abfd);
  if (target && target->name)
    {
      if (strcmp(target->name, "elf32-mc9xgate") == 0)
        {
          current_architecture = cpumc9xgate;
          default_cpu = "mc9xgate";
          //printf("\n found default cpu->%s\n",target->name);
          return;
        }
      else
        {
          as_bad(_("Default target `%s' is not supported."), target->name);
        }
    }
}

void
md_begin(void)
{
  struct mc9xgate_opcode *mc9xgate_opcode_ptr;
  struct mc9xgate_opcode *mc9xgate_op_table;
  struct mc9xgate_opcode_handle *op_handles = 0;
  //struct mc9xgate_opcode *mc9xgate_op_handle_ptr;
  //struct mc9xgate_opcode *op_alias;
  char prev_op_name[9];
  char handle_enum = 0;
  unsigned int number_of_handle_rows = 0;
  int i, j = 0;
  //int opcode_offset = 0;

  /* create a local copy of our opcode table including and an extra line for NULL termination*/
  mc9xgate_op_table = (struct mc9xgate_opcode *) xmalloc((mc9xgate_num_opcodes
      + 1) * sizeof(struct mc9xgate_opcode));
  memset(mc9xgate_op_table, 0, sizeof(struct mc9xgate_opcode)
      * (mc9xgate_num_opcodes + 1));

  for (i = (mc9xgate_num_opcodes + 1); i; i--)
    {

      mc9xgate_opcode_ptr = NULL;
    }

  for (mc9xgate_opcode_ptr = mc9xgate_opcodes, i = 0; i < mc9xgate_num_opcodes; i++)
    {
      mc9xgate_op_table[i] = mc9xgate_opcode_ptr[i];
    }

  qsort(mc9xgate_opcode_ptr, mc9xgate_num_opcodes,
      sizeof(struct mc9xgate_opcode), (int
      (*)(const void*, const void*)) cmp_opcode);

  /* Calculate number of handles since this will be smaller than the raw number of opcodes in the table */
  for (mc9xgate_opcode_ptr = mc9xgate_op_table; mc9xgate_opcode_ptr->name; mc9xgate_opcode_ptr++)
    {
      if (strcmp(prev_op_name, mc9xgate_opcode_ptr->name))
        {
          number_of_handle_rows++;
        }
      strcpy(prev_op_name, mc9xgate_opcode_ptr->name);
    }

  op_handles = (struct mc9xgate_opcode_handle *) xmalloc(
      sizeof(struct mc9xgate_opcode_handle) * (number_of_handle_rows + 1));

  /* insert opcode names into hash table, aliasing duplicates */
  /* TODO do this dynamically with xmalloc and a loop */
  mc9xgate_hash = hash_new(); /* create a new has control table */
  for (mc9xgate_opcode_ptr = mc9xgate_op_table, i = 0, j = 0; i
      < mc9xgate_num_opcodes; i++, mc9xgate_opcode_ptr++)
    {
      if (strcmp(prev_op_name, mc9xgate_opcode_ptr->name))
        {
          handle_enum = 1;
          if (i)
            {
              j++;
            }
          op_handles[j].name = mc9xgate_opcode_ptr->name;
          op_handles[j].opc0 = mc9xgate_opcode_ptr;
        }
      else
        {
          handle_enum++;
          switch (handle_enum)
            {
          case 2:
            op_handles[j].opc1 = mc9xgate_opcode_ptr;
            break;
          case 3:
            op_handles[j].opc2 = mc9xgate_opcode_ptr;
            break;
          case 4:
            op_handles[j].opc3 = mc9xgate_opcode_ptr;
          case 5:
            op_handles[j].opc4 = mc9xgate_opcode_ptr;
          default:
            as_bad(_(":error adding operand handle"));
            break;
            }
        }
      op_handles[j].number_of_modes = handle_enum;
      strcpy(prev_op_name, op_handles[j].name);
    }
  while (op_handles->name)
    {
      //printf("\n hash insert message %s",(hash_insert(mc9xgate_hash, op_handles->name, (char *) op_handles)) );
      hash_insert(mc9xgate_hash, op_handles->name, (char *) op_handles);
      op_handles++;
    }
}

void
mc9xgate_init_after_args(void)
{
  //return; // no return needed?????
}

void
md_show_usage(FILE *stream)
{
  get_default_target();
  fprintf(
      stream,
      _("\
      Motorola MC9XGATE co-processor options:\n\
      -mc9xgate               specify the processor [default %s]\n\
      -mshort                 use 16-bit int ABI (default)\n\
      -mlong                  use 32-bit int ABI\n\
      -mshort-double          use 32-bit double ABI\n\
      -mlong-double           use 64-bit double ABI (default)\n\
      --force-long-branches   always turn relative branches into absolute ones\n\
      -S,--short-branches     do not turn relative branches into absolute ones\n\
      when the offset is out of range\n\
      --strict-direct-mode    do not turn the direct mode into extended mode\n\
      when the instruction does not support direct mode\n\
      --print-insn-syntax     print the syntax of instruction in case of error\n\
      --print-opcodes         print the list of instructions with syntax\n\
      --generate-example      generate an example of each instruction\n\
  (used for testing)\n"),
      default_cpu);
}

enum bfd_architecture
mc9xgate_arch(void)
{
  get_default_target();
  if (current_architecture & cpumc9xgate) /* TODO same as above todo */
    // return bfd_arch_mc9xgate;
    return bfd_arch_mc9xgate;
  else
    //printf("\n error in tc-mc9s12x mc9xgate_arch \n");
    //return bfd_arch_mc9xgate;
    return bfd_arch_mc9xgate;
}

int
mc9xgate_mach(void)
{
  return 0;
}

void
mc9xgate_print_statistics(FILE *file)
{
  //  int i;
  struct mc9xgate_opcode *opc;

  hash_print_statistics(file, "opcode table", mc9xgate_hash);

  opc = mc9xgate_opcodes;
  if (opc == 0 || mc9xgate_nb_opcode_defs == 0)
    return;

  /* Dump the opcode statistics table.  */
  //  fprintf (file, _("Name   # Modes  Min ops  Max ops  Modes mask  # Used\n"));
  //  for (i = 0; i < mc9xgate_nb_opcode_defs; i++, opc++)
  //   {
  //     fprintf (file, "%-7.7s  %5d  %7d  %7d  0x%08lx  %7d\n",
  //	       opc->opcode->name,
  //	       opc->nb_modes,
  //	       opc->min_operands, opc->max_operands, opc->format, opc->used);
  //    }
}

const char *
mc9xgate_listing_header(void)
{
  if (current_architecture & cpumc9xgate)
    return "MC9S12X GAS "; /* TODO same as above */
  else
    return "ERROR MC9S12X GAS ";
}
symbolS *
md_undefined_symbol(char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align(asection *seg, valueT addr)
{
  int align = bfd_get_section_alignment (stdoutput, seg);
  return ((addr + (1 << align) - 1) & (-1 << align));
}

void md_assemble(char *input_line)
{
  struct mc9xgate_opcode *opcode = 0;
  struct mc9xgate_opcode *macro_opcode = 0;
  struct mc9xgate_opcode_handle *opcode_handle = 0;
  char *saved_input_line = input_line; /* caller expects it to be returned as it was passed */
  unsigned short opcode_bin = 0;
  char op_name[9] = { 0 };
  char handle_enum_alias = 0;
  unsigned int sh_format = 0;
  char *p = 0;
  unsigned int imm16_value0 = 0;
  unsigned int imm16_value1 = 0;
  char imm16_string[25] = { 0 };
  unsigned int imm16_string_index = 0;
  char *imm16_pointer = 0;
  //unsigned char format_found = 0;

  fixup_required = 0;
  numberOfCalls++; // for testing
  oper_check = 0; /* set error flags */

  input_line = extract_word(input_line, op_name, sizeof(op_name));
  /* check to make sure we are not reading a bugus line */
  if (!op_name[0])
    {
      as_bad(_("opcode missing or not found on input line"));
      //	 return;
    }
 //printf("\n read code %s\n", op_name);
  if (!(opcode_handle = (struct mc9xgate_opcode_handle *) hash_find(
      mc9xgate_hash, op_name)))
    {
      //	 //printf("\n %s\n",op_name);
      as_bad(_("opcode not found in hash"));
    }
  else
    {
      /* detect operand format so we can pull the proper opcode bin */
      handle_enum_alias = opcode_handle->number_of_modes;
      //printf("\n about to detect operands, %d combinations found", handle_enum_alias);
      //printf("\n inline reads %s", input_line);
      sh_format = mc9xgate_detect_format(input_line);
      //printf("\n detected shorthand %d of %d alias",sh_format, handle_enum_alias);

      if (handle_enum_alias > 1)
        {
          opcode = mc9xgate_find_match(opcode_handle, opcode_handle->number_of_modes, sh_format);
          printf("\n opcode match is %hx", opcode->bin_opcode);
        }
      else
        {
          opcode = opcode_handle->opc0;
          printf("\n opcode match is %hx", opcode->bin_opcode);
        }

      if (!opcode)
        {
          as_bad(_(":error matching operand format"));
          //TODO print list of possibilities
        }
      else if (opcode->size == 2)
        { /* if size is one word assemble that native insn */
          //printf("\n matched code %s and format %s",opcode->name, opcode->format);
          opcode_bin = mc9xgate_operands(opcode, &input_line);
          //printf("\n parsed bin_opcode is  %x", opcode_bin);
        }
      else
        { /* if insns is a simplified instruction expand it out */
    	  macroClipping = 1;
    	  char constraintString[50];
          unsigned int i;

          /* extract formal constraint string */
          for (i = 0, p = opcode->constraints; *p != ';'; i++, p++)
            {
              constraintString[i] = *p;
            }
          p++;
          constraintString[i] = 0;

//          sh_format = mc9xgate_detect_format(constraintString);

          input_line = skip_whitespace(input_line);

          //printf("\n inline reads %s", input_line);

          char *macro_inline = input_line;

          for (i = 0; *p && i < (opcode->size / 2); i++)
            { /* loop though macro operand list */
              input_line = macro_inline; /* rewind */
              p = extract_word(p, op_name, 10);
              //printf("\n read real opcode %s", op_name);
              if (!(opcode_handle
                  = (struct mc9xgate_opcode_handle *) hash_find(mc9xgate_hash,
                      op_name)))
                {
                  as_bad(_("opcode handle not found in hash"));
                  break;
                }
              else
                {
                  sh_format = mc9xgate_detect_format(input_line);
                  //printf("\n about to call find match then operands %s %s , number of modes %d with sh-format %d", op_name, input_line, opcode_handle->number_of_modes, sh_format);
                  macro_opcode = mc9xgate_find_match(opcode_handle, opcode_handle->number_of_modes, sh_format);
                  //printf("\n opcode is %s", opcode->name);
                  opcode_bin = mc9xgate_operands(macro_opcode, &input_line);
                }
            }

          if (0) //Disable block
            {
              if (*input_line == 'r' || *input_line == 'R')
                {
                  //	*bit_width = 3;
                  //	input_line = extract_word (input_line, imm16_string, sizeof (imm16_string));

                  for (imm16_string_index = 0; is_part_of_name(*input_line); imm16_string_index++, input_line++)
                    {
                      imm16_string[imm16_string_index] = *input_line;
                    }

                  if (*input_line != ',')
                    {
                      as_bad(_(":expected , operand seperator"));
                    }
                  else
                    {
                      imm16_string[imm16_string_index++] = *input_line++;
                    }

                  if (*input_line == '#')
                    { /* go past # character */
                      input_line++;
                      //printf("\n about to call get constant %s", input_line);
                      imm16_value0 = mc9xgate_get_constant(input_line, 0xFFFF);
                      imm16_value1 = (imm16_value0 & 0xFF00) >> 8;
                      imm16_value0 &= 0x00FF;
                      //printf("\n in case r extracted %s and read valueL %x valueH %x", imm16_string, imm16_value0, imm16_value1);
                    }
                  else
                    {
                      //printf("\n need to parse expression for macro");
                    }

                }
              else
                {
                  as_bad(_(": expected register name r0-r7 read %s "),
                      input_line);

                }

              for (i = 0; *p && i < 2; i++)
                { /* loop though macro operand list */
                  p = extract_word(p, op_name, 10);
                  if (!(opcode_handle
                      = (struct mc9xgate_opcode_handle *) hash_find(
                          mc9xgate_hash, op_name)))
                    {
                      as_bad(_("opcode not found in hash"));
                      break;
                    }
                  else
                    {
                      //printf("\n found %s", op_name);
                      opcode = mc9xgate_find_match(opcode_handle,
                          opcode_handle->number_of_modes, sh_format);
                    }

                  if (!i)
                    { /* write the low byte first since it will clear the high byte in the process */
                      sprintf(&imm16_string[imm16_string_index], "#%d",
                          imm16_value0);
                      //printf("\n paresd %s", imm16_string);
                      //imm16_string[imm16_string_index] = inttos();
                    }
                  else
                    {
                      sprintf(&imm16_string[imm16_string_index], "#%d",
                          imm16_value1);
                      //printf("\n parsed %s", imm16_string);
                      // opcode_bin = mc9xgate_operands(opcode, &input_line);
                    }
                  imm16_pointer = imm16_string;
                  //printf("\n parsed %s about to call operands", imm16_pointer);
                  opcode_bin = mc9xgate_operands(opcode, &imm16_pointer);
                }
              //printf("\n done processing macro");
            }
        }

    }
  macroClipping = 0;
  input_line = saved_input_line;
}

static void
s_mc9xgate_relax(int ignore ATTRIBUTE_UNUSED)
{
  //printf("\n in s_mc9xgate_relax");
  expressionS ex;

  expression (&ex);

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad(_("bad .relax format"));
      ignore_rest_of_line();
      return;
    }

  fix_new_exp(frag_now, frag_now_fix(), 0, &ex, 1, BFD_RELOC_MC9XGATE_RL_GROUP);

  demand_empty_rest_of_line();
}

/* Pseudo op to control the ELF flags.  */
static void
s_mc9xgate_mode(int x ATTRIBUTE_UNUSED)
{

}

static void
s_mc9xgate_mark_symbol(int mark)
{
  mark = 0; //for testing
}

/* Force truly undefined symbols to their maximum size, and generally set up
 the frag list to be relaxed.  */
int
md_estimate_size_before_relax(fragS *fragP, asection *segment)
{
  int temp = fragP->fr_fix;
  int temp2 = segment->entsize;
  temp2 = temp;
  return 0;
}

long
mc9xgate_relax_frag(segT seg ATTRIBUTE_UNUSED, fragS *fragP, long stretch ATTRIBUTE_UNUSED)
{
  //printf("\n in relax_frag");
  int temp = fragP->fr_var;
  int temp2;
  temp2 = temp;

  return 1;
}

/* Relocation, relaxation and frag conversions.  */

/* PC-relative offsets are relative to the start of the
 next instruction.  That is, the address of the offset, plus its
 size, since the offset is always the last part of the insn.  */
long
md_pcrel_from(fixS *fixP)
{
  //printf("\n in perel_from");
  if (fixP->fx_r_type == BFD_RELOC_MC9XGATE_RL_JUMP)
    return 0;

  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

/* If while processing a fixup, a reloc really needs to be created
 then it is done here.  */
arelent *
tc_gen_reloc(asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  //printf("\n in tc_gen_reloc fx_r_type is %d ", fixp->fx_r_type);

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
      //printf("\nlooking up howto based on fx_r_type");
    }

  if (fixp->fx_r_type == 14)
    {
      reloc->howto = bfd_reloc_name_lookup(stdoutput, "R_MC9XGATE_IMM8");
    }

  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where(fixp->fx_file, fixp->fx_line,
          _("Relocation %d is not supported by object file format."),
          (int) fixp->fx_r_type);
      return NULL;
    }

  /* Since we use Rel instead of Rela, encode the vtable entry to be
   used in the relocation's section offset.  */
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    reloc->address = fixp->fx_offset;

  reloc->addend = 0;
  //printf("\n about to return relocation name-%s size-%d type->%u", reloc->howto->name, reloc->howto->size, reloc->howto->type);
  return reloc;

  //	arelent *reloc;

  //	reloc = (arelent *) xmalloc (sizeof (arelent));
  //	reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  //	*reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  //	reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  //	if (fixp->fx_r_type == 0)
  //		reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_16);
  //	else /* TODO fix so it finds by type */
  //		reloc->howto = bfd_reloc_name_lookup(stdoutput, "R_MC9XGATE_IMM8");
  // reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  //	if (reloc->howto == (reloc_howto_type *) NULL)
  //	{
  //		as_bad_where (fixp->fx_file, fixp->fx_line,
  //				_("Relocation %d is not supported by object file format."),
  //				(int) fixp->fx_r_type);
  //		return NULL;
  //	}

  /* Since we use Rel instead of Rela, encode the vtable entry to be
   used in the relocation's section offset.  */
  //	if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
  //		reloc->address = fixp->fx_offset;

  //	//printf("\name of reloc is %s", reloc->howto->name);

  //	reloc->addend = 0;
  //	return reloc;
}

/* Patch the instruction with the resolved operand.  Elf relocation
 info will also be generated to take care of linker/loader fixups.
 The 68HC11 addresses only 64Kb, we are only concerned by 8 and 16-bit
 relocs.  BFD_RELOC_8 is basically used for .page0 access (the linker
 will warn for overflows).  BFD_RELOC_8_PCREL should not be generated
 because it's either resolved or turned out into non-relative insns (see
 relax table, bcc, bra, bsr transformations)
 The BFD_RELOC_32 is necessary for the support of --gstabs.

 */
void
md_apply_fix(fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  //printf("\n in md_apply_fix");

  char *where;
  long value = *valP;
  int op_type;
  int opcode = 0;
  ldiv_t result;

  /* if the fixup is done mark it done so no further symbol resoloution will take place */
  if (fixP->fx_addsy == (symbolS *) NULL)
    {
      fixP->fx_done = 1;
      //printf("\n fixup done");
    }

  /* We don't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != (symbolS *) NULL)
    as_bad_where(fixP->fx_file, fixP->fx_line, _("Expression too complex."));

  op_type = fixP->fx_r_type;

  where = fixP->fx_frag->fr_literal + fixP->fx_where;

  opcode = bfd_getl16(where);

  int mask = 0;

  //printf("\n read opcode %d from frag", opcode);

  switch (fixP->fx_r_type)
    {
  //   case BFD_RELOC_MC9XGATE_PCREL_9:
  case R_MC9XGATE_PCREL_9:
    if (value < -255 || value > 254)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value %ld too large for 9-bit PC-relative branch."), value);
    result = ldiv(value, 2); /* from bytes to words */
    value = result.quot;
    mask = 0x1FF; /* Clip into 8-bit field FIXME I'm sure there is a more proper place for this */
    value &= mask;
    number_to_chars_bigendian(where, (opcode | value), 2);
    //printf("\n fixup with operand is %ld",(opcode | value));
    break;
  case R_MC9XGATE_PCREL_10:
    if (value < -512 || value > 511)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value %ld too large for 10-bit PC-relative branch."), value);
    result = ldiv(value, 2); /* from bytes to words */
    value = result.quot;
    mask = 0x3FF; /* Clip into 9-bit field FIXME I'm sure there is a more proper place for this */
    value &= mask;
    number_to_chars_bigendian(where, (opcode | value), 2);
    //printf("\n fixup with operand is %ld",(opcode | value));
    break;
    break;
    //case R_MC9XGATE_32: // for testing
  case BFD_RELOC_MC9XGATE_24:
    //case R_MC9XGATE_IMM8_LO:
  case BFD_RELOC_MC9XGATE_IMM8_HI:
    if (value < -65537 || value > 65535)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 16-bit range."));
    value >>= 8;
    bfd_putb16((bfd_vma) value | opcode, (unsigned char *) where);
    //printf("\n fixup 8 with operand is %ld",(long int) opcode);
    break;

  case BFD_RELOC_MC9XGATE_IMM8_LO:
    if (value < -65537 || value > 65535)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 16-bit range."));
    value &= 0x00ff;
    bfd_putb16((bfd_vma) value | opcode, (unsigned char *) where);
    if (value < -65537 || value > 65535)
      as_bad_where(fixP->fx_file, fixP->fx_line,
          _("Value out of 16-bit range."));

    //number_to_chars_bigendian(where, opcode , 2);

    //printf("\n fixup 8LO with operand is %ld",(long int) opcode);

    // case R_MC9XGATE_16:
    //	   //printf("\n need to fixup 16");
    break;
  case BFD_RELOC_MC9XGATE_IMM3:
    if (value < 0 || value > 7)
      as_bad_where(fixP->fx_file, fixP->fx_line, _("Value out of 3-bit range."));
    //printf("\n need to process imm3 relocatoin");
    value <<= 8; /* alight the operand bits */

    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_MC9XGATE_IMM4:
    if (value < 0 || value > 15)
      as_bad_where(fixP->fx_file, fixP->fx_line, _("Value out of 4-bit range."));
    value <<= 4; /* alight the operand bits */
    //printf("\n need to process imm4 relocatoin");
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case BFD_RELOC_MC9XGATE_IMM5:
    if (value < 0 || value > 31)
      as_bad_where(fixP->fx_file, fixP->fx_line, _("Value out of 5-bit range."));
    value <<= 5; /* alight the operand bits */
    //printf("\n need to process imm4 relocatoin");
    number_to_chars_bigendian(where, (opcode | value), 2);
    break;
  case R_MC9XGATE_16:
  case 0x2: /* seems to be the default value for no fixup TODO figure out how to remove*/
    break;
  default:
    as_fatal(_("Line %d: unknown relocation type: 0x%x."), fixP->fx_line,
        fixP->fx_r_type);
    }
}

/* See whether we need to force a relocation into the output file.  */
int
tc_mc9xgate_force_relocation(fixS *fixP)
{
  //printf("\n in force_relocation");
  if (fixP->fx_r_type == BFD_RELOC_MC9XGATE_RL_GROUP)
    return 1;

  return generic_force_reloc(fixP);
}

/* Here we decide which fixups can be adjusted to make them relative
 to the beginning of the section instead of the symbol.  Basically
 we need to make sure that the linker relaxation is done
 correctly, so in some cases we force the original symbol to be
 used.  */
int
tc_mc9xgate_fix_adjustable(fixS *fixP)
{
  //printf("\n in fix_adjustable");

  switch (fixP->fx_r_type)
    {
  /* For the linker relaxation to work correctly, these relocs
   need to be on the symbol itself.  */
  case BFD_RELOC_MC9XGATE_IMM8_LO:
  case BFD_RELOC_MC9XGATE_IMM8_HI:
  case BFD_RELOC_16:
  case BFD_RELOC_MC9XGATE_RL_JUMP:
  case BFD_RELOC_MC9XGATE_RL_GROUP:
  case BFD_RELOC_VTABLE_INHERIT:
  case BFD_RELOC_VTABLE_ENTRY:
  case BFD_RELOC_32:

    /* The memory bank addressing translation also needs the original
     symbol.  */
  case BFD_RELOC_MC9XGATE_LO16:
  case BFD_RELOC_MC9XGATE_PAGE:
  case BFD_RELOC_MC9XGATE_24:
    return 0;

  default:
    return 1;
    }
}

void
md_convert_frag(bfd *abfd ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED, fragS *fragP)
{
  //printf("\n in convert_frag");

  int temp = fragP->fr_address;
  temp++;
  return;
}

/* Set the ELF specific flags.  */
void
mc9xgate_elf_final_processing(void)
{
  //printf("\n in elf_final processing");
  /* TODO make this always true */
  if (current_architecture & cpumc9xgate)
    elf_flags |= EF_MC9XGATE_MACH;
  elf_elfheader (stdoutput)->e_flags &= ~EF_MC9XGATE_ABI;
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}

static inline char *
skip_whitespace(char *s)
{
  while (*s == ' ' || *s == '\t' || *s == '(' || *s == ')')
    {
      s++;
    }
  ////printf("\n debug white space %c", *s);
  return s;
}

/* extract a word (continuous alpha-numeric chars) from the input line */
static char *
extract_word(char *from, char *to, int limit)
{
  char *op_start;
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_whitespace(from);
  *to = 0;

  /* Find the op code end.  */
  for (op_start = op_end = from; *op_end != 0 && is_part_of_name (*op_end);)
    {
      to[size++] = *op_end++;
      if (size + 1 >= limit)
        break;
    }

  to[size] = 0;
  return op_end;
}

/* Create a new fragment for our instruction. Record the
 line information of that insn in the dwarf2 debug sections. */
static char *
mc9xgate_parse_exp(char *s, expressionS *op);

static char *
mc9xgate_new_instruction(int size)
{

  char *f = frag_more(size);

  dwarf2_emit_insn(size); /* avr code passes 0 for size */

  return f;
}

unsigned short
mc9xgate_apply_operand(unsigned short new_mask,
    unsigned short *availiable_mask_bits, unsigned short mask,
    unsigned char n_bits)
{
  printf("\n need to apply %ux to %ux with %ux left", new_mask, mask, *availiable_mask_bits);
  unsigned short n_shifts;
  unsigned int n_drop_bits;

  /* shift until you find an availiable operand bit "1" and record the number of shifts */
  for (n_shifts = 0; !(*availiable_mask_bits & SIXTEENTH_BIT) && n_shifts < 16; n_shifts++)
    {
      *availiable_mask_bits <<= 1;
    }
  /* shift for the number of bits your operand requires while bits are avaibiable */
  for (n_drop_bits = n_bits; n_drop_bits && (*availiable_mask_bits
      & SIXTEENTH_BIT); --n_drop_bits)
    {
      *availiable_mask_bits <<= 1;

    }
  if (n_drop_bits)
    as_bad(_(":operand has too many bits"));

  *availiable_mask_bits >>= n_shifts + n_bits;

  if ((n_drop_bits == 0) && (*availiable_mask_bits == 0))
    {
      oper_check = 1; /* flag operand check as good */
      //printf("\n good to go\n");
    }

  new_mask <<= N_BITS_IN_WORD - (n_shifts + n_bits);
  mask |= new_mask;
  return mask;
}

/* Parse ordinary expression.  */
static char *
mc9xgate_parse_exp(char *s, expressionS *op)
{
  input_line_pointer = s;
  expression (op);
  if (op->X_op == O_absent)
    as_bad(_("missing operand"));
  return input_line_pointer;
}

static unsigned int
mc9xgate_get_constant(char *str, int max)
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

void
append_str(char *in, char c)
{
  unsigned char length = (unsigned char) strlen(in);
  in[length] = c;
  in[++length] = 0; /* terminate */
  return;
}

static int
cmp_opcode(struct mc9xgate_opcode *op1, struct mc9xgate_opcode *op2)
{
  return strcmp(op1->name, op2->name);
}

/* Parse instruction operands.
 Return binary opcode.  */
static unsigned int
mc9xgate_operands(struct mc9xgate_opcode *opcode, char **line)
{
  char *frag = mc9xgate_new_instruction(opcode->size);
  int where = frag - frag_now->fr_literal;
  char *op = opcode->constraints;
  unsigned int bin = (int) opcode->bin_opcode;
  char *str = *line;

  /* TODO Structure this */
  unsigned short oper_mask = 0;

  oper1 = 0;
  oper2 = 0;
  oper3 = 0;

  oper1_present = 0;
  oper1_bit_length = 0;
  oper2_present = 0;

  oper2_bit_length = 0;
  oper3_present = 0;
  oper3_bit_length = 0;

  char n_operand_bits = 0;
  char first_can_equal_second = 0;
  int i = 0;
  char c = 0;

  /* generate availible operand bits mask */
  for (i = 0; (c = opcode->format[i]); i++)
    {
      if (ISDIGIT(c) || (c == 's'))
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
  //printf("\n available operbit mask is %d with %d bits", oper_mask, n_operand_bits);
  /* Opcode have operands.  */
  /* Parse first operand.  */
  if (*op)
    {
      //printf("\n first char of op is %c",*op);
      if (*op == '=')
        {
          first_can_equal_second = 1;
          ++op;
        }
      oper1_present = 1;
      //printf("\n getting operand 1 , %c", *op);
      oper1 = mc9xgate_operand(opcode, &oper1_bit_length, where, &op, &str);
      ++op;

      /* Parse second operand.  */
      if (*op)
        {
          if (*op == ',')
            ++op;

          oper2_present = 1;
          str = skip_whitespace(str);
          //printf("\n string is now %c first_can_second is %c", *str, first_can_equal_second);
          if (*str++ != ',')
            {
              if (first_can_equal_second)
                {
                  oper2 = oper1;
                  ++op;
                  //op++;
                  //op++;
                  //printf("\n op string is now %c",*op);
                }
              else
                {
                  as_bad(_("`,' required before second operand"));
                }
            }
          else
            {
              str = skip_whitespace(str);
              //printf("\n operand before call to get_operand %c", *op);
              oper2 = mc9xgate_operand(opcode, &oper2_bit_length, where, &op,
                  &str);
              //printf("\n operand after call to get_operand %c", *op);
              ++op;
            }

        }
      /* parse the third register */
      if (*op)
        {
          if (*op == ',')
            ++op;

          oper3_present = 1;

          str = skip_whitespace(str);
          if (*str++ != ',')
            as_bad(_("`,' required before third operand"));
          str = skip_whitespace(str);
          //printf("\n getting operand 3 typs is, %c input string is %s", *op, str);
          oper3 = mc9xgate_operand(opcode, &oper3_bit_length, where, &op, &str);

        }

    }

  if (oper1_present)
    bin = mc9xgate_apply_operand(oper1, &oper_mask, bin, oper1_bit_length);
  if (oper2_present)
    bin = mc9xgate_apply_operand(oper2, &oper_mask, bin, oper2_bit_length);
  if (oper3_present)
    bin = mc9xgate_apply_operand(oper3, &oper_mask, bin, oper3_bit_length);

  if (opcode->size == 4 && fixup_required)
    {
      /* not used */
      //bfd_putl32(opcode->bin_opcode, frag);
    }
  else if (opcode->size == 2 && fixup_required)
    {

      bfd_putl16(bin, frag);

    }
  else if ((opcode->sh_format & MC9XGATE_PCREL))
    {
      //printf("\n PCREL instruction");
      bfd_putl16(opcode->bin_opcode, frag); /* write our data to a frag for further processing */
    }
  else
    { /* apply operand mask(s)to bin opcode and write the output */

      //printf("\n no fixup required writing final bin to object %d",bin);
      number_to_chars_bigendian(frag, bin, opcode->size); /* since we are done write this frag in xgate BE format */
    }
  //  printf("\n firt length is %d with operand %d second length is %d with operand %d third length is %d with operand %d",
  //		  oper1_bit_length, oper1, oper2_bit_length, oper2, oper3_bit_length, oper3);

  prev = bin;
  *line = str;
  return bin;
}

static unsigned int
mc9xgate_operand(struct mc9xgate_opcode *opcode, int *bit_width, int where,
    char **op_con, char **line)
{
  expressionS op_expr;
  opcode = opcode;
  char *op_constraint = *op_con;
  unsigned int op_mask = 0;
  char *str = skip_whitespace(*line);
  char r_name[20] = { 0 };
  unsigned int pp_fix = 0;
  unsigned short max_size = 0;
  char reg_expected = 0;
  int i;
  *bit_width = 0;

  /* reset */
  switch (*op_constraint)
    {
  /* TODO should be able to combine with with case R */
  case '+': /* indexed register operand +/- or plain r  */
    //pp_fix = 0x0b00; /* default to neither inc or dec */
    pp_fix = 0;
	  //TODO maybe use a loop so the order is not important
    *bit_width = 5;
    //printf("\n found +/- line reads %s", str);
    //str = extract_word (str, r_name, sizeof (r_name));
    str = skip_whitespace(str);
    //printf("\n found +/- extracted %s", r_name);

    while (*str != ' ' && *str != '\t')
      {
        if (*str == '-')
          pp_fix = DECREMENT;
        else if (*str == '+')
          pp_fix = INCREMENT;
        else if (*str == 'r' || *str == 'R')
          {
            reg_expected = 1;
            str = extract_word(str, r_name, sizeof(r_name));
            //printf("\n extracted %s %c and %c", r_name, r_name[0], r_name[1]);
            //str--; /* rewind */
            if (ISDIGIT (r_name[1]))
              {
                if (r_name[2] == '\0' && (r_name[1] - '0' < 8))
                  op_mask = r_name[1] - '0';
                if (r_name[2] != '\0' && (r_name[1] - '0' > 7))
                  as_bad(_(": expected register name r0-r7 read %s"), r_name);
                //else{ //if (r_name[1] != '0'
                //	&& ISDIGIT (r_name[2])
                //	&& r_name[3] == '\0')
                //	op_mask = (r_name[1] - '0') * 10 + r_name[2] - '0';
                /* TODO fix so it checks for range 0-7 */
                continue;
                as_bad(_(": not digit expected register name r0-r7 read %s"),
                    r_name);
              }
          }
        str++;
      }
    op_mask <<= 2;
    op_mask |= pp_fix;
    //printf("\n built code %d from prefix %d",op_mask,pp_fix);
    break;
  case 'r': /* register operand.  */
    if (*str == 'r' || *str == 'R')
      {
        *bit_width = 3;

        str = extract_word(str, r_name, sizeof(r_name));
        //  printf("\n in case r extracted %s", r_name);
        op_mask = 0xff;
        if (ISDIGIT (r_name[1]))
          {
            if (r_name[2] == '\0')
              op_mask = r_name[1] - '0';
            else if (r_name[1] != '0' && ISDIGIT (r_name[2]) && r_name[3]
                == '\0')
              op_mask = (r_name[1] - '0') * 10 + r_name[2] - '0';
            if (op_mask > MAXREGISTER)
              as_bad(_(": expected register name r0-r7 read %s "), r_name);
          }
      }
    else
      {
        as_bad(_(": expected register name r0-r7 read %s "), r_name);

      }
    break;

  case 'i': /* immediate value or expression expected */
    (*op_con)++; /* advance the original format pointer */
    op_constraint++;

    if (ISDIGIT(*op_constraint))
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

    if (*str == '#')
      { /* go past # character */
        str++;
        if (!ISDIGIT(*op_constraint))
          as_bad(
              _(":expected bit length with constraint type i(# immediate) read %c"),
              *op_constraint);
        //printf("\n case is i input string is %s",str);
        op_mask = mc9xgate_get_constant(str, 0xFFFF);
        if ((opcode->name[strlen(opcode->name) - 1] == 'l')
					&& macroClipping)
        	{
        		op_mask &= 0x00FF;//printf("\n not 0_register parsing IMM8 with reloc %d", BFD_RELOC_MC9XGATE_IMM8_LO);
		    }else if ((opcode->name[strlen(opcode->name) - 1]) == 'h'
					&& macroClipping)
		    {
				op_mask >>= 8;
			}
        /* make sure it fits */
        for (i = *bit_width; i; i--)
          {
            max_size <<= 1;
            max_size += 1;
          }
        if (op_mask > max_size)
          as_bad(_(":operand value(%d) too big for constraint"), op_mask);
      }
    else
      {

        //printf("\n getting relocatable expresson from string %s", str);
        str = mc9xgate_parse_exp(str, &op_expr);

        fixS *fixp = 0;
        fixup_required = 1;

        //printf("\n need to do fixup");
        if (*op_constraint == '8')
          { /* mode == M68XG_OP_REL9 */
            //printf("\n length of opcode name is %d ", strlen(opcode->name));
            //printf("\n char is %c",opcode->name[strlen(opcode->name)]);
            //printf("\n char is %c",opcode->name[3]);

            if ((opcode->name[strlen(opcode->name) - 1] == 'l') && macroClipping)
              {
                //printf("\n not 0_register parsing IMM8 with reloc %d", BFD_RELOC_MC9XGATE_IMM8_LO);
                fixp = fix_new_exp(frag_now, where, 2, &op_expr, FALSE,
                    BFD_RELOC_MC9XGATE_IMM8_LO); /* BFD_RELOC_MC9XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_MC9XGATE_HI8*/
                fixp->fx_pcrel_adjust = 0;

              }

            if ((opcode->name[strlen(opcode->name) - 1]) == 'h' && macroClipping)
              {
                //printf("\n not 0_register parsing IMM8 with reloc %d", BFD_RELOC_MC9XGATE_IMM8_HI);
                fixp = fix_new_exp(frag_now, where, 2, &op_expr, FALSE,
                    BFD_RELOC_MC9XGATE_IMM8_HI); /* BFD_RELOC_MC9XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_MC9XGATE_HI8*/
                fixp->fx_pcrel_adjust = 0;

              }
            if (!fixp)
              {
                as_bad(_(":unknown relocation"));
              }
          }
        else if (*op_constraint == '5')
                  {

                    //printf("\n not 0_register parsing IMM4 with reloc %d", BFD_RELOC_MC9XGATE_IMM4);
                    fixp = fix_new_exp(frag_now, where, 2, &op_expr, FALSE,
                        BFD_RELOC_MC9XGATE_IMM5); /* BFD_RELOC_MC9XGATE_IMM5 forced type into bfd-in-2 around line 2367 R_MC9XGATE_HI8*/
                    fixp->fx_pcrel_adjust = 0;
                    //printf("\n need to gen 4 bit fixup"); unknown relocation type

                  }
        else if (*op_constraint == '4')
          {

            //printf("\n not 0_register parsing IMM4 with reloc %d", BFD_RELOC_MC9XGATE_IMM4);
            fixp = fix_new_exp(frag_now, where, 2, &op_expr, FALSE,
                BFD_RELOC_MC9XGATE_IMM4); /* BFD_RELOC_MC9XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_MC9XGATE_HI8*/
            fixp->fx_pcrel_adjust = 0;
            //printf("\n need to gen 4 bit fixup");

          }
        else if (*op_constraint == '3')
          {
            //printf("\n not 0_register parsing IMM3 with reloc %d", BFD_RELOC_MC9XGATE_IMM3);
            fixp = fix_new_exp(frag_now, where, 2, &op_expr, FALSE,
                BFD_RELOC_MC9XGATE_IMM3); /* BFD_RELOC_MC9XGATE_IMM8 forced type into bfd-in-2 around line 2367 R_MC9XGATE_HI8*/
            fixp->fx_pcrel_adjust = 0;
            //printf("\n need to gen 3 bit fixup");
          }
        else
          { printf(" constraint is %c opcode is %s", *op_constraint, opcode->format);
            as_bad(_(":unknown relocation constraint size"));
            //if (*op_constraint == 'f') { /* mode == M68XG_OP_REL10 */
            //	printf("\n not 0_register parsing 16");
            //	fixp = fix_new_exp (frag_now, where , 4,
            //			&op_expr, FALSE, BFD_RELOC_MC9XGATE_IMM8_HI); /* forced type into bfd-in-2 around line 2367 */
            //	fixp->fx_pcrel_adjust = 0;
          }

      }
    //printf("\n length is %c and read %d from get_constant", *op_constraint, op_mask);
    break;
  case 'c': /* CCR register expected */
    if (*str == 'c' || *str == 'C')
      {
        *bit_width = 0;

        str = extract_word(str, r_name, sizeof(r_name));
        //printf("\n in case c extracted %s", r_name);

        if (!(strcmp(r_name, "ccr") || strcmp(r_name, "CCR")))
          as_bad(_(": expected register name ccr read %s "), r_name);

      }
    else
      {
        as_bad(_(": expected character c or C  read %c"), *str);
      }
    break;

  case 'p': /* PC register expected */
    if (*str == 'p' || *str == 'P')
      {
        *bit_width = 0;

        str = extract_word(str, r_name, sizeof(r_name));
        //printf("\n in case c extracted %s", r_name);

        if (!(strcmp(r_name, "pc") || strcmp(r_name, "PC")))
          as_bad(_(": expected register name pc read %s "), r_name);

      }
    else
      {
        as_bad(_(": expected character p or P read %c "), *str);
      }
    break;
  case 'b': /* branch expected */
    //printf("\n getting branch expresson");
    str = mc9xgate_parse_exp(str, &op_expr);
    (*op_con)++;
    op_constraint++;
    //   	if ((j = op_expr->X_add_number) > 9)
    //     		as_bad(_(":branch longer than max bits"));
    //printf("\n reloc number is %d and %d constraint is %c", R_MC9XGATE_PCREL_9, BFD_RELOC_MC9XGATE_PCREL_9, *op_constraint);

    if (op_expr.X_op != O_register)
      {

        if (*op_constraint == '9')
          { /* mode == M68XG_OP_REL9 */
            //printf("\n not 0_register parsing reloc 9");
            fixS *fixp;
            fixp = fix_new_exp(frag_now, where, 2, &op_expr, TRUE,
                R_MC9XGATE_PCREL_9); /* forced type into bfd-in-2 around line 2367 */
            fixp->fx_pcrel_adjust = 1;
          }
        else if (*op_constraint == 'a')
          { /* mode == M68XG_OP_REL10 */
            fixS *fixp;
            fixp = fix_new_exp(frag_now, where, 2, &op_expr, TRUE,
                R_MC9XGATE_PCREL_10); /* forced type into bfd-in-2 around line 2367 */
            fixp->fx_pcrel_adjust = 1;
          }
      }
    else
      {
        as_fatal(_("Operand `%x' not recognized in fixup8."), op_expr.X_op);
      }
    break;
  case 'm': /* immediate value from macro expansion */
    //printf("\n trying to apply macro value");
    (*op_con)++; /* advance the origional format pointer */
    op_constraint++;

    if (ISDIGIT(*op_constraint))
      {
        *bit_width = (int) *op_constraint - '0';
      }
    else if (*op_constraint == 'a')
      {
        *bit_width = 0xA;
      }
    //as_bad(_(":expected numerical value after i constraint"));

    if (*str == '#') /* go past # character */
      str++;
    if (!ISDIGIT(*op_constraint))
      as_bad(
          _(":expected bit length with constraint type i(# immediate) read %c"),
          *op_constraint);
    //printf("\n case is i input string is %s",str);
    op_mask = (unsigned int) strtol(str, &str, 10);
    /* make sure it fits */
    for (i = *bit_width; i; i--)
      {
        max_size <<= 1;
        max_size += 1;
      }
    if (op_mask > max_size)
      as_bad(_(":operand too big for constraint"));

    break;
  case '?':
    break;

  default:
    as_bad(_("unknown constraint `%c'"), *op_constraint);
    }

  *line = str;
  return op_mask;
}

unsigned int mc9xgate_detect_format(char *line_in) {
  char num_operands = 0;
  char *str = skip_whitespace(line_in);
  int i = 0;
  int j = 0;
  char c = 0;
  unsigned int stripped_length = 0;
  char sh_format[10] = {0}; /* shorthand format */
  char operands_stripped[3][20] = {{0}};

  /* if the length is zero this is an inherent instruction */
  if(strlen(str) == 0) {
	  return  MC9XGATE_INH;
  }

  for(i = 0, j = 0, num_operands = 1; (c = TOLOWER (*str)) != 0; str++) {
	  if (c == ' ' || c == '\t' || c == '(' || c == ')' || c == '-' || c == '+') {
		  continue;
	  }
	  if(c == ',') {
		  j++;
		  num_operands++;
		  i = 0;
		  continue;
	  }
	  if(i > 20) { /* we should never need more than 20 chars to figure out what it is */
		  continue;
	  }
	  operands_stripped[j][i++] = c;
  }
  /* process our substrings to see what we have */
  for(i = 0, j = 0; num_operands > i; i++) {
	  stripped_length = strlen(&operands_stripped[i][0]);
	  /* add separator if we have more than one operand */
	  if(i > 0) {
		  sh_format[j++] = ',';
	  }
	  /* try to process by length first */
	  if(stripped_length > 3) {
		  sh_format[j++] = 'i';
	  }else if(stripped_length == 1) {
		  sh_format[j++] = 'i';
	  }else if(stripped_length == 2) {
		  if (operands_stripped[i][0] == 'r' && ISDIGIT(operands_stripped[i][1])) {
			  sh_format[j++] = 'r';
		  }else if(operands_stripped[i][0] == 'p' && operands_stripped[i][1] == 'c') {
				  sh_format[j++] = 'p';
		  } else {
			  sh_format[j++] = 'i';
		  }
	  } else if(stripped_length == 3){
		  if(operands_stripped[i][0] == 'c' && (operands_stripped[i][1] == 'c' && operands_stripped[i][2] == 'r')) {
			  sh_format[j++] = 'c';
		  }else if(operands_stripped[i][0] == '#') {
			  sh_format[j++] = 'i';
		  }
	  }
  }
  /* strings TODO maybe structure this*/
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
  if (!strcmp(i_string, sh_format) && num_operands == 1)
    return MC9XGATE_I;
  if (!strcmp(r_i_string, sh_format) && num_operands == 2)
    return MC9XGATE_R_I;
  if (!strcmp(r_r_r_string, sh_format) && num_operands == 3)
    return MC9XGATE_R_R_R;
  if (!strcmp(r_r_string, sh_format) && num_operands == 2)
    return MC9XGATE_R_R;
  if (!strcmp(r_string, sh_format) && num_operands == 1)
    return MC9XGATE_R;
  if (!strcmp(r_c_string, sh_format) && num_operands == 2)
    return MC9XGATE_R_C;
  if (!strcmp(c_r_string, sh_format) && num_operands == 2)
    return MC9XGATE_C_R;
  if (!strcmp(r_p_string, sh_format) && num_operands == 2)
    return MC9XGATE_R_P;
  if (!strcmp(r_r_i_string, sh_format) && num_operands == 3) {
	  return MC9XGATE_R_R_I;
  }
  as_bad((":Error unable to detect operand format"));
  return 0;
}

static struct mc9xgate_opcode*
mc9xgate_find_match(struct mc9xgate_opcode_handle *opcode_handle,
    unsigned char numberOfModes, unsigned int sh_format)
{
  struct mc9xgate_opcode *opcode = 0;
  /* TODO this is a shit function that needs to be redone this makes optable order sensitive*/
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
      case 5:
        if (opcode_handle->opc2->sh_format & sh_format)
          opcode = opcode_handle->opc2;
        break;
      default:
        as_bad((":Error too many operand to opcode combinations"));
        }
      numberOfModes--;
    }
  return opcode;
}

