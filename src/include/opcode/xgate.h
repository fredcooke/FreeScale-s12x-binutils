/* xgate.h -- Motorola XGATE opcode list
   Copyright 2010, 2011, 2012 Free Software Foundation, Inc.
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
   MA 02110-1301, USA. */

#ifndef _OPCODE_XGATE_H
#define _OPCODE_XGATE_H

/* XGATE CCR flag definitions */
#define XGATE_N_BIT   0x08	/* XGN - Sign Flag */
#define XGATE_Z_BIT   0x04	/* XGZ - Zero Flag */
#define XGATE_V_BIT   0x02	/* XGV - Overflow Flag */
#define XGATE_C_BIT   0x01	/* XGC - Carry Flag */

/*              Access Detail Notation
V — Vector fetch: always an aligned word read, lasts for at least one RISC core cycle
P — Program word fetch: always an aligned word read, lasts for at least one RISC core cycle
r — 8-bit data read: lasts for at least one RISC core cycle
R — 16-bit data read: lasts for at least one RISC core cycle
w — 8-bit data write: lasts for at least one RISC core cycle
W — 16-bit data write: lasts for at least one RISC core cycle
A — Alignment cycle: no read or write, lasts for zero or one RISC core cycles
f — Free cycle: no read or write, lasts for one RISC core cycles */
#define XGATE_CYCLE_V	0x01
#define XGATE_CYCLE_P	0x02
#define XGATE_CYCLE_r	0x04
#define XGATE_CYCLE_R	0x08
#define XGATE_CYCLE_w	0x10
#define XGATE_CYCLE_W	0x20
#define XGATE_CYCLE_A	0x40
#define XGATE_CYCLE_f	0x80

/* Opcode format abbreviations */
#define XGATE_INH		0x0001  /* inherent */
#define XGATE_I		        0x0002  /* 3-bit immediate address */
#define XGATE_R_I		0x0004  /* register followed by 4/8-bit immediate value */
#define XGATE_R_R		0x0008  /* register followed by a register */
#define XGATE_R_R_R		0x0010  /* register followed by two registers */
#define XGATE_R		        0x0020  /* single register */
#define XGATE_PC		0x0040  /* pc relative 10 or 11 bit */
#define XGATE_R_C		0x0080  /* general register followed by ccr register */
#define XGATE_C_R		0x0100  /* ccr register followed by a general register */
#define XGATE_R_P		0x0200  /* general register followed by pc register */
#define XGATE_R_R_I		0x0400  /* two general registers followed by an immediate value */
#define XGATE_PCREL		0x0800  /* immediate value that is relative to the current pc */

/* XGATE operand formats as stored in the XGATE_opcode table.
   They are only used by GAS to recognize operands.  */
#define XGATE_OP_INH		""
#define XGATE_OP_TRI		"r,r,r"
#define XGATE_OP_DYA		"r,r"
#define XGATE_OP_IMM16          "r,if"
#define XGATE_OP_IMM8	        "r,i8"
#define XGATE_OP_IMM4           "r,i4"
#define XGATE_OP_IMM3	        "i3"
#define XGATE_OP_MON		"r"
#define XGATE_OP_MON_R_C	"r,c"
#define XGATE_OP_MON_C_R	"c,r"
#define XGATE_OP_MON_R_P	"r,p"
#define XGATE_OP_IDR		"r,r,+"
#define XGATE_OP_IDO5	        "r,r,i5"
#define XGATE_OP_REL9	        "b9"
#define XGATE_OP_REL10	        "ba"
#define XGATE_OP_DYA_MON	"=r,r"
/* macro definitions */
#define XGATE_OP_IMM16mADD    "r,if; addl addh"
#define XGATE_OP_IMM16mAND    "r,if; andl andh"
#define XGATE_OP_IMM16mCPC    "r,if; cmpl cpch"
#define XGATE_OP_IMM16mSUB    "r,if; subl subh"
#define XGATE_OP_IMM16mLDW    "r,if; ldl ldh"

/* CPU variant identification  */
#define XGATE_V1 0x1
#define XGATE_V2 0x2
#define XGATE_V3 0x4

/* Max opcodes per opcode handle */
#define MAX_OPCODES     0x05

#define MAX_DETECT_CHARS 0x10

/* The opcode table definitions */
struct xgate_opcode
{
  char* name;                   /* Op-code name */
  char* constraints;            /* constraint chars */
  char* format;                 /* bit definitions */
  unsigned int sh_format;       /* shorthand format mask */
  unsigned int size;            /* opcode size in bytes */
  unsigned int bin_opcode;      /* binary opcode with operands masked off */
  unsigned char cycles_min;     /* minimum cpu cycles needed */
  unsigned char cycles_max;     /* maximum cpu cycles needed */
  unsigned char set_flags_mask; /* CCR flags set */
  unsigned char clr_flags_mask; /* CCR flags cleared */
  unsigned char chg_flags_mask; /* CCR flags changed */
  unsigned char arch;           /* cpu variant */
};

/* The opcode table.  The table contains all the opcodes (all pages).
   You can't rely on the order.  */
extern const struct xgate_opcode xgate_opcodes[];
extern const int xgate_num_opcodes;

#endif /* _OPCODE_XGATE_H */
