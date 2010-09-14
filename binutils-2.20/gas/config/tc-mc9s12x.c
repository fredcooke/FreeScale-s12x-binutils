/* tc-m68hc11.c -- Assembler code for the Motorola 68HC11 & 68HC12.
   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009
   Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)

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
   Boston, MA 02110-1301, USA.  */
/*
 * scratch data
 * R_MC9S12X_16
 * howto is
 * Can't disassemble for architecture
 * failed to merge target specific data of file
 * Error 1
 * not valid
 * Indirect indexed addressing
 * architecture of input file
 * info_to_howto_rel
 */
#include "as.h"
#include "safe-ctype.h"
#include "subsegs.h"
#include "opcode/mc9s12x.h"
#include "dwarf2dbg.h"
#include "elf/mc9s12x.h"
#include "stdio.h" /* TODO only here for testing */

const char comment_chars[] = ";!";
const char line_comment_chars[] = "#*";
const char line_separator_chars[] = "";

const char EXP_CHARS[] = "eE";
const char FLT_CHARS[] = "dD";

#define STATE_CONDITIONAL_BRANCH	(1)
#define STATE_PC_RELATIVE		(2)
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

/* This macro has no side-effects.  */
#define ENCODE_RELAX(what,length) (((what) << 2) + (length))
#define RELAX_STATE(s) ((s) >> 2)
#define RELAX_LENGTH(s) ((s) & 3)

#define IS_OPCODE(C1,C2)        (((C1) & 0x0FF) == ((C2) & 0x0FF))

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

  /* Relax for bcc <L>.
     These insns are translated into b!cc +3 jmp L.  */
  {(127), (-128), 0, ENCODE_RELAX (STATE_CONDITIONAL_BRANCH, STATE_WORD)},
  {0, 0, 3, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},

  /* Relax for bsr <L> and bra <L>.
     These insns are translated into jsr and jmp.  */
  {(127), (-128), 0, ENCODE_RELAX (STATE_PC_RELATIVE, STATE_WORD)},
  {0, 0, 1, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},

  /* Relax for indexed offset: 5-bits, 9-bits, 16-bits.  */
  {(15), (-16), 0, ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS9)},
  {(255), (-256), 1, ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS16)},
  {0, 0, 2, 0},
  {1, 1, 0, 0},

  /* Relax for PC relative offset: 5-bits, 9-bits, 16-bits.
     For the 9-bit case, there will be a -1 correction to take into
     account the new byte that's why the range is -255..256.  */
  {(15), (-16), 0, ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS9)},
  {(256), (-255), 1, ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS16)},
  {0, 0, 2, 0},
  {1, 1, 0, 0},

  /* Relax for dbeq/ibeq/tbeq r,<L>:
     These insns are translated into db!cc +3 jmp L.  */
  {(255), (-256), 0, ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_WORD)},
  {0, 0, 3, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},

  /* Relax for bcc <L> on 68HC12.
     These insns are translated into lbcc <L>.  */
  {(127), (-128), 0, ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812, STATE_WORD)},
  {0, 0, 2, 0},
  {1, 1, 0, 0},
  {1, 1, 0, 0},

};

/* 68HC11 and 68HC12 registers.  They are numbered according to the 68HC12.  */
typedef enum register_id {
  REG_NONE = -1,
  REG_A = 0,
  REG_B = 1,
  REG_CCR = 2,
  REG_D = 4,
  REG_X = 5,
  REG_Y = 6,
  REG_SP = 7,
  REG_PC = 8
} register_id;

typedef struct operand {
  expressionS exp;
  register_id reg1;
  register_id reg2;
  int mode;
} operand;

struct mc9s12x_opcode_def {
  long format;
  int min_operands;
  int max_operands;
  int nb_modes;
  int used;
  struct mc9s12x_opcode *opcode;
};

static struct mc9s12x_opcode_def *mc9s12x_opcode_defs = 0;
static int mc9s12x_nb_opcode_defs = 0;
/* disable since we are not using
typedef struct alias {
  const char *name;
  const char *alias;
} alias;

static alias alias_opcodes[] = {
  {"cpd", "cmpd"},
  {"cpx", "cmpx"},
  {"cpy", "cmpy"},
  {0, 0}
};
*/
/* Local functions.  */
static register_id reg_name_search (char *);
static register_id register_name (void);
static int cmp_opcode (struct mc9s12x_opcode *, struct mc9s12x_opcode *);
static char *print_opcode_format (struct mc9s12x_opcode *, int);
static char *skip_whites (char *);
static int check_range (long, int);
static void print_opcode_list (void);
static void get_default_target (void);
static void print_insn_format (char *);
static int get_operand (operand *, int, long);
static void fixup8 (expressionS *, int, int);
static void fixup16 (expressionS *, int, int);
static void fixup24 (expressionS *, int, int);
static unsigned char convert_branch (unsigned char);
static char *mc9s12x_new_insn (int);
static void build_dbranch_insn (struct mc9s12x_opcode *,
                                operand *, int, int);
static int build_indexed_byte (operand *, int, int);
static int build_reg_mode (operand *, int);

static struct mc9s12x_opcode *find (struct mc9s12x_opcode_def *,
                                    operand *, int);
static struct mc9s12x_opcode *find_opcode (struct mc9s12x_opcode_def *,
                                           operand *, int *);
static void build_jump_insn (struct mc9s12x_opcode *, operand *, int, int);
static void build_insn (struct mc9s12x_opcode *, operand *, int);
static int relaxable_symbol (symbolS *);

/* Pseudo op to indicate a relax group.  */
static void s_mc9s12x_relax (int);

/* Pseudo op to control the ELF flags.  */
static void s_mc9s12x_mode (int);

/* Mark the symbols with STO_M68HC12_FAR to indicate the functions
   are using 'rtc' for returning.  It is necessary to use 'call'
   to invoke them.  This is also used by the debugger to correctly
   find the stack frame.  */
static void s_mc9s12x_mark_symbol (int);

/* Controls whether relative branches can be turned into long branches.
   When the relative offset is too large, the insn are changed:
    bra -> jmp
    bsr -> jsr
    bcc -> b!cc +3
           jmp L
    dbcc -> db!cc +3
            jmp L

  Setting the flag forbidds this.  */
static short flag_fixed_branches = 0;

/* Force to use long jumps (absolute) instead of relative branches.  */
static short flag_force_long_jumps = 0;

/* Change the direct addressing mode into an absolute addressing mode
   when the insn does not support direct addressing.
   For example, "clr *ZD0" is normally not possible and is changed
   into "clr ZDO".  */
static short flag_strict_direct_addressing = 1;

/* When an opcode has invalid operand, print out the syntax of the opcode
   to stderr.  */
static short flag_print_insn_syntax = 0;

/* Dumps the list of instructions with syntax and then exit:
   1 -> Only dumps the list (sorted by name)
   2 -> Generate an example (or test) that can be compiled.  */
static short flag_print_opcodes = 0;

/* Opcode hash table.  */
static struct hash_control *mc9s12x_hash;

/* Current cpu (either cpu6811 or cpu6812).  This is determined automagically
   by 'get_default_target' by looking at default BFD vector.  This is overridden
   with the -m<cpu> option.  */
static int current_architecture = 0;

/* Default cpu determined by 'get_default_target'.  */
static const char *default_cpu;

/* Number of opcodes in the sorted table (filtered by current cpu).  */
static int num_opcodes;

/* The opcodes sorted by name and filtered by current cpu.  */
static struct mc9s12x_opcode *mc9s12x_sorted_opcodes;

/* ELF flags to set in the output file header.  */
static int elf_flags = E_MC9S12X_F64;

/* These are the machine dependent pseudo-ops.  These are included so
   the assembler can work on the output from the SUN C compiler, which
   generates these.  */

/* This table describes all the machine specific pseudo-ops the assembler
   has to support.  The fields are:
   pseudo-op name without dot
   function to call to execute this pseudo-op
   Integer arg to pass to the function.  */
const pseudo_typeS md_pseudo_table[] = {
  /* The following pseudo-ops are supported for MRI compatibility.  */
  {"fcb", cons, 1},
  {"fdb", cons, 2},
  {"fcc", stringer, 8 + 1},
  {"rmb", s_space, 0},

  /* Motorola ALIS.  */
  {"xrefb", s_ignore, 0}, /* Same as xref  */

  /* Gcc driven relaxation.  */
  {"relax", s_mc9s12x_relax, 0},

  /* .mode instruction (ala SH).  */
  {"mode", s_mc9s12x_mode, 0},

  /* .far instruction.  */
  {"far", s_mc9s12x_mark_symbol, STO_MC9S12X_FAR},

  /* .interrupt instruction.  */
  {"interrupt", s_mc9s12x_mark_symbol, STO_MC9S12X_INTERRUPT},

  {0, 0, 0}
};

/* Options and initialization.  */

const char *md_shortopts = "Sm:";

struct option md_longopts[] = {
#define OPTION_FORCE_LONG_BRANCH (OPTION_MD_BASE)
  {"force-long-branches", no_argument, NULL, OPTION_FORCE_LONG_BRANCH},
  {"force-long-branchs", no_argument, NULL, OPTION_FORCE_LONG_BRANCH}, /* Misspelt version kept for backwards compatibility.  */

#define OPTION_SHORT_BRANCHES     (OPTION_MD_BASE + 1)
  {"short-branches", no_argument, NULL, OPTION_SHORT_BRANCHES},
  {"short-branchs", no_argument, NULL, OPTION_SHORT_BRANCHES}, /* Misspelt version kept for backwards compatibility.  */

#define OPTION_STRICT_DIRECT_MODE  (OPTION_MD_BASE + 2)
  {"strict-direct-mode", no_argument, NULL, OPTION_STRICT_DIRECT_MODE},

#define OPTION_PRINT_INSN_SYNTAX  (OPTION_MD_BASE + 3)
  {"print-insn-syntax", no_argument, NULL, OPTION_PRINT_INSN_SYNTAX},

#define OPTION_PRINT_OPCODES  (OPTION_MD_BASE + 4)
  {"print-opcodes", no_argument, NULL, OPTION_PRINT_OPCODES},

#define OPTION_GENERATE_EXAMPLE  (OPTION_MD_BASE + 5)
  {"generate-example", no_argument, NULL, OPTION_GENERATE_EXAMPLE},

#define OPTION_MSHORT  (OPTION_MD_BASE + 6)
  {"mshort", no_argument, NULL, OPTION_MSHORT},

#define OPTION_MLONG  (OPTION_MD_BASE + 7)
  {"mlong", no_argument, NULL, OPTION_MLONG},

#define OPTION_MSHORT_DOUBLE  (OPTION_MD_BASE + 8)
  {"mshort-double", no_argument, NULL, OPTION_MSHORT_DOUBLE},

#define OPTION_MLONG_DOUBLE  (OPTION_MD_BASE + 9)
  {"mlong-double", no_argument, NULL, OPTION_MLONG_DOUBLE},

  {NULL, no_argument, NULL, 0}
};
size_t md_longopts_size = sizeof (md_longopts);

/* Get the target cpu for the assembler.  This is based on the configure
   options and on the -m68hc11/-m68hc12 option.  If no option is specified,
   we must get the default.  */
const char *
mc9s12x_arch_format (void)
{
  get_default_target ();
  if (current_architecture & cpumc9s12x) /* TODO find 6811 and replace with s12x or refactor*/
    return "elf32-mc9s12x";
  else
	//printf("\n error in tc-mc9s12x line 348\n");
    return "error-line339";
}

enum bfd_architecture
mc9s12x_arch (void)
{
  get_default_target ();
  if (current_architecture & cpumc9s12x) /* TODO same as above todo */
    return bfd_arch_mc9s12x;
  else
	//printf("\n error in tc-mc9s12x line 348\n");
    return bfd_arch_mc9s12x;
}

int
mc9s12x_mach (void)
{
  return 0;
}

/* Listing header selected according to cpu.  */
const char *
mc9s12x_listing_header (void)
{
  if (current_architecture & cpumc9s12x)
    return "MC9S12X GAS "; /* TODO same as above */
  else
    return "ERROR MC9S12X GAS ";
}

void
md_show_usage (FILE *stream)   /* TODO fix options below */
{
  get_default_target ();
  fprintf (stream, _("\
Motorola 68HC11/68HC12/68HCS12 options:\n\
  -m68hc11 | -m68hc12 |\n\
  -m68hcs12               specify the processor [default %s]\n\
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
                          (used for testing)\n"), default_cpu);

}

/* Try to identify the default target based on the BFD library.  */
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
      if (strcmp (target->name, "elf32-m68hc12") == 0)
	{
	  current_architecture = cpumc9s12x;
	  default_cpu = "mc9s12x";
	}
      else if (strcmp (target->name, "elf32-mc9s12x") == 0)
	{
      current_architecture = cpumc9s12x;
	  default_cpu = "mc9s12x";
	}
      else
	{
	  as_bad (_("Default target `%s' is not supported."), target->name);
	}
    }
}

