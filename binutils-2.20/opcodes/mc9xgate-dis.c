/* xgate-dis.c -- Motorola XGATE disassembly
   Copyright 2009, 2010, 2011
   Free Software Foundation, Inc.
   Written by Sean Keys (skeys@ipdatasys.com)

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
   MA 02110-1301, USA.
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

#define MC9XGATE_TWO_BYTES      02 /* two bytes */
#define MC9XGATE_MIN_INS_SIZE	02 /* the minimum size(in bytes of an instruction */
#define MC9XGATE_MAX_INS_SIZE	04 /* the maximum size(in bytes) of an instruction */
#define MC9XGATE_NINE_BITS      0x1FF
#define MC9XGATE_TEN_BITS       0x3FF
#define MC9XGATE_NINE_MSB       0x100
#define MC9XGATE_TEN_MSB        0x200

//static const char * const reg_name[] =
//  { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "PC" };

//static const char * const reg_src_table[] =
//  { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "CCR", "TMP3" };

//static const char * const reg_dst_table[] =
//  { "R0", "R1", "R2", "R3", "R4", "R5", "R6", "CCR", "TMP2" };

/* Prototypes for local functions.  */
static int
print_insn(bfd_vma, struct disassemble_info *, int);
static int
read_memory(bfd_vma, bfd_byte*, int, struct disassemble_info*);
static int
ripBits(unsigned int *operandBitsRemaining, int numBitsRequested,
    struct mc9xgate_opcode *opcodePTR, unsigned int memory);
/* statics */
static unsigned int *operMasks;
static unsigned int *operMasksRegisterBits;
static int initialized;

/*
   Disassemble one instruction at address 'memaddr'.  Returns the number
   of bytes used by that instruction.
tmpdir/dump:     file format elf32-xgate
Disassembly of section .text:

00008000 <_start> ldl R1, #0xed
00008002 <_start+0x2> ldh R1, #0xfe
00008004 <_start+0x4> addl R5, #0xaf
00008006 <_start+0x6> addh R5, #0xde
00008008 <_start+0x8> ldl R2, #0x56
0000800a <_start+0xa> ldh R2, #0x34
0000800c <_start+0xc> ldl R3, #0x21
0000800e <_start+0xe> ldh R6, #0xfa
00008010 <_start+0x10> cmpl R1, #0xcd
00008012 <_start+0x12> cpch R1, #0xab
00008014 <_start+0x14> cmpl R2, #0xb2
00008016 <_start+0x16> cpch R2, #0xa1
*/

/* Disassemble one instruction at address 'memaddr'.  Returns the number
 of bytes used by that instruction.  */
