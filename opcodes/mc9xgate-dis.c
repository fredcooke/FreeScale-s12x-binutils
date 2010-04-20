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

#include <stdio.h>

#include "ansidecl.h"
#include "opcode/mc9xgate.h"
#include "dis-asm.h"

/* temp macros */
#define MC9XGATE_OP_REG 		44
#define MC9XGATE_OP_JUMP_REL	55
#define MC9XGATE_OP_JUMP_REL16	66
#define MC9XGATE_OP_IX			77
#define MC9XGATE_OP_IY			88
#define MC9XGATE_OP_REG_2		99
/* end temp macros */

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
static int read_memory (bfd_vma , bfd_byte*, int, struct disassemble_info*);

/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */
static int
print_insn (bfd_vma memaddr, struct disassemble_info* info, int arch)
{
 // printf("\nin print_insn\n");
  int temp = arch;
  temp++;
  int status;
  bfd_byte buffer[4];
  status = read_memory (memaddr, buffer, 1, info);
   if (status != 0)
     {
	   printf("\nstatus is NULL\n");
       return status;
     }
   /* TODO add code to determine the size of the instruction here */
   (*info->fprintf_func) (info->stream, "%d,%s",
   			     (int) 55, "brk");
   printf("\n read %d \n",status);
  return 2;
}


int
print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
{
   // printf("\nin print_insn_mc9xgate\n");
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

