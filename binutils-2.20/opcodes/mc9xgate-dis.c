/* m68hc11-dis.c -- Motorola 68HC11 & 68HC12 disassembly
   Copyright 1999, 2000, 2001, 2002, 2003, 2005, 2006, 2007
   Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */
/*
 * Can't disassemble for architecture UNKNOWN!
 * skipping as arch ==
 * MC9S12X
 * mc9s12x
 *    Can't disassemble for architecture
 *
 */

#include <assert.h>
#include <math.h>
#include "sysdep.h"
#include "dis-asm.h"
#include "opintl.h"
#include "libiberty.h"


#include <stdio.h>
#include "ansidecl.h"
#include "opcode/mc9xgate.h"


/* temp macros */
#define MC9XGATE_OP_REG 		44
#define MC9XGATE_OP_JUMP_REL	55
#define MC9XGATE_OP_JUMP_REL16	66
#define MC9XGATE_OP_IX			77
#define MC9XGATE_OP_IY			88
#define MC9XGATE_OP_REG_2		99
/* end temp macros */
#define MC9XGATE_TWO_BYTES				02 /* two bytes */
#define MC9XGATE_MIN_INS_SIZE			02 /* the minimum size(in bytes of an instruction */
#define MC9XGATE_MAX_INS_SIZE			04 /* the maximum size(in bytes) of an instruction */

#define PC_REGNUM 3

struct mc9xgate_opmatch{
	unsigned int bin_opcode;
	char name[10];
	unsigned int size;
};

static const char *const reg_name[] = {
		"R0", "R1", "R2", "R3", "R4", "R5", "R6", "PC"
};

static const char *const reg_src_table[] = {
		"R0", "R1", "R2", "R3", "R4", "R5", "R6", "CCR", "TMP3"
};

static const char *const reg_dst_table[] = {
		"R0", "R1", "R2", "R3", "R4", "R5", "R6","CCR", "TMP2"
};

#define OP_PAGE_MASK (MC9XGATE_OP_PAGE2|MC9XGATE_OP_PAGE3|MC9XGATE_OP_PAGE4)  // was M6811

static unsigned int *operMasks;
static int initialized;
/* Prototypes for local functions.  */
//static int read_memory (bfd_vma, bfd_byte *, int, struct disassemble_info *);
//static int print_indexed_operand (bfd_vma, struct disassemble_info *,
//                                 int*, int, int, bfd_vma);
static int print_insn (bfd_vma, struct disassemble_info *, int);
static int read_memory (bfd_vma , bfd_byte*, int, struct disassemble_info*);
static unsigned int ripBits(unsigned int *bitsLeft, unsigned int numBits, struct mc9xgate_opcode *opcodePTR, unsigned int raw_code);

static unsigned int
power(unsigned int base, unsigned int powerof);
//static unsigned short mc9xgatedis_opcode (bfd_vma addr, struct disassemble_info *info);


