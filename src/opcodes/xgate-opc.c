/* mc9xgate-opc.c -- Motorola XGATE opcode list
   Copyright 1999, 2000, 2002, 2005, 2007 Free Software Foundation, Inc.
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
   along with this file; see the file COPYING.  If not, write to the
   Free Software Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.
*/

#include <stdio.h>
#include "ansidecl.h"
#include "opcode/xgate.h"

#define TABLE_SIZE(X)       (sizeof(X) / sizeof(X[0]))

/* Combination of CCR flags.  */
/* ORDER HI TO LOW NZVC */
#define XGATE_NZ_BIT		XGATE_N_BIT|XGATE_Z_BIT
#define XGATE_NV_BIT		XGATE_N_BIT|XGATE_V_BIT
#define XGATE_NC_BIT		XGATE_N_BIT|XGATE_C_BIT
#define XGATE_ZV_BIT		XGATE_Z_BIT|XGATE_V_BIT
#define XGATE_ZC_BIT		XGATE_Z_BIT|XGATE_C_BIT
#define XGATE_VC_BIT		XGATE_V_BIT|XGATE_C_BIT
#define XGATE_NVC_BIT	XGATE_NV_BIT|XGATE_C_BIT
#define XGATE_NZC_BIT	XGATE_NZ_BIT|XGATE_C_BIT
#define XGATE_NZV_BIT	XGATE_N_BIT|XGATE_Z_BIT|XGATE_V_BIT
#define XGATE_ZVC_BIT	XGATE_VC_BIT|XGATE_Z_BIT
#define XGATE_NZVC_BIT	XGATE_NZV_BIT|XGATE_C_BIT

/* Flags when the insn only changes some CCR flags.  */
#define CHG_NONE        0,0,0
#define CHG_Z           0,0,XGATE_Z_BIT
#define CHG_C           0,0,XGATE_C_BIT
#define CHG_ZVC         0,0,XGATE_ZVC_BIT
#define CHG_NZC         0,0,XGATE_NZC_BIT
#define CHG_NZV         0,0,XGATE_NZV_BIT
#define CHG_NZVC        0,0,(XGATE_NZVC_BIT)
#define CHG_HNZVC       0,0,XGATE_HNZVC_BIT  // TODO DELETE
#define CHG_ALL         0,0,0xff

/* The insn clears and changes some flags.  */
#define CLR_I           0,XGATE_I_BIT,0
#define CLR_C           0,XGATE_C_BIT,0
#define CLR_V           0,XGATE_V_BIT,0
#define CLR_V_CHG_ZC    0,XGATE_V_BIT,XGATE_ZC_BIT
#define CLR_V_CHG_NZ    0,XGATE_V_BIT,XGATE_NZ_BIT
#define CLR_V_CHG_ZVC   0,XGATE_V_BIT,XGATE_ZVC_BIT
#define CLR_N_CHG_ZVC   0,XGATE_N_BIT,XGATE_ZVC_BIT /* Used by lsr */
#define CLR_VC_CHG_NZ   0,XGATE_VC_BIT,XGATE_NZ_BIT

/* The insn sets some flags.  */
#define SET_I                   XGATE_I_BIT,0,0
#define SET_C                   XGATE_C_BIT,0,0
#define SET_V                   XGATE_V_BIT,0,0
#define SET_Z_CLR_NVC           XGATE_Z_BIT,XGATE_NVC_BIT,0
#define SET_C_CLR_V_CHG_NZ      XGATE_C_BIT,XGATE_V_BIT,XGATE_NZ_BIT
#define SET_Z_CHG_HNVC          XGATE_Z_BIT,0,XGATE_HNVC_BIT