void
mc9s12x_print_statistics (FILE *file)
{
  int i;
  struct mc9s12x_opcode_def *opc;

  hash_print_statistics (file, "opcode table", mc9s12x_hash);

  opc = mc9s12x_opcode_defs;
  if (opc == 0 || mc9s12x_nb_opcode_defs == 0)
    return;

  /* Dump the opcode statistics table.  */
  fprintf (file, _("Name   # Modes  Min ops  Max ops  Modes mask  # Used\n"));
  for (i = 0; i < mc9s12x_nb_opcode_defs; i++, opc++)
    {
      fprintf (file, "%-7.7s  %5d  %7d  %7d  0x%08lx  %7d\n",
	       opc->opcode->name,
	       opc->nb_modes,
	       opc->min_operands, opc->max_operands, opc->format, opc->used);
    }
}

int
md_parse_option (int c, char *arg)
{
  get_default_target ();
  current_architecture = cpumc9s12x;
  switch (c)
    {
      /* -S means keep external to 2 bit offset rather than 16 bit one.  */
    case OPTION_SHORT_BRANCHES:
    case 'S':
      flag_fixed_branches = 1;
      break;

    case OPTION_FORCE_LONG_BRANCH:
      flag_force_long_jumps = 1;
      break;

    case OPTION_PRINT_INSN_SYNTAX:
      flag_print_insn_syntax = 1;
      break;

    case OPTION_PRINT_OPCODES:
      flag_print_opcodes = 1;
      break;

    case OPTION_STRICT_DIRECT_MODE:
      flag_strict_direct_addressing = 0;
      break;

    case OPTION_GENERATE_EXAMPLE:
      flag_print_opcodes = 2;
      break;

    case OPTION_MSHORT:
      elf_flags &= ~E_MC9S12X_I32;
      break;

    case OPTION_MLONG:
      elf_flags |= E_MC9S12X_I32;
      break;

    case OPTION_MSHORT_DOUBLE:
      elf_flags &= ~E_MC9S12X_F64;
      break;

    case OPTION_MLONG_DOUBLE:
      elf_flags |= E_MC9S12X_F64;
      break;

    case 'm':
      if (strcasecmp (arg, "MC9S12X") == 0)
	current_architecture = cpumc9s12x;
      else if (strcasecmp (arg, "MC9S12X") == 0)
	current_architecture = cpumc9s12x;
      else if (strcasecmp (arg, "68hcs12") == 0)
	current_architecture = cpumc9s12x | cpumc9s12x;
      /* TODO fix hack for gcc*/
      else if (strcasecmp (arg, "68hc11") == 0)
              current_architecture = cpumc9s12x | cpumc9s12x;
      else if (strcasecmp (arg, "c9s12x") == 0)
                    current_architecture = cpumc9s12x | cpumc9s12x;
      else if (strcasecmp (arg, "68hc12") == 0)
                          current_architecture = cpumc9s12x | cpumc9s12x;
      else
	as_bad (_("Option `%s' is not recognized."), arg);
      break;

    default:
      return 0;
    }

  return 1;
}

symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, TRUE);
}

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_get_section_alignment (stdoutput, seg);
  return ((addr + (1 << align) - 1) & (-1 << align));
}

static int
cmp_opcode (struct mc9s12x_opcode *op1, struct mc9s12x_opcode *op2)
{
  return strcmp (op1->name, op2->name);
}

#define IS_CALL_SYMBOL(MODE) \
(((MODE) & (MC9S12X_OP_PAGE|MC9S12X_OP_IND16)) \
  == ((MC9S12X_OP_PAGE|MC9S12X_OP_IND16)))

/* Initialize the assembler.  Create the opcode hash table
   (sorted on the names) with the M6811 opcode table
   (from opcode library).  */