static int
print_insn (bfd_vma memaddr, struct disassemble_info* info, int arch)
{
  int status;
  bfd_byte buffer[4];
  char opString[25] = {0};
  unsigned opIndex = 0;
  unsigned int raw_code;
  char *s = 0;
  long bytesRead = 0;
  int i = 0;
  struct mc9xgate_opcode *opcodePTR = mc9xgate_opcodes;
  unsigned int *operMaskPTR = 0;
  unsigned int *operMaskRegPTR = 0;
  unsigned int operandBits = 0;
  unsigned int operandRegisterBits = 0;
  signed int relAddr = 0;
  signed int operandOne;
  signed int operandTwo;
  signed int operandThree;
  int found = 0;
  /* initialize our array of opcode masks and check them against our constant table */
  if(!initialized){
      //todo struct these two
      operMasks =  xmalloc(sizeof(unsigned int) * mc9xgate_num_opcodes);
      operMasksRegisterBits =  xmalloc(sizeof(unsigned int) * mc9xgate_num_opcodes);
      for(i = 0, operMaskPTR = operMasks, operMaskRegPTR = operMasksRegisterBits; i < mc9xgate_num_opcodes; i++, operMaskRegPTR++, opcodePTR++, operMaskPTR++){
          unsigned int bin = 0;
          unsigned int mask = 0;
          for(s = opcodePTR->format; *s; s++){
              bin <<= 1;
              mask <<= 1;
              operandRegisterBits <<= 1;
              bin |= (*s == '1');
              mask |= (*s == '0' || *s == '1');
              operandRegisterBits |= (*s == 'r');
          }
          /* asserting will uncover inconsistencies in our table */
          assert ((s - opcodePTR->format) == 16 || (s - opcodePTR->format) == 32 );
          assert (opcodePTR->bin_opcode == bin);
          *operMaskPTR = mask;
          *operMaskRegPTR = operandRegisterBits;
      }
      initialized = 1;
  }
   /* read 16 bits */
  status = read_memory (memaddr, buffer, 2, info);
  raw_code = buffer[0];
  raw_code <<= 8;
  raw_code += buffer[1];
  /* special case for brk instruction */
  //if memory reads 0x0 print instruction doesn't seem to be called at all
  //if(raw_code == 0)
  //  {
  //    (*info->fprintf_func) (info->stream, "brk");
  //    return 2;
  //  }

  /* todo since we have macros and alias codes make this print all possible matches instead of just the first */
  for(i = 0, opcodePTR = mc9xgate_opcodes, operMaskPTR = operMasks, operMaskRegPTR = operMasksRegisterBits ; i < mc9xgate_num_opcodes; i++, operMaskPTR++, operMaskRegPTR++, opcodePTR++){
      if((raw_code & *operMaskPTR) == opcodePTR->bin_opcode){
          /* make sure we didn't run into a macro */
          if(opcodePTR->cycles_min != 0) {
              found = 1;
              break;
          } else {
              continue;
          }
      }
  }
  if(found){
      bytesRead = opcodePTR->size;
      operandBits = ~(*operMaskPTR) & 0xFFFF;
      (*info->fprintf_func) (info->stream, "%s", opcodePTR->name);
      int operand = 0;
      int operandSize = 0;
      /*  First we compare the shorthand format of the constraints. If we still are unable to pinpoint the operands
         we analyze the opcodes constraint string. */
      switch (opcodePTR->sh_format)
        {
      case MC9XGATE_R_C:
        (*info->fprintf_func) (info->stream, " R%x, CCR", (raw_code >> 8) & 0x7);
        break;
      case MC9XGATE_C_R:
        (*info->fprintf_func) (info->stream, " CCR, R%x", (raw_code >> 8) & 0x7);
        break;
      case MC9XGATE_R_P:
        (*info->fprintf_func) (info->stream, " R%x, PC", (raw_code >> 8) & 0x7);
        break;
      case MC9XGATE_INH:
        break;
      case MC9XGATE_R_R_R:
        if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_TRI))
          {
            (*info->fprintf_func)(info->stream, " R%x, R%x, R%x",
                (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                (raw_code >> 2) & 0x7);
          }
        else if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_IDR))
          {
            if (raw_code & 0x01)
              {
                (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x+)",
                    (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                    (raw_code >> 2) & 0x7);
              }
            else if (raw_code & 0x02)
              {
                (*info->fprintf_func)(info->stream, " R%x, (R%x, -R%x)",
                    (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                    (raw_code >> 2) & 0x7);
              }
            else
              {
                (*info->fprintf_func)(info->stream, " R%x, (R%x, R%x)",
                    (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7,
                    (raw_code >> 2) & 0x7);
              }
          }
        else
          {
            (*info->fprintf_func)(info->stream, " unhandled mode %s",
                opcodePTR->constraints);
          }
        break;
      case MC9XGATE_R_R:
        if(!strcmp(opcodePTR->constraints, MC9XGATE_OP_DYA_MON))
          {
            //(*info->fprintf_func) (info->stream, "\ttodo finish dynamic bit riper (dya-mon)");
            operandOne = ripBits(operMaskRegPTR, 3, opcodePTR, raw_code);
            operandTwo = ripBits(operMaskRegPTR, 3, opcodePTR, raw_code);
            (*info->fprintf_func)(info->stream, " R%x, R%x", operandOne, operandTwo);
          }
        else if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_DYA))
          {
            operandOne = ripBits(operMaskRegPTR, 3, opcodePTR, raw_code);
            operandTwo = ripBits(operMaskRegPTR, 3, opcodePTR, raw_code);
            (*info->fprintf_func)(info->stream, " R%x, R%x", operandOne, operandTwo);
          }
        else
          {
            (*info->fprintf_func)(info->stream, " unhandled mode %s",
                opcodePTR->constraints);
          }
        break;
      case MC9XGATE_R_R_I:
        (*info->fprintf_func) (info->stream, " R%x, (R%x, #0x%x)", (raw_code >> 8) & 0x7, (raw_code >> 5) & 0x7, raw_code & 0x1f );
        break;
      case MC9XGATE_R:
        operandOne = ripBits(operMaskRegPTR, 3, opcodePTR, raw_code);
        (*info->fprintf_func)(info->stream, " R%x", operandOne);
        break;
      case MC9XGATE_I | MC9XGATE_PCREL:
        if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_REL9))
          {
            /* if address is negative handle it accordingly */
            if (raw_code & MC9XGATE_NINE_MSB)
              {
                relAddr = MC9XGATE_NINE_BITS >> 1; //clip sign bit
                relAddr = ~relAddr; //make into a negative
                relAddr |= (raw_code & 0xFF) + 1; //apply our value
                relAddr <<= 1; //multiply by two as per xgate processor docs
              }
            else
              {
                relAddr = raw_code & 0xff;
                relAddr = (relAddr << 1) + 2;
              }
            (*info->fprintf_func)(info->stream, " *%d", relAddr);
            (*info->fprintf_func)(info->stream, "\tAbsolute: ");
            (*info->print_address_func)(memaddr + relAddr, info);
          }
        else if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_REL10))
          {
            /* if address is negative handle it accordingly */
            if (raw_code & MC9XGATE_TEN_MSB)
              {
                relAddr = MC9XGATE_TEN_BITS >> 1; //clip sign bit
                relAddr = ~relAddr; //make into a negative
                relAddr |= (raw_code & 0x1FF) + 1; //apply our value
                relAddr <<= 1; //multiply by two as per xgate processor docs
              }
            else
              {
                relAddr = raw_code & 0x1FF;
                relAddr = (relAddr << 1) + 2;
              }
            (*info->fprintf_func)(info->stream, " *%d", relAddr);
            (*info->fprintf_func)(info->stream, "\tAbsolute: ");
            (*info->print_address_func)(memaddr + relAddr, info);
          }
        else
          {
            (*info->fprintf_func)(info->stream,
                " Can't disassemble for mode) %s", opcodePTR->constraints);
          }
        break;
      case MC9XGATE_R_I:
        if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_IMM4))
          {
            (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
                (raw_code >> 8) & 0x7, (raw_code >> 4) & 0xF);
          }
        else if (!strcmp(opcodePTR->constraints, MC9XGATE_OP_IMM8))
          {
            (*info->fprintf_func)(info->stream, " R%x, #0x%02x",
                (raw_code >> 8) & 0x7, raw_code & 0xff);
          }
        else
          {
            (*info->fprintf_func)(info->stream,
                " Can't disassemble for mode) %s", opcodePTR->constraints);
          }
        break;
      case MC9XGATE_I:
        (*info->fprintf_func) (info->stream, " #0x%x", (raw_code >> 8) & 0x7 );
        break;
      default:
        (*info->fprintf_func)(info->stream, "address mode not found\t %x",
            opcodePTR->bin_opcode);
        break;
        }
  } else {
      printf("\n !!!! unable to find matching opcode !!!!!!!!");
  }
  if(status)
    printf("\n error reading memory");
  return bytesRead;
}

int
print_insn_mc9xgate (bfd_vma memaddr, struct disassemble_info* info)
{
  return print_insn (memaddr, info, cpumc9xgate);
}

static int
read_memory (bfd_vma memaddr, bfd_byte* buffer, int size,
    struct disassemble_info* info)
{
  int status;
  status = (*info->read_memory_func) (memaddr, buffer, size, info);
  if (status != 0)
    {
      (*info->memory_error_func) (status, memaddr, info);
      return -1;
    }
  return 0;
}

static int
ripBits(unsigned int *operandBitsRemaining, int numBitsRequested,
    struct mc9xgate_opcode *opcodePTR, unsigned int memory)
{
  unsigned int currentBit;
  int operand;
  int numBitsFound;
  for(operand = 0, numBitsFound = 0, currentBit = 1 << (opcodePTR->size * 8) - 1; (numBitsFound < numBitsRequested) && currentBit; currentBit >>= 1) {
      if(currentBit & *operandBitsRemaining) {
         *operandBitsRemaining &= ~(currentBit); /* consume the current bit */
         operand <<= 1; /* make room for our next bit */
         numBitsFound++;
         operand |= (currentBit & memory) > 0;
     }
  }
  return operand;
}