/* operand modes */
#define OP_NONE         XGATE_OP_NONE
#define OP_INH		XGATE_OP_INH
#define OP_TRI		XGATE_OP_TRI
#define OP_DYA		XGATE_OP_DYA
#define OP_DM   	XGATE_OP_DYA_MON
#define OP_IMM3		XGATE_OP_IMM3
#define OP_IMM4		XGATE_OP_IMM4
#define OP_IMM8		XGATE_OP_IMM8
#define OP_IMM16	XGATE_OP_IMM16
#define OP_MON		XGATE_OP_MON
#define OP_MON_R_C	XGATE_OP_MON_R_C
#define OP_MON_C_R	XGATE_OP_MON_C_R
#define OP_MON_R_P	XGATE_OP_MON_R_P
#define OP_IDR	        XGATE_OP_IDR
#define OP_IDO5		XGATE_OP_IDO5
#define OP_REL9		XGATE_OP_REL9
#define OP_REL10    	XGATE_OP_REL10
/* macro operand modes */
#define OP_mADD    XGATE_OP_IMM16mADD
#define OP_mAND    XGATE_OP_IMM16mAND
#define OP_mCPC    XGATE_OP_IMM16mCPC
#define OP_mLDW    XGATE_OP_IMM16mLDW
#define OP_mSUB    XGATE_OP_IMM16mSUB

#define XGATE_ALL      XGATE_V1|XGATE_V2|XGATE_V3
#define XGATE_IP      XGATE_I|XGATE_PCREL

