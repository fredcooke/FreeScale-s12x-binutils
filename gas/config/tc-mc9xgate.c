/* tc-m68hc11.c -- Assembler code for the Motorola 68HC11 & 68HC12.
   Copyright 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009
   Free Software Foundation, Inc.
   Written by Sean Keys (info@powerefi.com)
   Based on existing avr and hc12 ports.
*/
/*
Fatal error: selected target format
find length of asr
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

#define REGISTER_P(ptr)		(ptr == 'r')


/* This macro has no side-effects.  */
#define ENCODE_RELAX(what,length) (((what) << 2) + (length))
#define RELAX_STATE(s) ((s) >> 2)
#define RELAX_LENGTH(s) ((s) & 3)

#define IS_OPCODE(C1,C2)        (((C1) & 0x0FF) == ((C2) & 0x0FF))

/*  LOCAL FUNCTIONS */
static void s_mc9xgate_mode (int); /* Pseudo op to control the ELF flags.  */
static inline char * skip_whitespace(char *);
static void get_default_target (void); /* Pseudo op to indicate a relax group.  */
static void s_mc9xgate_relax (int);
static void s_mc9xgate_mode (int);
static void s_mc9xgate_mark_symbol (int);/* Pseudo op to control the ELF flags.  */
static char* extract_word(char *, char *, int);
static char * mc9xgate_new_instruction(int size); /* number of bytes */
unsigned short mc9xgate_apply_operand(unsigned short new_mask, unsigned short *oper_bits_availiable, unsigned short mask,  unsigned char n_bits);

static unsigned int mc9xgate_operands (struct mc9xgate_opcode *opcode, char **line);

static unsigned int mc9xgate_operand (struct mc9xgate_opcode *opcode, int where, char **op_con, char **line);

unsigned short mc9xgate_get_operands_old(char *input, struct mc9xgate_opcode *opcode);
static unsigned int mc9xgate_get_constant(char *input, int max);
void append_str(char *in, char c);
static int cmp_opcode(struct mc9xgate_opcode *, struct mc9xgate_opcode *);

//char get_address_mode(char *);
/* Mark the symbols with STO_M68HC12_FAR to indicate the functions
   are using 'rtc' for returning.  It is necessary to use 'call'
   to invoke them.  This is also used by the debugger to correctly
   find the stack frame.  */
/* END LOCAL FUNCTIONS */

/* LOCAL DATA */
static struct hash_control *mc9xgate_hash;
static char parse_error;
static char error_message[ sizeof( char) ];
static char oper_check;
static int oper1_bit_length = 0;
static int oper2_bit_length = 0;
static int oper3_bit_length = 0;
//static struct mc9xgate_opcode *mc9xgate_op_table = 0;


struct mc9xgate_opcode_handle{
	int number_of_modes;
	char *name;
	struct mc9xgate_opcode *opc0;
	struct mc9xgate_opcode *opc1;
	struct mc9xgate_opcode *opc2;
	struct mc9xgate_opcode *opc3;
	struct mc9xgate_opcode *opc4;
};

typedef struct mc9xgate_operand{
	expressionS exp0;
	expressionS exp1;
	unsigned char reg;
}operand;

/* */
//struct mc9xgate_opcode_s {
 // const char*    name;     /* Op-code name */
