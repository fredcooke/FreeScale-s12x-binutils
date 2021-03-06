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

#ifndef _ELF_MC9S12X_H
#define _ELF_MC9S12X_H

#include "elf/reloc-macros.h"

/* mc9s12x.h */

/* Relocation types.  */
START_RELOC_NUMBERS (elf_mc9s12x_reloc_type)
  RELOC_NUMBER (R_MC9S12X_NONE, 0)
  RELOC_NUMBER (R_MC9S12X_8, 1)
  RELOC_NUMBER (R_MC9S12X_HI8, 2)
  RELOC_NUMBER (R_MC9S12X_LO8, 3)
  RELOC_NUMBER (R_MC9S12X_PCREL_8, 4)
  RELOC_NUMBER (R_MC9S12X_16, 5)
  RELOC_NUMBER (R_MC9S12X_32, 6)
  RELOC_NUMBER (R_MC9S12X_3B, 7)
  RELOC_NUMBER (R_MC9S12X_PCREL_16, 8)

     /* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_MC9S12X_GNU_VTINHERIT, 9)
  RELOC_NUMBER (R_MC9S12X_GNU_VTENTRY, 10)

  RELOC_NUMBER (R_MC9S12X_24, 11)
  RELOC_NUMBER (R_MC9S12X_LO16, 12)
  RELOC_NUMBER (R_MC9S12X_PAGE, 13)
  RELOC_NUMBER (R_MC9XGATE_PCREL_9, 14)
  RELOC_NUMBER (R_MC9XGATE_PCREL_10, 15)
  RELOC_NUMBER (R_MC9XGATE_IMM8_LO, 16)
  RELOC_NUMBER (R_MC9XGATE_IMM8_HI, 17)
  RELOC_NUMBER (R_MC9XGATE_IMM3, 18)
  RELOC_NUMBER (R_MC9XGATE_IMM4, 19)

     /* GNU extension for linker relaxation.
        Mark beginning of a jump instruction (any form).  */
  RELOC_NUMBER (R_MC9S12X_RL_JUMP, 20)

     /* Mark beginning of Gcc relaxation group instruction.  */
  RELOC_NUMBER (R_MC9S12X_RL_GROUP, 21)
END_RELOC_NUMBERS (R_MC9S12X_max)

/* Processor specific flags for the ELF header e_flags field.  */

/* ABI identification.  */
#define EF_MC9S12X_ABI  0x00000000F

/* Integers are 32-bit long.  */
#define E_MC9S12X_I32   0x000000001

/* Doubles are 64-bit long.  */
#define E_MC9S12X_F64   0x000000002

/* Uses 68HC12 memory banks.  */
#define E_MC9S12X_BANKS 0x000000004

#define EF_MC9S12X_MACH_MASK 0xF0
#define EF_MC9S12X_GENERIC   0x00 /* Generic 68HC12/backward compatibility.  */
//#define EF_MC9S12X_MACH      0x10 /* 68HC12 microcontroller.  */
#define EF_MC9S12X_MACH     0x20 /* 68HCS12 microcontroller.  */
//#define EF_MC9S12X_MACH(mach) ((mach) & EF_MC9S12X_MACH_MASK)

/* True if we can merge machines.  A generic HC12 can work on any proc
   but once we have specific code, merge is not possible.  */

#define EF_MC9S12X_CAN_MERGE_MACH EF_MC9S12X_MACH   /* effectively disable merger since this is s12x ONLY */
//#define EF_MC9S12X_CAN_MERGE_MACH EF_MC9S12X_MACH
/*
#define EF_MC9S12X_CAN_MERGE_MACH(mach1, mach2) \
  ((EF_MC9S12X_MACH (mach1) == EF_MC9S12X_MACH (mach2)) \
   || (EF_MC9S12X_MACH (mach1) == EF_MC9S12X_GENERIC) \
   || (EF_MC9S12X_MACH (mach2) == EF_MC9S12X_GENERIC))

#define EF_MC9S12X_MERGE_MACH(mach1, mach2) \
  (((EF_MC9S12X_MACH (mach1) == EF_MC9S12X_MACH (mach2)) \
    || (EF_MC9S12X_MACH (mach1) == EF_MC9S12X_GENERIC)) ? \
      EF_MC9S12X_MACH (mach2) : EF_MC9S12X_MACH (mach1))
*/

/* Special values for the st_other field in the symbol table.  These
   are used for 68HC12 to identify far functions (must be called with
   'call' and returns with 'rtc').  */
#define STO_MC9S12X_FAR 0x80

/* Identify interrupt handlers.  This is used by the debugger to
   correctly compute the stack frame.  */
#define STO_MC9S12X_INTERRUPT 0x40
     
#endif
