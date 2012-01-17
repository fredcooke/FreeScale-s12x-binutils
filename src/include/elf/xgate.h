/* m68hc11 & m68hc12 ELF support for BFD.
   Copyright 1999, 2000, 2001, 2002 Free Software Foundation, Inc.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifndef _ELF_XGATE_H
#define _ELF_XGATE_H

#include "elf/reloc-macros.h"

/* Relocation types.  */
START_RELOC_NUMBERS (elf_xgate_reloc_type)
  RELOC_NUMBER (R_XGATE_NONE, 0)
  RELOC_NUMBER (R_XGATE_8, 1)
  RELOC_NUMBER (R_XGATE_HI8, 2)
  RELOC_NUMBER (R_XGATE_LO8, 3)
  RELOC_NUMBER (R_XGATE_PCREL_8, 4)
  RELOC_NUMBER (R_XGATE_16, 5)
  RELOC_NUMBER (R_XGATE_32, 6)
  RELOC_NUMBER (R_XGATE_3B, 7)
  RELOC_NUMBER (R_XGATE_PCREL_16, 8)

     /* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_XGATE_GNU_VTINHERIT, 9)
  RELOC_NUMBER (R_XGATE_GNU_VTENTRY, 10)

  RELOC_NUMBER (R_XGATE_24, 11)
  RELOC_NUMBER (R_XGATE_LO16, 12)
  RELOC_NUMBER (R_XGATE_PAGE, 13)
  RELOC_NUMBER (R_XGATE_PCREL_9, 14)
  RELOC_NUMBER (R_XGATE_PCREL_10, 15)
  RELOC_NUMBER (R_XGATE_IMM8_LO, 16)
  RELOC_NUMBER (R_XGATE_IMM8_HI, 17)
  RELOC_NUMBER (R_XGATE_IMM3, 18)
  RELOC_NUMBER (R_XGATE_IMM4, 19)
  RELOC_NUMBER (R_XGATE_IMM5, 20)

     /* GNU extension for linker relaxation.
        Mark beginning of a jump instruction (any form).  */
  RELOC_NUMBER (R_XGATE_RL_JUMP, 21)

     /* Mark beginning of Gcc relaxation group instruction.  */
  RELOC_NUMBER (R_XGATE_RL_GROUP, 22)
END_RELOC_NUMBERS (R_XGATE_max)

/* Processor specific flags for the ELF header e_flags field.  */

/* ABI identification.  */
#define EF_XGATE_ABI  0x00000000F

/* Integers are 32-bit long.  */
#define E_XGATE_I32   0x000000001

/* Doubles are 64-bit long.  */
#define E_XGATE_F64   0x000000002

/* Uses 68HC12 memory banks.  */
#define E_XGATE_BANKS 0x000000004

#define EF_XGATE_MACH_MASK 0xF0
#define EF_XGATE_GENERIC   0x00 /* Generic 68HC12/backward compatibility.  */
//#define EF_XGATE_MACH      0x10 /* 68HC12 microcontroller.  */
#define EF_XGATE_MACH     0x20 /* 68HCS12 microcontroller.  */
//#define EF_XGATE_MACH(mach) ((mach) & EF_XGATE_MACH_MASK)

/* True if we can merge machines.  A generic HC12 can work on any proc
   but once we have specific code, merge is not possible.  */

#define EF_XGATE_CAN_MERGE_MACH EF_XGATE_MACH   /* effectively disable merger since this is s12x ONLY */
//#define EF_XGATE_CAN_MERGE_MACH EF_XGATE_MACH
/*
#define EF_XGATE_CAN_MERGE_MACH(mach1, mach2) \
  ((EF_XGATE_MACH (mach1) == EF_XGATE_MACH (mach2)) \
   || (EF_XGATE_MACH (mach1) == EF_XGATE_GENERIC) \
   || (EF_XGATE_MACH (mach2) == EF_XGATE_GENERIC))

#define EF_XGATE_MERGE_MACH(mach1, mach2) \
  (((EF_XGATE_MACH (mach1) == EF_XGATE_MACH (mach2)) \
    || (EF_XGATE_MACH (mach1) == EF_XGATE_GENERIC)) ? \
      EF_XGATE_MACH (mach2) : EF_XGATE_MACH (mach1))
*/

/* Special values for the st_other field in the symbol table.  These
   are used for 68HC12 to identify far functions (must be called with
   'call' and returns with 'rtc').  */
#define STO_XGATE_FAR 0x80

/* Identify interrupt handlers.  This is used by the debugger to
   correctly compute the stack frame.  */
#define STO_XGATE_INTERRUPT 0x40
     
#endif