void
md_begin (void)
{
 // printf("\n IN MD_BEGIN\n"); /* TODO FOR TESTING */
  char *prev_name = "";
  struct mc9s12x_opcode *opcodes;
  struct mc9s12x_opcode_def *opc = 0;
  int i;  /* removed j */

  get_default_target ();

  mc9s12x_hash = hash_new ();

  opcodes = (struct mc9s12x_opcode *) xmalloc (mc9s12x_num_opcodes *
					       sizeof (struct
						       mc9s12x_opcode));
  mc9s12x_sorted_opcodes = opcodes;
  num_opcodes = 0;
  //printf("\n seize of root structure %d",sizeof (struct mc9s12x_opcode)); //returns 16 SAME AS ORIGI code
  //printf("\n size of opcodes %d \n",mc9s12x_num_opcodes);  //origional code returns 775  this returns 575 since we are dropping hc11 codes
  //printf("\nCurrent arch is %d\n",current_architecture);  // this code returns 4 orig returns 4

  for (i = 0; i < mc9s12x_num_opcodes; i++)
    {
      if (mc9s12x_opcodes[i].arch & current_architecture)
      {
      opcodes[num_opcodes] = mc9s12x_opcodes[i];
	  if (opcodes[num_opcodes].name[0] == 'b'   /* get the first letter of the name */
	      && opcodes[num_opcodes].format & MC9S12X_OP_JUMP_REL
	      && !(opcodes[num_opcodes].format & MC9S12X_OP_BITMASK))
	    {
		//  printf("\n num_opcodes++ %s",opcodes[num_opcodes].name);  //should return 618
	      opcodes[num_opcodes] = mc9s12x_opcodes[i];
	    }
	  num_opcodes++;
	  /*
	   * since we are dropping hc11 the code below will be dropped, if this code is to be renabled mc9s12x_num_opcodes must be added to the size
	   * of the alias table or qsort will segfault  or something like that
	   */

	  /*for (j = 0; alias_opcodes[j].name != 0; j++)
	    if (strcmp (mc9s12x_opcodes[i].name, alias_opcodes[j].name) == 0)
	      {
		opcodes[num_opcodes] = mc9s12x_opcodes[i];
		opcodes[num_opcodes].name = alias_opcodes[j].alias;
		num_opcodes++;
		break;
	      }
	      */
	}
    }
  //print_opcode_list();
  //printf("\n number of OPcodes %d",num_opcodes);  //should be 599
  //printf("\n done building table\n");
  qsort (opcodes, num_opcodes, sizeof (struct mc9s12x_opcode),
         (int (*) (const void*, const void*)) cmp_opcode);

  //print_opcode_list();
  //printf("\n done sorting \n");

  opc = (struct mc9s12x_opcode_def *)
    xmalloc (num_opcodes * sizeof (struct mc9s12x_opcode_def));
  mc9s12x_opcode_defs = opc--;

  /* Insert unique names into hash table.  The M6811 instruction set
     has several identical opcode names that have different opcodes based
     on the operands.  This hash table then provides a quick index to
     the first opcode with a particular name in the opcode table.  */
  for (i = 0; i < num_opcodes; i++, opcodes++)
    {
      int expect;

      if (strcmp (prev_name, opcodes->name))
	{
	  prev_name = (char *) opcodes->name;

	  opc++;
	  opc->format = 0;
	  opc->min_operands = 100;
	  opc->max_operands = 0;
	  opc->nb_modes = 0;
	  opc->opcode = opcodes;
	  opc->used = 0;
	  hash_insert (mc9s12x_hash, opcodes->name, opc);
	}
      opc->nb_modes++;
      opc->format |= opcodes->format;

      /* See how many operands this opcode needs.  */
      expect = 0;
      if (opcodes->format & MC9S12X_OP_MASK)
	expect++;
      if (opcodes->format & MC9S12X_OP_BITMASK)
	expect++;
      if (opcodes->format & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
	expect++;
      if (opcodes->format & (MC9S12X_OP_IND16_P2 | MC9S12X_OP_IDX_P2))
	expect++;
      /* Special case for call instruction.  */
      if ((opcodes->format & MC9S12X_OP_PAGE)
          && !(opcodes->format & MC9S12X_OP_IND16))
        expect++;

      if (expect < opc->min_operands)
	opc->min_operands = expect;
      if (IS_CALL_SYMBOL (opcodes->format))
         expect++;
      if (expect > opc->max_operands)
	opc->max_operands = expect;
    }
  opc++;
  mc9s12x_nb_opcode_defs = opc - mc9s12x_opcode_defs;


// printf("\n at end of MD_BEGIN \n");


  if (flag_print_opcodes)
    {
      print_opcode_list ();
      exit (EXIT_SUCCESS);
    }
}

void
mc9s12x_init_after_args (void)
{
}

/* Builtin help.  */

/* Return a string that represents the operand format for the instruction.
   When example is true, this generates an example of operand.  This is used
   to give an example and also to generate a test.  */
static char *
print_opcode_format (struct mc9s12x_opcode *opcode, int example)
{
  static char buf[128];
  int format = opcode->format;
  char *p;

  p = buf;
  buf[0] = 0;
  if (format & MC9S12X_OP_IMM8)
    {
      if (example)
	sprintf (p, "#%d", rand () & 0x0FF);
      else
	strcpy (p, _("#<imm8>"));
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_IMM16)
    {
      if (example)
	sprintf (p, "#%d", rand () & 0x0FFFF);
      else
	strcpy (p, _("#<imm16>"));
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_IX)
    {
      if (example)
	sprintf (p, "%d,X", rand () & 0x0FF);
      else
	strcpy (p, _("<imm8>,X"));
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_IY)
    {
      if (example)
	sprintf (p, "%d,X", rand () & 0x0FF);
      else
	strcpy (p, _("<imm8>,X"));
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_IDX)
    {
      if (example)
	sprintf (p, "%d,X", rand () & 0x0FF);
      else
	strcpy (p, "n,r");
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_PAGE)
    {
      if (example)
	sprintf (p, ", %d", rand () & 0x0FF);
      else
	strcpy (p, ", <page>");
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_DIRECT)
    {
      if (example)
	sprintf (p, "*Z%d", rand () & 0x0FF);
      else
	strcpy (p, _("*<abs8>"));
      p = &p[strlen (p)];
    }

  if (format & MC9S12X_OP_BITMASK)
    {
      if (buf[0])
	*p++ = ' ';

      if (example)
	sprintf (p, "#$%02x", rand () & 0x0FF);
      else
	strcpy (p, _("#<mask>"));

      p = &p[strlen (p)];
      if (format & MC9S12X_OP_JUMP_REL)
	*p++ = ' ';
    }

  if (format & MC9S12X_OP_IND16)
    {
      if (example)
	sprintf (p, _("symbol%d"), rand () & 0x0FF);
      else
	strcpy (p, _("<abs>"));

      p = &p[strlen (p)];
    }

  if (format & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
    {
      if (example)
	{
	  if (format & MC9S12X_OP_BITMASK)
	    {
	      sprintf (p, ".+%d", rand () & 0x7F);
	    }
	  else
	    {
	      sprintf (p, "L%d", rand () & 0x0FF);
	    }
	}
      else
	strcpy (p, _("<label>"));
    }

  return buf;
}

/* Prints the list of instructions with the possible operands.  */
static void
print_opcode_list (void)
{
  int i;
  char *prev_name = "";
  struct mc9s12x_opcode *opcodes;
  int example = flag_print_opcodes == 2;

  if (example)
    printf (_("# Example of `%s' instructions\n\t.sect .text\n_start:\n"),
	    default_cpu);

  opcodes = mc9s12x_sorted_opcodes;

  /* Walk the list sorted on names (by md_begin).  We only report
     one instruction per line, and we collect the different operand
     formats.  */
  for (i = 0; i < num_opcodes; i++, opcodes++)
    {
      char *fmt = print_opcode_format (opcodes, example);

      if (example)
	{
	  printf ("L%d:\t", i);
	  printf ("%s %s\n", opcodes->name, fmt);
	}
      else
	{
	  if (strcmp (prev_name, opcodes->name))
	    {
	      if (i > 0)
		printf ("\n");

	      printf ("%-5.5s ", opcodes->name);
	      prev_name = (char *) opcodes->name;
	    }
	  if (fmt[0])
	    printf ("  [%s]", fmt);
	}
    }
  printf ("\n");
}

/* Print the instruction format.  This operation is called when some
   instruction is not correct.  Instruction format is printed as an
   error message.  */
static void
print_insn_format (char *name)
{
  struct mc9s12x_opcode_def *opc;
  struct mc9s12x_opcode *opcode;
  char buf[128];

  opc = (struct mc9s12x_opcode_def *) hash_find (mc9s12x_hash, name);
  if (opc == NULL)
    {
      as_bad (_("Instruction `%s' is not recognized."), name);
      return;
    }
  opcode = opc->opcode;

  as_bad (_("Instruction formats for `%s':"), name);
  do
    {
      char *fmt;

      fmt = print_opcode_format (opcode, 0);
      sprintf (buf, "\t%-5.5s %s", opcode->name, fmt);

      as_bad ("%s", buf);
      opcode++;
    }
  while (strcmp (opcode->name, name) == 0);
}

/* Analysis of 68HC11 and 68HC12 operands.  */

/* reg_name_search() finds the register number given its name.
   Returns the register number or REG_NONE on failure.  */
static register_id
reg_name_search (char *name)
{
  if (strcasecmp (name, "x") == 0 || strcasecmp (name, "ix") == 0)
    return REG_X;
  if (strcasecmp (name, "y") == 0 || strcasecmp (name, "iy") == 0)
    return REG_Y;
  if (strcasecmp (name, "a") == 0)
    return REG_A;
  if (strcasecmp (name, "b") == 0)
    return REG_B;
  if (strcasecmp (name, "d") == 0)
    return REG_D;
  if (strcasecmp (name, "sp") == 0)
    return REG_SP;
  if (strcasecmp (name, "pc") == 0)
    return REG_PC;
  if (strcasecmp (name, "ccr") == 0)
    return REG_CCR;

  return REG_NONE;
}

static char *
skip_whites (char *p)
{
  while (*p == ' ' || *p == '\t')
    p++;

  return p;
}

/* Check the string at input_line_pointer
   to see if it is a valid register name.  */
static register_id
register_name (void)
{
  register_id reg_number;
  char c, *p = input_line_pointer;

  if (!is_name_beginner (*p++))
    return REG_NONE;

  while (is_part_of_name (*p++))
    continue;

  c = *--p;
  if (c)
    *p++ = 0;

  /* Look to see if it's in the register table.  */
  reg_number = reg_name_search (input_line_pointer);
  if (reg_number != REG_NONE)
    {
      if (c)
	*--p = c;

      input_line_pointer = p;
      return reg_number;
    }
  if (c)
    *--p = c;

  return reg_number;
}
#define MC9S12X_OP_CALL_ADDR    0x00800000
#define MC9S12X_OP_PAGE_ADDR    0x04000000

/* Parse a string of operands and return an array of expressions.

   Operand      mode[0]         mode[1]       exp[0]       exp[1]
   #n           M6811_OP_IMM16  -             O_*
   *<exp>       M6811_OP_DIRECT -             O_*
   .{+-}<exp>   M6811_OP_JUMP_REL -           O_*
   <exp>        M6811_OP_IND16  -             O_*
   ,r N,r       M6812_OP_IDX    M6812_OP_REG  O_constant   O_register
   n,-r         M6812_PRE_DEC   M6812_OP_REG  O_constant   O_register
   n,+r         M6812_PRE_INC   " "
   n,r-         M6812_POST_DEC  " "
   n,r+         M6812_POST_INC  " "
   A,r B,r D,r  M6811_OP_REG    M6812_OP_REG  O_register   O_register
   [D,r]        M6811_OP_D_IDX  M6812_OP_REG  O_register   O_register
   [n,r]        M6811_OP_D_IDX_2 M6812_OP_REG  O_constant   O_register  */
static int
get_operand (operand *oper, int which, long opmode)
{
  char *p = input_line_pointer;
  int mode;
  register_id reg;

  oper->exp.X_op = O_absent;
  oper->reg1 = REG_NONE;
  oper->reg2 = REG_NONE;
  mode = MC9S12X_OP_NONE;

  p = skip_whites (p);

  if (*p == 0 || *p == '\n' || *p == '\r')
    {
      input_line_pointer = p;
      return 0;
    }

  if (*p == '*' && (opmode & (MC9S12X_OP_DIRECT | MC9S12X_OP_IND16)))
    {
      mode = MC9S12X_OP_DIRECT;
      p++;
    }
  else if (*p == '#')
    {
      if (!(opmode & (MC9S12X_OP_IMM8 | MC9S12X_OP_IMM16 | MC9S12X_OP_BITMASK)))
	{
	  as_bad (_("Immediate operand is not allowed for operand %d."),
		  which);
	  return -1;
	}

      mode = MC9S12X_OP_IMM16;
      p++;
      if (strncmp (p, "%hi", 3) == 0)
	{
	  p += 3;
	  mode |= MC9S12X_OP_HIGH_ADDR;
	}
      else if (strncmp (p, "%lo", 3) == 0)
	{
	  p += 3;
	  mode |= MC9S12X_OP_LOW_ADDR;
	}
      /* %page modifier is used to obtain only the page number
         of the address of a function.  */
      else if (strncmp (p, "%page", 5) == 0)
	{
	  p += 5;
	  mode |= MC9S12X_OP_PAGE_ADDR;
	}

      /* %addr modifier is used to obtain the physical address part
         of the function (16-bit).  For 68HC12 the function will be
         mapped in the 16K window at 0x8000 and the value will be
         within that window (although the function address may not fit
         in 16-bit).  See bfd/elf32-m68hc12.c for the translation.  */
      else if (strncmp (p, "%addr", 5) == 0)
	{
	  p += 5;
	  mode |= MC9S12X_OP_CALL_ADDR;
	}
    }
  else if (*p == '.' && (p[1] == '+' || p[1] == '-'))
    {
      p++;
      mode = MC9S12X_OP_JUMP_REL;
    }
  else if (*p == '[')
    {
	  /* TODO fix */
      if (current_architecture != cpumc9s12x)
	as_bad (_("Indirect indexed addressing is not valid for 68HC11."));

      p++;
      mode = MC9S12X_OP_D_IDX;
      p = skip_whites (p);
    }
  else if (*p == ',')		/* Special handling of ,x and ,y.  */
    {
      p++;
      input_line_pointer = p;

      reg = register_name ();
      if (reg != REG_NONE)
	{
	  oper->reg1 = reg;
	  oper->exp.X_op = O_constant;
	  oper->exp.X_add_number = 0;
	//  printf("\n adding a numper to expression on line1054");
	  oper->mode = MC9S12X_OP_IDX;
	  return 1;
	}
      as_bad (_("Spurious `,' or bad indirect register addressing mode."));
      return -1;
    }
  /* Handle 68HC12 page specification in 'call foo,%page(bar)'.  */
  else if ((opmode & MC9S12X_OP_PAGE) && strncmp (p, "%page", 5) == 0)
    {
      p += 5;
      mode = MC9S12X_OP_PAGE_ADDR |MC9S12X_OP_PAGE | MC9S12X_OP_IND16;
    }
  input_line_pointer = p;

  if (mode == MC9S12X_OP_NONE || mode == MC9S12X_OP_D_IDX)
    reg = register_name ();
  else
    reg = REG_NONE;

  if (reg != REG_NONE)
    {
      p = skip_whites (input_line_pointer);
      if (*p == ']' && mode == MC9S12X_OP_D_IDX)
	{
	  as_bad
	    (_("Missing second register or offset for indexed-indirect mode."));
	  return -1;
	}

      oper->reg1 = reg;
      oper->mode = mode | MC9S12X_OP_REG;
      if (*p != ',')
	{
	  if (mode == MC9S12X_OP_D_IDX)
	    {
	      as_bad (_("Missing second register for indexed-indirect mode."));
	      return -1;
	    }
	  return 1;
	}

      p++;
      input_line_pointer = p;
      reg = register_name ();
      if (reg != REG_NONE)
	{
	  p = skip_whites (input_line_pointer);
	  if (mode == MC9S12X_OP_D_IDX)
	    {
	      if (*p != ']')
		{
		  as_bad (_("Missing `]' to close indexed-indirect mode."));
		  return -1;
		}
	      p++;
              oper->mode = MC9S12X_OP_D_IDX;
	    }
	  input_line_pointer = p;

	  oper->reg2 = reg;
	  return 1;
	}
      return 1;
    }

  /* In MRI mode, isolate the operand because we can't distinguish
     operands from comments.  */
  if (flag_mri)
    {
      char c = 0;

      p = skip_whites (p);
      while (*p && *p != ' ' && *p != '\t')
	p++;

      if (*p)
	{
	  c = *p;
	  *p = 0;
	}

      /* Parse as an expression.  */
      expression (&oper->exp);
    //  printf("\n read expression line 1137");

      if (c)
	{
	  *p = c;
	}
    }
  else
    {
      expression (&oper->exp);
      //printf("\n read expression line 1147, %d",(int) oper.exp->X_unsigned);
    }

  if (oper->exp.X_op == O_illegal)
    {
      as_bad (_("Illegal operand."));
      return -1;
    }
  else if (oper->exp.X_op == O_absent)
    {
      as_bad (_("Missing operand."));
      return -1;
    }

  p = input_line_pointer;

  if (mode == MC9S12X_OP_NONE || mode == MC9S12X_OP_DIRECT
      || mode == MC9S12X_OP_D_IDX)
    {
      p = skip_whites (input_line_pointer);

      if (*p == ',')
	{
	  int possible_mode = MC9S12X_OP_NONE;
	  char *old_input_line;

	  old_input_line = p;
	  p++;

	  /* 68HC12 pre increment or decrement.  */
	  if (mode == MC9S12X_OP_NONE)
	    {
	      if (*p == '-')
		{
		  possible_mode = MC9S12X_PRE_DEC;
		  p++;
		}
	      else if (*p == '+')
		{
		  possible_mode = MC9S12X_PRE_INC;
		  p++;
		}
	      p = skip_whites (p);
	    }
	  input_line_pointer = p;
	  reg = register_name ();

	  /* Backtrack if we have a valid constant expression and
	     it does not correspond to the offset of the 68HC12 indexed
	     addressing mode (as in N,x).  */
	  if (reg == REG_NONE && mode == MC9S12X_OP_NONE
	      && possible_mode != MC9S12X_OP_NONE)
	    {
	      oper->mode = MC9S12X_OP_IND16 | MC9S12X_OP_JUMP_REL;
	      input_line_pointer = skip_whites (old_input_line);
	      return 1;
	    }

	  if (possible_mode != MC9S12X_OP_NONE)
	    mode = possible_mode;

	  if ((current_architecture != cpumc9s12x)
	      && possible_mode != MC9S12X_OP_NONE)
	    as_bad (_("Pre-increment mode is not valid for 68HC11"));
	  /* Backtrack.  */
	  if (which == 0 && opmode & MC9S12X_OP_IDX_P2
	      && reg != REG_X && reg != REG_Y
	      && reg != REG_PC && reg != REG_SP)
	    {
	      reg = REG_NONE;
	      input_line_pointer = p;
	    }

	  if (reg == REG_NONE && mode != MC9S12X_OP_DIRECT
	      && !(mode == MC9S12X_OP_NONE && opmode & MC9S12X_OP_IND16))
	    {
	      as_bad (_("Wrong register in register indirect mode."));
	      return -1;
	    }
	  if (mode == MC9S12X_OP_D_IDX)
	    {
	      p = skip_whites (input_line_pointer);
	      if (*p++ != ']')
		{
		  as_bad (_("Missing `]' to close register indirect operand."));
		  return -1;
		}
	      input_line_pointer = p;
              oper->reg1 = reg;
              oper->mode = MC9S12X_OP_D_IDX_2;
              return 1;
	    }
	  if (reg != REG_NONE)
	    {
	      oper->reg1 = reg;
	      if (mode == MC9S12X_OP_NONE)
		{
		  p = input_line_pointer;
		  if (*p == '-')
		    {
		      mode = MC9S12X_POST_DEC;
		      p++;
		      if (current_architecture != cpumc9s12x)
			as_bad
			  (_("Post-decrement mode is not valid for 68HC11."));
		    }
		  else if (*p == '+')
		    {
		      mode = MC9S12X_POST_INC;
		      p++;
		      if (current_architecture != cpumc9s12x){
		        printf("\n current arch is %d", current_architecture);
		        as_bad
			  (_("Post-increment mode is not valid for 68HC11."));

		      }
		    }
		  else
		    mode = MC9S12X_OP_IDX;

		  input_line_pointer = p;
		}
	      else
		mode |= MC9S12X_OP_IDX;

	      oper->mode = mode;
	      return 1;
	    }
          input_line_pointer = old_input_line;
	}

      if (mode == MC9S12X_OP_D_IDX_2)
	{
	  as_bad (_("Invalid indexed indirect mode."));
	  return -1;
	}
    }

  /* If the mode is not known until now, this is either a label
     or an indirect address.  */
  if (mode == MC9S12X_OP_NONE)
    mode = MC9S12X_OP_IND16 | MC9S12X_OP_JUMP_REL;

  p = input_line_pointer;
  while (*p == ' ' || *p == '\t')
    p++;
  input_line_pointer = p;
  oper->mode = mode;

  return 1;
}

#define MC9S12X_AUTO_INC_DEC (MC9S12X_PRE_INC | MC9S12X_PRE_DEC \
                            | MC9S12X_POST_INC | MC9S12X_POST_DEC)

/* Checks that the number 'num' fits for a given mode.  */
static int
check_range (long num, int mode)
{
  /* Auto increment and decrement are ok for [-8..8] without 0.  */
  if (mode & MC9S12X_AUTO_INC_DEC)
    return (num != 0 && num <= 8 && num >= -8);

  /* The 68HC12 supports 5, 9 and 16-bit offsets.  */
  if (mode & (MC9S12X_INDEXED_IND | MC9S12X_INDEXED | MC9S12X_OP_IDX))
    mode = MC9S12X_OP_IND16;

  if (mode & MC9S12X_OP_JUMP_REL16)
    mode = MC9S12X_OP_IND16;

  mode &= ~MC9S12X_OP_BRANCH;
  switch (mode)
    {
    case MC9S12X_OP_IX:
    case MC9S12X_OP_IY:
    case MC9S12X_OP_DIRECT:
      return (num >= 0 && num <= 255) ? 1 : 0;

    case MC9S12X_OP_BITMASK:
    case MC9S12X_OP_IMM8:
    case MC9S12X_OP_PAGE:
      return (((num & 0xFFFFFF00) == 0) || ((num & 0xFFFFFF00) == 0xFFFFFF00))
	? 1 : 0;

    case MC9S12X_OP_JUMP_REL:
      return (num >= -128 && num <= 127) ? 1 : 0;

    case MC9S12X_OP_IND16:
    case MC9S12X_OP_IND16 | MC9S12X_OP_PAGE:
    case MC9S12X_OP_IMM16:
      return (((num & 0xFFFF0000) == 0) || ((num & 0xFFFF0000) == 0xFFFF0000))
	? 1 : 0;

    case MC9S12X_OP_IBCC_MARKER:
    case MC9S12X_OP_TBCC_MARKER:
    case MC9S12X_OP_DBCC_MARKER:
      return (num >= -256 && num <= 255) ? 1 : 0;

    case MC9S12X_OP_TRAP_ID:
      return ((num >= 0x30 && num <= 0x39)
	      || (num >= 0x40 && num <= 0x0ff)) ? 1 : 0;

    default:
      return 0;
    }
}

/* Gas fixup generation.  */

/* Put a 1 byte expression described by 'oper'.  If this expression contains
   unresolved symbols, generate an 8-bit fixup.  */
static void
fixup8 (expressionS *oper, int mode, int opmode)
{
  char *f;

  f = frag_more (1);

  if (oper->X_op == O_constant)
    {
      if (mode & MC9S12X_OP_TRAP_ID
	  && !check_range (oper->X_add_number, MC9S12X_OP_TRAP_ID))
	{
	  static char trap_id_warn_once = 0;

	  as_bad (_("Trap id `%ld' is out of range."), oper->X_add_number);
	  if (trap_id_warn_once == 0)
	    {
	      trap_id_warn_once = 1;
	      as_bad (_("Trap id must be within [0x30..0x39] or [0x40..0xff]."));
	    }
	}

      if (!(mode & MC9S12X_OP_TRAP_ID)
	  && !check_range (oper->X_add_number, mode))
	{
	  as_bad (_("Operand out of 8-bit range: `%ld'."), oper->X_add_number);
	}
      number_to_chars_bigendian (f, oper->X_add_number & 0x0FF, 1);
    }
  else if (oper->X_op != O_register)
    {
      if (mode & MC9S12X_OP_TRAP_ID)
	as_bad (_("The trap id must be a constant."));

      if (mode == MC9S12X_OP_JUMP_REL)
	{
	  fixS *fixp;

	  fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
			      oper, TRUE, BFD_RELOC_8_PCREL);
	  fixp->fx_pcrel_adjust = 1;
	}
      else
	{
	  fixS *fixp;
          int reloc;

	  /* Now create an 8-bit fixup.  If there was some %hi, %lo
	     or %page modifier, generate the reloc accordingly.  */
          if (opmode & MC9S12X_OP_HIGH_ADDR)
            reloc = BFD_RELOC_MC9S12X_HI8;
          else if (opmode & MC9S12X_OP_LOW_ADDR)
            reloc = BFD_RELOC_MC9S12X_LO8;
          else if (opmode & MC9S12X_OP_PAGE_ADDR)
            reloc = BFD_RELOC_MC9S12X_PAGE;
          else
            reloc = BFD_RELOC_8;

	  fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 1,
                              oper, FALSE, reloc);
          if (reloc != BFD_RELOC_8)
            fixp->fx_no_overflow = 1;
	}
      number_to_chars_bigendian (f, 0, 1);
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup8."), oper->X_op);
    }
}

