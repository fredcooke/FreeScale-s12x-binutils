/* m68hc11-opc.c -- Motorola 68HC11 & 68HC12 opcode list
   Copyright 1999, 2000, 2002, 2005, 2007 Free Software Foundation, Inc.
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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#include <stdio.h>
#include "ansidecl.h"
#include "opcode/mc9xgate.h"

#define TABLE_SIZE(X)       (sizeof(X) / sizeof(X[0]))

/* Combination of CCR flags.  */
/* ORDER HI TO LOW NZVC */
#define MC9XGATE_NZ_BIT		MC9XGATE_N_BIT|MC9XGATE_Z_BIT
#define MC9XGATE_NV_BIT		MC9XGATE_N_BIT|MC9XGATE_V_BIT
#define MC9XGATE_NC_BIT		MC9XGATE_N_BIT|MC9XGATE_C_BIT
#define MC9XGATE_ZV_BIT		MC9XGATE_Z_BIT|MC9XGATE_V_BIT
#define MC9XGATE_ZC_BIT		MC9XGATE_Z_BIT|MC9XGATE_C_BIT
#define MC9XGATE_VC_BIT		MC9XGATE_V_BIT|MC9XGATE_C_BIT
#define MC9XGATE_NVC_BIT	MC9XGATE_NV_BIT|MC9XGATE_C_BIT
#define MC9XGATE_NZC_BIT	MC9XGATE_NZ_BIT|MC9XGATE_C_BIT
#define MC9XGATE_NZV_BIT	MC9XGATE_N_BIT|MC9XGATE_Z_BIT|MC9XGATE_V_BIT
#define MC9XGATE_ZVC_BIT	MC9XGATE_VC_BIT|MC9XGATE_Z_BIT
#define MC9XGATE_NZVC_BIT	MC9XGATE_NZV_BIT|MC9XGATE_C_BIT

/* Flags when the insn only changes some CCR flags.  */
#define CHG_NONE        0,0,0
#define CHG_Z           0,0,MC9XGATE_Z_BIT
#define CHG_C           0,0,MC9XGATE_C_BIT
#define CHG_ZVC         0,0,MC9XGATE_ZVC_BIT
#define CHG_NZC         0,0,MC9XGATE_NZC_BIT
#define CHG_NZV         0,0,MC9XGATE_NZV_BIT
#define CHG_NZVC        0,0,MC9XGATE_NZVC_BIT
#define CHG_HNZVC       0,0,MC9XGATE_HNZVC_BIT  // TODO DELETE
#define CHG_ALL         0,0,0xff

/* The insn clears and changes some flags.  */
#define CLR_I           0,MC9XGATE_I_BIT,0
#define CLR_C           0,MC9XGATE_C_BIT,0
#define CLR_V           0,MC9XGATE_V_BIT,0
#define CLR_V_CHG_ZC    0,MC9XGATE_V_BIT,MC9XGATE_ZC_BIT
#define CLR_V_CHG_NZ    0,MC9XGATE_V_BIT,MC9XGATE_NZ_BIT
#define CLR_V_CHG_ZVC   0,MC9XGATE_V_BIT,MC9XGATE_ZVC_BIT
#define CLR_N_CHG_ZVC   0,MC9XGATE_N_BIT,MC9XGATE_ZVC_BIT /* Used by lsr */
#define CLR_VC_CHG_NZ   0,MC9XGATE_VC_BIT,MC9XGATE_NZ_BIT

/* The insn sets some flags.  */
#define SET_I           MC9XGATE_I_BIT,0,0
#define SET_C           MC9XGATE_C_BIT,0,0
#define SET_V           MC9XGATE_V_BIT,0,0
#define SET_Z_CLR_NVC   MC9XGATE_Z_BIT,MC9XGATE_NVC_BIT,0
#define SET_C_CLR_V_CHG_NZ MC9XGATE_C_BIT,MC9XGATE_V_BIT,MC9XGATE_NZ_BIT
#define SET_Z_CHG_HNVC  MC9XGATE_Z_BIT,0,MC9XGATE_HNVC_BIT

