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
   MA 02110-1301, USA.

   mc9xgate-opc.c

     */

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
#define CHG_NZVC        0,0,(MC9XGATE_NZVC_BIT)
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
#define OP_INH			MC9XGATE_OP_INH
#define OP_TRI			MC9XGATE_OP_TRI
#define OP_DYA			MC9XGATE_OP_DYA
#define OP_IMM4			MC9XGATE_OP_IMM4
#define OP_MON			MC9XGATE_OP_MON
#define OP_MON_R_C		MC9XGATE_OP_MON_R_C
#define OP_MON_C_R		MC9XGATE_OP_MON_C_R
#define OP_MON_R_P		MC9XGATE_OP_MON_R_P
#define OP_IDR	        MC9XGATE_OP_IDR
#define OP_IDO5		    MC9XGATE_OP_IDO5
#define OP_REL9			MC9XGATE_OP_REL9

#define BI_MODE(mode1,mode2)	"mode1 | mode2"

/*
                            Table 4-1 Access Detail Notation
V — Vector fetch: always an aligned word read, lasts for at least one RISC core cycle
P — Program word fetch: always an aligned word read, lasts for at least one RISC core cycle
r — 8-bit data read: lasts for at least one RISC core cycle
R — 16-bit data read: lasts for at least one RISC core cycle
w — 8-bit data write: lasts for at least one RISC core cycle
W — 16-bit data write: lasts for at least one RISC core cycle
A — Alignment cycle: no read or write, lasts for zero or one RISC core cycles
f — Free cycle: no read or write, lasts for one RISC core cycles

 */

/*
   { "test", OP_NONE,          1, 0x00,  5, _M,  CHG_NONE,  cpu6811 },
                                                            +-- cpu
  Name -+                                        +------- Insn CCR changes
  Format  ------+                            +----------- Max # cycles
  Size     --------------------+         +--------------- Min # cycles
                                   +--------------------- Opcode
*/

/* it seems each insns mode will need its own entry based on this example
                      Mode
ASR RD, #IMM4       IMM4     0 0 0 0 1  RD      IMM4   1 0 0 1 P
ASR RD, RS          DYA      0 0 0 0 1  RD      RS   1 0 0 0 1 P
 */

/* TODO reinstate const */
struct mc9xgate_opcode mc9xgate_opcodes[] = {

	{ "brk",      OP_INH, "0000000000000000",   MC9XGATE_INH, 2, 0x0000, 1, 1, CHG_NONE, cpumc9xgate},
	{ "nop",      OP_INH, "0000000100000000",   MC9XGATE_INH, 2, 0x0100, 1, 2, CHG_NONE, cpumc9xgate},
	{ "adc",      OP_TRI, "00011rrrrrrrrr11", MC9XGATE_R_R_R, 2, 0x1803, 1, 2, CHG_NZVC, cpumc9xgate},

	{ "bcc",     OP_REL9, "0010000iiiiiiiii",   MC9XGATE_I,   2, 0x2000, 1, 2, CHG_NZVC, cpumc9xgate},

	{ "asr",      OP_DYA, "00001rrrrrr10001",   MC9XGATE_R_R, 2, 0x0811, 1, 2, CHG_NZVC, cpumc9xgate},
	{ "asr",     OP_IMM4, "00001rrriiii1001",   MC9XGATE_R_I, 2, 0x0809, 1, 2, CHG_NZVC, cpumc9xgate},

	{ "test",    OP_IMM4, "0000011rrrrrrr11",   MC9XGATE_R_I, 2, 0x0603, 1, 2, CHG_NZVC, cpumc9xgate},

	{ "tfr",  OP_MON_R_C, "00000rrr11111000",   MC9XGATE_R_C, 2, 0x00F8, 1, 2, CHG_NONE, cpumc9xgate},
	{ "tfr",  OP_MON_C_R, "00000rrr11111001",   MC9XGATE_C_R, 2, 0x00F9, 1, 2, CHG_NONE, cpumc9xgate},
	{ "tfr",  OP_MON_R_P, "00000rrr11111010",   MC9XGATE_R_P, 2, 0x00FA, 1, 2, CHG_NONE, cpumc9xgate},

	{ "ldw",     OP_IDO5, "01001rrrrrriiiii", MC9XGATE_R_R_I, 2, 0x4800, 1, 2, CHG_NONE, cpumc9xgate},
	{ "ldw",      OP_IDR, "01101rrrrrrrrrrr", MC9XGATE_R_R_R, 2, 0x6800, 1, 2, CHG_NONE, cpumc9xgate},

	{ "com",      OP_DYA, "00010rrr000rrr11",   MC9XGATE_R_R, 2, 0x1003, 1, 2, CHG_NONE, cpumc9xgate}
//	{  NULL,    NULL,             NULL  , 0,      0, 0, 0,	0,0,0, 0}  /* NULL termination makes for less book keepign code */

};

const int mc9xgate_num_opcodes = TABLE_SIZE (mc9xgate_opcodes);

