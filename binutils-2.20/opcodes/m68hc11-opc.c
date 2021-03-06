/* m68hc11-opc.c -- Motorola 68HC11 & 68HC12 opcode list
   Copyright 1999, 2000, 2002, 2005, 2007 Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)

   S12X updates by Sean Keys(skeys@ipdatasys.com)

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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include "ansidecl.h"
#include "opcode/m68hc11.h"

#define TABLE_SIZE(X)       (sizeof(X) / sizeof(X[0]))

/* Combination of CCR flags.  */
#define M6811_ZC_BIT    M6811_Z_BIT|M6811_C_BIT
#define M6811_NZ_BIT    M6811_N_BIT|M6811_Z_BIT
#define M6811_NZV_BIT   M6811_N_BIT|M6811_Z_BIT|M6811_V_BIT
#define M6811_NZC_BIT   M6811_N_BIT|M6811_Z_BIT|M6811_C_BIT
#define M6811_NVC_BIT   M6811_N_BIT|M6811_V_BIT|M6811_C_BIT
#define M6811_ZVC_BIT   M6811_Z_BIT|M6811_V_BIT|M6811_C_BIT
#define M6811_NZVC_BIT  M6811_ZVC_BIT|M6811_N_BIT
#define M6811_HNZVC_BIT M6811_NZVC_BIT|M6811_H_BIT
#define M6811_HNVC_BIT  M6811_NVC_BIT|M6811_H_BIT
#define M6811_VC_BIT    M6811_V_BIT|M6811_C_BIT

/* Flags when the insn only changes some CCR flags.  */
#define CHG_NONE        0,0,0
#define CHG_Z           0,0,M6811_Z_BIT
#define CHG_C           0,0,M6811_C_BIT
#define CHG_ZVC         0,0,M6811_ZVC_BIT
#define CHG_NZC         0,0,M6811_NZC_BIT
#define CHG_NZV         0,0,M6811_NZV_BIT
#define CHG_NZVC        0,0,M6811_NZVC_BIT
#define CHG_HNZVC       0,0,M6811_HNZVC_BIT
#define CHG_ALL         0,0,0xff

/* The insn clears and changes some flags.  */
#define CLR_I           0,M6811_I_BIT,0
#define CLR_C           0,M6811_C_BIT,0
#define CLR_V           0,M6811_V_BIT,0
#define CLR_V_CHG_ZC    0,M6811_V_BIT,M6811_ZC_BIT
#define CLR_V_CHG_NZ    0,M6811_V_BIT,M6811_NZ_BIT
#define CLR_V_CHG_ZVC   0,M6811_V_BIT,M6811_ZVC_BIT
#define CLR_N_CHG_ZVC   0,M6811_N_BIT,M6811_ZVC_BIT /* Used by lsr */
#define CLR_VC_CHG_NZ   0,M6811_VC_BIT,M6811_NZ_BIT

/* The insn sets some flags.  */
#define SET_I           M6811_I_BIT,0,0
#define SET_C           M6811_C_BIT,0,0
#define SET_V           M6811_V_BIT,0,0
#define SET_Z_CLR_NVC   M6811_Z_BIT,M6811_NVC_BIT,0
#define SET_C_CLR_V_CHG_NZ M6811_C_BIT,M6811_V_BIT,M6811_NZ_BIT
#define SET_Z_CHG_HNVC  M6811_Z_BIT,0,M6811_HNVC_BIT

#define _M 0xff
#define OP_NONE         M6811_OP_NONE
#define OP_PAGE2        M6811_OP_PAGE2
#define OP_PAGE3        M6811_OP_PAGE3
#define OP_PAGE4        M6811_OP_PAGE4
#define OP_IMM8         M6811_OP_IMM8
#define OP_IMM16        M6811_OP_IMM16
#define OP_IX           M6811_OP_IX
#define OP_IY           M6811_OP_IY
#define OP_IND16        M6811_OP_IND16
#define OP_PAGE         M6812_OP_PAGE
#define OP_IDX          M6812_OP_IDX
#define OP_IDX_1        M6812_OP_IDX_1
#define OP_IDX_2        M6812_OP_IDX_2
#define OP_D_IDX        M6812_OP_D_IDX
#define OP_D_IDX_2      M6812_OP_D_IDX_2
#define OP_DIRECT       M6811_OP_DIRECT
#define OP_BITMASK      M6811_OP_BITMASK
#define OP_BRANCH       M6811_OP_BRANCH
#define OP_JUMP_REL     (M6811_OP_JUMP_REL|OP_BRANCH)
#define OP_JUMP_REL16   (M6812_OP_JUMP_REL16|OP_BRANCH)
#define OP_REG          M6812_OP_REG
#define OP_REG_1        M6812_OP_REG
#define OP_REG_2        M6812_OP_REG_2
#define OP_IDX_p2       M6812_OP_IDX_P2
#define OP_IND16_p2     M6812_OP_IND16_P2
#define OP_TRAP_ID      M6812_OP_TRAP_ID
#define OP_EXG_MARKER   M6812_OP_EXG_MARKER
#define OP_TFR_MARKER   M6812_OP_TFR_MARKER
#define OP_DBEQ_MARKER  (M6812_OP_DBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_DBNE_MARKER  (M6812_OP_DBCC_MARKER)
#define OP_TBEQ_MARKER  (M6812_OP_TBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_TBNE_MARKER  (M6812_OP_TBCC_MARKER)
#define OP_IBEQ_MARKER  (M6812_OP_IBCC_MARKER|M6812_OP_EQ_MARKER)
#define OP_IBNE_MARKER  (M6812_OP_IBCC_MARKER)