/* Put a 2 byte expression described by 'oper'.  If this expression contains
   unresolved symbols, generate a 16-bit fixup.  */
static void
fixup16 (expressionS *oper, int mode, int opmode ATTRIBUTE_UNUSED)
{
  char *f;

  f = frag_more (2);

  if (oper->X_op == O_constant)
    {
      if (!check_range (oper->X_add_number, mode))
	{
	  as_bad (_("Operand out of 16-bit range: `%ld'."),
		  oper->X_add_number);
	}
      number_to_chars_bigendian (f, oper->X_add_number & 0x0FFFF, 2);

    }
  else if (oper->X_op != O_register)
    {
      fixS *fixp;
      int reloc;

      if ((opmode & MC9S12X_OP_CALL_ADDR) && (mode & MC9S12X_OP_IMM16))
        reloc = BFD_RELOC_MC9S12X_LO16;
      else if (mode & MC9S12X_OP_JUMP_REL16)
        reloc = BFD_RELOC_16_PCREL;
      else if (mode & MC9S12X_OP_PAGE)
        reloc = BFD_RELOC_MC9S12X_LO16;
      else
        reloc = BFD_RELOC_16;

      /* Now create a 16-bit fixup.  */
      fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 2,
			  oper,
			  reloc == BFD_RELOC_16_PCREL,
                          reloc);
      number_to_chars_bigendian (f, 0, 2);
      if (reloc == BFD_RELOC_16_PCREL)
	fixp->fx_pcrel_adjust = 2;
      if (reloc == BFD_RELOC_MC9S12X_LO16)
        fixp->fx_no_overflow = 1;
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup16."), oper->X_op);
    }
}

/* Put a 3 byte expression described by 'oper'.  If this expression contains
   unresolved symbols, generate a 24-bit fixup.  */
static void
fixup24 (expressionS *oper, int mode, int opmode ATTRIBUTE_UNUSED)
{
  char *f;

  f = frag_more (3);

  if (oper->X_op == O_constant)
    {
      if (!check_range (oper->X_add_number, mode))
	{
	  as_bad (_("Operand out of 16-bit range: `%ld'."),
		  oper->X_add_number);
	}
      number_to_chars_bigendian (f, oper->X_add_number & 0x0FFFFFF, 3);
     // printf("\n in fixup24 line 1494");
    }
  else if (oper->X_op != O_register)
    {
      fixS *fixp;

      /* Now create a 24-bit fixup.  */
      fixp = fix_new_exp (frag_now, f - frag_now->fr_literal, 3,
			  oper, FALSE, BFD_RELOC_MC9S12X_24);
      number_to_chars_bigendian (f, 0, 3);
    }
  else
    {
      as_fatal (_("Operand `%x' not recognized in fixup16."), oper->X_op);
    }
}

/* 68HC11 and 68HC12 code generation.  */

/* Translate the short branch/bsr instruction into a long branch.  */
static unsigned char
convert_branch (unsigned char code)
{
  if (IS_OPCODE (code, MC9S12X_BSR2))
    return MC9S12X_JSR;
  else if (IS_OPCODE (code, MC9S12X_BSR))
    return MC9S12X_JSR;
  else if (IS_OPCODE (code, MC9S12X_BRA))
    return (current_architecture & cpumc9s12x) ? MC9S12X_JMP2 : MC9S12X_JMP2;
  else
    as_fatal (_("Unexpected branch conversion with `%x'"), code);

  /* Keep gcc happy.  */
  return MC9S12X_JSR2;
}

/* Start a new insn that contains at least 'size' bytes.  Record the
   line information of that insn in the dwarf2 debug sections.  */
static char *
mc9s12x_new_insn (int size)
{
  char *f;

  f = frag_more (size);

  dwarf2_emit_insn (size);

  return f;
}

/* Builds a jump instruction (bra, bcc, bsr).  */
static void
build_jump_insn (struct mc9s12x_opcode *opcode, operand operands[],
                 int nb_operands, int jmp_mode)
{
  unsigned char code;
  char *f;
  unsigned long n;
  fragS *frag;
  int where;

 // printf("\n in build_jump_insn %s",opcode->name);


  /* The relative branch conversion is not supported for
     brclr and brset.  */
  gas_assert ((opcode->format & MC9S12X_OP_BITMASK) == 0);
  gas_assert (nb_operands == 1);
  gas_assert (operands[0].reg1 == REG_NONE && operands[0].reg2 == REG_NONE);

  code = opcode->opcode;

  n = operands[0].exp.X_add_number;

  /* Turn into a long branch:
     - when force long branch option (and not for jbcc pseudos),
     - when jbcc and the constant is out of -128..127 range,
     - when branch optimization is allowed and branch out of range.  */
  if ((jmp_mode == 0 && flag_force_long_jumps)
      || (operands[0].exp.X_op == O_constant
	  && (!check_range (n, opcode->format) &&
	      (jmp_mode == 1 || flag_fixed_branches == 0))))
    {
      frag = frag_now;
      where = frag_now_fix ();

      fix_new (frag_now, frag_now_fix (), 0,
               &abs_symbol, 0, 1, BFD_RELOC_MC9S12X_RL_JUMP);

      if (code == MC9S12X_BSR || code == MC9S12X_BRA || code == MC9S12X_BSR2)
	{
	  code = convert_branch (code);

	  f = mc9s12x_new_insn (1);
	  number_to_chars_bigendian (f, code, 1);
	}
      else if (current_architecture & cpumc9s12x)
	{
	  /* 68HC12: translate the bcc into a lbcc.  */
	  f = mc9s12x_new_insn (2);
	  number_to_chars_bigendian (f, MC9S12X_OPCODE_PAGE2, 1);
	  number_to_chars_bigendian (f + 1, code, 1);
	  fixup16 (&operands[0].exp, MC9S12X_OP_JUMP_REL16,
		   MC9S12X_OP_JUMP_REL16);
	  return;
	}
      else
	{
	  /* 68HC11: translate the bcc into b!cc +3; jmp <L>.  */
	  f = mc9s12x_new_insn (3);
	  code ^= 1;
	  number_to_chars_bigendian (f, code, 1);
	  number_to_chars_bigendian (f + 1, 3, 1);
	  number_to_chars_bigendian (f + 2, MC9S12X_JMP, 1);
	}
      fixup16 (&operands[0].exp, MC9S12X_OP_IND16, MC9S12X_OP_IND16);
      return;
    }

  /* Branch with a constant that must fit in 8-bits.  */
  if (operands[0].exp.X_op == O_constant)
    {
      if (!check_range (n, opcode->format))
	{
	  as_bad (_("Operand out of range for a relative branch: `%ld'"),
                  n);
	}
      else if (opcode->format & MC9S12X_OP_JUMP_REL16)
	{
	  f = mc9s12x_new_insn (4);
	  number_to_chars_bigendian (f, MC9S12X_OPCODE_PAGE2, 1);
	  number_to_chars_bigendian (f + 1, code, 1);
	  number_to_chars_bigendian (f + 2, n & 0x0ffff, 2);
	}
      else
	{
	  f = mc9s12x_new_insn (2);
	  number_to_chars_bigendian (f, code, 1);
	  number_to_chars_bigendian (f + 1, n & 0x0FF, 1);
	}
    }
  else if (opcode->format & MC9S12X_OP_JUMP_REL16)
    {
      frag = frag_now;
      where = frag_now_fix ();

      fix_new (frag_now, frag_now_fix (), 0,
               &abs_symbol, 0, 1, BFD_RELOC_MC9S12X_RL_JUMP);

      f = mc9s12x_new_insn (2);
      number_to_chars_bigendian (f, MC9S12X_OPCODE_PAGE2, 1);
      number_to_chars_bigendian (f + 1, code, 1);
      fixup16 (&operands[0].exp, MC9S12X_OP_JUMP_REL16, MC9S12X_OP_JUMP_REL16);
    }
  else
    {
      char *opcode;

      frag = frag_now;
      where = frag_now_fix ();
      
      fix_new (frag_now, frag_now_fix (), 0,
               &abs_symbol, 0, 1, BFD_RELOC_MC9S12X_RL_JUMP);

      /* Branch offset must fit in 8-bits, don't do some relax.  */
      if (jmp_mode == 0 && flag_fixed_branches)
	{
	  opcode = mc9s12x_new_insn (1);
	  number_to_chars_bigendian (opcode, code, 1);
	  fixup8 (&operands[0].exp, MC9S12X_OP_JUMP_REL, MC9S12X_OP_JUMP_REL);
	}

      /* bra/bsr made be changed into jmp/jsr.  */
      else if (code == MC9S12X_BSR || code == MC9S12X_BRA || code == MC9S12X_BSR2)
	{
          /* Allocate worst case storage.  */
	  opcode = mc9s12x_new_insn (3);
	  number_to_chars_bigendian (opcode, code, 1);
	  number_to_chars_bigendian (opcode + 1, 0, 1);
	  frag_variant (rs_machine_dependent, 1, 1,
                        ENCODE_RELAX (STATE_PC_RELATIVE, STATE_UNDF),
                        operands[0].exp.X_add_symbol, (offsetT) n,
                        opcode);
	}
      else if (current_architecture & cpumc9s12x)
	{
	  opcode = mc9s12x_new_insn (2);
	  number_to_chars_bigendian (opcode, code, 1);
	  number_to_chars_bigendian (opcode + 1, 0, 1);
	  frag_var (rs_machine_dependent, 2, 2,
		    ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812, STATE_UNDF),
		    operands[0].exp.X_add_symbol, (offsetT) n, opcode);
	}
      else
	{
	  opcode = mc9s12x_new_insn (2);
	  number_to_chars_bigendian (opcode, code, 1);
	  number_to_chars_bigendian (opcode + 1, 0, 1);
	  frag_var (rs_machine_dependent, 3, 3,
		    ENCODE_RELAX (STATE_CONDITIONAL_BRANCH, STATE_UNDF),
		    operands[0].exp.X_add_symbol, (offsetT) n, opcode);
	}
    }
}

