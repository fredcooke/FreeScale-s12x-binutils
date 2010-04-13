/* tc-m68hc11.c -- Assembler code for the Motorola 68HC11 & 68HC12.
   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009
   Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)
*/
/*
Fatal error: selected target format
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

/*  LOCAL FUNCTIONS */
static void s_mc9xgate_mode (int); /* Pseudo op to control the ELF flags.  */
static void get_default_target (void); /* Pseudo op to indicate a relax group.  */
static void s_mc9xgate_relax (int);
static void s_mc9xgate_mode (int);
static void s_mc9xgate_mark_symbol (int);/* Pseudo op to control the ELF flags.  */
/* Mark the symbols with STO_M68HC12_FAR to indicate the functions
   are using 'rtc' for returning.  It is necessary to use 'call'
   to invoke them.  This is also used by the debugger to correctly
   find the stack frame.  */
/* END LOCAL FUNCTIONS */

/* LOCAL DATA */
static struct hash_control *mc9xgate_hash;

struct mc9xgate_opcode_def {
  long format;
  int min_operands;
  int max_operands;
  int nb_modes;
  int used;
  struct mc9xgate_opcode *opcode;
};

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

/* mc9Xgate registers all are 16-bit.  They are numbered according to the mc9xgate.  */
typedef enum register_id {
  REG_NONE = -1,
  REG_A = 0,  /* is tied to the value “$0000”
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
const pseudo_typeS md_pseudo_table[] = {
  /* The following pseudo-ops are supported for MRI compatibility.  */
  {"fcb", cons, 1},
  {"fdb", cons, 2},
  {"fcc", stringer, 8 + 1},
  {"rmb", s_space, 0},

  /* Motorola ALIS.  */
  {"xrefb", s_ignore, 0}, /* Same as xref  */

  /* Gcc driven relaxation.  */
  {"relax", s_mc9xgate_relax, 0},

  /* .mode instruction (ala SH).  */
  {"mode", s_mc9xgate_mode, 0},

  /* .far instruction.  */
  {"far", s_mc9xgate_mark_symbol, STO_MC9XGATE_FAR},

  /* .interrupt instruction.  */
  {"interrupt", s_mc9xgate_mark_symbol, STO_MC9XGATE_INTERRUPT},

  {0, 0, 0}
};

/* statics */
static int current_architecture;
static int mc9xgate_nb_opcode_defs = 0;
static const char *default_cpu;
static struct hash_control *mc9xgate_hash;
static struct mc9xgate_opcode_def *mc9xgate_opcode_defs = 0;
/* ELF flags to set in the output file header.  */
static int elf_flags = E_MC9XGATE_F64;



/* Declaired to keep the current make happy with alot of baggage from the old port  TODO clean up*/
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

size_t md_longopts_size = sizeof(md_longopts);

char *
md_atof (int type, char *litP, int *sizeP)
{
  return ieee_md_atof (type, litP, sizeP, TRUE);
}

int
md_parse_option (int c, char *arg){
	c = 0; //for testing
	*arg =  't'; //for testing
	return 1;
}

const char *
mc9xgate_arch_format(void)
{
	get_default_target();
	if (current_architecture & cpumc9xgate){
		return "elf32-mc9xgate";
	}else {
		printf("\n ERROR in mc9xgate_arch_format ---unknown----\n");  //for debugging
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
    if (strcmp (target->name, "elf32-mc9xgate") == 0)
	{
      current_architecture = cpumc9xgate;
	  default_cpu = "mc9xgate";
	  printf("\n found default cpu->%s\n",target->name);
	  return;
	}
      else
	{
	  as_bad (_("Default target `%s' is not supported."), target->name);
	}
    }
}

/* end baggage */

void md_begin(void){
	struct mc9xgate_opcode *mc9xgate_opcode_ptr;
	mc9xgate_hash = hash_new(); /* create a new has control table */

	/* TODO create a copy of the opcode table, this prevents damaging the origional accidently
		for now remove const from our "const mc9xgate_opcodes" pointer*/
	for(mc9xgate_opcode_ptr = mc9xgate_opcodes; mc9xgate_opcode_ptr->name; mc9xgate_opcode_ptr++){
		//printf("\n inserted %s\n",mc9xgate_opcode_prt->)
		hash_insert(mc9xgate_hash,mc9xgate_opcode_ptr->name, (char *) mc9xgate_opcode_ptr->bin_opcode);
	}
	return;
}

void
mc9xgate_init_after_args(void){
//return; // no return needed?????
}

