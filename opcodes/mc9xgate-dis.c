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




/* Read the 68HC12 indexed operand byte and print the corresponding mode.
   Returns the number of bytes read or -1 if failure.  */

/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */
static int
print_insn (bfd_vma memaddr, struct disassemble_info* info, int arch)
{
  unsigned long temp = memaddr;
  int temp2 = info->bytes_per_line;
  temp2 = arch;
  temp = temp2;
  return 1;
}

/* Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.  */
//int
//print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
//{
//  return print_insn (memaddr, info, cpu6811);
//}

int
print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
{
//  printf("\nin print_insn_mc9xgate");
  return print_insn (memaddr, info, cpumc9xgate);
}