//  const char*    constraints; /* */
//  const char*    format;   /* format string */
//  unsigned int   size;   /* size in words */
//  unsigned int   bin_opcode;  /* binary opcode with operands masked off */
//  unsigned char  cycles_min; /* minimum cpu cycles needed */
//  unsigned char  cycles_max; /* maximum cpu cycles needed */
//  unsigned char  set_flags_mask; /* CCR flags set */
//  unsigned char  clr_flags_mask; /* CCR flags cleared */
// unsigned char  chg_flags_mask; /* CCR flags changed */
//  unsigned char  arch; /* cpu type, may always be mc9s12 only */
//};

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
//static struct hash_control *mc9xgate_hash;
//static struct mc9xgate_opcode_def *mc9xgate_opcode_defs = 0;
static unsigned int numberOfCalls = 0;
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
	struct mc9xgate_opcode *mc9xgate_op_table ;
	struct mc9xgate_opcode_handle *op_handles = 0;
	struct mc9xgate_opcode *mc9xgate_op_handle_ptr;
	char prev_op_name[9];
	char handle_enum = 0;
	unsigned int number_of_handle_rows = 0;
	int i, j = 0;

	/* create a local copy of our opcode table including and extral line for NULL termination*/
	mc9xgate_op_table = (struct mc9xgate_opcode *) xmalloc( (mc9xgate_num_opcodes + 1) * sizeof(struct mc9xgate_opcode));
	for(mc9xgate_opcode_ptr = mc9xgate_opcodes, i = 0; i < mc9xgate_num_opcodes; i++){
		mc9xgate_op_table[i] =  mc9xgate_opcode_ptr[i];
	}

	qsort(mc9xgate_opcode_ptr, mc9xgate_num_opcodes, sizeof(struct mc9xgate_opcode),
			(int (*) (const void*, const void*)) cmp_opcode);

	/* mark the end of the table with a NULL entry */
	mc9xgate_opcode_ptr += sizeof(struct mc9xgate_opcode) * (mc9xgate_num_opcodes + 1);
	mc9xgate_opcode_ptr->name = NULL;

	/* calcuate number of handles since this will be smaller than the raw number of opcodes in the table */
	for(mc9xgate_opcode_ptr = mc9xgate_op_table; mc9xgate_opcode_ptr->name; mc9xgate_opcode_ptr++){
		if(strcmp(prev_op_name,mc9xgate_opcode_ptr->name)){
			number_of_handle_rows++;
		}
		strcpy(prev_op_name, mc9xgate_opcode_ptr->name);
	}

	op_handles = (struct mc9xgate_opcode_handle *) xmalloc(sizeof(struct mc9xgate_opcode_handle) * (number_of_handle_rows + 1));

	/* insert opcode names into hash table, aliasing duplicates */
	mc9xgate_hash = hash_new(); /* create a new has control table */
	for(mc9xgate_opcode_ptr = mc9xgate_op_table, i = 0, j = -1 ; i < mc9xgate_num_opcodes; i++, mc9xgate_opcode_ptr++){
		if(strcmp(prev_op_name,mc9xgate_opcode_ptr->name)){
			handle_enum = 0;
			j++;
			op_handles[j].name = mc9xgate_opcode_ptr->name;
			op_handles[j].opc0 = mc9xgate_opcode_ptr;
			mc9xgate_op_handle_ptr = op_handles[j].opc0;
		}else{
			handle_enum++;
			switch (handle_enum) {
			case 1:
				op_handles[j].opc1 = mc9xgate_opcode_ptr;
				break;
			case 2:
				op_handles[j].opc2 = mc9xgate_opcode_ptr;
				break;
			case 3:
				op_handles[j].opc3 = mc9xgate_opcode_ptr;
			case 4:
				op_handles[j].opc4 = mc9xgate_opcode_ptr;
			default:
				as_bad(_(":error adding operand handle"));
				break;
			}
			op_handles[j].number_of_modes = handle_enum;
		}
		strcpy(prev_op_name, op_handles[j].name);
	}
	while(op_handles->name){
		printf("\n hash insert message %s",(hash_insert(mc9xgate_hash, op_handles->name, (char *) op_handles)) );
		op_handles++;
	}
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
//  int i;
  struct mc9xgate_opcode *opc;

  hash_print_statistics (file, "opcode table", mc9xgate_hash);

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
md_assemble (char *input_line)
{

// char io_buffer[20]; /* general purpose buffer */
 struct mc9xgate_opcode *opcode;
 struct mc9xgate_opcode_handle *opcode_handle;
 char *saved_input_line = input_line; /* caller expects it to be returned as it was passed */
 char *f; /* instruction fragment pointer */
 unsigned short operand_mask = 0;
 char op_name[9];
 char handle_enum_alias = 0;
 char parse_complete = 0;
// operand operands[3]; /* seems theres always 3 opers max */

 //unsigned int number_of_operands = 0;

 //unsigned short oper_bits_availiable = 0;

 numberOfCalls++; // for testing

 oper_check = 0; /* set error flags */

 input_line = extract_word(input_line, op_name, sizeof(op_name));
 if (!op_name[0]){
	 as_bad(_("opcode missing or not found on input line"));
//	 return;
 }

 printf("\n found code %s\n", op_name);

 // use hash find to find a match

 // calculate min and max ops for a paticular opcode
 // size operand array to max
 // gather operands from input line
 // try to apply opers to opcode if error try the next entry with matching name if non work fail
 // write opcode to ouput

 //strcpy(op_name, io_buffer);

 while(!parse_complete){

	 if(!(opcode_handle = (struct mc9xgate_opcode_handle *) hash_find (mc9xgate_hash, op_name) )){
		 printf("\n %s\n",op_name);
		 as_bad(_("opcode not found in hash"));
	  }
	 handle_enum_alias = opcode_handle->number_of_modes;
	 printf("\n this opcode has %d possobile addressing modes", handle_enum_alias);
	 opcode = opcode_handle->opc0;

		 //unsigned short test = mc9xgate_get_constant(input_line, 0xFFF);
	 operand_mask = mc9xgate_operands(opcode, &input_line);

	 printf("\n operand mask is  %d", operand_mask);
//	 operand_mask = mc9xgate_get_operands(input_line, opcode);
//	 printf("\n need to check op against mask %d\n",operand_mask);
	 printf("\n oper_check status %d", oper_check);
//	 if(oper_check){
//		 parse_complete = 1;
//	 }else {

//		 append_str(op_name, 'a' + '0');
//		 input_line = saved_input_line; /* rewind */
//		 printf("\n new name is %s", op_name);
		 /* find next opcode and repeat */
// 	 }

	 parse_complete = 1;
 }

// if(!parse_error){
	 f = mc9xgate_new_instruction(opcode->size);
// 	 number_to_chars_bigendian (f, opcode->bin_opcode | operand_mask, opcode->size);
// 	 printf("\n wrote op %x\n", opcode->bin_opcode | operand_mask );
// }

 input_line = saved_input_line;

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

static inline char *
skip_whitespace(char *s){
	while (*s == ' ' || *s == '\t'){
		s++;
	}
	return s;
}

/* extract a word (continuous chars) from the input line */
static char *
extract_word (char *from, char *to, int limit)
{
  char *op_start;
  char *op_end;
  int size = 0;

  /* Drop leading whitespace.  */
  from = skip_whitespace (from);
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
mc9xgate_new_instruction(int size)
{

	char *f = frag_more(size);

	dwarf2_emit_insn(size); /* avr code passes 0 for size */

	return f;
}
/*
unsigned char get_address_mode(int opcode){
	char number = 0;


}
*/

unsigned short
mc9xgate_apply_operand(unsigned short new_mask, unsigned short *availiable_mask_bits,
						unsigned short mask, unsigned char n_bits){
	unsigned short n_shifts;
	unsigned int n_drop_bits;

	for(n_shifts = 0; !(*availiable_mask_bits & SIXTEENTH_BIT) && n_shifts < 16; n_shifts++){
		*availiable_mask_bits <<= 1;
    }

	for(n_drop_bits = n_bits; n_drop_bits && (*availiable_mask_bits & SIXTEENTH_BIT); --n_drop_bits ){
		*availiable_mask_bits <<= 1;

	}
	if(n_drop_bits)
		//	as_bad(_(":operand has too many bits"));

	*availiable_mask_bits >>= n_shifts + n_bits;

	if( (n_drop_bits == 0) && (*availiable_mask_bits == 0) ){
			oper_check = 1; /* flag operand check as good */
			printf("\n good to go\n");
		}

	new_mask <<=  N_BITS_IN_WORD - ( n_shifts + n_bits);
	mask |= new_mask ;
	return mask;
}

unsigned short
mc9xgate_get_operands_old(char *input_string, struct mc9xgate_opcode *opcode){
	char buffer[30];
	unsigned short op_mask = 0;
//	unsigned short op = 0;
	unsigned char oper_bits_remaining = 0;
	unsigned char number_of_constraints = 1; /* if we got here there is at least 1 constraint */
	char *c;
	char reg_number = 0; /* TODO maybe change this so it defaults to error code or "r" could be the same as "r0" */
	unsigned short oper_bits_availiable = 0;
	char *buffer_ptr = &buffer[0];
	char seperator_expected = 0;
	unsigned short constant = 0;

	/* build operand mask */
    for(c = (char*) &opcode->format[0]; *c; c++){
		 if(*c == '0' || *c == '1'){
			 oper_bits_availiable <<= 1;
		 } else{
			 oper_bits_availiable <<= 1;
			 oper_bits_availiable += 1;
			 oper_bits_remaining++;
		 }
	 }

    if(!oper_bits_remaining){ /* no operands to apply return */
    	oper_check = 1; /* good to go */
    	return op_mask;
    }

	/* get number of constraints */
	for(c = (char *) &opcode->constraints[0]; *c; c++){
		if (*c == ',')
			number_of_constraints++;
	}


	for ( c = (char*) &opcode->constraints[0]; *c; c++){
	//	unsigned char oper_size; /* in bits */
	//	printf("\n read constraint %c \n",*c);
		unsigned short oper_constant_bit_length = 0xFFFF;
		switch (*c){
		case 'r': /* expecting register operand */
			/* TODO RI mode always uses 5 bits */
			input_string = extract_word(input_string, buffer, sizeof(buffer));
			if(*buffer_ptr == 'r'){
				reg_number = buffer[1] - '0';
				if (reg_number >= 0 && reg_number <= 7){
					constant = (unsigned short) reg_number;
					op_mask = mc9xgate_apply_operand(constant, &oper_bits_availiable, op_mask, 3); /* regs always use 3 bits */
					number_of_constraints--;
					if(number_of_constraints)
						seperator_expected = 1;
				} else{
				//	as_bad(_(":expected register operand r0-r7"));
					strcpy(error_message, ":expected register operand r0-r7");
					parse_error = 1;
				}
			}
			break;
		case 'i':/* expecting immediate operand */
			if(!ISDIGIT (++*c))
				strcpy(error_message,":expected number of immediate bits in operand constraint");
			oper_constant_bit_length = ~ (oper_constant_bit_length << ( (*c - '0') + 1) ); /* TODO macro this */
			printf("\n number of bits read in ASCII %c max int is %d",*c, oper_constant_bit_length);
			constant = mc9xgate_get_constant(buffer_ptr, oper_constant_bit_length);
			printf("\n constant is %d\n",constant);
			op_mask = mc9xgate_apply_operand(constant, &oper_bits_availiable, op_mask, oper_constant_bit_length);

			if(number_of_constraints)
				seperator_expected = 1;
			break;
		case ',':
		//	if(!(*buffer_ptr == ',' && seperator_expected))
		//		as_bad(_(":constraint seperator not expected"));
			break;
			}
	}
    printf("\n number of constraints left %d \n",number_of_constraints);
	return op_mask;
}

static unsigned int
mc9xgate_get_constant (char *str, int max)
{
  expressionS ex;

  str = skip_whitespace (str);
  input_line_pointer = str;
  expression (& ex);

  if (ex.X_op != O_constant)
    as_bad (_("constant value required"));

  if (ex.X_add_number > max || ex.X_add_number < 0)
    as_bad (_("number must be positive and less than %d"), max + 1);

  return ex.X_add_number;
}


void
append_str(char *in, char c){
	unsigned char length = (unsigned char) strlen(in);
	in[length] = c;
	in[++length] = 0; /* terminate */
	return;
}

static int
cmp_opcode (struct mc9xgate_opcode *op1, struct mc9xgate_opcode *op2)
{
	 return strcmp (op1->name, op2->name);
}


/* Parse ordinary expression.  */

//static char *
//parse_exp (char *s, expressionS *op)
//{
//  input_line_pointer = s;
//  expression (op);
//  if (op->X_op == O_absent)
//    as_bad (_("missing operand"));
//  return input_line_pointer;
//}

/* Parse instruction operands.
   Return binary opcode.  */

static unsigned int
mc9xgate_operands (struct mc9xgate_opcode *opcode, char **line)
{
  char *op = opcode->constraints;
  unsigned int bin = opcode->bin_opcode;
  char *frag = frag_more (opcode->size * 2); /* all opcodes are at lest 2 bytes in size */
  char *str = *line;
  int where = frag - frag_now->fr_literal;
  static unsigned int prev = 0;  /* Previous opcode.  */

  unsigned int oper1 = 0;
  unsigned int oper2 = 0;
  unsigned int oper3 = 0;
  int oper1_present = 0;
 //      int oper1_bit_length = 0;
  int oper2_present = 0;
 //      int oper2_bit_length = 0;
  int oper3_present = 0;
 //      int oper3_bit_length = 0;

  /* Opcode have operands.  */
  if (*op)
    {

      /* Parse first operand.  */
//      if (REGISTER_P (*op))
//    oper1_length = 3;
//      if (*op)
      oper1_present = 1;
      printf("\n getting operand 1 , %c", *op);
      oper1 = mc9xgate_operand (opcode, where, &op, &str);
      ++op;

      /* Parse second operand.  */
      if (*op)
	{
	  if (*op == ',')
	    ++op;

	  if (*op == '=') /* TODO not sure if we want this */
	    {
		  oper2 = oper1;
		  oper2_present = 1;
	    }
	  else
	    {
//	      if (REGISTER_P (*op))
		oper2_present = 1;

//	      ++*op
	      str = skip_whitespace (str);
	      printf("\n string is now %c", *str);
	      if (*str++ != ',')
		as_bad (_("`,' required"));
	      str = skip_whitespace (str);
	      printf("\n operand before call to get_operand %c", *op);
	      oper2 = mc9xgate_operand (opcode, where, &op, &str);
	      printf("\n operand after call to get_operand %c", *op);
	      ++op;
	    }

/* TODO something like this may work
	  if (reg1_present && reg2_present)
	    reg2 = (reg2 & 0xf) | ((reg2 << 5) & 0x200);
	  else if (reg2_present)
	    reg2 <<= 4;
	}
      if (reg1_present)
	reg1 <<= 4;
      bin |= reg1 | reg2;
 */
    }
         if (*op)
    	{
    	  if (*op == ',')
    	    ++op;

    	  if (*op == '=') /* TODO not sure if we want this */
    	    {
    	      oper3 = oper1;
    	      oper3_present = 1;
    	    }
    	  else
    	    {
 //   	      if (REGISTER_P (*op))
    		oper3_present = 1;

    	      str = skip_whitespace (str);
    	      if (*str++ != ',')
    		as_bad (_("`,' required"));
    	      str = skip_whitespace (str);
    	     printf("\n getting operand 3 typs is, %c input string is %s", *op, str);
    	     oper3 = mc9xgate_operand (opcode, where, &op, &str);
    	    }
    	}


  /* Detect undefined combinations (like ld r31,Z+).  */
//  if (!avr_opt.all_opcodes && AVR_UNDEF_P (bin))
 //   as_warn (_("undefined combination of operands"));

//  if (opcode->insn_size == 2)
//    {
      /* Warn if the previous opcode was cpse/sbic/sbis/sbrc/sbrs
         (AVR core bug, fixed in the newer devices).  */
//      if (!(avr_opt.no_skip_bug ||
//            (avr_mcu->isa & (AVR_ISA_MUL | AVR_ISA_MOVW)))
//	  && AVR_SKIP_P (prev))
//	as_warn (_("skipping two-word instruction"));

//      bfd_putl32 ((bfd_vma) bin, frag);
//    }
//  else

   // bfd_putl16 ((bfd_vma) bin, frag); /* TODO dont think we can write yet must check first*/
    }



  bfd_putl16 ((bfd_vma) bin, frag); /* TODO dont think we can write yet must check first*/

  prev = bin;
  *line = str;
  return bin;
 }
/* Parse one instruction operand.
   Return operand bitmask.  Also fixups can be generated.  */

static unsigned int
mc9xgate_operand (struct mc9xgate_opcode *opcode, int where, char **op_con, char **line){
/* dummy code */
//  expressionS op_expr;
	where ++;
	opcode = opcode;
  char *op_constraint = *op_con;
  unsigned int op_mask = 0;
  char *str = skip_whitespace (*line);
  char r_name[20];

  /* reset */
  oper1_bit_length = 0;
  oper2_bit_length = 0;
  oper3_bit_length = 0;

  switch (*op_constraint)
    {
      /* Any register operand.  */
//    case 'w':
//    case 'd':
//    case 'r':
//    case 'a':
    case 'r':

    	if (*str == 'r' || *str == 'R')
	{

    	/* TODO detect +- code here always last register that is - or + and always last two bits of opcode */
      oper1_bit_length = 3; /* 111 */
      // if next is + or - length should be 5

	  str = extract_word (str, r_name, sizeof (r_name));
	  op_mask = 0xff;
	  if (ISDIGIT (r_name[1]))
	    {
	      if (r_name[2] == '\0')
		op_mask = r_name[1] - '0';
	      else if (r_name[1] != '0'
		       && ISDIGIT (r_name[2])
		       && r_name[3] == '\0')
		op_mask = (r_name[1] - '0') * 10 + r_name[2] - '0';
	      /* TODO fix so it checks for range 0-7 */
	    }
	}
      else
	{
	 as_bad(_(": expected register name r0-r7 read %s "), r_name );

	}
    break;

    case 'i':
    	(*op_con)++; /* advance the origional pointer */
    	op_constraint++;
    	oper1_bit_length = *op_constraint;
    	if(!ISDIGIT(*op_constraint))
      as_bad(_(":expected bit length with constraint type i(immediate) read %c"), *op_constraint);
    	//printf("\n input string is %s",str);
    	op_mask = mc9xgate_get_constant (str, 0xFFFF);
    	printf("\n length is %c and read %d from get_constant", *op_constraint, op_mask);
    break;

    case '?':
      break;

    default:
      as_bad (_("unknown constraint `%c'"), *op_constraint);
    }

  *line = str;
  return op_mask;
}

//void mc9xgate_operand_params(char *constraints, int bit_size){
//
//	return;
//}