void md_show_usage(FILE *stream){
	get_default_target();
	fprintf(stream, _("\
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
			                          (used for testing)\n"), default_cpu);
}

enum bfd_architecture
mc9xgate_arch (void)
{
  get_default_target ();
  if (current_architecture & cpumc9xgate) /* TODO same as above todo */
    return bfd_arch_mc9xgate;
  else
	printf("\n error in tc-mc9s12x mc9xgate_arch \n");
    return bfd_arch_mc9s12x;
}

int
mc9xgate_mach (void)
{
  return 0;
}

void
mc9xgate_print_statistics (FILE *file)
{
  int i;
  struct mc9xgate_opcode_def *opc;

  hash_print_statistics (file, "opcode table", mc9xgate_hash);

  opc = mc9xgate_opcode_defs;
  if (opc == 0 || mc9xgate_nb_opcode_defs == 0)
    return;

  /* Dump the opcode statistics table.  */
  fprintf (file, _("Name   # Modes  Min ops  Max ops  Modes mask  # Used\n"));
  for (i = 0; i < mc9xgate_nb_opcode_defs; i++, opc++)
    {
      fprintf (file, "%-7.7s  %5d  %7d  %7d  0x%08lx  %7d\n",
	       opc->opcode->name,
	       opc->nb_modes,
	       opc->min_operands, opc->max_operands, opc->format, opc->used);
    }
}

const char *
mc9xgate_listing_header (void)
{
  if (current_architecture & cpumc9xgate)
    return "MC9S12X GAS "; /* TODO same as above */
  else
    return "ERROR MC9S12X GAS ";
}
symbolS *
md_undefined_symbol (char *name ATTRIBUTE_UNUSED)
{
  return 0;
}

valueT
md_section_align (asection *seg, valueT addr)
{
  int align = bfd_get_section_alignment (stdoutput, seg);
  return ((addr + (1 << align) - 1) & (-1 << align));
}

void
md_assemble (char *str)
{
 *str = (char) 't';  //for testing duh
}

static void
s_mc9xgate_relax (int ignore ATTRIBUTE_UNUSED)
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
               BFD_RELOC_MC9XGATE_RL_GROUP);

  demand_empty_rest_of_line ();
}

/* Pseudo op to control the ELF flags.  */
static void
s_mc9xgate_mode (int x ATTRIBUTE_UNUSED)
{

}

static void
s_mc9xgate_mark_symbol (int mark)
{
	mark = 0; //for testing
}

/* Force truly undefined symbols to their maximum size, and generally set up
   the frag list to be relaxed.  */
int
md_estimate_size_before_relax (fragS *fragP, asection *segment)
{
 int temp = fragP->fr_fix;
 int temp2 = segment->entsize;
 temp2 = temp;
 return 0;
}

long
mc9xgate_relax_frag (segT seg ATTRIBUTE_UNUSED, fragS *fragP,
                    long stretch ATTRIBUTE_UNUSED)
{
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
md_pcrel_from (fixS *fixP)
{
  if (fixP->fx_r_type == BFD_RELOC_MC9XGATE_RL_JUMP)
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
  return reloc;
}

void
md_apply_fix (fixS *fixP, valueT *valP, segT seg ATTRIBUTE_UNUSED)
{
	int temp1 = fixP->fx_addnumber;
	temp1++;
	int temp2 =(int) valP;
	temp2++;
}

/* See whether we need to force a relocation into the output file.  */
int
tc_mc9xgate_force_relocation (fixS *fixP)
{
  if (fixP->fx_r_type == BFD_RELOC_MC9XGATE_RL_GROUP)
    return 1;

  return generic_force_reloc (fixP);
}

/* Here we decide which fixups can be adjusted to make them relative
   to the beginning of the section instead of the symbol.  Basically
   we need to make sure that the linker relaxation is done
   correctly, so in some cases we force the original symbol to be
   used.  */
int
tc_mc9xgate_fix_adjustable (fixS *fixP)
{
  switch (fixP->fx_r_type)
    {
      /* For the linker relaxation to work correctly, these relocs
         need to be on the symbol itself.  */
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
md_convert_frag (bfd *abfd ATTRIBUTE_UNUSED, asection *sec ATTRIBUTE_UNUSED,
                 fragS *fragP)
{
  int temp = fragP->fr_address;
  temp++;
  return;
}

/* Set the ELF specific flags.  */
void
mc9xgate_elf_final_processing (void)
{
	/* TODO make this always true */
  if (current_architecture & cpumc9xgate)
    elf_flags |= EF_MC9XGATE_MACH;
  elf_elfheader (stdoutput)->e_flags &= ~EF_MC9XGATE_ABI;
  elf_elfheader (stdoutput)->e_flags |= elf_flags;
}
