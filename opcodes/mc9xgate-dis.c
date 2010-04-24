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
 */

#include <assert.h>
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

/* Prototypes for local functions.  */
//static int read_memory (bfd_vma, bfd_byte *, int, struct disassemble_info *);
//static int print_indexed_operand (bfd_vma, struct disassemble_info *,
 //                                 int*, int, int, bfd_vma);
static int print_insn (bfd_vma, struct disassemble_info *, int);
//static int read_memory (bfd_vma , bfd_byte*, int, struct disassemble_info*);
static unsigned short mc9xgatedis_opcode (bfd_vma addr, struct disassemble_info *info);


/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */
static int
print_insn (bfd_vma memaddr, struct disassemble_info* info, int arch)
{
 // printf("\nin print_insn\n");
  int temp = arch;
  temp++;
  unsigned  int insn_size = 0;
  unsigned int mc9xgate_insn;
  const struct mc9xgate_opcode *opcode;
  static unsigned int *mc9xgate_bin_masks; /* 16-bit bin mask */
  //static unsigned int mc9xgate_bin_mask;
  static unsigned int *mc9xgate_bin_maskptr;
  unsigned int mc9xgate_nopcodes = mc9xgate_num_opcodes;
  //int ins_size = 0;
 // bfd_byte buffer[MC9XGATE_MAX_INS_SIZE];
 // status = read_memory (memaddr, &buffer[0], MC9XGATE_MIN_INS_SIZE, info);
  static int initialized;

  /*
  if (status != 0)
     {
	   printf("\nstatus is NULL\n"); // debug
       return status;
     }
	*/
   /* create an array of operand masks */

   if (!initialized){
	   //printf("\n initializing mask table\n");
   mc9xgate_bin_masks = xmalloc (mc9xgate_nopcodes * sizeof(unsigned int) );

   for(mc9xgate_bin_maskptr = mc9xgate_bin_masks, opcode = mc9xgate_opcodes;
		   opcode->name;
		   opcode++, mc9xgate_bin_maskptr++)
   {
	   char *s;
	   unsigned int bin = 0;
	   unsigned int mask = 0;
	   for(s = (char*) opcode->format; *s; ++s){
		   mask <<= 1;
		   bin <<= 1;
		   mask |= (*s == '1' || *s == '0');
		   bin |= (*s == '1');
		//  printf("\n decode %c %s\n",*s,opcode->name);
	   }
	   assert (opcode->bin_opcode == bin);
	   assert (s - opcode->format == 16); /* check the length */
	   *mc9xgate_bin_maskptr = mask;
    }
	   initialized = 1;
   }


   mc9xgate_insn = (int) mc9xgatedis_opcode(memaddr, info); /* get 16-bits from input stream */

	   /* TODO add code to determine the size of the instruction here */

   for(mc9xgate_bin_maskptr = mc9xgate_bin_masks, opcode = mc9xgate_opcodes; opcode->name; mc9xgate_bin_maskptr++, opcode++){
	   if( (mc9xgate_insn & *mc9xgate_bin_maskptr) == opcode->bin_opcode){
		   insn_size = opcode->size;
		   break;
	   }
}

    (*info->fprintf_func) (info->stream, " %s, %s ",opcode->name,opcode->format);
  // 			     buffer, "brk");
  // printf("\n read %x %x \n", buffer[0], buffer[1]);
   //printf("\n buffer conents %s\n", &buffer[0]);

  return insn_size; /* TODO properly determine the instruction size in bytes */
}

int
print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
{
   // printf("\nin print_insn_mc9xgate\n");
    return print_insn (memaddr, info, cpumc9xgate);
//	return 1;
}

/* old s12 code
static int
read_memory (bfd_vma memaddr, bfd_byte* buffer, int size,
             struct disassemble_info* info)
{
  int status;
*/
//  /* Get first byte.  Only one at a time because we don't know the
//     size of the insn.  */
//  status = (*info->read_memory_func) (memaddr, buffer, size, info);
//  if (status != 0)
//    {
//      (*info->memory_error_func) (status, memaddr, info);
//      return -1;
//    }
//  return 0;
//}
//

/* return two bytes read from input address */
static unsigned short
mc9xgatedis_opcode (bfd_vma addr, struct disassemble_info *info)
{
  bfd_byte buffer[2];
  int status;

  status = info->read_memory_func (addr, buffer, 2, info);

  if (status == 0)
	  //return bfd_getl16 (buffer); /* doesnt seem to work wrong Endian maybe*/
	  return ((buffer[0] << 8) + buffer[1]);
  info->memory_error_func (status, addr, info);
  return -1;
}
