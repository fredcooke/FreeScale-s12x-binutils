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
//  RELOC_NUMBER (R_XGATE_HI8, 2)
//  RELOC_NUMBER (R_XGATE_LO8, 2)
  RELOC_NUMBER (R_XGATE_PCREL_8, 2)
  RELOC_NUMBER (R_XGATE_16, 3)
  RELOC_NUMBER (R_XGATE_32, 4)
  RELOC_NUMBER (R_XGATE_3B, 5)
  RELOC_NUMBER (R_XGATE_PCREL_16, 6)

     /* These are GNU extensions to enable C++ vtable garbage collection.  */
  RELOC_NUMBER (R_XGATE_GNU_VTINHERIT, 7)
  RELOC_NUMBER (R_XGATE_GNU_VTENTRY, 8)

  RELOC_NUMBER (R_XGATE_24, 9)
  RELOC_NUMBER (R_XGATE_LO16, 10)
  RELOC_NUMBER (R_XGATE_PAGE, 11)
  RELOC_NUMBER (R_XGATE_PCREL_9, 12)
  RELOC_NUMBER (R_XGATE_PCREL_10, 13)
  RELOC_NUMBER (R_XGATE_IMM8_LO, 14)
  RELOC_NUMBER (R_XGATE_IMM8_HI, 15)
  RELOC_NUMBER (R_XGATE_IMM3, 16)
  RELOC_NUMBER (R_XGATE_IMM4, 17)
  RELOC_NUMBER (R_XGATE_IMM5, 18)

     /* GNU extension for linker relaxation.
        Mark beginning of a jump instruction (any form).  */
  RELOC_NUMBER (R_XGATE_RL_JUMP, 19)

     /* Mark beginning of Gcc relaxation group instruction.  */
  RELOC_NUMBER (R_XGATE_RL_GROUP, 20)
END_RELOC_NUMBERS (R_XGATE_max)

/* Processor specific flags for the ELF header e_flags field.  */

/* ABI identification.  */
#define EF_XGATE_ABI  0x00000000F

/* Integers are 32-bit long.  */
#define E_XGATE_I32   0x000000001

/* Doubles are 64-bit long.  */
#define E_XGATE_F64   0x000000002



#define EF_XGATE_MACH_MASK 0xF0
#define EF_XGATE_GENERIC   0x00 /* Generic XGATE backward compatibility.  */
#define EF_XGATE_MACH     0x20 /* 68HCS12 microcontroller.  */

/* True if we can merge machines.  A generic HC12 can work on any proc
   but once we have specific code, merge is not possible.  */

#define EF_XGATE_CAN_MERGE_MACH EF_XGATE_MACH   /* effectively disable merger since this is s12x ONLY */

/* Special values for the st_other field in the symbol table.  These
   are used for 68HC12 to identify far functions (must be called with
   'call' and returns with 'rtc').  */
//#define STO_XGATE_FAR 0x80

/* Identify interrupt handlers.  This is used by the debugger to
   correctly compute the stack frame.  */
#define STO_XGATE_INTERRUPT 0x40
     
#endif