/* Builds a dbne/dbeq/tbne/tbeq instruction.  */
static void
build_dbranch_insn (struct mc9s12x_opcode *opcode, operand operands[],
                    int nb_operands, int jmp_mode)
{
  unsigned char code;
  char *f;
  unsigned long n;

 // printf("\n in build_dbranch_insn %s",opcode->name);


  /* The relative branch conversion is not supported for
     brclr and brset.  */
  gas_assert ((opcode->format & MC9S12X_OP_BITMASK) == 0);
  gas_assert (nb_operands == 2);
  gas_assert (operands[0].reg1 != REG_NONE);

  code = opcode->opcode & 0x0FF;

  f = mc9s12x_new_insn (1);
  number_to_chars_bigendian (f, code, 1);

  n = operands[1].exp.X_add_number;
  code = operands[0].reg1;

  if (operands[0].reg1 == REG_NONE || operands[0].reg1 == REG_CCR
      || operands[0].reg1 == REG_PC)
    as_bad (_("Invalid register for dbcc/tbcc instruction."));

  if (opcode->format & MC9S12X_OP_IBCC_MARKER)
    code |= 0x80;
  else if (opcode->format & MC9S12X_OP_TBCC_MARKER)
    code |= 0x40;

  if (!(opcode->format & MC9S12X_OP_EQ_MARKER))
    code |= 0x20;

  /* Turn into a long branch:
     - when force long branch option (and not for jbcc pseudos),
     - when jdbcc and the constant is out of -256..255 range,
     - when branch optimization is allowed and branch out of range.  */
  if ((jmp_mode == 0 && flag_force_long_jumps)
      || (operands[1].exp.X_op == O_constant
	  && (!check_range (n, MC9S12X_OP_IBCC_MARKER) &&
	      (jmp_mode == 1 || flag_fixed_branches == 0))))
    {
      f = frag_more (2);
      code ^= 0x20;
      number_to_chars_bigendian (f, code, 1);
      number_to_chars_bigendian (f + 1, MC9S12X_JMP2, 1);
      fixup16 (&operands[0].exp, MC9S12X_OP_IND16, MC9S12X_OP_IND16);
      return;
    }

  /* Branch with a constant that must fit in 9-bits.  */
  if (operands[1].exp.X_op == O_constant)
    {
      if (!check_range (n, MC9S12X_OP_IBCC_MARKER))
	{
	  as_bad (_("Operand out of range for a relative branch: `%ld'"),
                  n);
	}
      else
	{
	  if ((long) n < 0)
	    code |= 0x10;

	  f = frag_more (2);
	  number_to_chars_bigendian (f, code, 1);
	  number_to_chars_bigendian (f + 1, n & 0x0FF, 1);
	}
    }
  else
    {
      /* Branch offset must fit in 8-bits, don't do some relax.  */
      if (jmp_mode == 0 && flag_fixed_branches)
	{
	  fixup8 (&operands[0].exp, MC9S12X_OP_JUMP_REL, MC9S12X_OP_JUMP_REL);
	}

      else
	{
	  f = frag_more (2);
	  number_to_chars_bigendian (f, code, 1);
	  number_to_chars_bigendian (f + 1, 0, 1);
	  frag_var (rs_machine_dependent, 3, 3,
		    ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_UNDF),
		    operands[1].exp.X_add_symbol, (offsetT) n, f);
	}
    }
}

#define OP_EXTENDED (MC9S12X_OP_PAGE2 | MC9S12X_OP_PAGE3 | MC9S12X_OP_PAGE4)

/* Assemble the post index byte for 68HC12 extended addressing modes.  */
static int
build_indexed_byte (operand *op, int format ATTRIBUTE_UNUSED, int move_insn)
{
  unsigned char byte = 0;
  char *f;
  int mode;
  long val;

  //printf("\n in build_indexed_byte %s",opcode->name);


  val = op->exp.X_add_number;
  mode = op->mode;
  if (mode & MC9S12X_AUTO_INC_DEC)
    {
      byte = 0x20;
      if (mode & (MC9S12X_POST_INC | MC9S12X_POST_DEC))
	byte |= 0x10;

      if (op->exp.X_op == O_constant)
	{
	  if (!check_range (val, mode))
	    {
	      as_bad (_("Increment/decrement value is out of range: `%ld'."),
		      val);
	    }
	  if (mode & (MC9S12X_POST_INC | MC9S12X_PRE_INC))
	    byte |= (val - 1) & 0x07;
	  else
	    byte |= (8 - ((val) & 7)) | 0x8;
	}
      switch (op->reg1)
	{
	case REG_NONE:
	  as_fatal (_("Expecting a register."));

	case REG_X:
	  byte |= 0;
	  break;

	case REG_Y:
	  byte |= 0x40;
	  break;

	case REG_SP:
	  byte |= 0x80;
	  break;

	default:
	  as_bad (_("Invalid register for post/pre increment."));
	  break;
	}

      f = frag_more (1);
      number_to_chars_bigendian (f, byte, 1);
      return 1;
    }

  if (mode & (MC9S12X_OP_IDX | MC9S12X_OP_D_IDX_2))
    {
      switch (op->reg1)
	{
	case REG_X:
	  byte = 0;
	  break;

	case REG_Y:
	  byte = 1;
	  break;

	case REG_SP:
	  byte = 2;
	  break;

	case REG_PC:
	  byte = 3;
	  break;

	default:
	  as_bad (_("Invalid register."));
	  break;
	}
      if (op->exp.X_op == O_constant)
	{
	  if (!check_range (val, MC9S12X_OP_IDX))
	    {
	      as_bad (_("Offset out of 16-bit range: %ld."), val);
	    }

	  if (move_insn && !(val >= -16 && val <= 15))
	    {
	      as_bad (_("Offset out of 5-bit range for movw/movb insn: %ld."),
		      val);
	      return -1;
	    }

	  if (val >= -16 && val <= 15 && !(mode & MC9S12X_OP_D_IDX_2))
	    {
	      byte = byte << 6;
	      byte |= val & 0x1f;
	      f = frag_more (1);
	      number_to_chars_bigendian (f, byte, 1);
	      return 1;
	    }
	  else if (val >= -256 && val <= 255 && !(mode & MC9S12X_OP_D_IDX_2))
	    {
	      byte = byte << 3;
	      byte |= 0xe0;
	      if (val < 0)
		byte |= 0x1;
	      f = frag_more (2);
	      number_to_chars_bigendian (f, byte, 1);
	      number_to_chars_bigendian (f + 1, val & 0x0FF, 1);
	      return 2;
	    }
	  else
	    {
	      byte = byte << 3;
	      if (mode & MC9S12X_OP_D_IDX_2)
		byte |= 0xe3;
	      else
		byte |= 0xe2;

	      f = frag_more (3);
	      number_to_chars_bigendian (f, byte, 1);
	      number_to_chars_bigendian (f + 1, val & 0x0FFFF, 2);
	      return 3;
	    }
	}
      if (mode & MC9S12X_OP_D_IDX_2)
        {
          byte = (byte << 3) | 0xe3;
          f = frag_more (1);
          number_to_chars_bigendian (f, byte, 1);

          fixup16 (&op->exp, 0, 0);
        }
      else if (op->reg1 != REG_PC)
	{
          symbolS *sym;
          offsetT off;

	  f = frag_more (1);
	  number_to_chars_bigendian (f, byte, 1);
          sym = op->exp.X_add_symbol;
          off = op->exp.X_add_number;
          if (op->exp.X_op != O_symbol)
            {
              sym = make_expr_symbol (&op->exp);
              off = 0;
            }
	  /* movb/movw cannot be relaxed.  */
	  if (move_insn)
	    {
	      byte <<= 6;
	      number_to_chars_bigendian (f, byte, 1);
	      fix_new (frag_now, f - frag_now->fr_literal, 1,
		       sym, off, 0, BFD_RELOC_MC9S12X_5B);
	      return 1;
	    }
	  else
	    {
	      number_to_chars_bigendian (f, byte, 1);
	      frag_var (rs_machine_dependent, 2, 2,
			ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_UNDF),
			sym, off, f);
	    }
	}
      else
	{
	  f = frag_more (1);
	  /* movb/movw cannot be relaxed.  */
	  if (move_insn)
	    {
	      byte <<= 6;
	      number_to_chars_bigendian (f, byte, 1);
	      fix_new (frag_now, f - frag_now->fr_literal, 1,
		       op->exp.X_add_symbol, op->exp.X_add_number, 0, BFD_RELOC_MC9S12X_5B);
	      return 1;
	    }
	  else
	    {
	      number_to_chars_bigendian (f, byte, 1);
	      frag_var (rs_machine_dependent, 2, 2,
			ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_UNDF),
			op->exp.X_add_symbol,
			op->exp.X_add_number, f);
	    }
	}
      return 3;
    }

  if (mode & (MC9S12X_OP_REG | MC9S12X_OP_D_IDX))
    {
      if (mode & MC9S12X_OP_D_IDX)
	{
	  if (op->reg1 != REG_D)
	    as_bad (_("Expecting register D for indexed indirect mode."));
	  if (move_insn)
	    as_bad (_("Indexed indirect mode is not allowed for movb/movw."));

	  byte = 0xE7;
	}
      else
	{
	  switch (op->reg1)
	    {
	    case REG_A:
	      byte = 0xE4;
	      break;

	    case REG_B:
	      byte = 0xE5;
	      break;

	    default:
	      as_bad (_("Invalid accumulator register."));

	    case REG_D:
	      byte = 0xE6;
	      break;
	    }
	}
      switch (op->reg2)
	{
	case REG_X:
	  break;

	case REG_Y:
	  byte |= (1 << 3);
	  break;

	case REG_SP:
	  byte |= (2 << 3);
	  break;

	case REG_PC:
	  byte |= (3 << 3);
	  break;

	default:
	  as_bad (_("Invalid indexed register."));
	  break;
	}
      f = frag_more (1);
      number_to_chars_bigendian (f, byte, 1);
      return 1;
    }

  as_fatal (_("Addressing mode not implemented yet."));
  return 0;
}

/* Assemble the 68HC12 register mode byte.  */
static int
build_reg_mode (operand *op, int format)
{
  unsigned char byte;
  char *f;

  if (format & MC9S12X_OP_SEX_MARKER
      && op->reg1 != REG_A && op->reg1 != REG_B && op->reg1 != REG_CCR)
    as_bad (_("Invalid source register for this instruction, use 'tfr'."));
  else if (op->reg1 == REG_NONE || op->reg1 == REG_PC)
    as_bad (_("Invalid source register."));

  if (format & MC9S12X_OP_SEX_MARKER
      && op->reg2 != REG_D
      && op->reg2 != REG_X && op->reg2 != REG_Y && op->reg2 != REG_SP)
    as_bad (_("Invalid destination register for this instruction, use 'tfr'."));
  else if (op->reg2 == REG_NONE || op->reg2 == REG_PC)
    as_bad (_("Invalid destination register."));

  byte = (op->reg1 << 4) | (op->reg2);
  if (format & MC9S12X_OP_EXG_MARKER)
    byte |= 0x80;

  f = frag_more (1);
  number_to_chars_bigendian (f, byte, 1);
  return 1;
}

/* build_insn takes a pointer to the opcode entry in the opcode table,
   the array of operand expressions and builds the corresponding instruction.
   This operation only deals with non relative jumps insn (need special
   handling).  */
static void
build_insn (struct mc9s12x_opcode *opcode, operand operands[],
            int nb_operands ATTRIBUTE_UNUSED)
{
  int i;
  char *f;
  long format;
  int move_insn = 0;
  //printf("\n in build_insn %s",opcode->name);
  /* Put the page code instruction if there is one.  */
  format = opcode->format;

  if (format & MC9S12X_OP_BRANCH)
    fix_new (frag_now, frag_now_fix (), 0,
             &abs_symbol, 0, 1, BFD_RELOC_MC9S12X_RL_JUMP);

  if (format & OP_EXTENDED)
    {
      int page_code;

      f = mc9s12x_new_insn (2);
      if (format & MC9S12X_OP_PAGE2)
	page_code = MC9S12X_OPCODE_PAGE2;
      else if (format & MC9S12X_OP_PAGE3)
	page_code = MC9S12X_OPCODE_PAGE3;
      else
	page_code = MC9S12X_OPCODE_PAGE4;

      number_to_chars_bigendian (f, page_code, 1);
      f++;
    }
  else
    f = mc9s12x_new_insn (1);

  number_to_chars_bigendian (f, opcode->opcode, 1);

  i = 0;

  /* The 68HC12 movb and movw instructions are special.  We have to handle
     them in a special way.  */
  if (format & (MC9S12X_OP_IND16_P2 | MC9S12X_OP_IDX_P2))
    {
      move_insn = 1;
      if (format & MC9S12X_OP_IDX)
	{
	  build_indexed_byte (&operands[0], format, 1);
	  i = 1;
	  format &= ~MC9S12X_OP_IDX;
	}
      if (format & MC9S12X_OP_IDX_P2)
	{
	  build_indexed_byte (&operands[1], format, 1);
	  i = 0;
	  format &= ~MC9S12X_OP_IDX_P2;
	}
    }

  if (format & (MC9S12X_OP_DIRECT | MC9S12X_OP_IMM8))
    {
      fixup8 (&operands[i].exp,
	      format & (MC9S12X_OP_DIRECT | MC9S12X_OP_IMM8 | MC9S12X_OP_TRAP_ID),
	      operands[i].mode);
      i++;
    }
  else if (IS_CALL_SYMBOL (format) && nb_operands == 1)
    {
      format &= ~MC9S12X_OP_PAGE;
      fixup24 (&operands[i].exp, format & MC9S12X_OP_IND16,
	       operands[i].mode);
      i++;
    }
  else if (format & (MC9S12X_OP_IMM16 | MC9S12X_OP_IND16))
    {
      fixup16 (&operands[i].exp,
               format & (MC9S12X_OP_IMM16 | MC9S12X_OP_IND16 | MC9S12X_OP_PAGE),
	       operands[i].mode);
      i++;
    }
  else if (format & (MC9S12X_OP_IX | MC9S12X_OP_IY))
    {
      if ((format & MC9S12X_OP_IX) && (operands[0].reg1 != REG_X))
	as_bad (_("Invalid indexed register, expecting register X."));
      if ((format & MC9S12X_OP_IY) && (operands[0].reg1 != REG_Y))
	as_bad (_("Invalid indexed register, expecting register Y."));

      fixup8 (&operands[0].exp, MC9S12X_OP_IX, operands[0].mode);
      i = 1;
    }
  else if (format &
	   (MC9S12X_OP_IDX | MC9S12X_OP_IDX_2 | MC9S12X_OP_IDX_1
            | MC9S12X_OP_D_IDX | MC9S12X_OP_D_IDX_2))
    {
      build_indexed_byte (&operands[i], format, move_insn);
      i++;
    }
  else if (format & MC9S12X_OP_REG && current_architecture & cpumc9s12x)
    {
      build_reg_mode (&operands[i], format);
      i++;
    }
  if (format & MC9S12X_OP_BITMASK)
    {
      fixup8 (&operands[i].exp, MC9S12X_OP_BITMASK, operands[i].mode);
      i++;
    }
  if (format & MC9S12X_OP_JUMP_REL)
    {
      fixup8 (&operands[i].exp, MC9S12X_OP_JUMP_REL, operands[i].mode);
    }
  else if (format & MC9S12X_OP_IND16_P2)
    {
      fixup16 (&operands[1].exp, MC9S12X_OP_IND16, operands[1].mode);
    }
  if (format & MC9S12X_OP_PAGE)
    {
      fixup8 (&operands[i].exp, MC9S12X_OP_PAGE, operands[i].mode);
    }
}