/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */
static int
print_insn (bfd_vma memaddr, struct disassemble_info* info, int arch)
{
	printf("\n in print_insn arch is %d", arch);
	printf("\n memaddr is %d",(unsigned int)memaddr);
	printf("\n dis info    symtab size is %d",info->symtab_size);
	printf("\n number of symbols is %d",info->num_symbols);
	printf("\n trying print address fuction ");
	info->print_address_func(memaddr, info);


	int status;
	bfd_byte buffer[4];
	//unsigned int code = 0;
	unsigned int raw_code;
	//unsigned int operandMask;
	char *s = 0;
	//unsigned int mask;
	//long format, pos, i;
	long pos;
	//short sval;
	int i = 0;
	struct mc9xgate_opcode *opcodePTR = mc9xgate_opcodes;
	unsigned int *operMaskPTR = 0;
	unsigned int operandMask = 0;
	int found = 0;
	//struct mc9xgate_opcode *opcTablePtr = opcodeTable;

	pos = 2; /* default to one word read from mem */

	if(!initialized){

		operMasks =  xmalloc(sizeof(unsigned int) * mc9xgate_num_opcodes);

		for(i = 0, operMaskPTR = operMasks; i < mc9xgate_num_opcodes; i++, opcodePTR++, operMaskPTR++){
			unsigned int bin = 0;
			unsigned int mask = 0;

			for(s = opcodePTR->format; *s; s++){
				bin <<= 1;
				mask <<= 1;
				bin |= (*s == '1');
				mask |= (*s == '0' || *s == '1');
			}
			/* asserting will uncover inconsistencies in our table */
			assert ((s - opcodePTR->format) == 16 || (s - opcodePTR->format) == 32 );
			assert (opcodePTR->bin_opcode == bin);
			*operMaskPTR = mask;
		}

		initialized = 1;
	}

	/* read one word */
	status = read_memory (memaddr, buffer, 2, info);
	raw_code = buffer[0];
	raw_code <<= 8;
	raw_code += buffer[1];

	for(i = 0, opcodePTR = mc9xgate_opcodes, operMaskPTR = operMasks; i < mc9xgate_num_opcodes; i++, operMaskPTR++, opcodePTR++){

		if((raw_code & *operMaskPTR) == opcodePTR->bin_opcode){
			found = 1;
			break;

		}
	}

	if(found){
		pos = opcodePTR->size;
		operandMask = ~*operMaskPTR;
		printf("\n match found-%s constraints-%s rawcode-%x", opcodePTR->name, opcodePTR->format, raw_code);
		printf("\nsymbol at address %d", info->symbol_at_address_func(memaddr, info));

		int operand = 0;
		int operandSize = 0;
		for(s = opcodePTR->constraints; *s; s++){

			switch (*s){

			case 's':
			case 'r':
				operandSize = 3;
				printf("\n ripped R%x\n", (ripBits(&operandMask, operandSize, opcodePTR, raw_code)) );
				break;
			case 'i':
				printf("\n need to diss an immediate operand");
				  (*info->fprintf_func) (info->stream, " #$%02x%s",
							 buffer[0] & 0x0FF, " ");
							 //(format & MC9S12X_OP_JUMP_REL ? " " : ""));
				break;
			case 'b': /* either 9 or 10 bit PC real operand */
				if(info->symbol_at_address_func(memaddr, info)){

					if(*(++s) == '9'){
						operandSize = 9;
					}else if(*s == 'a'){
						operandSize = 10;
					}

					operand = (ripBits(&operandMask, *(s++) - '0', opcodePTR, raw_code) + 1);
					/* covert sign */
					if((operand & 0x0200) && (operandSize == 10))
						operand |= 0xFFFFFC00;
					if((operand & 0x0100) && (operandSize == 9))
						operand |= 0xFFFFFE00;
					/* words to bytes */
					operand *= 2;
					printf("\n ripped %d\n", operand);

				}
			default:
				//as_error(_(": unhandled constraint"));
				break;
			}

		}

	}
	//printf("\n extracted from memory a-%x b-%x c-%x d-%x \n",(unsigned int) buffer[0], (unsigned int)buffer[1]
	//                                  ,(unsigned int) buffer[2], (unsigned int)buffer[3] );

	if(status)
		printf("\n error reading memory");


	return pos;
}

/* Disassemble one instruction at address 'memaddr'.  Returns the number
 of bytes used by that instruction.  */
//int
//print_insn_MC9XGATE (bfd_vma memaddr, struct disassemble_info* info)
//{
//  return print_insn (memaddr, info, cpu6811);
//}

int
print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
{
	printf("\nin print_insn_mc9xgate\n");
	return print_insn (memaddr, info, cpumc9xgate);
	//	return 1;
}


static int
read_memory (bfd_vma memaddr, bfd_byte* buffer, int size,
		struct disassemble_info* info)
{
	int status;

	/* Get first byte.  Only one at a time because we don't know the
     size of the insn.  */
	status = (*info->read_memory_func) (memaddr, buffer, size, info);
	if (status != 0)
	{
		(*info->memory_error_func) (status, memaddr, info);
		return -1;
	}
	return 0;
}

static unsigned int
ripBits(unsigned int *bitsLeft, unsigned int numBits, struct mc9xgate_opcode *opcodePTR, unsigned int raw_code){
	unsigned int bitsToProcess = (opcodePTR->size) * 8;
	unsigned int bitPointer = power(2,bitsToProcess - 1 );
	unsigned int operand = 0;

	for(;bitsToProcess && numBits;  bitsToProcess--){
		bitPointer = power(2,bitsToProcess - 1);
		if(*bitsLeft & bitPointer){
			operand <<= 1;
			if(raw_code & bitPointer)
				operand += 1;
			/*flag bit as used */
			bitPointer = ~bitPointer;
			*bitsLeft &= bitPointer;
			numBits--;
		}
	}

	return operand;
}

static unsigned int
power(unsigned int number, unsigned int exponent){
	int product = 1;
	unsigned int i = 0;
	while(i < exponent){
		product *= number;
		i++;
	}
	return product;
}