#define _M 0xff
#define OP_NONE         MC9XGATE_OP_NONE
#define OP_PAGE2        MC9XGATE_OP_PAGE2
#define OP_PAGE3        MC9XGATE_OP_PAGE3
#define OP_PAGE4        MC9XGATE_OP_PAGE4
#define OP_IMM8         MC9XGATE_OP_IMM8
#define OP_IMM16        MC9XGATE_OP_IMM16
#define OP_IX           MC9XGATE_OP_IX
#define OP_IY           MC9XGATE_OP_IY
#define OP_IND16        MC9XGATE_OP_IND16
#define OP_PAGE         MC9XGATE_OP_PAGE  //
#define OP_IDX          MC9XGATE_OP_IDX  //was saved
#define OP_IDX_1        MC9XGATE_OP_IDX_1//
#define OP_IDX_2        MC9XGATE_OP_IDX_2//
#define OP_D_IDX        MC9XGATE_OP_D_IDX//
#define OP_D_IDX_2      MC9XGATE_OP_D_IDX_2//
#define OP_DIRECT       MC9XGATE_OP_DIRECT
#define OP_BITMASK      MC9XGATE_OP_BITMASK
#define OP_BRANCH       MC9XGATE_OP_BRANCH
#define OP_JUMP_REL     (MC9XGATE_OP_JUMP_REL|OP_BRANCH)
#define OP_JUMP_REL16   (MC9XGATE_OP_JUMP_REL16|OP_BRANCH)//
#define OP_REG          MC9XGATE_OP_REG//
#define OP_REG_1        MC9XGATE_OP_REG//
#define OP_REG_2        MC9XGATE_OP_REG_2//
#define OP_IDX_p2       MC9XGATE_OP_IDX_P2//
#define OP_IND16_p2     MC9XGATE_OP_IND16_P2//
#define OP_TRAP_ID      MC9XGATE_OP_TRAP_ID//
#define OP_EXG_MARKER   MC9XGATE_OP_EXG_MARKER
#define OP_TFR_MARKER   MC9XGATE_OP_TFR_MARKER//
#define OP_DBEQ_MARKER  (MC9XGATE_OP_DBCC_MARKER|MC9XGATE_OP_EQ_MARKER)//
#define OP_DBNE_MARKER  (MC9XGATE_OP_DBCC_MARKER)//
#define OP_TBEQ_MARKER  (MC9XGATE_OP_TBCC_MARKER|MC9XGATE_OP_EQ_MARKER)//
#define OP_TBNE_MARKER  (MC9XGATE_OP_TBCC_MARKER)//
#define OP_IBEQ_MARKER  (MC9XGATE_OP_IBCC_MARKER|MC9XGATE_OP_EQ_MARKER)//
#define OP_IBNE_MARKER  (MC9XGATE_OP_IBCC_MARKER)//


/* ######################  OLD STRUCTURE ####################################################### */
/*
   { "test", OP_NONE,          1, 0x00,  5, _M,  CHG_NONE,  cpu6811 },
                                                            +-- cpu
  Name -+                                        +------- Insn CCR changes
  Format  ------+                            +----------- Max # cycles
  Size     --------------------+         +--------------- Min # cycles
                                   +--------------------- Opcode
*/


/*
 { "test", IMM3,			1, 0xAB,	3,	1, CHG_NONE, cpumc9xgate }



 */
const struct mc9xgate_opcode mc9xgate_opcodes[] = {

	{ "brk", OP_NONE,			1,0xaa,	1,	1, CHG_NONE,cpumc9xgate},// for testing 0xaa should be 0x0000
	{ "csem",OP_NONE,			1,0xff,1,2,CHG_NONE,cpumc9xgate}
//  { "aba",  OP_NONE,           1, 0x1b,  2,  2,  CHG_HNZVC, cpu6811 },
//  { "aba",  OP_NONE | OP_PAGE2,2, 0x06,  2,  2,  CHG_HNZVC, cpumc9xgate },
//  { "abx",  OP_NONE,           1, 0x3a,  3,  3,  CHG_NONE,  cpu6811 },
//  { "xgdy", OP_NONE | OP_PAGE2,2, 0x8f,  4,  4,  CHG_NONE, cpu6811 }
//   and so on */
};

const int mc9xgate_num_opcodes = TABLE_SIZE (mc9xgate_opcodes);

/* The following alias table provides source compatibility to
   move from 68HC11 assembly to 68HC12.  */
const struct mc9xgate_opcode_alias mc9xgate_alias[] = {
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
const int mc9xgate_num_alias = TABLE_SIZE (mc9xgate_alias);