/*
   { "test", OP_NONE,          1, 0x00,  5, _M,  CHG_NONE,  cpu6811 },
                                                            +-- cpu
  Name -+                                        +------- Insn CCR changes
  Format  ------+                            +----------- Max # cycles
  Size     --------------------+         +--------------- Min # cycles
                                   +--------------------- Opcode
*/
const struct m68hc11_opcode m68hc11_opcodes[] = {
  { "aba",  OP_NONE,           1, 0x1b,  2,  2,  CHG_HNZVC, cpu6811 },
  { "aba",  OP_NONE | OP_PAGE2,2, 0x06,  2,  2,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "abx",  OP_NONE,           1, 0x3a,  3,  3,  CHG_NONE,  cpu6811 },
  { "aby",  OP_NONE | OP_PAGE2,2, 0x3a,  4,  4,  CHG_NONE,  cpu6811 },

  { "adca", OP_IMM8,           2, 0x89,  1,  1,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adca", OP_DIRECT,         2, 0x99,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adca", OP_IND16,          3, 0xb9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adca", OP_IX,             2, 0xa9,  4,  4,  CHG_HNZVC, cpu6811 },
  { "adca", OP_IY | OP_PAGE2,  3, 0xa9,  5,  5,  CHG_HNZVC, cpu6811 },
  { "adca", OP_IDX,            2, 0xa9,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adca", OP_IDX_1,          3, 0xa9,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adca", OP_IDX_2,          4, 0xa9,  4,  4,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adca", OP_D_IDX,          2, 0xa9,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adca", OP_D_IDX_2,        4, 0xa9,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "adcb", OP_IMM8,           2, 0xc9,  1,  1,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adcb", OP_DIRECT,         2, 0xd9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adcb", OP_IND16,          3, 0xf9,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adcb", OP_IX,             2, 0xe9,  4,  4,  CHG_HNZVC, cpu6811 },
  { "adcb", OP_IY | OP_PAGE2,  3, 0xe9,  5,  5,  CHG_HNZVC, cpu6811 },
  { "adcb", OP_IDX,            2, 0xe9,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adcb", OP_IDX_1,          3, 0xe9,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adcb", OP_IDX_2,          4, 0xe9,  4,  4,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adcb", OP_D_IDX,          2, 0xe9,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adcb", OP_D_IDX_2,        4, 0xe9,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "adda", OP_IMM8,           2, 0x8b,  1,  1,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adda", OP_DIRECT,         2, 0x9b,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adda", OP_IND16,          3, 0xbb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "adda", OP_IX,             2, 0xab,  4,  4,  CHG_HNZVC, cpu6811 },
  { "adda", OP_IY | OP_PAGE2,  3, 0xab,  5,  5,  CHG_HNZVC, cpu6811 },
  { "adda", OP_IDX,            2, 0xab,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adda", OP_IDX_1,          3, 0xab,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adda", OP_IDX_2,          4, 0xab,  4,  4,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adda", OP_D_IDX,          2, 0xab,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "adda", OP_D_IDX_2,        4, 0xab,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "addb", OP_IMM8,           2, 0xcb,  1,  1,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "addb", OP_DIRECT,         2, 0xdb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "addb", OP_IND16,          3, 0xfb,  3,  3,  CHG_HNZVC, cpu6811|cpu6812 | cpu6812x },
  { "addb", OP_IX,             2, 0xeb,  4,  4,  CHG_HNZVC, cpu6811 },
  { "addb", OP_IY | OP_PAGE2,  3, 0xeb,  5,  5,  CHG_HNZVC, cpu6811 },
  { "addb", OP_IDX,            2, 0xeb,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "addb", OP_IDX_1,          3, 0xeb,  3,  3,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "addb", OP_IDX_2,          4, 0xeb,  4,  4,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "addb", OP_D_IDX,          2, 0xeb,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "addb", OP_D_IDX_2,        4, 0xeb,  6,  6,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "addd", OP_IMM16,          3, 0xc3,  2,  2,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "addd", OP_DIRECT,         2, 0xd3,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "addd", OP_IND16,          3, 0xf3,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "addd", OP_IX,             2, 0xe3,  6,  6,  CHG_NZVC, cpu6811 },
  { "addd", OP_IY | OP_PAGE2,  3, 0xe3,  7,  7,  CHG_NZVC, cpu6811 },
  { "addd", OP_IDX,            2, 0xe3,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addd", OP_IDX_1,          3, 0xe3,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addd", OP_IDX_2,          4, 0xe3,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addd", OP_D_IDX,          2, 0xe3,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addd", OP_D_IDX_2,        4, 0xe3,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "addx", OP_IMM16,          3, 0x8b,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_DIRECT,         2, 0x9b,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IND16,          3, 0xbb,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IX,             2, 0xab,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IY | OP_PAGE2,  3, 0xab,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IDX,            2, 0xab,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IDX_1,          3, 0xab,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_IDX_2,          4, 0xab,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_D_IDX,          2, 0xab,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "addx", OP_D_IDX_2,        4, 0xab,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "andy", OP_IMM16 | OP_PAGE2,         2, 0xc4,  1,  1,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_DIRECT | OP_PAGE2,       2, 0xd4,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_IND16 | OP_PAGE2,        3, 0xf4,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_IDX | OP_PAGE2,            2, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_IDX_1 | OP_PAGE2,          3, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_IDX_2 | OP_PAGE2,          4, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_D_IDX | OP_PAGE2,          2, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "andy", OP_D_IDX_2 | OP_PAGE2,        4, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812x},



  { "anda", OP_IMM8,         2, 0x84,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "anda", OP_DIRECT,       2, 0x94,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "anda", OP_IND16,        3, 0xb4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "anda", OP_IX,             2, 0xa4,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "anda", OP_IY | OP_PAGE2,  3, 0xa4,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "anda", OP_IDX,            2, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "anda", OP_IDX_1,          3, 0xa4,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "anda", OP_IDX_2,          4, 0xa4,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "anda", OP_D_IDX,          2, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "anda", OP_D_IDX_2,        4, 0xa4,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "andb", OP_IMM8,         2, 0xc4,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "andb", OP_DIRECT,       2, 0xd4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "andb", OP_IND16,        3, 0xf4,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "andb", OP_IX,             2, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "andb", OP_IY | OP_PAGE2,  3, 0xe4,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "andb", OP_IDX,            2, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "andb", OP_IDX_1,          3, 0xe4,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "andb", OP_IDX_2,          4, 0xe4,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "andb", OP_D_IDX,          2, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "andb", OP_D_IDX_2,        4, 0xe4,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "andcc", OP_IMM8,          2, 0x10,  1,  1,  CHG_ALL,  cpu6812 | cpu6812x },

  { "asl",  OP_IND16,          3, 0x78,  4,  4,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "asl",  OP_IX,             2, 0x68,  6,  6,  CHG_NZVC, cpu6811 },
  { "asl",  OP_IY | OP_PAGE2,  3, 0x68,  7,  7,  CHG_NZVC, cpu6811 },
  { "asl",  OP_IDX,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asl",  OP_IDX_1,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asl",  OP_IDX_2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asl",  OP_D_IDX,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asl",  OP_D_IDX_2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "asla", OP_NONE,           1, 0x48,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "aslb", OP_NONE,           1, 0x58,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "asld", OP_NONE,           1, 0x05,  3,  3,  CHG_NZVC, cpu6811 },
  { "asld", OP_NONE,           1, 0x59,  1,  1,  CHG_NZVC, cpu6812 | cpu6812x },

  { "aslw",  OP_IND16 | OP_PAGE2,          3, 0x78,  4,  4,  CHG_NZVC, cpu6812x},
  { "aslw",  OP_IDX | OP_PAGE2,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812x},
  { "aslw",  OP_IDX_1 | OP_PAGE2,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812x},
  { "aslw",  OP_IDX_2 | OP_PAGE2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812x},
  { "aslw",  OP_D_IDX | OP_PAGE2,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812x},
  { "aslw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812x},

  { "aslx", OP_NONE | OP_PAGE2,           1, 0x48,  1,  1,  CHG_NZVC, cpu6812x},

  { "asly", OP_NONE | OP_PAGE2,           1, 0x58,  1,  1,  CHG_NZVC, cpu6812x},

  { "asr",  OP_IND16,          3, 0x77,  4,  4,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "asr",  OP_IX,             2, 0x67,  6,  6,  CHG_NZVC, cpu6811 },
  { "asr",  OP_IY | OP_PAGE2,  3, 0x67,  7,  7,  CHG_NZVC, cpu6811 },
  { "asr",  OP_IDX,            2, 0x67,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asr",  OP_IDX_1,          3, 0x67,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asr",  OP_IDX_2,          4, 0x67,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asr",  OP_D_IDX,          2, 0x67,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "asr",  OP_D_IDX_2,        4, 0x67,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "asra", OP_NONE,           1, 0x47,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "asrb", OP_NONE,           1, 0x57,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },

  { "asrw",  OP_IND16 | OP_PAGE2,          3, 0x77,  4,  4,  CHG_NZVC, cpu6812x},
  { "asrw",  OP_IDX | OP_PAGE2,            2, 0x67,  3,  3,  CHG_NZVC, cpu6812x},
  { "asrw",  OP_IDX_1 | OP_PAGE2,          3, 0x67,  4,  4,  CHG_NZVC, cpu6812x},
  { "asrw",  OP_IDX_2 | OP_PAGE2,          4, 0x67,  5,  5,  CHG_NZVC, cpu6812x},
  { "asrw",  OP_D_IDX | OP_PAGE2,          2, 0x67,  6,  6,  CHG_NZVC, cpu6812x},
  { "asrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x67,  6,  6,  CHG_NZVC, cpu6812x},

  { "asrx", OP_NONE | OP_PAGE2,           1, 0x47,  1,  1,  CHG_NZVC, cpu6812x},

  { "asry", OP_NONE | OP_PAGE2,           1, 0x57,  1,  1,  CHG_NZVC, cpu6812x},

  { "bcc", OP_JUMP_REL,        2, 0x24,  1,  3,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },

  { "bclr", OP_BITMASK|OP_DIRECT,  3, 0x15,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "bclr", OP_BITMASK|OP_IX,       3, 0x1d,  7,  7,  CLR_V_CHG_NZ, cpu6811 },
  { "bclr", OP_BITMASK|OP_IY|OP_PAGE2, 4, 0x1d, 8, 8, CLR_V_CHG_NZ, cpu6811},
  { "bclr", OP_BITMASK|OP_DIRECT,   3, 0x4d,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bclr", OP_BITMASK|OP_IND16,    4, 0x1d,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bclr", OP_BITMASK|OP_IDX,      3, 0x0d,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bclr", OP_BITMASK|OP_IDX_1,    4, 0x0d,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bclr", OP_BITMASK|OP_IDX_2,    5, 0x0d,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "bcs", OP_JUMP_REL,        2, 0x25,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "beq", OP_JUMP_REL,        2, 0x27,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bge", OP_JUMP_REL,        2, 0x2c,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },

  { "bgnd", OP_NONE,           1, 0x00,  5,  5, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },

  { "bgt", OP_JUMP_REL,        2, 0x2e,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bhi", OP_JUMP_REL,        2, 0x22,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bhs", OP_JUMP_REL,        2, 0x24,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  
  { "bita", OP_IMM8,          2, 0x85,  1,  1, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bita", OP_DIRECT,        2, 0x95,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bita", OP_IND16,         3, 0xb5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bita", OP_IX,             2, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "bita", OP_IY | OP_PAGE2,  3, 0xa5,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "bita", OP_IDX,            2, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bita", OP_IDX_1,          3, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bita", OP_IDX_2,          4, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bita", OP_D_IDX,          2, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bita", OP_D_IDX_2,        4, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "bitb", OP_IMM8,          2, 0xc5,  1,  1, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bitb", OP_DIRECT,        2, 0xd5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bitb", OP_IND16,         3, 0xf5,  3,  3, CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "bitb", OP_IX,             2, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "bitb", OP_IY | OP_PAGE2,  3, 0xe5,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "bitb", OP_IDX,            2, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bitb", OP_IDX_1,          3, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bitb", OP_IDX_2,          4, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bitb", OP_D_IDX,          2, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bitb", OP_D_IDX_2,        4, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "bitx", OP_IMM16 | OP_PAGE2,          2, 0x85,  1,  1, CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_DIRECT | OP_PAGE2,        2, 0x95,  3,  3, CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_IND16 | OP_PAGE2,         3, 0xb5,  3,  3, CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_IDX | OP_PAGE2,            2, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_IDX_1 | OP_PAGE2,          3, 0xa5,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_IDX_2 | OP_PAGE2,          4, 0xa5,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_D_IDX | OP_PAGE2,          2, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "bitx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa5,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

  { "bity", OP_IMM16 | OP_PAGE2,          2, 0xc5,  1,  1, CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_DIRECT | OP_PAGE2,        2, 0xd5,  3,  3, CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_IND16 | OP_PAGE2,         3, 0xf5,  3,  3, CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_IDX | OP_PAGE2,            2, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_IDX_1 | OP_PAGE2,          3, 0xe5,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_IDX_2 | OP_PAGE2,          4, 0xe5,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_D_IDX | OP_PAGE2,          2, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "bity", OP_D_IDX_2 | OP_PAGE2,        4, 0xe5,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

  { "ble", OP_JUMP_REL,        2, 0x2f,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "blo", OP_JUMP_REL,        2, 0x25,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bls", OP_JUMP_REL,        2, 0x23,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "blt", OP_JUMP_REL,        2, 0x2d,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bmi", OP_JUMP_REL,        2, 0x2b,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bne", OP_JUMP_REL,        2, 0x26,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bpl", OP_JUMP_REL,        2, 0x2a,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bra", OP_JUMP_REL,        2, 0x20,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },

  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x13,  6,  6, CHG_NONE, cpu6811 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IX,             4, 0x1f,  7,  7, CHG_NONE, cpu6811 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IY | OP_PAGE2,  5, 0x1f,  8,  8, CHG_NONE, cpu6811 },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x4f,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IND16,          5, 0x1f,  5,  5,  CHG_NONE, cpu6812 | cpu6812x },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IDX,            4, 0x0f,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "brclr", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_1,          5, 0x0f,  6,  6,  CHG_NONE, cpu6812 | cpu6812x },
  { "brclr", OP_BITMASK
           | OP_JUMP_REL
           | OP_IDX_2,          6, 0x0f,  8,  8,  CHG_NONE, cpu6812 | cpu6812x },

  { "brn", OP_JUMP_REL,         2, 0x21,  1,  3,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },

  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,         4, 0x12,  6,  6,  CHG_NONE, cpu6811 },
  { "brset", OP_BITMASK
           | OP_JUMP_REL
           | OP_IX,             4, 0x1e,  7,  7,  CHG_NONE, cpu6811 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IY | OP_PAGE2,  5, 0x1e,  8,  8,  CHG_NONE, cpu6811 },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_DIRECT,   4, 0x4e,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IND16,    5, 0x1e,  5,  5,  CHG_NONE, cpu6812 | cpu6812x },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX,            4, 0x0e,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_1,          5, 0x0e,  6,  6,  CHG_NONE, cpu6812 | cpu6812x },
  { "brset", OP_BITMASK | OP_JUMP_REL
           | OP_IDX_2,          6, 0x0e,  8,  8,  CHG_NONE, cpu6812 | cpu6812x },


  { "bset", OP_BITMASK | OP_DIRECT,   3, 0x14,  6,  6, CLR_V_CHG_NZ, cpu6811 },
  { "bset", OP_BITMASK | OP_IX,       3, 0x1c,  7,  7, CLR_V_CHG_NZ, cpu6811 },
  { "bset", OP_BITMASK|OP_IY|OP_PAGE2, 4, 0x1c, 8, 8, CLR_V_CHG_NZ, cpu6811 },
  { "bset", OP_BITMASK|OP_DIRECT,   3, 0x4c,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bset", OP_BITMASK|OP_IND16,    4, 0x1c,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bset", OP_BITMASK|OP_IDX,      3, 0x0c,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bset", OP_BITMASK|OP_IDX_1,    4, 0x0c,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "bset", OP_BITMASK|OP_IDX_2,    5, 0x0c,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "bsr",  OP_JUMP_REL,       2, 0x8d,  6,  6, CHG_NONE, cpu6811 },
  { "bsr",  OP_JUMP_REL,       2, 0x07,  4,  4, CHG_NONE, cpu6812 | cpu6812x },

  { "btas", OP_BITMASK|OP_DIRECT | OP_PAGE2,   3, 0x35,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "btas", OP_BITMASK|OP_IND16 | OP_PAGE2,    4, 0x36,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "btas", OP_BITMASK|OP_IDX | OP_PAGE2,      3, 0x37,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "btas", OP_BITMASK|OP_IDX_1 | OP_PAGE2,    4, 0x37,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "btas", OP_BITMASK|OP_IDX_2 | OP_PAGE2,    5, 0x37,  6,  6,  CLR_V_CHG_NZ, cpu6812x},


  { "bvc",  OP_JUMP_REL,       2, 0x28,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },
  { "bvs",  OP_JUMP_REL,       2, 0x29,  1,  3, CHG_NONE, cpu6811 | cpu6812 | cpu6812x },

  { "call", OP_IND16 | OP_PAGE
          | OP_BRANCH,         4, 0x4a,  8,  8,  CHG_NONE, cpu6812 | cpu6812x },
  { "call", OP_IDX | OP_PAGE
          | OP_BRANCH,         3, 0x4b,  8,  8,  CHG_NONE, cpu6812 | cpu6812x },
  { "call", OP_IDX_1 | OP_PAGE
          | OP_BRANCH,         4, 0x4b,  8,  8,  CHG_NONE, cpu6812 | cpu6812x },
  { "call", OP_IDX_2 | OP_PAGE
          | OP_BRANCH,         5, 0x4b,  9,  9,  CHG_NONE, cpu6812 | cpu6812x },
  { "call", OP_D_IDX
          | OP_BRANCH,         2, 0x4b, 10, 10,  CHG_NONE, cpu6812 | cpu6812x },
  { "call", OP_D_IDX_2
          | OP_BRANCH,         4, 0x4b, 10, 10,  CHG_NONE, cpu6812 | cpu6812x },

  { "cba",  OP_NONE,           1, 0x11,  2,  2,  CHG_NZVC, cpu6811 },
  { "cba",  OP_NONE | OP_PAGE2,2, 0x17,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },

  { "clc",  OP_NONE,           1, 0x0c,  2,  2,  CLR_C, cpu6811 },
  { "cli",  OP_NONE,           1, 0x0e,  2,  2,  CLR_I, cpu6811 },

  { "clr", OP_IND16,           3, 0x7f,  6,  6,  SET_Z_CLR_NVC, cpu6811 },
  { "clr", OP_IX,              2, 0x6f,  6,  6,  SET_Z_CLR_NVC, cpu6811 },
  { "clr", OP_IY | OP_PAGE2,   3, 0x6f,  7,  7,  SET_Z_CLR_NVC, cpu6811 },
  { "clr", OP_IND16,           3, 0x79,  3,  3,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clr", OP_IDX,             2, 0x69,  2,  2,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clr", OP_IDX_1,           3, 0x69,  3,  3,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clr", OP_IDX_2,           4, 0x69,  4,  4,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clr", OP_D_IDX,           2, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clr", OP_D_IDX_2,         4, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },

  { "clra", OP_NONE,           1, 0x4f,  2,  2,  SET_Z_CLR_NVC, cpu6811 },
  { "clrb", OP_NONE,           1, 0x5f,  2,  2,  SET_Z_CLR_NVC, cpu6811 },
  { "clra", OP_NONE,           1, 0x87,  1,  1,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },
  { "clrb", OP_NONE,           1, 0xc7,  1,  1,  SET_Z_CLR_NVC, cpu6812 | cpu6812x },

  { "clrw",  OP_IND16 | OP_PAGE2,          3, 0x79,  4,  4,  SET_Z_CLR_NVC, cpu6812x},
  { "clrw",  OP_IDX | OP_PAGE2,            2, 0x69,  3,  3,  SET_Z_CLR_NVC, cpu6812x},
  { "clrw",  OP_IDX_1 | OP_PAGE2,          3, 0x69,  4,  4,  SET_Z_CLR_NVC, cpu6812x},
  { "clrw",  OP_IDX_2 | OP_PAGE2,          4, 0x69,  5,  5,  SET_Z_CLR_NVC, cpu6812x},
  { "clrw",  OP_D_IDX | OP_PAGE2,          2, 0x69,  6,  6,  SET_Z_CLR_NVC, cpu6812x},
  { "clrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x69,  6,  6,  SET_Z_CLR_NVC, cpu6812x},

  { "clrx",  OP_NONE | OP_PAGE2,          3, 0x87,  4,  4,  SET_Z_CLR_NVC, cpu6812x},

  { "clry",  OP_NONE | OP_PAGE2,          3, 0xc7,  4,  4,  SET_Z_CLR_NVC, cpu6812x},

  { "clv",  OP_NONE,           1, 0x0a,  2,  2,  CLR_V, cpu6811 },

  { "cmpa", OP_IMM8,           2, 0x81,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpa", OP_DIRECT,         2, 0x91,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpa", OP_IND16,          3, 0xb1,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpa", OP_IX,             2, 0xa1,  4,  4,  CHG_NZVC, cpu6811 },
  { "cmpa", OP_IY | OP_PAGE2,  3, 0xa1,  5,  5,  CHG_NZVC, cpu6811 },
  { "cmpa", OP_IDX,            2, 0xa1,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpa", OP_IDX_1,          3, 0xa1,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpa", OP_IDX_2,          4, 0xa1,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpa", OP_D_IDX,          2, 0xa1,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpa", OP_D_IDX_2,        4, 0xa1,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "cmpb", OP_IMM8,           2, 0xc1,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpb", OP_DIRECT,         2, 0xd1,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpb", OP_IND16,          3, 0xf1,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "cmpb", OP_IX,             2, 0xe1,  4,  4,  CHG_NZVC, cpu6811 },
  { "cmpb", OP_IY | OP_PAGE2,  3, 0xe1,  5,  5,  CHG_NZVC, cpu6811 },
  { "cmpb", OP_IDX,            2, 0xe1,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpb", OP_IDX_1,          3, 0xe1,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpb", OP_IDX_2,          4, 0xe1,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpb", OP_D_IDX,          2, 0xe1,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cmpb", OP_D_IDX_2,        4, 0xe1,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "com", OP_IND16,           3, 0x73,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6811 },
  { "com", OP_IX,              2, 0x63,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6811 },
  { "com", OP_IY | OP_PAGE2,   3, 0x63,  7,  7,  SET_C_CLR_V_CHG_NZ, cpu6811 },
  { "com", OP_IND16,           3, 0x71,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "com", OP_IDX,             2, 0x61,  3,  3,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "com", OP_IDX_1,           3, 0x61,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "com", OP_IDX_2,           4, 0x61,  5,  5,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "com", OP_D_IDX,           2, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "com", OP_D_IDX_2,         4, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "coma", OP_NONE,           1, 0x43,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6811 },
  { "coma", OP_NONE,           1, 0x41,  1,  1,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "comb", OP_NONE,           1, 0x53,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6811 },
  { "comb", OP_NONE,           1, 0x51,  1,  1,  SET_C_CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "comw",  OP_IND16 | OP_PAGE2,          3, 0x71,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812x},
  { "comw",  OP_IDX | OP_PAGE2,            2, 0x61,  3,  3,  SET_C_CLR_V_CHG_NZ, cpu6812x},
  { "comw",  OP_IDX_1 | OP_PAGE2,          3, 0x61,  4,  4,  SET_C_CLR_V_CHG_NZ, cpu6812x},
  { "comw",  OP_IDX_2 | OP_PAGE2,          4, 0x61,  5,  5,  SET_C_CLR_V_CHG_NZ, cpu6812x},
  { "comw",  OP_D_IDX | OP_PAGE2,          2, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812x},
  { "comw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x61,  6,  6,  SET_C_CLR_V_CHG_NZ, cpu6812x},

  { "comx", OP_NONE | OP_PAGE2,           1, 0x41,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6812x},

  { "comy", OP_NONE | OP_PAGE2,           1, 0x51,  2,  2,  SET_C_CLR_V_CHG_NZ, cpu6812x},


  { "cpd", OP_IMM16 | OP_PAGE3,  4, 0x83,  5,  5,  CHG_NZVC, cpu6811 },
  { "cpd", OP_DIRECT | OP_PAGE3, 3, 0x93,  6,  6,  CHG_NZVC, cpu6811 },
  { "cpd", OP_IND16 | OP_PAGE3,  4, 0xb3,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpd", OP_IX | OP_PAGE3,     3, 0xa3,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpd", OP_IY | OP_PAGE4,     3, 0xa3,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpd", OP_IMM16,             3, 0x8c,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_DIRECT,            2, 0x9c,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_IND16,             3, 0xbc,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_IDX,               2, 0xac,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_IDX_1,             3, 0xac,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_IDX_2,             4, 0xac,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_D_IDX,             2, 0xac,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpd", OP_D_IDX_2,           4, 0xac,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "cped", OP_IMM16 | OP_PAGE2,             3, 0x8c,  2,  2,  CHG_NZVC, cpu6812x},
  { "cped", OP_DIRECT | OP_PAGE2,            2, 0x9c,  3,  3,  CHG_NZVC, cpu6812x},
  { "cped", OP_IND16 | OP_PAGE2,             3, 0xbc,  3,  3,  CHG_NZVC, cpu6812x},
  { "cped", OP_IDX | OP_PAGE2,               2, 0xac,  3,  3,  CHG_NZVC, cpu6812x},
  { "cped", OP_IDX_1 | OP_PAGE2,             3, 0xac,  3,  3,  CHG_NZVC, cpu6812x},
  { "cped", OP_IDX_2 | OP_PAGE2,             4, 0xac,  4,  4,  CHG_NZVC, cpu6812x},
  { "cped", OP_D_IDX | OP_PAGE2,             2, 0xac,  6,  6,  CHG_NZVC, cpu6812x},
  { "cped", OP_D_IDX_2 | OP_PAGE2,           4, 0xac,  6,  6,  CHG_NZVC, cpu6812x},

  { "cpes", OP_IMM16 | OP_PAGE2,             3, 0x8f,  2,  2,  CHG_NZVC, cpu6812x},
  { "cpes", OP_DIRECT | OP_PAGE2,            2, 0x9f,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpes", OP_IND16 | OP_PAGE2,             3, 0xbf,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpes", OP_IDX | OP_PAGE2,               2, 0xaf,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpes", OP_IDX_1 | OP_PAGE2,             3, 0xaf,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpes", OP_IDX_2 | OP_PAGE2,             4, 0xaf,  4,  4,  CHG_NZVC, cpu6812x},
  { "cpes", OP_D_IDX | OP_PAGE2,             2, 0xaf,  6,  6,  CHG_NZVC, cpu6812x},
  { "cpes", OP_D_IDX_2 | OP_PAGE2,           4, 0xaf,  6,  6,  CHG_NZVC, cpu6812x},

  { "cpex", OP_IMM16 | OP_PAGE2,             3, 0x8e,  2,  2,  CHG_NZVC, cpu6812x},
  { "cpex", OP_DIRECT | OP_PAGE2,            2, 0x9e,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpex", OP_IND16 | OP_PAGE2,             3, 0xbe,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpex", OP_IDX | OP_PAGE2,               2, 0xae,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpex", OP_IDX_1 | OP_PAGE2,             3, 0xae,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpex", OP_IDX_2 | OP_PAGE2,             4, 0xae,  4,  4,  CHG_NZVC, cpu6812x},
  { "cpex", OP_D_IDX | OP_PAGE2,             2, 0xae,  6,  6,  CHG_NZVC, cpu6812x},
  { "cpex", OP_D_IDX_2 | OP_PAGE2,           4, 0xae,  6,  6,  CHG_NZVC, cpu6812x},

  { "cpey", OP_IMM16 | OP_PAGE2,             3, 0x8d,  2,  2,  CHG_NZVC, cpu6812x},
  { "cpey", OP_DIRECT | OP_PAGE2,            2, 0x9d,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpey", OP_IND16 | OP_PAGE2,             3, 0xbd,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpey", OP_IDX | OP_PAGE2,               2, 0xad,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpey", OP_IDX_1 | OP_PAGE2,             3, 0xad,  3,  3,  CHG_NZVC, cpu6812x},
  { "cpey", OP_IDX_2 | OP_PAGE2,             4, 0xad,  4,  4,  CHG_NZVC, cpu6812x},
  { "cpey", OP_D_IDX | OP_PAGE2,             2, 0xad,  6,  6,  CHG_NZVC, cpu6812x},
  { "cpey", OP_D_IDX_2 | OP_PAGE2,           4, 0xad,  6,  6,  CHG_NZVC, cpu6812x},


  { "cps", OP_IMM16,             3, 0x8f,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_DIRECT,            2, 0x9f,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_IND16,             3, 0xbf,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_IDX,               2, 0xaf,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_IDX_1,             3, 0xaf,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_IDX_2,             4, 0xaf,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_D_IDX,             2, 0xaf,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cps", OP_D_IDX_2,           4, 0xaf,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "cpx", OP_IMM16,             3, 0x8c,  4,  4,  CHG_NZVC, cpu6811 },
  { "cpx", OP_DIRECT,            2, 0x9c,  5,  5,  CHG_NZVC, cpu6811 },
  { "cpx", OP_IND16,             3, 0xbc,  5,  5,  CHG_NZVC, cpu6811 },
  { "cpx", OP_IX,                2, 0xac,  6,  6,  CHG_NZVC, cpu6811 },
  { "cpx", OP_IY | OP_PAGE4,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpx", OP_IMM16,             3, 0x8e,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_DIRECT,            2, 0x9e,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_IND16,             3, 0xbe,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_IDX,               2, 0xae,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_IDX_1,             3, 0xae,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_IDX_2,             4, 0xae,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_D_IDX,             2, 0xae,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpx", OP_D_IDX_2,           4, 0xae,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "cpy", OP_PAGE2 | OP_IMM16,  4, 0x8c,  5,  5,  CHG_NZVC, cpu6811 },
  { "cpy", OP_PAGE2 | OP_DIRECT, 3, 0x9c,  6,  6,  CHG_NZVC, cpu6811 },
  { "cpy", OP_PAGE2 | OP_IY,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpy", OP_PAGE2 | OP_IND16,  4, 0xbc,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpy", OP_PAGE3 | OP_IX,     3, 0xac,  7,  7,  CHG_NZVC, cpu6811 },
  { "cpy", OP_IMM16,             3, 0x8d,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_DIRECT,            2, 0x9d,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_IND16,             3, 0xbd,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_IDX,               2, 0xad,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_IDX_1,             3, 0xad,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_IDX_2,             4, 0xad,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_D_IDX,             2, 0xad,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "cpy", OP_D_IDX_2,           4, 0xad,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  /* After 'daa', the Z flag is undefined. Mark it as changed.  */
  { "daa", OP_NONE,              1, 0x19,  2,  2,  CHG_NZVC, cpu6811 },
  { "daa", OP_NONE | OP_PAGE2,  2, 0x07,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },

  { "dbeq", OP_DBEQ_MARKER
          | OP_REG | OP_JUMP_REL,3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },
  { "dbne", OP_DBNE_MARKER
          | OP_REG | OP_JUMP_REL,3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },

  { "dec", OP_IX,                2, 0x6a,  6,  6,  CHG_NZV, cpu6811 },
  { "dec", OP_IND16,             3, 0x7a,  6,  6,  CHG_NZV, cpu6811 },
  { "dec", OP_IY | OP_PAGE2,     3, 0x6a,  7,  7,  CHG_NZV, cpu6811 },
  { "dec", OP_IND16,             3, 0x73,  4,  4,  CHG_NZV, cpu6812 | cpu6812x },
  { "dec", OP_IDX,               2, 0x63,  3,  3,  CHG_NZV, cpu6812 | cpu6812x },
  { "dec", OP_IDX_1,             3, 0x63,  4,  4,  CHG_NZV, cpu6812 | cpu6812x },
  { "dec", OP_IDX_2,             4, 0x63,  5,  5,  CHG_NZV, cpu6812 | cpu6812x },
  { "dec", OP_D_IDX,             2, 0x63,  6,  6,  CHG_NZV, cpu6812 | cpu6812x },
  { "dec", OP_D_IDX_2,           4, 0x63,  6,  6,  CHG_NZV, cpu6812 | cpu6812x },

  { "des",  OP_NONE,             1, 0x34,  3,  3,  CHG_NONE, cpu6811 },

  { "deca", OP_NONE,             1, 0x4a,  2,  2,  CHG_NZV, cpu6811 },
  { "deca", OP_NONE,             1, 0x43,  1,  1,  CHG_NZV, cpu6812 | cpu6812x },
  { "decb", OP_NONE,             1, 0x5a,  2,  2,  CHG_NZV, cpu6811 },
  { "decb", OP_NONE,             1, 0x53,  1,  1,  CHG_NZV, cpu6812 | cpu6812x },

  { "decw",  OP_IND16 | OP_PAGE2,          3, 0x73,  4,  4,  CHG_NZV, cpu6812x},
  { "decw",  OP_IDX | OP_PAGE2,            2, 0x63,  3,  3,  CHG_NZV, cpu6812x},
  { "decw",  OP_IDX_1 | OP_PAGE2,          3, 0x63,  4,  4,  CHG_NZV, cpu6812x},
  { "decw",  OP_IDX_2 | OP_PAGE2,          4, 0x63,  5,  5,  CHG_NZV, cpu6812x},
  { "decw",  OP_D_IDX | OP_PAGE2,          2, 0x63,  6,  6,  CHG_NZV, cpu6812x},
  { "decw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x63,  6,  6,  CHG_NZV, cpu6812x},

  { "decx",  OP_NONE | OP_PAGE2,          3, 0x43,  4,  4,  CHG_NZV, cpu6812x},

  { "decy",  OP_NONE | OP_PAGE2,          3, 0x53,  4,  4,  CHG_NZV, cpu6812x},


  { "dex",  OP_NONE,             1, 0x09,  1,  1,  CHG_Z, cpu6812 | cpu6812x|cpu6811 },
  { "dey",  OP_NONE | OP_PAGE2,  2, 0x09,  4,  4,  CHG_Z, cpu6811 },
  { "dey",  OP_NONE,             1, 0x03,  1,  1,  CHG_Z, cpu6812 | cpu6812x },

  { "ediv", OP_NONE,             1, 0x11,  11,  11,  CHG_NZVC, cpu6812 | cpu6812x },
  { "edivs", OP_NONE | OP_PAGE2, 2, 0x14,  12,  12,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emacs", OP_IND16 | OP_PAGE2, 4, 0x12,  13,  13,  CHG_NZVC, cpu6812 | cpu6812x },

  { "emaxd", OP_IDX | OP_PAGE2,     3, 0x1a,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxd", OP_IDX_1 | OP_PAGE2,   4, 0x1a,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxd", OP_IDX_2 | OP_PAGE2,   5, 0x1a,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxd", OP_D_IDX | OP_PAGE2,   3, 0x1a,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxd", OP_D_IDX_2 | OP_PAGE2, 5, 0x1a,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "emaxm", OP_IDX | OP_PAGE2,     3, 0x1e,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxm", OP_IDX_1 | OP_PAGE2,   4, 0x1e,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxm", OP_IDX_2 | OP_PAGE2,   5, 0x1e,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxm", OP_D_IDX | OP_PAGE2,   3, 0x1e,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emaxm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1e,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "emind", OP_IDX | OP_PAGE2,     3, 0x1b,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emind", OP_IDX_1 | OP_PAGE2,   4, 0x1b,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emind", OP_IDX_2 | OP_PAGE2,   5, 0x1b,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emind", OP_D_IDX | OP_PAGE2,   3, 0x1b,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "emind", OP_D_IDX_2 | OP_PAGE2, 5, 0x1b,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "eminm", OP_IDX | OP_PAGE2,     3, 0x1f,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "eminm", OP_IDX_1 | OP_PAGE2,   4, 0x1f,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "eminm", OP_IDX_2 | OP_PAGE2,   5, 0x1f,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "eminm", OP_D_IDX | OP_PAGE2,   3, 0x1f,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "eminm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1f,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "emul",  OP_NONE,               1, 0x13,  3,  3,  CHG_NZC, cpu6812 | cpu6812x },
  { "emuls", OP_NONE | OP_PAGE2,    2, 0x13,  3,  3,  CHG_NZC, cpu6812 | cpu6812x },

  { "eora", OP_IMM8,         2, 0x88,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eora", OP_DIRECT,       2, 0x98,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eora", OP_IND16,        3, 0xb8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eora", OP_IX,             2, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "eora", OP_IY | OP_PAGE2,  3, 0xa8,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "eora", OP_IDX,            2, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eora", OP_IDX_1,          3, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eora", OP_IDX_2,          4, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eora", OP_D_IDX,          2, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eora", OP_D_IDX_2,        4, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "eorb", OP_IMM8,         2, 0xc8,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eorb", OP_DIRECT,       2, 0xd8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eorb", OP_IND16,        3, 0xf8,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "eorb", OP_IX,             2, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "eorb", OP_IY | OP_PAGE2,  3, 0xe8,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "eorb", OP_IDX,            2, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eorb", OP_IDX_1,          3, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eorb", OP_IDX_2,          4, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eorb", OP_D_IDX,          2, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "eorb", OP_D_IDX_2,        4, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "eorx", OP_IMM16 | OP_PAGE2,         2, 0x88,  1,  1,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_DIRECT | OP_PAGE2,       2, 0x98,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_IND16 | OP_PAGE2,        3, 0xb8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_IDX | OP_PAGE2,            2, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_IDX_1 | OP_PAGE2,          3, 0xa8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_IDX_2 | OP_PAGE2,          4, 0xa8,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_D_IDX | OP_PAGE2,          2, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "eorx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa8,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

  { "eory", OP_IMM16 | OP_PAGE2,         2, 0xc8,  1,  1,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_DIRECT | OP_PAGE2,       2, 0xd8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_IND16 | OP_PAGE2,        3, 0xf8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_IDX | OP_PAGE2,            2, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_IDX_1 | OP_PAGE2,          3, 0xe8,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_IDX_2 | OP_PAGE2,          4, 0xe8,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_D_IDX | OP_PAGE2,          2, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "eory", OP_D_IDX_2 | OP_PAGE2,        4, 0xe8,  6,  6,  CLR_V_CHG_NZ, cpu6812x},


  { "etbl", OP_IDX | OP_PAGE2,3, 0x3f, 10, 10,  CHG_NZC, cpu6812 | cpu6812x },

  /* TODO S12X supports more exg variants than this */
  { "exg",  OP_EXG_MARKER
          | OP_REG | OP_REG_2, 2, 0xb7, 1, 1,  CHG_NONE, cpu6812 | cpu6812x },

  { "fdiv", OP_NONE,             1, 0x03,  3, 41, CHG_ZVC, cpu6811},
  { "fdiv", OP_NONE | OP_PAGE2, 2, 0x11, 12, 12, CHG_ZVC, cpu6812 | cpu6812x },
  { "gldaa", OP_DIRECT | OP_PAGE2,       2, 0x96,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_IND16 | OP_PAGE2,        3, 0xb6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_IDX | OP_PAGE2,            2, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_IDX_1 | OP_PAGE2,          3, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_IDX_2 | OP_PAGE2,          4, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_D_IDX | OP_PAGE2,          2, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gldaa", OP_D_IDX_2 | OP_PAGE2,        4, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gldab", OP_DIRECT | OP_PAGE2,       2, 0xd6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_IND16 | OP_PAGE2,        3, 0xf6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_IDX | OP_PAGE2,            2, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_IDX_1 | OP_PAGE2,          3, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_IDX_2 | OP_PAGE2,          4, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_D_IDX | OP_PAGE2,          2, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gldab", OP_D_IDX_2 | OP_PAGE2,        4, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gldd", OP_DIRECT | OP_PAGE2,       2, 0xdc,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_IND16 | OP_PAGE2,        3, 0xfc,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_IDX | OP_PAGE2,            2, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_IDX_1 | OP_PAGE2,          3, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_IDX_2 | OP_PAGE2,          4, 0xec,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_D_IDX | OP_PAGE2,          2, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gldd", OP_D_IDX_2 | OP_PAGE2,        4, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "glds", OP_DIRECT | OP_PAGE2,       2, 0xdf,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_IND16 | OP_PAGE2,        3, 0xff,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_IDX | OP_PAGE2,            2, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_IDX_1 | OP_PAGE2,          3, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_IDX_2 | OP_PAGE2,          4, 0xef,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_D_IDX | OP_PAGE2,          2, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "glds", OP_D_IDX_2 | OP_PAGE2,        4, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gldx", OP_DIRECT | OP_PAGE2,       2, 0xde,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_IND16 | OP_PAGE2,        3, 0xfe,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_IDX | OP_PAGE2,            2, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_IDX_1 | OP_PAGE2,          3, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_IDX_2 | OP_PAGE2,          4, 0xee,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_D_IDX | OP_PAGE2,          2, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gldx", OP_D_IDX_2 | OP_PAGE2,        4, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gldy", OP_DIRECT | OP_PAGE2,       2, 0xdd,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_IND16 | OP_PAGE2,        3, 0xfd,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_IDX | OP_PAGE2,            2, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_IDX_1 | OP_PAGE2,          3, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_IDX_2 | OP_PAGE2,          4, 0xed,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_D_IDX | OP_PAGE2,          2, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gldy", OP_D_IDX_2 | OP_PAGE2,        4, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gstaa", OP_DIRECT | OP_PAGE2,       2, 0x5a,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_IND16 | OP_PAGE2,        3, 0x7a,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_IDX | OP_PAGE2,            2, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_IDX_1 | OP_PAGE2,          3, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_IDX_2 | OP_PAGE2,          4, 0x6a,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_D_IDX | OP_PAGE2,          2, 0x6a,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gstaa", OP_D_IDX_2 | OP_PAGE2,        4, 0x6a,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gstab", OP_DIRECT | OP_PAGE2,       2, 0x5b,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_IND16 | OP_PAGE2,        3, 0x7b,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_IDX | OP_PAGE2,            2, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_IDX_1 | OP_PAGE2,          3, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_IDX_2 | OP_PAGE2,          4, 0x6b,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_D_IDX | OP_PAGE2,          2, 0x6b,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gstab", OP_D_IDX_2 | OP_PAGE2,        4, 0x6b,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gstd", OP_DIRECT | OP_PAGE2,       2, 0x5c,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_IND16 | OP_PAGE2,        3, 0x7c,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_IDX | OP_PAGE2,            2, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_IDX_1 | OP_PAGE2,          3, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_IDX_2 | OP_PAGE2,          4, 0x6c,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_D_IDX | OP_PAGE2,          2, 0x6c,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gstd", OP_D_IDX_2 | OP_PAGE2,        4, 0x6c,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gsts", OP_DIRECT | OP_PAGE2,       2, 0x5f,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_IND16 | OP_PAGE2,        3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_IDX | OP_PAGE2,            2, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_IDX_1 | OP_PAGE2,          3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_IDX_2 | OP_PAGE2,          4, 0x6f,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_D_IDX | OP_PAGE2,          2, 0x6f,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gsts", OP_D_IDX_2 | OP_PAGE2,        4, 0x6f,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gstx", OP_DIRECT | OP_PAGE2,       2, 0x5e,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_IND16 | OP_PAGE2,        3, 0x7e,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_IDX | OP_PAGE2,            2, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_IDX_1 | OP_PAGE2,          3, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_IDX_2 | OP_PAGE2,          4, 0x6e,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_D_IDX | OP_PAGE2,          2, 0x6e,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gstx", OP_D_IDX_2 | OP_PAGE2,        4, 0x6e,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "gsty", OP_DIRECT | OP_PAGE2,       2, 0x5d,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_IND16 | OP_PAGE2,        3, 0x7d,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_IDX | OP_PAGE2,            2, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_IDX_1 | OP_PAGE2,          3, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_IDX_2 | OP_PAGE2,          4, 0x6d,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_D_IDX | OP_PAGE2,          2, 0x6d,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
   { "gsty", OP_D_IDX_2 | OP_PAGE2,        4, 0x6d,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

   { "ibeq", OP_IBEQ_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },
   { "ibne", OP_IBNE_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },

  { "idiv",  OP_NONE,              1, 0x02,  3, 41, CLR_V_CHG_ZC, cpu6811},
  { "idiv",  OP_NONE | OP_PAGE2,  2, 0x10, 12, 12, CLR_V_CHG_ZC, cpu6812 | cpu6812x },
  { "idivs", OP_NONE | OP_PAGE2,  2, 0x15, 12, 12, CHG_NZVC, cpu6812 | cpu6812x },

  { "inc", OP_IX,                  2, 0x6c,  6,  6,  CHG_NZV, cpu6811 },
  { "inc", OP_IND16,               3, 0x7c,  6,  6,  CHG_NZV, cpu6811 },
  { "inc", OP_IY | OP_PAGE2,       3, 0x6c,  7,  7,  CHG_NZV, cpu6811 },
  { "inc", OP_IND16,               3, 0x72,  4,  4,  CHG_NZV, cpu6812 | cpu6812x },
  { "inc", OP_IDX,                 2, 0x62,  3,  3,  CHG_NZV, cpu6812 | cpu6812x },
  { "inc", OP_IDX_1,               3, 0x62,  4,  4,  CHG_NZV, cpu6812 | cpu6812x },
  { "inc", OP_IDX_2,               4, 0x62,  5,  5,  CHG_NZV, cpu6812 | cpu6812x },
  { "inc", OP_D_IDX,               2, 0x62,  6,  6,  CHG_NZV, cpu6812 | cpu6812x },
  { "inc", OP_D_IDX_2,             4, 0x62,  6,  6,  CHG_NZV, cpu6812 | cpu6812x },

  { "inca", OP_NONE,               1, 0x4c,  2,  2,  CHG_NZV, cpu6811 },
  { "inca", OP_NONE,               1, 0x42,  1,  1,  CHG_NZV, cpu6812 | cpu6812x },
  { "incb", OP_NONE,               1, 0x5c,  2,  2,  CHG_NZV, cpu6811 },
  { "incb", OP_NONE,               1, 0x52,  1,  1,  CHG_NZV, cpu6812 | cpu6812x },

  { "incw",  OP_IND16 | OP_PAGE2,          3, 0x72,  4,  4,  CHG_NZV, cpu6812x},
  { "incw",  OP_IDX | OP_PAGE2,            2, 0x62,  3,  3,  CHG_NZV, cpu6812x},
  { "incw",  OP_IDX_1 | OP_PAGE2,          3, 0x62,  4,  4,  CHG_NZV, cpu6812x},
  { "incw",  OP_IDX_2 | OP_PAGE2,          4, 0x62,  5,  5,  CHG_NZV, cpu6812x},
  { "incw",  OP_D_IDX | OP_PAGE2,          2, 0x62,  6,  6,  CHG_NZV, cpu6812x},
  { "incw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x62,  6,  6,  CHG_NZV, cpu6812x},

  { "incx",  OP_NONE | OP_PAGE2,          3, 0x42,  4,  4,  CHG_NZV, cpu6812x},

  { "incy",  OP_NONE | OP_PAGE2,          3, 0x52,  4,  4,  CHG_NZV, cpu6812x},

  { "ins",  OP_NONE,               1, 0x31,  3,  3,  CHG_NONE, cpu6811 },

  { "inx",  OP_NONE,               1, 0x08,  1,  1,  CHG_Z, cpu6811|cpu6812 | cpu6812x },
  { "iny",  OP_NONE |OP_PAGE2,     2, 0x08,  4,  4,  CHG_Z, cpu6811 },
  { "iny",  OP_NONE,               1, 0x02,  1,  1,  CHG_Z, cpu6812 | cpu6812x },

  { "jmp",  OP_IND16 | OP_BRANCH,  3, 0x7e,  3,  3,  CHG_NONE, cpu6811 },
  { "jmp",  OP_IX,                 2, 0x6e,  3,  3,  CHG_NONE, cpu6811 },
  { "jmp",  OP_IY | OP_PAGE2,      3, 0x6e,  4,  4,  CHG_NONE, cpu6811 },
  { "jmp",  OP_IND16 | OP_BRANCH,  3, 0x06,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "jmp",  OP_IDX,                2, 0x05,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "jmp",  OP_IDX_1,              3, 0x05,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "jmp",  OP_IDX_2,              4, 0x05,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "jmp",  OP_D_IDX,              2, 0x05,  6,  6,  CHG_NONE, cpu6812 | cpu6812x },
  { "jmp",  OP_D_IDX_2,            4, 0x05,  6,  6,  CHG_NONE, cpu6812 | cpu6812x },

  { "jsr",  OP_DIRECT | OP_BRANCH, 2, 0x9d,  5,  5,  CHG_NONE, cpu6811 },
  { "jsr",  OP_IND16 | OP_BRANCH,  3, 0xbd,  6,  6,  CHG_NONE, cpu6811 },
  { "jsr",  OP_IX,                 2, 0xad,  6,  6,  CHG_NONE, cpu6811 },
  { "jsr",  OP_IY | OP_PAGE2,      3, 0xad,  6,  6,  CHG_NONE, cpu6811 },
  { "jsr",  OP_DIRECT | OP_BRANCH, 2, 0x17,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_IND16 | OP_BRANCH,  3, 0x16,  4,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_IDX,                2, 0x15,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_IDX_1,              3, 0x15,  4,  4,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_IDX_2,              4, 0x15,  5,  5,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_D_IDX,              2, 0x15,  7,  7,  CHG_NONE, cpu6812 | cpu6812x },
  { "jsr",  OP_D_IDX_2,            4, 0x15,  7,  7,  CHG_NONE, cpu6812 | cpu6812x },

  { "lbcc", OP_JUMP_REL16 | OP_PAGE2,  4, 0x24,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbcs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x25,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbeq", OP_JUMP_REL16 | OP_PAGE2,  4, 0x27,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbge", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2c,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbgt", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2e,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbhi", OP_JUMP_REL16 | OP_PAGE2,  4, 0x22,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbhs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x24,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lble", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2f,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lblo", OP_JUMP_REL16 | OP_PAGE2,  4, 0x25,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbls", OP_JUMP_REL16 | OP_PAGE2,  4, 0x23,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lblt", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2d,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbmi", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2b,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbne", OP_JUMP_REL16 | OP_PAGE2,  4, 0x26,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbpl", OP_JUMP_REL16 | OP_PAGE2,  4, 0x2a,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbra", OP_JUMP_REL16 | OP_PAGE2,  4, 0x20,  4,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbrn", OP_JUMP_REL16 | OP_PAGE2,  4, 0x21,  3,  3, CHG_NONE, cpu6812 | cpu6812x },
  { "lbvc", OP_JUMP_REL16 | OP_PAGE2,  4, 0x28,  3,  4, CHG_NONE, cpu6812 | cpu6812x },
  { "lbvs", OP_JUMP_REL16 | OP_PAGE2,  4, 0x29,  3,  4, CHG_NONE, cpu6812 | cpu6812x },

  { "ldaa", OP_IMM8,         2, 0x86,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldaa", OP_DIRECT,       2, 0x96,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldaa", OP_IND16,        3, 0xb6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldaa", OP_IX,             2, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "ldaa", OP_IY | OP_PAGE2,  3, 0xa6,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "ldaa", OP_IDX,            2, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldaa", OP_IDX_1,          3, 0xa6,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldaa", OP_IDX_2,          4, 0xa6,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldaa", OP_D_IDX,          2, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldaa", OP_D_IDX_2,        4, 0xa6,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "ldab", OP_IMM8,         2, 0xc6,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldab", OP_DIRECT,       2, 0xd6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldab", OP_IND16,        3, 0xf6,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldab", OP_IX,             2, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "ldab", OP_IY | OP_PAGE2,  3, 0xe6,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "ldab", OP_IDX,            2, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldab", OP_IDX_1,          3, 0xe6,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldab", OP_IDX_2,          4, 0xe6,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldab", OP_D_IDX,          2, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldab", OP_D_IDX_2,        4, 0xe6,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "ldd", OP_IMM16,         3, 0xcc,  2,  2,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldd", OP_DIRECT,        2, 0xdc,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldd", OP_IND16,         3, 0xfc,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldd", OP_IX,              2, 0xec,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "ldd", OP_IY | OP_PAGE2,   3, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "ldd", OP_IDX,             2, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldd", OP_IDX_1,           3, 0xec,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldd", OP_IDX_2,           4, 0xec,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldd", OP_D_IDX,           2, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldd", OP_D_IDX_2,         4, 0xec,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "lds",  OP_IMM16,          3, 0x8e,  3,  3,  CLR_V_CHG_NZ, cpu6811 },
  { "lds",  OP_DIRECT,         2, 0x9e,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "lds",  OP_IND16,          3, 0xbe,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "lds",  OP_IX,             2, 0xae,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "lds",  OP_IY | OP_PAGE2,  3, 0xae,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "lds",  OP_IMM16,          3, 0xcf,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_DIRECT,         2, 0xdf,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_IND16,          3, 0xff,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_IDX,            2, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_IDX_1,          3, 0xef,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_IDX_2,          4, 0xef,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_D_IDX,          2, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "lds",  OP_D_IDX_2,        4, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "ldx",  OP_IMM16,        3, 0xce,  2,  2,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldx",  OP_DIRECT,       2, 0xde,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldx",  OP_IND16,        3, 0xfe,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "ldx",  OP_IX,             2, 0xee,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "ldx",  OP_IY | OP_PAGE4,  3, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "ldx",  OP_IDX,            2, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldx",  OP_IDX_1,          3, 0xee,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldx",  OP_IDX_2,          4, 0xee,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldx",  OP_D_IDX,          2, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldx",  OP_D_IDX_2,        4, 0xee,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "ldy",  OP_IMM16 | OP_PAGE2,  4, 0xce, 4, 4, CLR_V_CHG_NZ, cpu6811 },
  { "ldy",  OP_DIRECT | OP_PAGE2, 3, 0xde, 5, 5, CLR_V_CHG_NZ, cpu6811 },
  { "ldy",  OP_IND16 | OP_PAGE2,  4, 0xfe, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "ldy",  OP_IX | OP_PAGE3,     3, 0xee, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "ldy",  OP_IY | OP_PAGE2,     3, 0xee, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "ldy",  OP_IMM16,          3, 0xcd,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_DIRECT,         2, 0xdd,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_IND16,          3, 0xfd,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_IDX,            2, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_IDX_1,          3, 0xed,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_IDX_2,          4, 0xed,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_D_IDX,          2, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "ldy",  OP_D_IDX_2,        4, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "leas", OP_IDX,            2, 0x1b,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leas", OP_IDX_1,          3, 0x1b,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leas", OP_IDX_2,          4, 0x1b,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },

  { "leax", OP_IDX,            2, 0x1a,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leax", OP_IDX_1,          3, 0x1a,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leax", OP_IDX_2,          4, 0x1a,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },

  { "leay", OP_IDX,            2, 0x19,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leay", OP_IDX_1,          3, 0x19,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "leay", OP_IDX_2,          4, 0x19,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },

  { "lsl",  OP_IND16,          3, 0x78,  4,  4,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "lsl",  OP_IX,             2, 0x68,  6,  6,  CHG_NZVC, cpu6811 },
  { "lsl",  OP_IY | OP_PAGE2,  3, 0x68,  7,  7,  CHG_NZVC, cpu6811 },
  { "lsl",  OP_IDX,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "lsl",  OP_IDX_1,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "lsl",  OP_IDX_2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "lsl",  OP_D_IDX,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "lsl",  OP_D_IDX_2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "lsla", OP_NONE,           1, 0x48,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "lslb", OP_NONE,           1, 0x58,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "lsld", OP_NONE,           1, 0x05,  3,  3,  CHG_NZVC, cpu6811 },
  { "lsld", OP_NONE,           1, 0x59,  1,  1,  CHG_NZVC, cpu6812 | cpu6812x },

  /* TODO verify same as aslw */
    { "lslw",  OP_IND16 | OP_PAGE2,          3, 0x78,  4,  4,  CHG_NZVC, cpu6812x},
    { "lslw",  OP_IDX | OP_PAGE2,            2, 0x68,  3,  3,  CHG_NZVC, cpu6812x},
    { "lslw",  OP_IDX_1 | OP_PAGE2,          3, 0x68,  4,  4,  CHG_NZVC, cpu6812x},
    { "lslw",  OP_IDX_2 | OP_PAGE2,          4, 0x68,  5,  5,  CHG_NZVC, cpu6812x},
    { "lslw",  OP_D_IDX | OP_PAGE2,          2, 0x68,  6,  6,  CHG_NZVC, cpu6812x},
    { "lslw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x68,  6,  6,  CHG_NZVC, cpu6812x},
  /* TODO verify same as lslw */
    { "lslx", OP_NONE | OP_PAGE2,           1, 0x48,  1,  1,  CHG_NZVC, cpu6812x},
  /* TODO verify same as lslw */
    { "lsly", OP_NONE | OP_PAGE2,           1, 0x58,  1,  1,  CHG_NZVC, cpu6812x},

  { "lsr",  OP_IND16,        3, 0x74,  4,  4,  CLR_N_CHG_ZVC, cpu6811|cpu6812 | cpu6812x},
  { "lsr",  OP_IX,             2, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6811 },
  { "lsr",  OP_IY | OP_PAGE2,  3, 0x64,  7,  7,  CLR_V_CHG_ZVC, cpu6811 },
  { "lsr",  OP_IDX,            2, 0x64,  3,  3,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },
  { "lsr",  OP_IDX_1,          3, 0x64,  4,  4,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },
  { "lsr",  OP_IDX_2,          4, 0x64,  5,  5,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },
  { "lsr",  OP_D_IDX,          2, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },
  { "lsr",  OP_D_IDX_2,        4, 0x64,  6,  6,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },

  { "lsra", OP_NONE,         1, 0x44,  1,  1,  CLR_N_CHG_ZVC, cpu6811|cpu6812 | cpu6812x},
  { "lsrb", OP_NONE,         1, 0x54,  1,  1,  CLR_N_CHG_ZVC, cpu6811|cpu6812 | cpu6812x},
  { "lsrd", OP_NONE,           1, 0x04,  3,  3,  CLR_N_CHG_ZVC, cpu6811 },
  { "lsrd", OP_NONE,           1, 0x49,  1,  1,  CLR_N_CHG_ZVC, cpu6812 | cpu6812x },

  { "lsrw",  OP_IND16 | OP_PAGE2,          3, 0x74,  4,  4,  CHG_NZVC, cpu6812x},
  { "lsrw",  OP_IDX | OP_PAGE2,            2, 0x64,  3,  3,  CHG_NZVC, cpu6812x},
  { "lsrw",  OP_IDX_1 | OP_PAGE2,          3, 0x64,  4,  4,  CHG_NZVC, cpu6812x},
  { "lsrw",  OP_IDX_2 | OP_PAGE2,          4, 0x64,  5,  5,  CHG_NZVC, cpu6812x},
  { "lsrw",  OP_D_IDX | OP_PAGE2,          2, 0x64,  6,  6,  CHG_NZVC, cpu6812x},
  { "lsrw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x64,  6,  6,  CHG_NZVC, cpu6812x},

  { "lsrx", OP_NONE | OP_PAGE2,           1, 0x44,  1,  1,  CHG_NZVC, cpu6812x},

  { "lsry", OP_NONE | OP_PAGE2,           1, 0x54,  1,  1,  CHG_NZVC, cpu6812x},

  { "maxa", OP_IDX | OP_PAGE2,     3, 0x18,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxa", OP_IDX_1 | OP_PAGE2,   4, 0x18,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxa", OP_IDX_2 | OP_PAGE2,   5, 0x18,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxa", OP_D_IDX | OP_PAGE2,   3, 0x18,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxa", OP_D_IDX_2 | OP_PAGE2, 5, 0x18,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "maxm", OP_IDX | OP_PAGE2,     3, 0x1c,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxm", OP_IDX_1 | OP_PAGE2,   4, 0x1c,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxm", OP_IDX_2 | OP_PAGE2,   5, 0x1c,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxm", OP_D_IDX | OP_PAGE2,   3, 0x1c,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "maxm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1c,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "mem",  OP_NONE,                1, 0x01,  5,  5,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "mina", OP_IDX | OP_PAGE2,     3, 0x19,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "mina", OP_IDX_1 | OP_PAGE2,   4, 0x19,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "mina", OP_IDX_2 | OP_PAGE2,   5, 0x19,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "mina", OP_D_IDX | OP_PAGE2,   3, 0x19,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "mina", OP_D_IDX_2 | OP_PAGE2, 5, 0x19,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  { "minm", OP_IDX | OP_PAGE2,     3, 0x1d,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "minm", OP_IDX_1 | OP_PAGE2,   4, 0x1d,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "minm", OP_IDX_2 | OP_PAGE2,   5, 0x1d,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "minm", OP_D_IDX | OP_PAGE2,   3, 0x1d,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },
  { "minm", OP_D_IDX_2 | OP_PAGE2, 5, 0x1d,  7,  7,  CHG_NZVC, cpu6812 | cpu6812x },

  /* TODO S12X additional modes not implemented */
    { "movb", OP_IMM8|OP_IND16_p2|OP_PAGE2,       5, 0x0b,  4,  4,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movb", OP_IMM8|OP_IDX_p2|OP_PAGE2,         4, 0x08,  4,  4,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
   // { "movb", OP_IMM8|OP_IDX1_p2|OP_PAGE2,        5, 0x08,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IMM8|OP_IDX2_p2|OP_PAGE2,        4, 0x08,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IMM8|OP_D_IDX_p2|OP_PAGE2,       5, 0x08,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IMM8|OP_D_IDX2_p2|OP_PAGE2,      4, 0x08,  4,  4,  CHG_NONE, cpu6812x},

    { "movb", OP_IND16|OP_IND16_p2|OP_PAGE2,      6, 0x0c,  6,  6,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movb", OP_IND16|OP_IDX_p2|OP_PAGE2,        5, 0x09,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
  //  { "movb", OP_IND16|OP_IDX1_p2|OP_PAGE2,       6, 0x09,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IND16|OP_IDX2_p2|OP_PAGE2,       5, 0x09,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IND16|OP_D_IDX_p2|OP_PAGE2,    6, 0x09,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IND16|OP_D_IDX2_p2|OP_PAGE2,     5, 0x09,  5,  5,  CHG_NONE, cpu6812x},

    { "movb", OP_IDX|OP_IND16_p2|OP_PAGE2,        5, 0x0d,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movb", OP_IDX|OP_IDX_p2|OP_PAGE2,          4, 0x0a,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
  //  { "movb", OP_IDX|OP_IDX1_p2|OP_PAGE2,         5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX|OP_IDX2_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX|OP_D_IDX_p2|OP_PAGE2,        5, 0x0d,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX|OP_D_IDX2_p2|OP_PAGE2,       4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movb", OP_IDX_1|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_1|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_1|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_1|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_1|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_1|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movb", OP_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movb", OP_D_IDX|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movb", OP_D_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x0d,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x0a,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movb", OP_D_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x0a,  5,  5,  CHG_NONE, cpu6812x},

  /* TODO S12X additional modes not implemented */
    { "movw", OP_IMM16 | OP_IND16_p2 | OP_PAGE2,  6, 0x03,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movw", OP_IMM16 | OP_IDX_p2 | OP_PAGE2,    5, 0x00,  4,  4,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
  //  { "movw", OP_IMM16|OP_IDX1_p2|OP_PAGE2,        5, 0x00,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IMM16|OP_IDX2_p2|OP_PAGE2,        4, 0x00,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IMM16|OP_D_IDX_p2|OP_PAGE2,       5, 0x00,  4,  4,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IMM16|OP_D_IDX2_p2|OP_PAGE2,      4, 0x00,  4,  4,  CHG_NONE, cpu6812x},

    { "movw", OP_IND16 | OP_IND16_p2 | OP_PAGE2,  6, 0x04,  6,  6,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movw", OP_IND16 | OP_IDX_p2 | OP_PAGE2,    5, 0x01,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
  //  { "movw", OP_IND16|OP_IDX1_p2|OP_PAGE2,       6, 0x01,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IND16|OP_IDX2_p2|OP_PAGE2,       5, 0x01,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IND16|OP_D_IDX_p2|OP_PAGE2,      6, 0x01,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IND16|OP_D_IDX2_p2|OP_PAGE2,     5, 0x01,  5,  5,  CHG_NONE, cpu6812x},

    { "movw", OP_IDX | OP_IND16_p2 | OP_PAGE2,    5, 0x05,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
    { "movw", OP_IDX | OP_IDX_p2 | OP_PAGE2,      4, 0x02,  5,  5,  CHG_NONE, cpu6812 | cpu6812x|cpu6812x},
  //  { "movw", OP_IDX|OP_IDX1_p2|OP_PAGE2,         5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX|OP_IDX2_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX|OP_D_IDX_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX|OP_D_IDX2_p2|OP_PAGE2,       4, 0x02,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movw", OP_IDX_1|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_1|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_1|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_1|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_1|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_1|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movw", OP_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movw", OP_D_IDX|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu6812x},

  //  { "movw", OP_D_IDX_2|OP_IND16_p2|OP_PAGE2,       5, 0x05,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX_2|OP_IDX_p2|OP_PAGE2,         4, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX_2|OP_IDX1_p2|OP_PAGE2,        6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX_2|OP_IDX2_p2|OP_PAGE2,        5, 0x02,  5,  5,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX_2|OP_D_IDX_p2|OP_PAGE2,       6, 0x02,  6,  6,  CHG_NONE, cpu6812x},
  //  { "movw", OP_D_IDX_2|OP_D_IDX2_p2|OP_PAGE2,      5, 0x02,  5,  5,  CHG_NONE, cpu6812x},

  { "mul",  OP_NONE,           1, 0x3d,  3, 10,  CHG_C, cpu6811 },
  { "mul",  OP_NONE,           1, 0x12,  3,  3,  CHG_C, cpu6812 | cpu6812x },

  { "neg",  OP_IND16,          3, 0x70,  4,  4,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "neg",  OP_IX,             2, 0x60,  6,  6,  CHG_NZVC, cpu6811 },
  { "neg",  OP_IY | OP_PAGE2,  3, 0x60,  7,  7,  CHG_NZVC, cpu6811 },
  { "neg",  OP_IDX,            2, 0x60,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "neg",  OP_IDX_1,          3, 0x60,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "neg",  OP_IDX_2,          4, 0x60,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "neg",  OP_D_IDX,          2, 0x60,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "neg",  OP_D_IDX_2,        4, 0x60,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "nega", OP_NONE,           1, 0x40,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "negb", OP_NONE,           1, 0x50,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "negw",  OP_IND16| OP_PAGE2,          3, 0x70,  4,  4,  CHG_NZVC, cpu6812x},

  { "negw",  OP_IDX| OP_PAGE2,            2, 0x60,  3,  3,  CHG_NZVC, cpu6812x},
  { "negw",  OP_IDX_1| OP_PAGE2,          3, 0x60,  4,  4,  CHG_NZVC, cpu6812x},
  { "negw",  OP_IDX_2| OP_PAGE2,          4, 0x60,  5,  5,  CHG_NZVC, cpu6812x},
  { "negw",  OP_D_IDX| OP_PAGE2,          2, 0x60,  6,  6,  CHG_NZVC, cpu6812x},
  { "negw",  OP_D_IDX_2| OP_PAGE2,        4, 0x60,  6,  6,  CHG_NZVC, cpu6812x},

  { "negx", OP_NONE| OP_PAGE2,           1, 0x40,  1,  1,  CHG_NZVC, cpu6812x},

  { "negy", OP_NONE| OP_PAGE2,           1, 0x50,  1,  1,  CHG_NZVC, cpu6812x},

  { "nop",  OP_NONE,           1, 0x01,  2,  2,  CHG_NONE, cpu6811 },
  { "nop",  OP_NONE,           1, 0xa7,  1,  1,  CHG_NONE, cpu6812 | cpu6812x },

  { "oraa", OP_IMM8,         2, 0x8a,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "oraa", OP_DIRECT,       2, 0x9a,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "oraa", OP_IND16,        3, 0xba,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "oraa", OP_IX,             2, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "oraa", OP_IY | OP_PAGE2,  3, 0xaa,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "oraa", OP_IDX,            2, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "oraa", OP_IDX_1,          3, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "oraa", OP_IDX_2,          4, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "oraa", OP_D_IDX,          2, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "oraa", OP_D_IDX_2,        4, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "orab", OP_IMM8,         2, 0xca,  1,  1,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "orab", OP_DIRECT,       2, 0xda,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "orab", OP_IND16,        3, 0xfa,  3,  3,  CLR_V_CHG_NZ, cpu6811|cpu6812 | cpu6812x },
  { "orab", OP_IX,             2, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "orab", OP_IY | OP_PAGE2,  3, 0xea,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "orab", OP_IDX,            2, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "orab", OP_IDX_1,          3, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "orab", OP_IDX_2,          4, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "orab", OP_D_IDX,          2, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "orab", OP_D_IDX_2,        4, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "orcc", OP_IMM8,           2, 0x14,  1,  1,  CHG_ALL, cpu6812 | cpu6812x },

  { "orx", OP_IMM16| OP_PAGE2,  2, 0x8a,  1,  1,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_DIRECT| OP_PAGE2, 2, 0x9a,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_IND16| OP_PAGE2,  3, 0xba,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_IDX| OP_PAGE2,    2, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_IDX_1| OP_PAGE2,  3, 0xaa,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_IDX_2| OP_PAGE2,  4, 0xaa,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_D_IDX| OP_PAGE2,  2, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "orx", OP_D_IDX_2| OP_PAGE2,4, 0xaa,  6,  6,  CLR_V_CHG_NZ, cpu6812x},

  { "ory", OP_IMM16| OP_PAGE2,  2, 0xca,  1,  1,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_DIRECT| OP_PAGE2, 2, 0xda,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_IND16| OP_PAGE2,  3, 0xfa,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_IDX| OP_PAGE2,    2, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_IDX_1| OP_PAGE2,  3, 0xea,  3,  3,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_IDX_2| OP_PAGE2,  4, 0xea,  4,  4,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_D_IDX| OP_PAGE2,  2, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812x},
  { "ory", OP_D_IDX_2| OP_PAGE2,4, 0xea,  6,  6,  CLR_V_CHG_NZ, cpu6812x},


  { "psha", OP_NONE,           1, 0x36,  2,  2,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },
  { "pshb", OP_NONE,           1, 0x37,  2,  2,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },
  { "pshc", OP_NONE,           1, 0x39,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "pshd", OP_NONE,           1, 0x3b,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "pshx", OP_NONE,           1, 0x3c,  4,  4,  CHG_NONE, cpu6811 },
  { "pshx", OP_NONE,           1, 0x34,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },
  { "pshy", OP_NONE | OP_PAGE2,2, 0x3c,  5,  5,  CHG_NONE, cpu6811 },
  { "pshy", OP_NONE,           1, 0x35,  2,  2,  CHG_NONE, cpu6812 | cpu6812x },

  { "pula", OP_NONE,           1, 0x32,  3,  3,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },
  { "pulb", OP_NONE,           1, 0x33,  3,  3,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },
  { "pulc", OP_NONE,           1, 0x38,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "puld", OP_NONE,           1, 0x3a,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "pulx", OP_NONE,           1, 0x38,  5,  5,  CHG_NONE, cpu6811 },
  { "pulx", OP_NONE,           1, 0x30,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },
  { "puly", OP_NONE | OP_PAGE2,2, 0x38,  6,  6,  CHG_NONE, cpu6811 },
  { "puly", OP_NONE,           1, 0x31,  3,  3,  CHG_NONE, cpu6812 | cpu6812x },

  { "rev",  OP_NONE | OP_PAGE2, 2, 0x3a,  _M,  _M,  CHG_HNZVC, cpu6812 | cpu6812x },
  { "revw", OP_NONE | OP_PAGE2, 2, 0x3b,  _M,  _M,  CHG_HNZVC, cpu6812 | cpu6812x },

  { "rol",  OP_IND16,          3, 0x79,  6,  6,  CHG_NZVC, cpu6811 },
  { "rol",  OP_IX,             2, 0x69,  6,  6,  CHG_NZVC, cpu6811 },
  { "rol",  OP_IY | OP_PAGE2,  3, 0x69,  7,  7,  CHG_NZVC, cpu6811 },
  { "rol",  OP_IND16,          3, 0x75,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rol",  OP_IDX,            2, 0x65,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rol",  OP_IDX_1,          3, 0x65,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rol",  OP_IDX_2,          4, 0x65,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rol",  OP_D_IDX,          2, 0x65,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rol",  OP_D_IDX_2,        4, 0x65,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "rola", OP_NONE,           1, 0x49,  2,  2,  CHG_NZVC, cpu6811 },
  { "rola", OP_NONE,           1, 0x45,  1,  1,  CHG_NZVC, cpu6812 | cpu6812x },
  { "rolb", OP_NONE,           1, 0x59,  2,  2,  CHG_NZVC, cpu6811 },
  { "rolb", OP_NONE,           1, 0x55,  1,  1,  CHG_NZVC, cpu6812 | cpu6812x },

  { "rolw",  OP_IND16 | OP_PAGE2,          3, 0x75,  4,  4,  CHG_NZVC, cpu6812x},
  { "rolw",  OP_IDX | OP_PAGE2,            2, 0x65,  3,  3,  CHG_NZVC, cpu6812x},
  { "rolw",  OP_IDX_1 | OP_PAGE2,          3, 0x65,  4,  4,  CHG_NZVC, cpu6812x},
  { "rolw",  OP_IDX_2 | OP_PAGE2,          4, 0x65,  5,  5,  CHG_NZVC, cpu6812x},
  { "rolw",  OP_D_IDX | OP_PAGE2,          2, 0x65,  6,  6,  CHG_NZVC, cpu6812x},
  { "rolw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x65,  6,  6,  CHG_NZVC, cpu6812x},

  { "rolx", OP_NONE | OP_PAGE2,           1, 0x45,  1,  1,  CHG_NZVC, cpu6812x},
  { "roly", OP_NONE | OP_PAGE2,           1, 0x55,  1,  1,  CHG_NZVC, cpu6812x},

  { "ror",  OP_IND16,          3, 0x76,  4,  4,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "ror",  OP_IX,             2, 0x66,  6,  6,  CHG_NZVC, cpu6811 },
  { "ror",  OP_IY | OP_PAGE2,  3, 0x66,  7,  7,  CHG_NZVC, cpu6811 },
  { "ror",  OP_IDX,            2, 0x66,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "ror",  OP_IDX_1,          3, 0x66,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "ror",  OP_IDX_2,          4, 0x66,  5,  5,  CHG_NZVC, cpu6812 | cpu6812x },
  { "ror",  OP_D_IDX,          2, 0x66,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "ror",  OP_D_IDX_2,        4, 0x66,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "rora", OP_NONE,           1, 0x46,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "rorb", OP_NONE,           1, 0x56,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },

  { "rorw",  OP_IND16 | OP_PAGE2,          3, 0x76,  4,  4,  CHG_NZVC, cpu6812x},
  { "rorw",  OP_IDX | OP_PAGE2,            2, 0x66,  3,  3,  CHG_NZVC, cpu6812x},
  { "rorw",  OP_IDX_1 | OP_PAGE2,          3, 0x66,  4,  4,  CHG_NZVC, cpu6812x},
  { "rorw",  OP_IDX_2 | OP_PAGE2,          4, 0x66,  5,  5,  CHG_NZVC, cpu6812x},
  { "rorw",  OP_D_IDX | OP_PAGE2,          2, 0x66,  6,  6,  CHG_NZVC, cpu6812x},
  { "rorw",  OP_D_IDX_2 | OP_PAGE2,        4, 0x66,  6,  6,  CHG_NZVC, cpu6812x},

  { "rorx", OP_NONE | OP_PAGE2,           1, 0x46,  1,  1,  CHG_NZVC, cpu6812x},
  { "rorx", OP_NONE | OP_PAGE2,           1, 0x56,  1,  1,  CHG_NZVC, cpu6812x},

  { "rtc",  OP_NONE,           1, 0x0a,  6,  6,  CHG_NONE, cpu6812 | cpu6812x },
  { "rti",  OP_NONE,           1, 0x3b, 12, 12,  CHG_ALL, cpu6811},
  { "rti",  OP_NONE,           1, 0x0b,  8, 10,  CHG_ALL, cpu6812 | cpu6812x},
  { "rts",  OP_NONE,           1, 0x39,  5,  5,  CHG_NONE, cpu6811 },
  { "rts",  OP_NONE,           1, 0x3d,  5,  5,  CHG_NONE, cpu6812 | cpu6812x },

  { "sba",  OP_NONE,             1, 0x10,  2,  2,  CHG_NZVC, cpu6811 },
  { "sba",  OP_NONE | OP_PAGE2, 2, 0x16,  2,  2,  CHG_NZVC, cpu6812 | cpu6812x },

  { "sbca", OP_IMM8,           2, 0x82,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbca", OP_DIRECT,         2, 0x92,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbca", OP_IND16,          3, 0xb2,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbca", OP_IX,             2, 0xa2,  4,  4,  CHG_NZVC, cpu6811 },
  { "sbca", OP_IY | OP_PAGE2,  3, 0xa2,  5,  5,  CHG_NZVC, cpu6811 },
  { "sbca", OP_IDX,            2, 0xa2,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbca", OP_IDX_1,          3, 0xa2,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbca", OP_IDX_2,          4, 0xa2,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbca", OP_D_IDX,          2, 0xa2,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbca", OP_D_IDX_2,        4, 0xa2,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "sbcb", OP_IMM8,           2, 0xc2,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbcb", OP_DIRECT,         2, 0xd2,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbcb", OP_IND16,          3, 0xf2,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "sbcb", OP_IX,             2, 0xe2,  4,  4,  CHG_NZVC, cpu6811 },
  { "sbcb", OP_IY | OP_PAGE2,  3, 0xe2,  5,  5,  CHG_NZVC, cpu6811 },
  { "sbcb", OP_IDX,            2, 0xe2,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbcb", OP_IDX_1,          3, 0xe2,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbcb", OP_IDX_2,          4, 0xe2,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbcb", OP_D_IDX,          2, 0xe2,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "sbcb", OP_D_IDX_2,        4, 0xe2,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "sbed", OP_IMM16 | OP_PAGE2,          3, 0x83,  2,  2,  CHG_NZVC, cpu6812x},
  { "sbed", OP_DIRECT | OP_PAGE2,         2, 0x93,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbed", OP_IND16 | OP_PAGE2,          3, 0xb3,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbed", OP_IDX | OP_PAGE2,            2, 0xa3,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbed", OP_IDX_1 | OP_PAGE2,          3, 0xa3,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbed", OP_IDX_2 | OP_PAGE2,          4, 0xa3,  4,  4,  CHG_NZVC, cpu6812x},
  { "sbed", OP_D_IDX | OP_PAGE2,          2, 0xa3,  6,  6,  CHG_NZVC, cpu6812x},
  { "sbed", OP_D_IDX_2 | OP_PAGE2,        4, 0xa3,  6,  6,  CHG_NZVC, cpu6812x},

  { "sbex", OP_IMM16 | OP_PAGE2,          3, 0x82,  2,  2,  CHG_NZVC, cpu6812x},
  { "sbex", OP_DIRECT | OP_PAGE2,         2, 0x92,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbex", OP_IND16 | OP_PAGE2,          3, 0xb2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbex", OP_IDX | OP_PAGE2,            2, 0xa2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbex", OP_IDX_1 | OP_PAGE2,          3, 0xa2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbex", OP_IDX_2 | OP_PAGE2,          4, 0xa2,  4,  4,  CHG_NZVC, cpu6812x},
  { "sbex", OP_D_IDX | OP_PAGE2,          2, 0xa2,  6,  6,  CHG_NZVC, cpu6812x},
  { "sbex", OP_D_IDX_2 | OP_PAGE2,        4, 0xa2,  6,  6,  CHG_NZVC, cpu6812x},

  { "sbey", OP_IMM16 | OP_PAGE2,          3, 0xc2,  2,  2,  CHG_NZVC, cpu6812x},
  { "sbey", OP_DIRECT | OP_PAGE2,         2, 0xd2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbey", OP_IND16 | OP_PAGE2,          3, 0xf2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbey", OP_IDX | OP_PAGE2,            2, 0xe2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbey", OP_IDX_1 | OP_PAGE2,          3, 0xe2,  3,  3,  CHG_NZVC, cpu6812x},
  { "sbey", OP_IDX_2 | OP_PAGE2,          4, 0xe2,  4,  4,  CHG_NZVC, cpu6812x},
  { "sbey", OP_D_IDX | OP_PAGE2,          2, 0xe2,  6,  6,  CHG_NZVC, cpu6812x},
  { "sbey", OP_D_IDX_2 | OP_PAGE2,        4, 0xe2,  6,  6,  CHG_NZVC, cpu6812x},

  { "sec",  OP_NONE,           1, 0x0d,  2,  2,  SET_C, cpu6811 },
  { "sei",  OP_NONE,           1, 0x0f,  2,  2,  SET_I, cpu6811 },
  { "sev",  OP_NONE,           1, 0x0b,  2,  2,  SET_V, cpu6811 },

  /* TODO  S12X has more modes */
  { "sex",  M6812_OP_SEX_MARKER
          | OP_REG | OP_REG_2, 2, 0xb7,  1,  1,  CHG_NONE, cpu6812 | cpu6812x },

  { "staa", OP_IND16,          3, 0xb7,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "staa", OP_DIRECT,         2, 0x97,  3,  3,  CLR_V_CHG_NZ, cpu6811 },
  { "staa", OP_IX,             2, 0xa7,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "staa", OP_IY | OP_PAGE2,  3, 0xa7,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "staa", OP_DIRECT,         2, 0x5a,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_IND16,          3, 0x7a,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_IDX,            2, 0x6a,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_IDX_1,          3, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_IDX_2,          4, 0x6a,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_D_IDX,          2, 0x6a,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "staa", OP_D_IDX_2,        4, 0x6a,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "stab", OP_IND16,          3, 0xf7,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "stab", OP_DIRECT,         2, 0xd7,  3,  3,  CLR_V_CHG_NZ, cpu6811 },
  { "stab", OP_IX,             2, 0xe7,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "stab", OP_IY | OP_PAGE2,  3, 0xe7,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "stab", OP_DIRECT,         2, 0x5b,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_IND16,          3, 0x7b,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_IDX,            2, 0x6b,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_IDX_1,          3, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_IDX_2,          4, 0x6b,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_D_IDX,          2, 0x6b,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stab", OP_D_IDX_2,        4, 0x6b,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "std",  OP_IND16,          3, 0xfd,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "std",  OP_DIRECT,         2, 0xdd,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "std",  OP_IX,             2, 0xed,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "std",  OP_IY | OP_PAGE2,  3, 0xed,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "std",  OP_DIRECT,         2, 0x5c,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_IND16,          3, 0x7c,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_IDX,            2, 0x6c,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_IDX_1,          3, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_IDX_2,          4, 0x6c,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_D_IDX,          2, 0x6c,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "std",  OP_D_IDX_2,        4, 0x6c,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "stop", OP_NONE,           1, 0xcf,  2,  2,  CHG_NONE, cpu6811 },
  { "stop", OP_NONE | OP_PAGE2,2, 0x3e,  2,  9,  CHG_NONE, cpu6812 | cpu6812x },

  { "sts",  OP_IND16,          3, 0xbf,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "sts",  OP_DIRECT,         2, 0x9f,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "sts",  OP_IX,             2, 0xaf,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "sts",  OP_IY | OP_PAGE2,  3, 0xaf,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "sts",  OP_DIRECT,         2, 0x5f,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_IND16,          3, 0x7f,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_IDX,            2, 0x6f,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_IDX_1,          3, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_IDX_2,          4, 0x6f,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_D_IDX,          2, 0x6f,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sts",  OP_D_IDX_2,        4, 0x6f,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "stx",  OP_IND16,          3, 0xff,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "stx",  OP_DIRECT,         2, 0xdf,  4,  4,  CLR_V_CHG_NZ, cpu6811 },
  { "stx",  OP_IX,             2, 0xef,  5,  5,  CLR_V_CHG_NZ, cpu6811 },
  { "stx",  OP_IY | OP_PAGE4,  3, 0xef,  6,  6,  CLR_V_CHG_NZ, cpu6811 },
  { "stx",  OP_DIRECT,         2, 0x5e,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_IND16,          3, 0x7e,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_IDX,            2, 0x6e,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_IDX_1,          3, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_IDX_2,          4, 0x6e,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_D_IDX,          2, 0x6e,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "stx",  OP_D_IDX_2,        4, 0x6e,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "sty",  OP_IND16 | OP_PAGE2,  4, 0xff, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "sty",  OP_DIRECT | OP_PAGE2, 3, 0xdf, 5, 5, CLR_V_CHG_NZ, cpu6811 },
  { "sty",  OP_IY | OP_PAGE2,     3, 0xef, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "sty",  OP_IX | OP_PAGE3,     3, 0xef, 6, 6, CLR_V_CHG_NZ, cpu6811 },
  { "sty",  OP_DIRECT,         2, 0x5d,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_IND16,          3, 0x7d,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_IDX,            2, 0x6d,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_IDX_1,          3, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_IDX_2,          4, 0x6d,  3,  3,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_D_IDX,          2, 0x6d,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },
  { "sty",  OP_D_IDX_2,        4, 0x6d,  5,  5,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "suba", OP_IMM8,           2, 0x80,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "suba", OP_DIRECT,         2, 0x90,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "suba", OP_IND16,          3, 0xb0,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "suba", OP_IX,             2, 0xa0,  4,  4,  CHG_NZVC, cpu6811 },
  { "suba", OP_IY | OP_PAGE2,  3, 0xa0,  5,  5,  CHG_NZVC, cpu6811 },
  { "suba", OP_IDX,            2, 0xa0,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "suba", OP_IDX_1,          3, 0xa0,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "suba", OP_IDX_2,          4, 0xa0,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "suba", OP_D_IDX,          2, 0xa0,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "suba", OP_D_IDX_2,        4, 0xa0,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "subb", OP_IMM8,           2, 0xc0,  1,  1,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subb", OP_DIRECT,         2, 0xd0,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subb", OP_IND16,          3, 0xf0,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subb", OP_IX,             2, 0xe0,  4,  4,  CHG_NZVC, cpu6811 },
  { "subb", OP_IY | OP_PAGE2,  3, 0xe0,  5,  5,  CHG_NZVC, cpu6811 },
  { "subb", OP_IDX,            2, 0xe0,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subb", OP_IDX_1,          3, 0xe0,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subb", OP_IDX_2,          4, 0xe0,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subb", OP_D_IDX,          2, 0xe0,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subb", OP_D_IDX_2,        4, 0xe0,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "subd", OP_IMM16,          3, 0x83,  2,  2,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subd", OP_DIRECT,         2, 0x93,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subd", OP_IND16,          3, 0xb3,  3,  3,  CHG_NZVC, cpu6811|cpu6812 | cpu6812x },
  { "subd", OP_IX,             2, 0xa3,  6,  6,  CHG_NZVC, cpu6811 },
  { "subd", OP_IY | OP_PAGE2,  3, 0xa3,  7,  7,  CHG_NZVC, cpu6811 },
  { "subd", OP_IDX,            2, 0xa3,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subd", OP_IDX_1,          3, 0xa3,  3,  3,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subd", OP_IDX_2,          4, 0xa3,  4,  4,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subd", OP_D_IDX,          2, 0xa3,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },
  { "subd", OP_D_IDX_2,        4, 0xa3,  6,  6,  CHG_NZVC, cpu6812 | cpu6812x },

  { "subx", OP_IMM16 | OP_PAGE2,          3, 0x80,  2,  2,  CHG_NZVC, cpu6812x},
  { "subx", OP_DIRECT | OP_PAGE2,         2, 0x90,  3,  3,  CHG_NZVC, cpu6812x},
  { "subx", OP_IND16 | OP_PAGE2,          3, 0xb0,  3,  3,  CHG_NZVC, cpu6812x},
  { "subx", OP_IDX | OP_PAGE2,            2, 0xa0,  3,  3,  CHG_NZVC, cpu6812x},
  { "subx", OP_IDX_1 | OP_PAGE2,          3, 0xa0,  3,  3,  CHG_NZVC, cpu6812x},
  { "subx", OP_IDX_2 | OP_PAGE2,          4, 0xa0,  4,  4,  CHG_NZVC, cpu6812x},
  { "subx", OP_D_IDX | OP_PAGE2,          2, 0xa0,  6,  6,  CHG_NZVC, cpu6812x},
  { "subx", OP_D_IDX_2 | OP_PAGE2,        4, 0xa0,  6,  6,  CHG_NZVC, cpu6812x},

  { "suby", OP_IMM16 | OP_PAGE2,          3, 0xc0,  2,  2,  CHG_NZVC, cpu6812x},
  { "suby", OP_DIRECT | OP_PAGE2,         2, 0xd0,  3,  3,  CHG_NZVC, cpu6812x},
  { "suby", OP_IND16 | OP_PAGE2,          3, 0xf0,  3,  3,  CHG_NZVC, cpu6812x},
  { "suby", OP_IDX | OP_PAGE2,            2, 0xe0,  3,  3,  CHG_NZVC, cpu6812x},
  { "suby", OP_IDX_1 | OP_PAGE2,          3, 0xe0,  3,  3,  CHG_NZVC, cpu6812x},
  { "suby", OP_IDX_2 | OP_PAGE2,          4, 0xe0,  4,  4,  CHG_NZVC, cpu6812x},
  { "suby", OP_D_IDX | OP_PAGE2,          2, 0xe0,  6,  6,  CHG_NZVC, cpu6812x},
  { "suby", OP_D_IDX_2 | OP_PAGE2,        4, 0xe0,  6,  6,  CHG_NZVC, cpu6812x},

  { "swi",  OP_NONE,           1, 0x3f,  9,  9,  CHG_NONE, cpu6811|cpu6812 | cpu6812x },

  { "tab",  OP_NONE,           1, 0x16,  2,  2,  CLR_V_CHG_NZ, cpu6811 },
  { "tab",  OP_NONE | OP_PAGE2,2, 0x0e,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "tap",  OP_NONE,           1, 0x06,  2,  2,  CHG_ALL, cpu6811 },

  { "tba",  OP_NONE,           1, 0x17,  2,  2,  CLR_V_CHG_NZ, cpu6811 },
  { "tba",  OP_NONE | OP_PAGE2,2, 0x0f,  2,  2,  CLR_V_CHG_NZ, cpu6812 | cpu6812x },

  { "test", OP_NONE,           1, 0x00,  5, _M,  CHG_NONE, cpu6811 },

  { "tpa",  OP_NONE,           1, 0x07,  2,  2,  CHG_NONE, cpu6811 },

  { "tbeq", OP_TBEQ_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },

  { "tbl",  OP_IDX | OP_PAGE2,  3, 0x3d,  8,  8, CHG_NZC, cpu6812 | cpu6812x },

  { "tbne", OP_TBNE_MARKER
          | OP_REG | OP_JUMP_REL,  3, 0x04,  3,  3, CHG_NONE, cpu6812 | cpu6812x },

          /* FIXME S12X has more options */
  { "tfr",  OP_TFR_MARKER
          | OP_REG_1 | OP_REG_2, 2, 0xb7, 1, 1,  CHG_NONE, cpu6812 | cpu6812x },

  { "trap", OP_IMM8 | OP_TRAP_ID, 2, 0x18,  11,  11,  SET_I, cpu6812 | cpu6812x },

  { "tst",  OP_IND16,          3, 0x7d,  6,  6,  CLR_VC_CHG_NZ, cpu6811 },
  { "tst",  OP_IX,             2, 0x6d,  6,  6,  CLR_VC_CHG_NZ, cpu6811 },
  { "tst",  OP_IY | OP_PAGE2,  3, 0x6d,  7,  7,  CLR_VC_CHG_NZ, cpu6811 },
  { "tst",  OP_IND16,          3, 0xf7,  3,  3,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tst",  OP_IDX,            2, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tst",  OP_IDX_1,          3, 0xe7,  3,  3,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tst",  OP_IDX_2,          4, 0xe7,  4,  4,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tst",  OP_D_IDX,          2, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tst",  OP_D_IDX_2,        4, 0xe7,  6,  6,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },

  { "tsta", OP_NONE,           1, 0x4d,  2,  2,  CLR_VC_CHG_NZ, cpu6811 },
  { "tsta", OP_NONE,           1, 0x97,  1,  1,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },
  { "tstb", OP_NONE,           1, 0x5d,  2,  2,  CLR_VC_CHG_NZ, cpu6811 },
  { "tstb", OP_NONE,           1, 0xd7,  1,  1,  CLR_VC_CHG_NZ, cpu6812 | cpu6812x },

  { "tsx",  OP_NONE,           1, 0x30,  3,  3,  CHG_NONE, cpu6811 },
  { "tsy",  OP_NONE | OP_PAGE2,2, 0x30,  4,  4,  CHG_NONE, cpu6811 },
  { "txs",  OP_NONE,           1, 0x35,  3,  3,  CHG_NONE, cpu6811 },
  { "tys",  OP_NONE | OP_PAGE2,2, 0x35,  4,  4,  CHG_NONE, cpu6811 },

  { "wai",  OP_NONE,           1, 0x3e,  5,  _M, CHG_NONE, cpu6811|cpu6812 | cpu6812x },

  { "wav",  OP_NONE | OP_PAGE2, 2, 0x3c,  8,  _M, SET_Z_CHG_HNVC, cpu6812 | cpu6812x },

  { "xgdx", OP_NONE,           1, 0x8f,  3,  3,  CHG_NONE, cpu6811 },
  { "xgdy", OP_NONE | OP_PAGE2,2, 0x8f,  4,  4,  CHG_NONE, cpu6811 }
};

const int m68hc11_num_opcodes = TABLE_SIZE (m68hc11_opcodes);

/* The following alias table provides source compatibility to
   move from 68HC11 assembly to 68HC12.  */
const struct m68hc12_opcode_alias m68hc12_alias[] = {
  { "abx", "leax b,x",   2, 0x1a, 0xe5 },
  { "aby", "leay b,y",   2, 0x19, 0xed },
  { "clc", "andcc #$fe", 2, 0x10, 0xfe },
  { "cli", "andcc #$ef", 2, 0x10, 0xef },
  { "clv", "andcc #$fd", 2, 0x10, 0xfd },
  { "des", "leas -1,sp", 2, 0x1b, 0x9f },
  { "ins", "leas 1,sp",  2, 0x1b, 0x81 },
  { "sec", "orcc #$01",  2, 0x14, 0x01 },
  { "sei", "orcc #$10",  2, 0x14, 0x10 },
  { "sev", "orcc #$02",  2, 0x14, 0x02 },
  { "tap", "tfr a,ccr",  2, 0xb7, 0x02 },
  { "tpa", "tfr ccr,a",  2, 0xb7, 0x20 },
  { "tsx", "tfr sp,x",   2, 0xb7, 0x75 },
  { "tsy", "tfr sp,y",   2, 0xb7, 0x76 },
  { "txs", "tfr x,sp",   2, 0xb7, 0x57 },
  { "tys", "tfr y,sp",   2, 0xb7, 0x67 },
  { "xgdx","exg d,x",    2, 0xb7, 0xc5 },
  { "xgdy","exg d,y",    2, 0xb7, 0xc6 }
};
const int m68hc12_num_alias = TABLE_SIZE (m68hc12_alias);