const struct xgate_opcode xgate_opcodes[] = {
/*    Name -+                                                                                 +-- cpu
Constraints --------+                                                              +------- Insn CCR changes
  Format ------------------+                                               +----------- Max # cycles
Format Bin -------------------------------------+                       +--------------- Min # cycles
Size --------------------------------------------------------+   +--------------------- Opcode */
    {   "adc",   OP_TRI, "00011rrrrrrrrr11", XGATE_R_R_R, 2, 0x1803, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "add",   OP_TRI, "00011rrrrrrrrr10", XGATE_R_R_R, 2, 0x1802, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "addh",  OP_IMM8, "11101rrriiiiiiii",   XGATE_R_I, 2, 0xE800, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "addl",  OP_IMM8, "11100rrriiiiiiii",   XGATE_R_I, 2, 0xE000, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "and",   OP_TRI, "00010rrrrrrrrr00", XGATE_R_R_R, 2, 0x1000, 1, 1,  CHG_NZV, XGATE_ALL},
    {  "andh",  OP_IMM8, "10001rrriiiiiiii",   XGATE_R_I, 2, 0x8800, 1, 1,  CHG_NZV, XGATE_ALL},
    {  "andl",  OP_IMM8, "10000rrriiiiiiii",   XGATE_R_I, 2, 0x8000, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "asr",  OP_IMM4, "00001rrriiii1001",   XGATE_R_I, 2, 0x0809, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "asr",   OP_DYA, "00001rrrrrr10001",   XGATE_R_R, 2, 0x0811, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "bcc",  OP_REL9, "0010000iiiiiiiii",   XGATE_IP,  2, 0x2000, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bcs",  OP_REL9, "0010001iiiiiiiii",   XGATE_IP,  2, 0x2200, 1, 2, CHG_NONE, XGATE_ALL},
    {   "beq",  OP_REL9, "0010011iiiiiiiii",   XGATE_IP,  2, 0x2600, 1, 2, CHG_NONE, XGATE_ALL},
    { "bfext",   OP_TRI, "01100rrrrrrrrr11", XGATE_R_R_R, 2, 0x6003, 1, 1,  CHG_NZV, XGATE_ALL},
    {  "bffo",   OP_DYA, "00001rrrrrr10000",   XGATE_R_R, 2, 0x0810, 1, 1, CHG_NZVC, XGATE_ALL},
    { "bfins",   OP_TRI, "01101rrrrrrrrr11", XGATE_R_R_R, 2, 0x6803, 1, 1,  CHG_NZV, XGATE_ALL},
    {"bfinsi",   OP_TRI, "01110rrrrrrrrr11", XGATE_R_R_R, 2, 0x7003, 1, 1,  CHG_NZV, XGATE_ALL},
    {"bfinsx",   OP_TRI, "01111rrrrrrrrr11", XGATE_R_R_R, 2, 0x7803, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "bge",  OP_REL9, "0011010iiiiiiiii",   XGATE_IP,  2, 0x3400, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bgt",  OP_REL9, "0011100iiiiiiiii",   XGATE_IP,  2, 0x3800, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bhi",  OP_REL9, "0011000iiiiiiiii",   XGATE_IP,  2, 0x3000, 1, 2, CHG_NONE, XGATE_ALL},
    {  "bith",  OP_IMM8, "10011rrriiiiiiii",   XGATE_R_I, 2, 0x9800, 1, 1,  CHG_NZV, XGATE_ALL},
    {  "bitl",  OP_IMM8, "10010rrriiiiiiii",   XGATE_R_I, 2, 0x9000, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "ble",  OP_REL9, "0011101iiiiiiiii",   XGATE_IP,  2, 0x3A00, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bls",  OP_REL9, "0011001iiiiiiiii",   XGATE_IP,  2, 0x3200, 1, 2, CHG_NONE, XGATE_ALL},
    {   "blt",  OP_REL9, "0011011iiiiiiiii",   XGATE_IP,  2, 0x3600, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bmi",  OP_REL9, "0010101iiiiiiiii",   XGATE_IP,  2, 0x2A00, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bne",  OP_REL9, "0010010iiiiiiiii",   XGATE_IP,  2, 0x2400, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bpl",  OP_REL9, "0010100iiiiiiiii",   XGATE_IP,  2, 0x2800, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bra", OP_REL10, "001111iiiiiiiiii",   XGATE_IP,  2, 0x3C00, 2, 2, CHG_NONE, XGATE_ALL},
    {   "brk",   OP_INH, "0000000000000000",   XGATE_INH, 2, 0x0000, 1, 1, CHG_NONE, XGATE_ALL},
    {   "bvc",  OP_REL9, "0010110iiiiiiiii",   XGATE_IP,  2, 0x2C00, 1, 2, CHG_NONE, XGATE_ALL},
    {   "bvs",  OP_REL9, "0010111iiiiiiiii",   XGATE_IP,  2, 0x2E00, 1, 2, CHG_NONE, XGATE_ALL},
    {  "cmpl",  OP_IMM8, "11010rrriiiiiiii",   XGATE_R_I, 2, 0xD000, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "cpch",  OP_IMM8, "11011rrriiiiiiii",   XGATE_R_I, 2, 0xD800, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "csem",  OP_IMM3, "00000iii11110000",    XGATE_I , 2, 0x00F0, 1, 1, CHG_NONE, XGATE_ALL},
    {  "csem",   OP_MON, "00000rrr11110001",     XGATE_R, 2, 0x00F1, 1, 1, CHG_NONE, XGATE_ALL},
    {   "csl",  OP_IMM4, "00001rrriiii1010",   XGATE_R_I, 2, 0x080A, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "csl",   OP_DYA, "00001rrrrrr10010",   XGATE_R_R, 2, 0x0812, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "csr",  OP_IMM4, "00001rrriiii1011",   XGATE_R_I, 2, 0x080B, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "csr",   OP_DYA, "00001rrrrrr10011",   XGATE_R_R, 2, 0x0813, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "jal",   OP_MON, "00000rrr11110110",     XGATE_R, 2, 0x00F6, 2, 2, CHG_NONE, XGATE_ALL},
    {   "ldb",  OP_IDO5, "01000rrrrrriiiii", XGATE_R_R_I, 2, 0x4000, 2, 2, CHG_NONE, XGATE_ALL},
    {   "ldb",   OP_IDR, "01100rrrrrrrrrrr", XGATE_R_R_R, 2, 0x6000, 2, 2, CHG_NONE, XGATE_ALL},
    {   "ldh",  OP_IMM8, "11111rrriiiiiiii",   XGATE_R_I, 2, 0xF800, 1, 1, CHG_NONE, XGATE_ALL},
    {   "ldl",  OP_IMM8, "11110rrriiiiiiii",   XGATE_R_I, 2, 0xF000, 1, 1, CHG_NONE, XGATE_ALL},
    {   "ldw",  OP_IDO5, "01001rrrrrriiiii", XGATE_R_R_I, 2, 0x4800, 2, 2, CHG_NONE, XGATE_ALL},
    {   "ldw",   OP_IDR, "01101rrrrrrrrrrr", XGATE_R_R_R, 2, 0x6800, 2, 2, CHG_NONE, XGATE_ALL},
    {   "lsl",  OP_IMM4, "00001rrriiii1100",   XGATE_R_I, 2, 0x080C, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "lsl",   OP_DYA, "00001rrrrrr10100",   XGATE_R_R, 2, 0x0814, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "lsr",  OP_IMM4, "00001rrriiii1101",   XGATE_R_I, 2, 0x080D, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "lsr",   OP_DYA, "00001rrrrrr10101",   XGATE_R_R, 2, 0x0815, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "nop",   OP_INH, "0000000100000000",   XGATE_INH, 2, 0x0100, 1, 1, CHG_NONE, XGATE_ALL},
    {    "or",   OP_TRI, "00010rrrrrrrrr10", XGATE_R_R_R, 2, 0x1002, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "orh",  OP_IMM8, "10101rrriiiiiiii",   XGATE_R_I, 2, 0xA800, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "orl",  OP_IMM8, "10100rrriiiiiiii",   XGATE_R_I, 2, 0xA000, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "par",   OP_MON, "00000rrr11110101",     XGATE_R, 2, 0x00F5, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "rol",  OP_IMM4, "00001rrriiii1110",   XGATE_R_I, 2, 0x080E, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "rol",   OP_DYA, "00001rrrrrr10110",   XGATE_R_R, 2, 0x0816, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "ror",  OP_IMM4, "00001rrriiii1111",   XGATE_R_I, 2, 0x080F, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "ror",   OP_DYA, "00001rrrrrr10111",   XGATE_R_R, 2, 0x0817, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "rts",   OP_INH, "0000001000000000",   XGATE_INH, 2, 0x0200, 2, 2, CHG_NONE, XGATE_ALL},
    {   "sbc",   OP_TRI, "00011rrrrrrrrr01", XGATE_R_R_R, 2, 0x1801, 1, 1,  CHG_NZV, XGATE_ALL},
    {  "ssem",  OP_IMM3, "00000iii11110010",    XGATE_I , 2, 0x00F2, 2, 2,    CHG_C, XGATE_ALL},
    {  "ssem",   OP_MON, "00000rrr11110011",     XGATE_R, 2, 0x00F3, 2, 2,    CHG_C, XGATE_ALL},
    {   "sex",   OP_MON, "00000rrr11110100",     XGATE_R, 2, 0x00F4, 1, 1,  CHG_NZV, XGATE_ALL},
    {   "sif",   OP_INH, "0000001100000000",   XGATE_INH, 2, 0x0300, 2, 2, CHG_NONE, XGATE_ALL},
    {   "sif",   OP_MON, "00000rrr11110111",     XGATE_R, 2, 0x00F7, 2, 2, CHG_NONE, XGATE_ALL},
    {   "stb",  OP_IDO5, "01010rrrrrriiiii", XGATE_R_R_I, 2, 0x5000, 2, 2, CHG_NONE, XGATE_ALL},
    {   "stb",   OP_IDR, "01110rrrrrrrrrrr", XGATE_R_R_R, 2, 0x7000, 2, 2, CHG_NONE, XGATE_ALL},
    {   "stw",  OP_IDO5, "01011rrrrrriiiii", XGATE_R_R_I, 2, 0x5800, 2, 2, CHG_NONE, XGATE_ALL},
    {   "stw",   OP_IDR, "01111rrrrrrrrrrr", XGATE_R_R_R, 2, 0x7800, 2, 2, CHG_NONE, XGATE_ALL},
    {   "sub",   OP_TRI, "00011rrrrrrrrr00", XGATE_R_R_R, 2, 0x1800, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "subh",  OP_IMM8, "11001rrriiiiiiii",   XGATE_R_I, 2, 0xC800, 1, 1, CHG_NZVC, XGATE_ALL},
    {  "subl",  OP_IMM8, "11000rrriiiiiiii",   XGATE_R_I, 2, 0xC000, 1, 1, CHG_NZVC, XGATE_ALL},
    {   "tfr",  OP_MON_R_C, "00000rrr11111000",XGATE_R_C, 2, 0x00F8, 1, 1, CHG_NONE, XGATE_ALL},
    {   "tfr",  OP_MON_C_R, "00000rrr11111001",XGATE_C_R, 2, 0x00F9, 1, 1, CHG_NONE, XGATE_ALL},
    {   "tfr",  OP_MON_R_P, "00000rrr11111010",XGATE_R_P, 2, 0x00FA, 1, 1, CHG_NONE, XGATE_ALL},
    {  "xnor",   OP_TRI, "00010rrrrrrrrr11", XGATE_R_R_R, 2, 0x1003, 1, 1,  CHG_NZV, XGATE_ALL},
    { "xnorh",  OP_IMM8, "10111rrriiiiiiii",   XGATE_R_I, 2, 0xB800, 1, 1,  CHG_NZV, XGATE_ALL},
    { "xnorl",  OP_IMM8, "10110rrriiiiiiii",   XGATE_R_I, 2, 0xB000, 1, 1,  CHG_NZV, XGATE_ALL},
    /*          macro and alias codes           */
    {   "add", OP_mADD,  "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "and", OP_mAND,  "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "bhs",  OP_REL9, "0010000iiiiiiiii",   XGATE_IP,  2, 0x2000, 0, 0, CHG_NONE, XGATE_ALL},
    {   "blo",  OP_REL9, "0010001iiiiiiiii",   XGATE_IP,  2, 0x2200, 0, 0, CHG_NONE, XGATE_ALL},
    {   "cmp",  OP_mCPC, "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "cmp",   OP_DYA, "00011sssrrrrrr00",   XGATE_R_R, 2, 0x1800, 0, 0, CHG_NZVC, XGATE_ALL},
    {   "com",  OP_DM, "00010rrrsssrrr11",     XGATE_R_R, 2, 0x1003, 0, 0,  CHG_NZV, XGATE_ALL},
    {   "cpc",   OP_DYA, "00011sssrrrrrr01",   XGATE_R_R, 2, 0x1801, 0, 0, CHG_NZVC, XGATE_ALL},
    {   "ld",   OP_mLDW, "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "ldd",  OP_mLDW, "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "ldw",  OP_mLDW, "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "mov",   OP_DYA, "00010rrrsssrrr10",   XGATE_R_R, 2, 0x1002, 0, 0, CHG_NZVC, XGATE_ALL},
    {   "neg",   OP_DYA, "00011rrrsssrrr00",   XGATE_R_R, 2, 0x1800, 0, 0, CHG_NZVC, XGATE_ALL},
    {   "sub",  OP_mSUB, "----------------",   XGATE_R_I, 4,      0, 0, 0, CHG_NONE, XGATE_ALL},
    {   "tst",   OP_MON, "00011sssrrrsss00",     XGATE_R, 2, 0x1800, 0, 0,  CHG_NZV, XGATE_ALL}
};

const int xgate_num_opcodes = TABLE_SIZE (xgate_opcodes);