/* Opcode identification and operand analysis.  */

/* find() gets a pointer to an entry in the opcode table.  It must look at all
   opcodes with the same name and use the operands to choose the correct
   opcode.  Returns the opcode pointer if there was a match and 0 if none.  */
static struct mc9s12x_opcode *
find (struct mc9s12x_opcode_def *opc, operand operands[], int nb_operands)
{
  int i, match, pos;
  struct mc9s12x_opcode *opcode;
  struct mc9s12x_opcode *op_indirect;

  op_indirect = 0;
  opcode = opc->opcode;

  /* Now search the opcode table table for one with operands
     that matches what we've got.  We're only done if the operands matched so
     far AND there are no more to check.  */
  for (pos = match = 0; match == 0 && pos < opc->nb_modes; pos++, opcode++)
    {
      int poss_indirect = 0;
      long format = opcode->format;
      int expect;

      expect = 0;
      if (opcode->format & MC9S12X_OP_MASK)
	expect++;
      if (opcode->format & MC9S12X_OP_BITMASK)
	expect++;
      if (opcode->format & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
	expect++;
      if (opcode->format & (MC9S12X_OP_IND16_P2 | MC9S12X_OP_IDX_P2))
	expect++;
      if ((opcode->format & MC9S12X_OP_PAGE)
          && (!IS_CALL_SYMBOL (opcode->format) || nb_operands == 2))
        expect++;

      for (i = 0; expect == nb_operands && i < nb_operands; i++)
	{
	  int mode = operands[i].mode;

	  if (mode & MC9S12X_OP_IMM16)
	    {
	      if (format &
		  (MC9S12X_OP_IMM8 | MC9S12X_OP_IMM16 | MC9S12X_OP_BITMASK))
		continue;
	      break;
	    }
	  if (mode == MC9S12X_OP_DIRECT)
	    {
	      if (format & MC9S12X_OP_DIRECT)
		continue;

	      /* If the operand is a page 0 operand, remember a
	         possible <abs-16> addressing mode.  We mark
	         this and continue to check other operands.  */
	      if (format & MC9S12X_OP_IND16
		  && flag_strict_direct_addressing && op_indirect == 0)
		{
		  poss_indirect = 1;
		  continue;
		}
	      break;
	    }
	  if (mode & MC9S12X_OP_IND16)
	    {
	      if (i == 0 && (format & MC9S12X_OP_IND16) != 0)
		continue;
              if (i != 0 && (format & MC9S12X_OP_PAGE) != 0)
                continue;
	      if (i != 0 && (format & MC9S12X_OP_IND16_P2) != 0)
		continue;
	      if (i == 0 && (format & MC9S12X_OP_BITMASK))
		break;
	    }
	  if (mode & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
	    {
	      if (format & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
		continue;
	    }
	  if (mode & MC9S12X_OP_REG)
	    {
	      if (i == 0
		  && (format & MC9S12X_OP_REG)
		  && (operands[i].reg2 == REG_NONE))
		continue;
	      if (i == 0
		  && (format & MC9S12X_OP_REG)
		  && (format & MC9S12X_OP_REG_2)
		  && (operands[i].reg2 != REG_NONE))
		continue;
	      if (i == 0
		  && (format & MC9S12X_OP_IDX)
		  && (operands[i].reg2 != REG_NONE))
		continue;
	      if (i == 0
		  && (format & MC9S12X_OP_IDX)
		  && (format & (MC9S12X_OP_IND16_P2 | MC9S12X_OP_IDX_P2)))
		continue;
	      if (i == 1
		  && (format & MC9S12X_OP_IDX_P2))
		continue;
	      break;
	    }
	  if (mode & MC9S12X_OP_IDX)
	    {
	      if (format & MC9S12X_OP_IX && operands[i].reg1 == REG_X)
		continue;
	      if (format & MC9S12X_OP_IY && operands[i].reg1 == REG_Y)
		continue;
	      if (i == 0
		  && format & (MC9S12X_OP_IDX | MC9S12X_OP_IDX_1 | MC9S12X_OP_IDX_2)
		  && (operands[i].reg1 == REG_X
		      || operands[i].reg1 == REG_Y
		      || operands[i].reg1 == REG_SP
		      || operands[i].reg1 == REG_PC))
		continue;
	      if (i == 1 && format & MC9S12X_OP_IDX_P2)
		continue;
	    }
          if (mode & format & (MC9S12X_OP_D_IDX | MC9S12X_OP_D_IDX_2))
            {
              if (i == 0)
                continue;
            }
	  if (mode & MC9S12X_AUTO_INC_DEC)
	    {
	      if (i == 0
		  && format & (MC9S12X_OP_IDX | MC9S12X_OP_IDX_1 |
			       MC9S12X_OP_IDX_2))
		continue;
	      if (i == 1 && format & MC9S12X_OP_IDX_P2)
		continue;
	    }
	  break;
	}
      match = i == nb_operands;

      /* Operands are ok but an operand uses page 0 addressing mode
         while the insn supports abs-16 mode.  Keep a reference to this
         insns in case there is no insn supporting page 0 addressing.  */
      if (match && poss_indirect)
	{
	  op_indirect = opcode;
	  match = 0;
	}
      if (match)
	break;
    }

  /* Page 0 addressing is used but not supported by any insn.
     If absolute addresses are supported, we use that insn.  */
  if (match == 0 && op_indirect)
    {
      opcode = op_indirect;
      match = 1;
    }

  if (!match)
    {
      return (0);
    }

  return opcode;
}

/* Find the real opcode and its associated operands.  We use a progressive
   approach here.  On entry, 'opc' points to the first opcode in the
   table that matches the opcode name in the source line.  We try to
   isolate an operand, find a possible match in the opcode table.
   We isolate another operand if no match were found.  The table 'operands'
   is filled while operands are recognized.

   Returns the opcode pointer that matches the opcode name in the
   source line and the associated operands.  */
static struct mc9s12x_opcode *
find_opcode (struct mc9s12x_opcode_def *opc, operand operands[],
             int *nb_operands)
{
  struct mc9s12x_opcode *opcode;
  int i;

  if (opc->max_operands == 0)
    {
      *nb_operands = 0;
      return opc->opcode;
    }

  for (i = 0; i < opc->max_operands;)
    {
      int result;

      result = get_operand (&operands[i], i, opc->format);
   //   printf("\n result of get_operands is %d",result);
      if (result <= 0)
	return 0;

      /* Special case where the bitmask of the bclr/brclr
         instructions is not introduced by #.
         Example: bclr 3,x $80.  */
      if (i == 1 && (opc->format & MC9S12X_OP_BITMASK)
	  && (operands[i].mode & MC9S12X_OP_IND16))
	{
	  operands[i].mode = MC9S12X_OP_IMM16;
	}

      i += result;
      *nb_operands = i;
      if (i >= opc->min_operands)
	{
	  opcode = find (opc, operands, i);

          /* Another special case for 'call foo,page' instructions.
             Since we support 'call foo' and 'call foo,page' we must look
             if the optional page specification is present otherwise we will
             assemble immediately and treat the page spec as garbage.  */
          if (opcode && !(opcode->format & MC9S12X_OP_PAGE))
             return opcode;

	  if (opcode && *input_line_pointer != ',')
	    return opcode;
	}

      if (*input_line_pointer == ',')
	input_line_pointer++;
    }

  return 0;
}

#define MC9S12X_XBCC_MARKER (MC9S12X_OP_TBCC_MARKER \
                           | MC9S12X_OP_DBCC_MARKER \
                           | MC9S12X_OP_IBCC_MARKER)

/* Gas line assembler entry point.  */

/* This is the main entry point for the machine-dependent assembler.  str
   points to a machine-dependent instruction.  This function is supposed to
   emit the frags/bytes it assembles to.  */
void
md_assemble (char *str)
{

  struct mc9s12x_opcode_def *opc;
  struct mc9s12x_opcode *opcode;

  unsigned char *op_start, *op_end;
  char *save;
  char name[20];
  int nlen = 0;
  operand operands[MC9S12X_MAX_OPERANDS];
  int nb_operands = 0;
  int branch_optimize = 0;
  int alias_id = -1;

  /* Drop leading whitespace.  */
  while (*str == ' ')
    str++;

  /* Find the opcode end and get the opcode in 'name'.  The opcode is forced
     lower case (the opcode table only has lower case op-codes).  */
  for (op_start = op_end = (unsigned char *) str;
       *op_end && nlen < 20 && !is_end_of_line[*op_end] && *op_end != ' ';
       op_end++)
    {
      name[nlen] = TOLOWER (op_start[nlen]);
      nlen++;
    }
  name[nlen] = 0;

  if (nlen == 0)
    {
      as_bad (_("No instruction or missing opcode."));
      return;
    }

  /* Find the opcode definition given its name.  */
  opc = (struct mc9s12x_opcode_def *) hash_find (mc9s12x_hash, name);

  /* If it's not recognized, look for 'jbsr' and 'jbxx'.  These are
     pseudo insns for relative branch.  For these branches, we always
     optimize them (turned into absolute branches) even if --short-branches
     is given.  */
  if (opc == NULL && name[0] == 'j' && name[1] == 'b')
    {
      opc = (struct mc9s12x_opcode_def *) hash_find (mc9s12x_hash, &name[1]);
      if (opc
	  && (!(opc->format & MC9S12X_OP_JUMP_REL)
	      || (opc->format & MC9S12X_OP_BITMASK)))
	opc = 0;
      if (opc)
	branch_optimize = 1;
    }

 // printf("\n found %s", opc->opcode->name);

  /* The following test should probably be removed.  This is not conform
     to Motorola assembler specs.  */
  if (opc == NULL && flag_mri)
    {
      if (*op_end == ' ' || *op_end == '\t')
	{
	  while (*op_end == ' ' || *op_end == '\t')
	    op_end++;

	  if (nlen < 19
	      && (*op_end &&
		  (is_end_of_line[op_end[1]]
		   || op_end[1] == ' ' || op_end[1] == '\t'
		   || !ISALNUM (op_end[1])))
	      && (*op_end == 'a' || *op_end == 'b'
		  || *op_end == 'A' || *op_end == 'B'
		  || *op_end == 'd' || *op_end == 'D'
		  || *op_end == 'x' || *op_end == 'X'
		  || *op_end == 'y' || *op_end == 'Y'))
	    {
	      name[nlen++] = TOLOWER (*op_end++);
	      name[nlen] = 0;
	      opc = (struct mc9s12x_opcode_def *) hash_find (mc9s12x_hash,
							     name);
	    }
	}
    }

  /* Identify a possible instruction alias.  There are some on the
     68HC12 to emulate a few 68HC11 instructions.  */
  /* TODO make sure this always reads true */
  if (opc == NULL && (current_architecture & cpumc9s12x))
    {
      int i;

      for (i = 0; i < mc9s12x_num_alias; i++)
	if (strcmp (mc9s12x_alias[i].name, name) == 0)
	  {
	    alias_id = i;
	    break;
	  }
    }
  if (opc == NULL && alias_id < 0)
    {
      as_bad (_("Opcode `%s' is not recognized."), name);
      return;
    }
  save = input_line_pointer;
  input_line_pointer = (char *) op_end;

  if (opc)
    {
      opc->used++;
      opcode = find_opcode (opc, operands, &nb_operands);
    }
  else
    opcode = 0;

  if ((opcode || alias_id >= 0) && !flag_mri)
    {
      char *p = input_line_pointer;

      while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
	p++;

      if (*p != '\n' && *p)
	as_bad (_("Garbage at end of instruction: `%s'."), p);
    }

  input_line_pointer = save;

  if (alias_id >= 0)
    {
      char *f = mc9s12x_new_insn (mc9s12x_alias[alias_id].size);

      number_to_chars_bigendian (f, mc9s12x_alias[alias_id].code1, 1);
      if (mc9s12x_alias[alias_id].size > 1)
	number_to_chars_bigendian (f + 1, mc9s12x_alias[alias_id].code2, 1);

      return;
    }

  /* Opcode is known but does not have valid operands.  Print out the
     syntax for this opcode.  */
  if (opcode == 0)
    {
      if (flag_print_insn_syntax)
	print_insn_format (name);

      as_bad (_("Invalid operand for `%s'"), name);
      return;
    }

  /* Treat dbeq/ibeq/tbeq instructions in a special way.  The branch is
     relative and must be in the range -256..255 (9-bits).  */
  if ((opcode->format & MC9S12X_XBCC_MARKER)
      && (opcode->format & MC9S12X_OP_JUMP_REL))
    build_dbranch_insn (opcode, operands, nb_operands, branch_optimize);

  /* Relative jumps instructions are taken care of separately.  We have to make
     sure that the relative branch is within the range -128..127.  If it's out
     of range, the instructions are changed into absolute instructions.
     This is not supported for the brset and brclr instructions.  */
  else if ((opcode->format & (MC9S12X_OP_JUMP_REL | MC9S12X_OP_JUMP_REL16))
	   && !(opcode->format & MC9S12X_OP_BITMASK))
    build_jump_insn (opcode, operands, nb_operands, branch_optimize);
  else
    build_insn (opcode, operands, nb_operands);
}


/* Pseudo op to control the ELF flags.  */
static void
s_mc9s12x_mode (int x ATTRIBUTE_UNUSED)
{
  char *name = input_line_pointer, ch;

  while (!is_end_of_line[(unsigned char) *input_line_pointer])
    input_line_pointer++;
  ch = *input_line_pointer;
  *input_line_pointer = '\0';

  if (strcmp (name, "mshort") == 0)
    {
      elf_flags &= ~E_MC9S12X_I32;
    }
  else if (strcmp (name, "mlong") == 0)
    {
      elf_flags |= E_MC9S12X_I32;
    }
  else if (strcmp (name, "mshort-double") == 0)
    {
      elf_flags &= ~E_MC9S12X_F64;
    }
  else if (strcmp (name, "mlong-double") == 0)
    {
      elf_flags |= E_MC9S12X_F64;
    }
  else
    {
      as_warn (_("Invalid mode: %s\n"), name);
    }
  *input_line_pointer = ch;
  demand_empty_rest_of_line ();
}

/* Mark the symbols with STO_M68HC12_FAR to indicate the functions
   are using 'rtc' for returning.  It is necessary to use 'call'
   to invoke them.  This is also used by the debugger to correctly
   find the stack frame.  */
static void
s_mc9s12x_mark_symbol (int mark)
{
  char *name;
  int c;
  symbolS *symbolP;
  asymbol *bfdsym;
  elf_symbol_type *elfsym;

  do
    {
      name = input_line_pointer;
      c = get_symbol_end ();
      symbolP = symbol_find_or_make (name);
      *input_line_pointer = c;

      SKIP_WHITESPACE ();

      bfdsym = symbol_get_bfdsym (symbolP);
      elfsym = elf_symbol_from (bfd_asymbol_bfd (bfdsym), bfdsym);

      gas_assert (elfsym);

      /* Mark the symbol far (using rtc for function return).  */
      elfsym->internal_elf_sym.st_other |= mark;

      if (c == ',')
	{
	  input_line_pointer ++;

	  SKIP_WHITESPACE ();

	  if (*input_line_pointer == '\n')
	    c = '\n';
	}
    }
  while (c == ',');

  demand_empty_rest_of_line ();
}

static void
s_mc9s12x_relax (int ignore ATTRIBUTE_UNUSED)
{
  expressionS ex;

  expression (&ex);

  if (ex.X_op != O_symbol || ex.X_add_number != 0)
    {
      as_bad (_("bad .relax format"));
      ignore_rest_of_line ();
      return;
    }

  fix_new_exp (frag_now, frag_now_fix (), 0, &ex, 1,
               BFD_RELOC_MC9S12X_RL_GROUP);

  demand_empty_rest_of_line ();
}


/* Relocation, relaxation and frag conversions.  */

/* PC-relative offsets are relative to the start of the
   next instruction.  That is, the address of the offset, plus its
   size, since the offset is always the last part of the insn.  */
long
md_pcrel_from (fixS *fixP)
{
  if (fixP->fx_r_type == BFD_RELOC_MC9S12X_RL_JUMP)
    return 0;

  return fixP->fx_size + fixP->fx_where + fixP->fx_frag->fr_address;
}

/* If while processing a fixup, a reloc really needs to be created
   then it is done here.  */
arelent *
tc_gen_reloc (asection *section ATTRIBUTE_UNUSED, fixS *fixp)
{
  arelent *reloc;

  reloc = (arelent *) xmalloc (sizeof (arelent));
  reloc->sym_ptr_ptr = (asymbol **) xmalloc (sizeof (asymbol *));
  *reloc->sym_ptr_ptr = symbol_get_bfdsym (fixp->fx_addsy);
  reloc->address = fixp->fx_frag->fr_address + fixp->fx_where;
  if (fixp->fx_r_type == 0)
    reloc->howto = bfd_reloc_type_lookup (stdoutput, BFD_RELOC_16);
  else
    reloc->howto = bfd_reloc_type_lookup (stdoutput, fixp->fx_r_type);
  if (reloc->howto == (reloc_howto_type *) NULL)
    {
      as_bad_where (fixp->fx_file, fixp->fx_line,
		    _("Relocation %d is not supported by object file format."),
		    (int) fixp->fx_r_type);
      return NULL;
    }

  /* Since we use Rel instead of Rela, encode the vtable entry to be
     used in the relocation's section offset.  */
  if (fixp->fx_r_type == BFD_RELOC_VTABLE_ENTRY)
    reloc->address = fixp->fx_offset;

  reloc->addend = 0;
 // printf("\n about to return relocation-%d name-%s",(int) reloc, reloc->howto->name);
  return reloc;
}

/* We need a port-specific relaxation function to cope with sym2 - sym1
   relative expressions with both symbols in the same segment (but not
   necessarily in the same frag as this insn), for example:
     ldab sym2-(sym1-2),pc
    sym1:
   The offset can be 5, 9 or 16 bits long.  */

long
mc9s12x_relax_frag (segT seg ATTRIBUTE_UNUSED, fragS *fragP,
                    long stretch ATTRIBUTE_UNUSED)
{
  long growth;
  offsetT aim = 0;
  symbolS *symbolP;
  const relax_typeS *this_type;
  const relax_typeS *start_type;
  relax_substateT next_state;
  relax_substateT this_state;
  const relax_typeS *table = TC_GENERIC_RELAX_TABLE;

  /* We only have to cope with frags as prepared by
     md_estimate_size_before_relax.  The STATE_BITS16 case may geet here
     because of the different reasons that it's not relaxable.  */
  switch (fragP->fr_subtype)
    {
    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS16):
    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS16):
      /* When we get to this state, the frag won't grow any more.  */
      return 0;

    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS5):
    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS5):
    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS9):
    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS9):
      if (fragP->fr_symbol == NULL
	  || S_GET_SEGMENT (fragP->fr_symbol) != absolute_section)
	as_fatal (_("internal inconsistency problem in %s: fr_symbol %lx"),
		  __FUNCTION__, (long) fragP->fr_symbol);
      symbolP = fragP->fr_symbol;
      if (symbol_resolved_p (symbolP))
	as_fatal (_("internal inconsistency problem in %s: resolved symbol"),
		  __FUNCTION__);
      aim = S_GET_VALUE (symbolP);
      break;

    default:
      as_fatal (_("internal inconsistency problem in %s: fr_subtype %d"),
		  __FUNCTION__, fragP->fr_subtype);
    }

  /* The rest is stolen from relax_frag.  There's no obvious way to
     share the code, but fortunately no requirement to keep in sync as
     long as fragP->fr_symbol does not have its segment changed.  */

  this_state = fragP->fr_subtype;
  start_type = this_type = table + this_state;

  if (aim < 0)
    {
      /* Look backwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim >= this_type->rlx_backward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }
  else
    {
      /* Look forwards.  */
      for (next_state = this_type->rlx_more; next_state;)
	if (aim <= this_type->rlx_forward)
	  next_state = 0;
	else
	  {
	    /* Grow to next state.  */
	    this_state = next_state;
	    this_type = table + this_state;
	    next_state = this_type->rlx_more;
	  }
    }

  growth = this_type->rlx_length - start_type->rlx_length;
  if (growth != 0)
    fragP->fr_subtype = this_state;
  return growth;
}

