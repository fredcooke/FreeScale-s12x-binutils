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
#define OP_INH			MC9XGATE_OP_INH


/*
   { "test", OP_NONE,          1, 0x00,  5, _M,  CHG_NONE,  cpu6811 },
                                                            +-- cpu
  Name -+                                        +------- Insn CCR changes
  Format  ------+                            +----------- Max # cycles
  Size     --------------------+         +--------------- Min # cycles
                                   +--------------------- Opcode
*/

/* TODO reinstate const */
struct mc9xgate_opcode mc9xgate_opcodes[] = {

	{ "brk",  OP_INH, "0000000000000000", 1, 0x0000, 1, 1, CHG_NONE, cpumc9xgate},
	{ "csem", OP_INH, "0000000100000000", 1, 0x0100, 1, 2, CHG_NONE, cpumc9xgate},
	{  NULL,    NULL,             NULL  , 0,      0, 0, 0,	0,0,0, 0}  /* NULL termination makes for less book keepign code */

};

const int mc9xgate_num_opcodes = TABLE_SIZE (mc9xgate_opcodes);

//const int mc9xgate_num_alias = TABLE_SIZE (mc9xgate_alias);