void
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED,
                 fragS *fragP)
{
	//printf("\n in md_convert_frag");
  fixS *fixp;
  long value;
  long disp;
  char *buffer_address = fragP->fr_literal;

  /* Address in object code of the displacement.  */
  register int object_address = fragP->fr_fix + fragP->fr_address;

  buffer_address += fragP->fr_fix;

  /* The displacement of the address, from current location.  */
  value = S_GET_VALUE (fragP->fr_symbol);
  disp = (value + fragP->fr_offset) - object_address;

  switch (fragP->fr_subtype)
    {
    case ENCODE_RELAX (STATE_PC_RELATIVE, STATE_BYTE):
      fragP->fr_opcode[1] = disp;
      break;

    case ENCODE_RELAX (STATE_PC_RELATIVE, STATE_WORD):
      /* This relax is only for bsr and bra.  */
      gas_assert (IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR)
	      || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BRA)
	      || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR2));

      fragP->fr_opcode[0] = convert_branch (fragP->fr_opcode[0]);

      fix_new (fragP, fragP->fr_fix - 1, 2,
	       fragP->fr_symbol, fragP->fr_offset, 0, BFD_RELOC_16);
      fragP->fr_fix += 1;
      break;

    case ENCODE_RELAX (STATE_CONDITIONAL_BRANCH, STATE_BYTE):
    case ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812, STATE_BYTE):
      fragP->fr_opcode[1] = disp;
      break;

    case ENCODE_RELAX (STATE_CONDITIONAL_BRANCH, STATE_WORD):
      /* Invert branch.  */
      fragP->fr_opcode[0] ^= 1;
      fragP->fr_opcode[1] = 3;	/* Branch offset.  */
      buffer_address[0] = MC9S12X_JMP;
      fix_new (fragP, fragP->fr_fix + 1, 2,
	       fragP->fr_symbol, fragP->fr_offset, 0, BFD_RELOC_16);
      fragP->fr_fix += 3;
      break;

    case ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812, STATE_WORD):
      /* Translate branch into a long branch.  */
      fragP->fr_opcode[1] = fragP->fr_opcode[0];
      fragP->fr_opcode[0] = MC9S12X_OPCODE_PAGE2;

      fixp = fix_new (fragP, fragP->fr_fix, 2,
		      fragP->fr_symbol, fragP->fr_offset, 1,
		      BFD_RELOC_16_PCREL);
      fixp->fx_pcrel_adjust = 2;
      fragP->fr_fix += 2;
      break;

    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS5):
      if (fragP->fr_symbol != 0
          && S_GET_SEGMENT (fragP->fr_symbol) != absolute_section)
        value = disp;
      /* fall through  */

    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS5):
      fragP->fr_opcode[0] = fragP->fr_opcode[0] << 6;
      fragP->fr_opcode[0] |= value & 0x1f;
      break;

    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS9):
      /* For a PC-relative offset, use the displacement with a -1 correction
         to take into account the additional byte of the insn.  */
      if (fragP->fr_symbol != 0
          && S_GET_SEGMENT (fragP->fr_symbol) != absolute_section)
        value = disp - 1;
      /* fall through  */

    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS9):
      fragP->fr_opcode[0] = (fragP->fr_opcode[0] << 3);
      fragP->fr_opcode[0] |= 0xE0;
      fragP->fr_opcode[0] |= (value >> 8) & 1;
      fragP->fr_opcode[1] = value;
      fragP->fr_fix += 1;
      break;

    case ENCODE_RELAX (STATE_INDEXED_PCREL, STATE_BITS16):
    case ENCODE_RELAX (STATE_INDEXED_OFFSET, STATE_BITS16):
      fragP->fr_opcode[0] = (fragP->fr_opcode[0] << 3);
      fragP->fr_opcode[0] |= 0xe2;
      if ((fragP->fr_opcode[0] & 0x0ff) == 0x0fa
          && fragP->fr_symbol != 0
          && S_GET_SEGMENT (fragP->fr_symbol) != absolute_section)
	{
	  fixp = fix_new (fragP, fragP->fr_fix, 2,
			  fragP->fr_symbol, fragP->fr_offset,
			  1, BFD_RELOC_16_PCREL);
	}
      else
	{
	  fix_new (fragP, fragP->fr_fix, 2,
		   fragP->fr_symbol, fragP->fr_offset, 0, BFD_RELOC_16);
	}
      fragP->fr_fix += 2;
      break;

    case ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_BYTE):
      if (disp < 0)
	fragP->fr_opcode[0] |= 0x10;

      fragP->fr_opcode[1] = disp & 0x0FF;
      break;

    case ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_WORD):
      /* Invert branch.  */
      fragP->fr_opcode[0] ^= 0x20;
      fragP->fr_opcode[1] = 3;	/* Branch offset.  */
      buffer_address[0] = MC9S12X_JMP2;
      fix_new (fragP, fragP->fr_fix + 1, 2,
	       fragP->fr_symbol, fragP->fr_offset, 0, BFD_RELOC_16);
      fragP->fr_fix += 3;
      break;

    default:
      break;
    }
}

/* On an ELF system, we can't relax a weak symbol.  The weak symbol
   can be overridden at final link time by a non weak symbol.  We can
   relax externally visible symbol because there is no shared library
   and such symbol can't be overridden (unless they are weak).  */
static int
relaxable_symbol (symbolS *symbol)
{
  return ! S_IS_WEAK (symbol);
}

/* Force truly undefined symbols to their maximum size, and generally set up
   the frag list to be relaxed.  */
int
md_estimate_size_before_relax (fragS *fragP, asection *segment)
{
  if (RELAX_LENGTH (fragP->fr_subtype) == STATE_UNDF)
    {
      if (S_GET_SEGMENT (fragP->fr_symbol) != segment
	  || !relaxable_symbol (fragP->fr_symbol)
          || (segment != absolute_section
              && RELAX_STATE (fragP->fr_subtype) == STATE_INDEXED_OFFSET))
	{
	  /* Non-relaxable cases.  */
	  int old_fr_fix;
	  char *buffer_address;

	  old_fr_fix = fragP->fr_fix;
	  buffer_address = fragP->fr_fix + fragP->fr_literal;

	  switch (RELAX_STATE (fragP->fr_subtype))
	    {
	    case STATE_PC_RELATIVE:

	      /* This relax is only for bsr and bra.  */
	      gas_assert (IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR)
		      || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BRA)
		      || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR2));

	      if (flag_fixed_branches)
		as_bad_where (fragP->fr_file, fragP->fr_line,
			      _("bra or bsr with undefined symbol."));

	      /* The symbol is undefined or in a separate section.
		 Turn bra into a jmp and bsr into a jsr.  The insn
		 becomes 3 bytes long (instead of 2).  A fixup is
		 necessary for the unresolved symbol address.  */
	      fragP->fr_opcode[0] = convert_branch (fragP->fr_opcode[0]);

	      fix_new (fragP, fragP->fr_fix - 1, 2, fragP->fr_symbol,
		       fragP->fr_offset, 0, BFD_RELOC_16);
	      fragP->fr_fix++;
	      break;

	    case STATE_CONDITIONAL_BRANCH:
	      gas_assert (current_architecture & cpumc9s12x);

	      fragP->fr_opcode[0] ^= 1;	/* Reverse sense of branch.  */
	      fragP->fr_opcode[1] = 3;	/* Skip next jmp insn (3 bytes).  */

	      /* Don't use fr_opcode[2] because this may be
		 in a different frag.  */
	      buffer_address[0] = MC9S12X_JMP;

	      fragP->fr_fix++;
	      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
		       fragP->fr_offset, 0, BFD_RELOC_16);
	      fragP->fr_fix += 2;
	      break;

	    case STATE_INDEXED_OFFSET:
	      gas_assert (current_architecture & cpumc9s12x);

              if (fragP->fr_symbol
                  && S_GET_SEGMENT (fragP->fr_symbol) == absolute_section)
                {
                   fragP->fr_subtype = ENCODE_RELAX (STATE_INDEXED_OFFSET,
                                                     STATE_BITS5);
                   /* Return the size of the variable part of the frag. */
                   return md_relax_table[fragP->fr_subtype].rlx_length;
                }
              else
                {
                   /* Switch the indexed operation to 16-bit mode.  */
                   fragP->fr_opcode[0] = fragP->fr_opcode[0] << 3;
                   fragP->fr_opcode[0] |= 0xe2;
                   fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
                            fragP->fr_offset, 0, BFD_RELOC_16);
                   fragP->fr_fix += 2;
                }
	      break;

	    case STATE_INDEXED_PCREL:
	      gas_assert (current_architecture & cpumc9s12x);

              if (fragP->fr_symbol
                  && S_GET_SEGMENT (fragP->fr_symbol) == absolute_section)
                {
                   fragP->fr_subtype = ENCODE_RELAX (STATE_INDEXED_PCREL,
                                                     STATE_BITS5);
                   /* Return the size of the variable part of the frag. */
                   return md_relax_table[fragP->fr_subtype].rlx_length;
                }
              else
                {
                   fixS* fixp;

                   fragP->fr_opcode[0] = fragP->fr_opcode[0] << 3;
                   fragP->fr_opcode[0] |= 0xe2;
                   fixp = fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
                                   fragP->fr_offset, 1, BFD_RELOC_16_PCREL);
                   fragP->fr_fix += 2;
                }
	      break;

	    case STATE_XBCC_BRANCH:
	      gas_assert (current_architecture & cpumc9s12x);

	      fragP->fr_opcode[0] ^= 0x20;	/* Reverse sense of branch.  */
	      fragP->fr_opcode[1] = 3;	/* Skip next jmp insn (3 bytes).  */

	      /* Don't use fr_opcode[2] because this may be
		 in a different frag.  */
	      buffer_address[0] = MC9S12X_JMP2;

	      fragP->fr_fix++;
	      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
		       fragP->fr_offset, 0, BFD_RELOC_16);
	      fragP->fr_fix += 2;
	      break;

	    case STATE_CONDITIONAL_BRANCH_6812:
	      gas_assert (current_architecture & cpumc9s12x);

	      /* Translate into a lbcc branch.  */
	      fragP->fr_opcode[1] = fragP->fr_opcode[0];
	      fragP->fr_opcode[0] = MC9S12X_OPCODE_PAGE2;

	      fix_new (fragP, fragP->fr_fix, 2, fragP->fr_symbol,
                       fragP->fr_offset, 1, BFD_RELOC_16_PCREL);
	      fragP->fr_fix += 2;
	      break;

	    default:
	      as_fatal (_("Subtype %d is not recognized."), fragP->fr_subtype);
	    }
	  frag_wane (fragP);

	  /* Return the growth in the fixed part of the frag.  */
	  return fragP->fr_fix - old_fr_fix;
	}

      /* Relaxable cases.  */
      switch (RELAX_STATE (fragP->fr_subtype))
	{
	case STATE_PC_RELATIVE:
	  /* This relax is only for bsr and bra.  */
	  gas_assert (IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR)
		  || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BRA)
		  || IS_OPCODE (fragP->fr_opcode[0], MC9S12X_BSR2));

	  fragP->fr_subtype = ENCODE_RELAX (STATE_PC_RELATIVE, STATE_BYTE);
	  break;

	case STATE_CONDITIONAL_BRANCH:
	  gas_assert (current_architecture & cpumc9s12x);

	  fragP->fr_subtype = ENCODE_RELAX (STATE_CONDITIONAL_BRANCH,
					    STATE_BYTE);
	  break;

	case STATE_INDEXED_OFFSET:
	  gas_assert (current_architecture & cpumc9s12x);

	  fragP->fr_subtype = ENCODE_RELAX (STATE_INDEXED_OFFSET,
					    STATE_BITS5);
	  break;

	case STATE_INDEXED_PCREL:
	  gas_assert (current_architecture & cpumc9s12x);

	  fragP->fr_subtype = ENCODE_RELAX (STATE_INDEXED_PCREL,
					    STATE_BITS5);
	  break;

	case STATE_XBCC_BRANCH:
	  gas_assert (current_architecture & cpumc9s12x);

	  fragP->fr_subtype = ENCODE_RELAX (STATE_XBCC_BRANCH, STATE_BYTE);
	  break;

	case STATE_CONDITIONAL_BRANCH_6812:
	  gas_assert (current_architecture & cpumc9s12x);

	  fragP->fr_subtype = ENCODE_RELAX (STATE_CONDITIONAL_BRANCH_6812,
					    STATE_BYTE);
	  break;
	}
    }

  if (fragP->fr_subtype >= sizeof (md_relax_table) / sizeof (md_relax_table[0]))
    as_fatal (_("Subtype %d is not recognized."), fragP->fr_subtype);

  /* Return the size of the variable part of the frag.  */
  return md_relax_table[fragP->fr_subtype].rlx_length;
}

/* See whether we need to force a relocation into the output file.  */
int
tc_mc9s12x_force_relocation (fixS *fixP)
{
  if (fixP->fx_r_type == BFD_RELOC_MC9S12X_RL_GROUP)
    return 1;

  return generic_force_reloc (fixP);
}

/* Here we decide which fixups can be adjusted to make them relative
   to the beginning of the section instead of the symbol.  Basically
   we need to make sure that the linker relaxation is done
   correctly, so in some cases we force the original symbol to be
   used.  */
int
tc_mc9s12x_fix_adjustable (fixS *fixP)
{
  switch (fixP->fx_r_type)
    {
      /* For the linker relaxation to work correctly, these relocs
         need to be on the symbol itself.  */
    case BFD_RELOC_16:
    case BFD_RELOC_MC9S12X_RL_JUMP:
    case BFD_RELOC_MC9S12X_RL_GROUP:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
    case BFD_RELOC_32:

      /* The memory bank addressing translation also needs the original
         symbol.  */
    case BFD_RELOC_MC9S12X_LO16:
    case BFD_RELOC_MC9S12X_PAGE:
    case BFD_RELOC_MC9S12X_24:
      return 0;

    default:
      return 1;
    }
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
  char *where;
  long value = * valP;
  int op_type;

  if (fixP->fx_addsy == (symbolS *) NULL)
    fixP->fx_done = 1;

  /* We don't actually support subtracting a symbol.  */
  if (fixP->fx_subsy != (symbolS *) NULL)
    as_bad_where (fixP->fx_file, fixP->fx_line, _("Expression too complex."));

  op_type = fixP->fx_r_type;

  /* Patch the instruction with the resolved operand.  Elf relocation
     info will also be generated to take care of linker/loader fixups.
     The 68HC11 addresses only 64Kb, we are only concerned by 8 and 16-bit
     relocs.  BFD_RELOC_8 is basically used for .page0 access (the linker
     will warn for overflows).  BFD_RELOC_8_PCREL should not be generated
     because it's either resolved or turned out into non-relative insns (see
     relax table, bcc, bra, bsr transformations)

     The BFD_RELOC_32 is necessary for the support of --gstabs.  */
  where = fixP->fx_frag->fr_literal + fixP->fx_where;

  switch (fixP->fx_r_type)
    {
    case BFD_RELOC_32:
      bfd_putb32 ((bfd_vma) value, (unsigned char *) where);
      break;

    case BFD_RELOC_24:
    case BFD_RELOC_MC9S12X_24:
      bfd_putb16 ((bfd_vma) (value & 0x0ffff), (unsigned char *) where);
      ((bfd_byte*) where)[2] = ((value >> 16) & 0x0ff);
      break;

    case BFD_RELOC_16:
    case BFD_RELOC_16_PCREL:
    case BFD_RELOC_MC9S12X_LO16:
      bfd_putb16 ((bfd_vma) value, (unsigned char *) where);
      if (value < -65537 || value > 65535)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Value out of 16-bit range."));
      break;

    case BFD_RELOC_MC9S12X_HI8:
      value = value >> 8;
      /* Fall through.  */

    case BFD_RELOC_MC9S12X_LO8:
    case BFD_RELOC_8:
    case BFD_RELOC_MC9S12X_PAGE:
      ((bfd_byte *) where)[0] = (bfd_byte) value;
      break;

    case BFD_RELOC_8_PCREL:
      ((bfd_byte *) where)[0] = (bfd_byte) value;

      if (value < -128 || value > 127)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Value %ld too large for 8-bit PC-relative branch."),
		      value);
      break;

    case BFD_RELOC_MC9S12X_3B:
      if (value <= 0 || value > 8)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Auto increment/decrement offset '%ld' is out of range."),
		      value);
      if (where[0] & 0x8)
	value = 8 - value;
      else
	value--;

      where[0] = where[0] | (value & 0x07);
      break;

    case BFD_RELOC_MC9S12X_5B:
      if (value < -16 || value > 15)
	as_bad_where (fixP->fx_file, fixP->fx_line,
		      _("Offset out of 5-bit range for movw/movb insn: %ld"),
		      value);
      if (value >= 0)
	where[0] |= value;
      else 
	where[0] |= (0x10 | (16 + value));
      break;

    case BFD_RELOC_MC9S12X_RL_JUMP:
    case BFD_RELOC_MC9S12X_RL_GROUP:
    case BFD_RELOC_VTABLE_INHERIT:
    case BFD_RELOC_VTABLE_ENTRY:
      fixP->fx_done = 0;
      return;

    default:
      as_fatal (_("Line %d: unknown relocation type: 0x%x."),
		fixP->fx_line, fixP->fx_r_type);
    }
}

/* Set the ELF specific flags.  */
void
mc9s12x_elf_final_processing (void)
{
	/* TODO make this always true */
  if (current_architecture & cpumc9s12x)
    elf_flags |= EF_MC9S12X_MACH;
  elf_elfheader (stdoutput)->e_flags &= ~EF_MC9S12X_ABI;
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}
