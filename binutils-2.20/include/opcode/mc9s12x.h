/* m68hc11.h -- Header file for Motorola 68HC11 & 68HC12 opcode table
   Copyright 1999, 2000, 2002, 2003 Free Software Foundation, Inc.
   Written by Stephane Carrez (stcarrez@nerim.fr)

This file is part of GDB, GAS, and the GNU binutils.

GDB, GAS, and the GNU binutils are free software; you can redistribute
them and/or modify them under the terms of the GNU General Public
License as published by the Free Software Foundation; either version
1, or (at your option) any later version.

GDB, GAS, and the GNU binutils are distributed in the hope that they
will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file; see the file COPYING.  If not, write to the Free
Software Foundation, 51 Franklin Street - Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
 * :1: error: "MC9S12X_JMP" redefined
.././../../binutils-2.20/bfd/../include/opcode/mc9s12x.h:311:1: error: this is the location of the previous definition
.././../../binutils-2.20/bfd/../include/opcode/mc9s12x.h:315:1: error: "MC9S12X_BSR" redefined
.././../../binutils-2.20/bfd/../include/opcode/mc9s12x.h:312:1: error: this is the location of the previous definition
.././../../binutils-2.20/bfd/../include/opcode/mc9s12x.h:316:1: error: "MC9S12X_JSR" redefined
 *
 *
 *
 */

#ifndef _OPCODE_MC9S12X_H
#define _OPCODE_MC9S12X_H

/* Flags for the definition of the 68HC11 & 68HC12 CCR.  */
#define MC9S12X_S_BIT     0x80	/* Stop disable */
#define MC9S12X_X_BIT     0x40	/* X-interrupt mask */
#define MC9S12X_H_BIT     0x20	/* Half carry flag */
#define MC9S12X_I_BIT     0x10	/* I-interrupt mask */
#define MC9S12X_N_BIT     0x08	/* Negative */
#define MC9S12X_Z_BIT     0x04	/* Zero */
#define MC9S12X_V_BIT     0x02	/* Overflow */
#define MC9S12X_C_BIT     0x01	/* Carry */

/* 68HC11 register address offsets (range 0..0x3F or 0..64).
   The absolute address of the I/O register depends on the setting
   of the MC9S12X_INIT register.  At init time, the I/O registers are
   mapped at 0x1000.  Address of registers is then:

   0x1000 + MC9S12X_xxx
*/
#define MC9S12X_PORTA     0x00	/* Port A register */
#define MC9S12X__RES1	0x01	/* Unused/Reserved */
#define MC9S12X_PIOC	0x02	/* Parallel I/O Control register */
#define MC9S12X_PORTC	0x03	/* Port C register */
#define MC9S12X_PORTB	0x04	/* Port B register */
#define MC9S12X_PORTCL	0x05	/* Alternate latched port C */
#define MC9S12X__RES6	0x06	/* Unused/Reserved */
#define MC9S12X_DDRC	0x07	/* Data direction register for port C */
#define MC9S12X_PORTD	0x08	/* Port D register */
#define MC9S12X_DDRD	0x09	/* Data direction register for port D */
#define MC9S12X_PORTE	0x0A	/* Port E input register */
#define MC9S12X_CFORC	0x0B	/* Compare Force Register */
#define MC9S12X_OC1M	0x0C	/* OC1 Action Mask register */
#define MC9S12X_OC1D	0x0D	/* OC1 Action Data register */
#define MC9S12X_TCTN	0x0E	/* Timer Counter Register */
#define MC9S12X_TCTN_H	0x0E	/* "	 "	 " High part */
#define MC9S12X_TCTN_L	0x0F	/* "	 "	 " Low part */
#define MC9S12X_TIC1	0x10	/* Input capture 1 register */
#define MC9S12X_TIC1_H	0x10	/* "	 "	 " High part */
#define MC9S12X_TIC1_L	0x11	/* "	 "	 " Low part */
#define MC9S12X_TIC2	0x12	/* Input capture 2 register */
#define MC9S12X_TIC2_H	0x12	/* "	 "	 " High part */
#define MC9S12X_TIC2_L	0x13	/* "	 "	 " Low part */
#define MC9S12X_TIC3	0x14	/* Input capture 3 register */
#define MC9S12X_TIC3_H	0x14	/* "	 "	 " High part */
#define MC9S12X_TIC3_L	0x15	/* "	 "	 " Low part */
#define MC9S12X_TOC1	0x16	/* Output Compare 1 register */
#define MC9S12X_TOC1_H	0x16	/* "	 "	 " High part */
#define MC9S12X_TOC1_L	0x17	/* "	 "	 " Low part */
#define MC9S12X_TOC2	0x18	/* Output Compare 2 register */
#define MC9S12X_TOC2_H	0x18	/* "	 "	 " High part */
#define MC9S12X_TOC2_L	0x19	/* "	 "	 " Low part */
#define MC9S12X_TOC3	0x1A	/* Output Compare 3 register */
#define MC9S12X_TOC3_H	0x1A	/* "	 "	 " High part */
#define MC9S12X_TOC3_L	0x1B	/* "	 "	 " Low part */
#define MC9S12X_TOC4	0x1C	/* Output Compare 4 register */
#define MC9S12X_TOC4_H	0x1C	/* "	 "	 " High part */
#define MC9S12X_TOC4_L	0x1D	/* "	 "	 " Low part */
#define MC9S12X_TOC5	0x1E	/* Output Compare 5 register */
#define MC9S12X_TOC5_H	0x1E	/* "	 "	 " High part */
#define MC9S12X_TOC5_L	0x1F	/* "	 "	 " Low part */
#define MC9S12X_TCTL1	0x20	/* Timer Control register 1 */
#define MC9S12X_TCTL2	0x21	/* Timer Control register 2 */
#define MC9S12X_TMSK1	0x22	/* Timer Interrupt Mask Register 1 */
#define MC9S12X_TFLG1	0x23	/* Timer Interrupt Flag Register 1 */
#define MC9S12X_TMSK2	0x24	/* Timer Interrupt Mask Register 2 */
#define MC9S12X_TFLG2	0x25	/* Timer Interrupt Flag Register 2 */
#define MC9S12X_PACTL	0x26	/* Pulse Accumulator Control Register */
#define MC9S12X_PACNT	0x27	/* Pulse Accumulator Count Register */
#define MC9S12X_SPCR	0x28	/* SPI Control register */
#define MC9S12X_SPSR	0x29	/* SPI Status register */
#define MC9S12X_SPDR	0x2A	/* SPI Data register */
#define MC9S12X_BAUD	0x2B	/* SCI Baud register */
#define MC9S12X_SCCR1	0x2C	/* SCI Control register 1 */
#define MC9S12X_SCCR2	0x2D	/* SCI Control register 2 */
#define MC9S12X_SCSR	0x2E	/* SCI Status register */
#define MC9S12X_SCDR	0x2F	/* SCI Data (Read => RDR, Write => TDR) */
#define MC9S12X_ADCTL	0x30	/* A/D Control register */
#define MC9S12X_ADR1	0x31	/* A/D, Analog Result register 1 */
#define MC9S12X_ADR2	0x32	/* A/D, Analog Result register 2 */
#define MC9S12X_ADR3	0x33	/* A/D, Analog Result register 3 */
#define MC9S12X_ADR4	0x34	/* A/D, Analog Result register 4 */
#define MC9S12X__RES35	0x35
#define MC9S12X__RES36	0x36
#define MC9S12X__RES37	0x37
#define MC9S12X__RES38	0x38
#define MC9S12X_OPTION	0x39	/* System Configuration Options */
#define MC9S12X_COPRST	0x3A	/* Arm/Reset COP Timer Circuitry */
#define MC9S12X_PPROG	0x3B	/* EEPROM Programming Control Register */
#define MC9S12X_HPRIO	0x3C	/* Highest priority I-Bit int and misc */
#define MC9S12X_INIT	0x3D	/* Ram and I/O mapping register */
#define MC9S12X_TEST1	0x3E	/* Factory test control register */
#define MC9S12X_CONFIG	0x3F	/* COP, ROM and EEPROM enables */


/* Flags of the CONFIG register (in EEPROM).  */
#define MC9S12X_NOSEC     0x08    /* Security mode disable */
#define MC9S12X_NOCOP     0x04    /* COP system disable */
#define MC9S12X_ROMON     0x02    /* Enable on-chip rom */
#define MC9S12X_EEON      0x01    /* Enable on-chip eeprom */

/* Flags of the PPROG register.  */
#define MC9S12X_BYTE	0x10	/* Byte mode */
#define MC9S12X_ROW       0x08	/* Row mode */
#define MC9S12X_ERASE	0x04	/* Erase mode select (1 = erase, 0 = read) */
#define MC9S12X_EELAT     0x02	/* EEPROM Latch Control */
#define MC9S12X_EEPGM     0x01	/* EEPROM Programming Voltage Enable */

/* Flags of the PIOC register.  */
#define MC9S12X_STAF	0x80	/* Strobe A Interrupt Status Flag */
#define MC9S12X_STAI	0x40	/* Strobe A Interrupt Enable Mask */
#define MC9S12X_CWOM	0x20	/* Port C Wire OR mode */
#define MC9S12X_HNDS	0x10	/* Handshake mode */
#define MC9S12X_OIN	0x08	/* Output or Input handshaking */
#define MC9S12X_PLS	0x04	/* Pulse/Interlocked Handshake Operation */
#define MC9S12X_EGA	0x02	/* Active Edge for Strobe A */
#define MC9S12X_INVB	0x01	/* Invert Strobe B */

/* Flags of the SCCR1 register.  */
#define MC9S12X_R8	0x80	/* Receive Data bit 8 */
#define MC9S12X_T8	0x40	/* Transmit data bit 8 */
#define MC9S12X__SCCR1_5  0x20	/* Unused */
#define MC9S12X_M		0x10	/* SCI Character length */
#define MC9S12X_WAKE	0x08	/* Wake up method select (0=idle, 1=addr mark) */

/* Flags of the SCCR2 register.  */
#define MC9S12X_TIE	0x80	/* Transmit Interrupt enable */
#define MC9S12X_TCIE	0x40	/* Transmit Complete Interrupt Enable */
#define MC9S12X_RIE	0x20	/* Receive Interrupt Enable */
#define MC9S12X_ILIE	0x10	/* Idle Line Interrupt Enable */
#define MC9S12X_TE	0x08	/* Transmit Enable */
#define MC9S12X_RE	0x04	/* Receive Enable */
#define MC9S12X_RWU	0x02	/* Receiver Wake Up */
#define MC9S12X_SBK	0x01	/* Send Break */

/* Flags of the SCSR register.  */
#define MC9S12X_TDRE	0x80	/* Transmit Data Register Empty */
#define MC9S12X_TC	0x40	/* Transmit Complete */
#define MC9S12X_RDRF	0x20	/* Receive Data Register Full */
#define MC9S12X_IDLE	0x10	/* Idle Line Detect */
#define MC9S12X_OR	0x08	/* Overrun Error */
#define MC9S12X_NF	0x04	/* Noise Flag */
#define MC9S12X_FE	0x02	/* Framing Error */
#define MC9S12X__SCSR_0	0x01	/* Unused */

/* Flags of the BAUD register.  */
#define MC9S12X_TCLR	0x80	/* Clear Baud Rate (TEST mode) */
#define MC9S12X__BAUD_6	0x40	/* Not used */
#define MC9S12X_SCP1	0x20	/* SCI Baud rate prescaler select */
#define MC9S12X_SCP0	0x10
#define MC9S12X_RCKB	0x08	/* Baud Rate Clock Check (TEST mode) */
#define MC9S12X_SCR2	0x04	/* SCI Baud rate select */
#define MC9S12X_SCR1	0x02
#define MC9S12X_SCR0	0x01

#define MC9S12X_BAUD_DIV_1	(0)
#define MC9S12X_BAUD_DIV_3	(MC9S12X_SCP0)
#define MC9S12X_BAUD_DIV_4	(MC9S12X_SCP1)
#define MC9S12X_BAUD_DIV_13	(MC9S12X_SCP1|MC9S12X_SCP0)

/* Flags of the SPCR register.  */
#define MC9S12X_SPIE	0x80	/* Serial Peripheral Interrupt Enable */
#define MC9S12X_SPE	0x40	/* Serial Peripheral System Enable */
#define MC9S12X_DWOM	0x20	/* Port D Wire-OR mode option */
#define MC9S12X_MSTR	0x10	/* Master Mode Select */
#define MC9S12X_CPOL	0x08	/* Clock Polarity */
#define MC9S12X_CPHA	0x04	/* Clock Phase */
#define MC9S12X_SPR1	0x02	/* SPI Clock Rate Select */
#define MC9S12X_SPR0	0x01

/* Flags of the SPSR register.  */
#define MC9S12X_SPIF	0x80	/* SPI Transfer Complete flag */
#define MC9S12X_WCOL	0x40	/* Write Collision */
#define MC9S12X_MODF	0x10	/* Mode Fault */

/* Flags of the ADCTL register.  */
#define MC9S12X_CCF	0x80	/* Conversions Complete Flag */
#define MC9S12X_SCAN	0x20	/* Continuous Scan Control */
#define MC9S12X_MULT	0x10	/* Multiple Channel/Single Channel Control */
#define MC9S12X_CD	0x08	/* Channel Select D */
#define MC9S12X_CC	0x04	/*                C */
#define MC9S12X_CB	0x02	/*                B */
#define MC9S12X_CA	0x01	/*                A */

/* Flags of the CFORC register.  */
#define MC9S12X_FOC1	0x80	/* Force Output Compare 1 */
#define MC9S12X_FOC2	0x40	/*			2 */
#define MC9S12X_FOC3	0x20	/*			3 */
#define MC9S12X_FOC4	0x10	/*			4 */
#define MC9S12X_FOC5	0x08	/*			5 */

/* Flags of the OC1M register.  */
#define MC9S12X_OC1M7	0x80	/* Output Compare 7 */
#define MC9S12X_OC1M6	0x40	/*                6 */
#define MC9S12X_OC1M5	0x20	/*                5 */
#define MC9S12X_OC1M4	0x10	/*                4 */
#define MC9S12X_OC1M3	0x08	/*                3 */

/* Flags of the OC1D register.  */
#define MC9S12X_OC1D7	0x80
#define MC9S12X_OC1D6	0x40
#define MC9S12X_OC1D5	0x20
#define MC9S12X_OC1D4	0x10
#define MC9S12X_OC1D3	0x08

/* Flags of the TCTL1 register.  */
#define MC9S12X_OM2	0x80	/* Output Mode 2 */
#define MC9S12X_OL2	0x40	/* Output Level 2 */
#define MC9S12X_OM3	0x20
#define MC9S12X_OL3	0x10
#define MC9S12X_OM4	0x08
#define MC9S12X_OL4	0x04
#define MC9S12X_OM5	0x02
#define MC9S12X_OL5	0x01

/* Flags of the TCTL2 register.  */
#define MC9S12X_EDG1B	0x20	/* Input Edge Capture Control 1 */
#define MC9S12X_EDG1A	0x10
#define MC9S12X_EDG2B	0x08	/* Input 2 */
#define MC9S12X_EDG2A	0x04
#define MC9S12X_EDG3B	0x02	/* Input 3 */
#define MC9S12X_EDG3A	0x01

/* Flags of the TMSK1 register.  */
#define MC9S12X_OC1I	0x80	/* Output Compare 1 Interrupt */
#define MC9S12X_OC2I	0x40	/*		  2	      */
#define MC9S12X_OC3I	0x20	/*		  3	      */
#define MC9S12X_OC4I	0x10	/*		  4	      */
#define MC9S12X_OC5I	0x08	/*		  5	      */
#define MC9S12X_IC1I	0x04	/* Input Capture  1 Interrupt */
#define MC9S12X_IC2I	0x02	/*		  2	      */
#define MC9S12X_IC3I	0x01	/*		  3	      */

/* Flags of the TFLG1 register.  */
#define MC9S12X_OC1F	0x80	/* Output Compare 1 Flag */
#define MC9S12X_OC2F	0x40	/*		  2	 */
#define MC9S12X_OC3F	0x20	/*		  3	 */
#define MC9S12X_OC4F	0x10	/*		  4	 */
#define MC9S12X_OC5F	0x08	/*		  5	 */
#define MC9S12X_IC1F	0x04	/* Input Capture  1 Flag */
#define MC9S12X_IC2F	0x02	/*		  2	 */
#define MC9S12X_IC3F	0x01	/*		  3	 */

/* Flags of Timer Interrupt Mask Register 2 (TMSK2).  */
#define MC9S12X_TOI       0x80    /* Timer Overflow Interrupt Enable */
#define MC9S12X_RTII      0x40    /* RTI Interrupt Enable */
#define MC9S12X_PAOVI     0x20    /* Pulse Accumulator Overflow Interrupt En. */
#define MC9S12X_PAII      0x10    /* Pulse Accumulator Interrupt Enable */
#define MC9S12X_PR1       0x02    /* Timer prescaler */
#define MC9S12X_PR0       0x01    /* Timer prescaler */
#define MC9S12X_TPR_1     0x00    /* " " prescale div 1 */
#define MC9S12X_TPR_4     0x01    /* " " prescale div 4 */
#define MC9S12X_TPR_8     0x02    /* " " prescale div 8 */
#define MC9S12X_TPR_16    0x03    /* " " prescale div 16 */

/* Flags of Timer Interrupt Flag Register 2 (MC9S12X_TFLG2).  */
#define MC9S12X_TOF       0x80    /* Timer overflow bit */
#define MC9S12X_RTIF      0x40    /* Read time interrupt flag */
#define MC9S12X_PAOVF     0x20    /* Pulse accumulator overflow Interrupt flag */
#define MC9S12X_PAIF      0x10    /* Pulse accumulator Input Edge " " " */

/* Flags of Pulse Accumulator Control Register (PACTL).  */
#define MC9S12X_DDRA7     0x80    /* Data direction for port A bit 7 */
#define MC9S12X_PAEN      0x40    /* Pulse accumulator system enable */
#define MC9S12X_PAMOD     0x20    /* Pulse accumulator mode */
#define MC9S12X_PEDGE     0x10    /* Pulse accumulator edge control */
#define MC9S12X_RTR1      0x02    /* RTI Interrupt rates select */
#define MC9S12X_RTR0      0x01    /* " " " " */

/* Flags of the Options register.  */
#define MC9S12X_ADPU      0x80    /* A/D Powerup */
#define MC9S12X_CSEL      0x40    /* A/D/EE Charge pump clock source select */
#define MC9S12X_IRQE      0x20    /* IRQ Edge/Level sensitive */
#define MC9S12X_DLY       0x10    /* Stop exit turn on delay */
#define MC9S12X_CME       0x08    /* Clock Monitor enable */
#define MC9S12X_CR1       0x02    /* COP timer rate select */
#define MC9S12X_CR0       0x01    /* COP timer rate select */

/* Flags of the HPRIO register.  */
#define MC9S12X_RBOOT	0x80	/* Read Bootstrap ROM */
#define MC9S12X_SMOD	0x40	/* Special Mode */
#define MC9S12X_MDA	0x20	/* Mode Select A */
#define MC9S12X_IRV	0x10	/* Internal Read Visibility */
#define MC9S12X_PSEL3	0x08	/* Priority Select */
#define MC9S12X_PSEL2	0x04
#define MC9S12X_PSEL1	0x02
#define MC9S12X_PSEL0	0x01

/* Some insns used by gas to turn relative branches into absolute ones.  */
#define MC9S12X_BRA	0x20
#define MC9S12X_JMP	0x7e
#define MC9S12X_BSR	0x8d
#define MC9S12X_JSR	0xbd
#define MC9S12X_JMP2       0x06
#define MC9S12X_BSR2       0x07
#define MC9S12X_JSR2       0x16

/* Instruction code pages. Code page 1 is the default.  */
/*#define	MC9S12X_OPCODE_PAGE1	0x00*/
#define	MC9S12X_OPCODE_PAGE2	0x18
#define	MC9S12X_OPCODE_PAGE3	0x1A
#define	MC9S12X_OPCODE_PAGE4	0xCD


/* 68HC11 operands formats as stored in the MC9S12X_opcode table.  These
   flags do not correspond to anything in the 68HC11 or 68HC12.
   They are only used by GAS to recognize operands.  */
#define MC9S12X_OP_NONE         0        /* No operand */
#define MC9S12X_OP_DIRECT       0x0001   /* Page 0 addressing:   *<val-8bits>  */
#define MC9S12X_OP_IMM8         0x0002   /*  8 bits immediat:    #<val-8bits>  */
#define MC9S12X_OP_IMM16        0x0004   /* 16 bits immediat:    #<val-16bits> */
#define MC9S12X_OP_IND16        0x0008   /* Indirect abs:        <val-16>      */
#define MC9S12X_OP_IND16_P2     0x0010   /* Second parameter indirect abs.     */
#define MC9S12X_OP_REG          0x0020   /* Register operand 1                 */
#define MC9S12X_OP_REG_2        0x0040   /* Register operand 2                 */

#define MC9S12X_OP_IX           0x0080   /* Indirect IX:         <val-8>,x     */
#define MC9S12X_OP_IY           0x0100   /* Indirect IY:         <val-8>,y     */
#define MC9S12X_OP_IDX          0x0200   /* Indirect: N,r N,[+-]r[+-] N:5-bits */  //was SAVE
#define MC9S12X_OP_IDX_1        0x0400   /* N,r N:9-bits  */
#define MC9S12X_OP_IDX_2        0x0800   /* N,r N:16-bits */
#define MC9S12X_OP_D_IDX        0x1000   /* Indirect indexed: [D,r] */
#define MC9S12X_OP_D_IDX_2      0x2000   /* [N,r] N:16-bits */
#define MC9S12X_OP_PAGE         0x4000   /* Page number */
#define MC9S12X_OP_MASK         0x07FFF
#define MC9S12X_OP_BRANCH       0x00008000 /* Branch, jsr, call */
#define MC9S12X_OP_BITMASK      0x00010000 /* Bitmask:             #<val-8>    */
#define MC9S12X_OP_JUMP_REL     0x00020000 /* Pc-Relative:         <val-8>     */
#define MC9S12X_OP_JUMP_REL16   0x00040000 /* Pc-relative:         <val-16>    */
#define MC9S12X_OP_PAGE1        0x0000
#define MC9S12X_OP_PAGE2        0x00080000 /* Need a page2 opcode before       */
#define MC9S12X_OP_PAGE3        0x00100000 /* Need a page3 opcode before       */
#define MC9S12X_OP_PAGE4        0x00200000 /* Need a page4 opcode before       */
#define MC9S12X_MAX_OPERANDS    3     /* Max operands: brset <dst> <mask> <b> */

#define MC9S12X_ACC_OFFSET      0x00400000 /* A,r B,r D,r			    */
#define MC9S12X_ACC_IND         0x00800000 /* [D,r]			    */
#define MC9S12X_PRE_INC         0x01000000 /* n,+r   n = -8..8		    */
#define MC9S12X_PRE_DEC         0x02000000 /* n,-r			    */
#define MC9S12X_POST_INC        0x04000000 /* n,r+			    */
#define MC9S12X_POST_DEC        0x08000000 /* n,r-			    */
#define MC9S12X_INDEXED_IND     0x10000000 /* [n,r]  n = 16-bits		    */
#define MC9S12X_INDEXED         0x20000000 /* n,r    n = 5, 9 or 16-bits	    */
#define MC9S12X_OP_IDX_P2       0x40000000

/* Markers to identify some instructions.  */
#define MC9S12X_OP_EXG_MARKER   0x01000000 /* exg r1,r2 */
#define MC9S12X_OP_TFR_MARKER   0x02000000 /* tfr r1,r2 */
#define MC9S12X_OP_SEX_MARKER   0x04000000 /* sex r1,r2 */

#define MC9S12X_OP_EQ_MARKER    0x80000000 /* dbeq/ibeq/tbeq */
#define MC9S12X_OP_DBCC_MARKER  0x04000000 /* dbeq/dbne */
#define MC9S12X_OP_IBCC_MARKER  0x02000000 /* ibeq/ibne */
#define MC9S12X_OP_TBCC_MARKER  0x01000000

#define MC9S12X_OP_TRAP_ID      0x80000000 /* trap #N */

#define MC9S12X_OP_HIGH_ADDR    0x01000000 /* Used internally by gas.  */
#define MC9S12X_OP_LOW_ADDR     0x02000000

#define MC9S12X_BANK_VIRT 0x010000
#define MC9S12X_BANK_MASK 0x00003fff
#define MC9S12X_BANK_BASE 0x00008000
#define MC9S12X_BANK_SHIFT 14
#define MC9S12X_BANK_PAGE_MASK 0x0ff


/* CPU identification.  */
#define cpu6811 0x01
#define cpu6812 0x02
#define cpumc9s12x 0x06  /* was 0x4 somewhere it is bit-wised to 0x06 otherwise objdump will see it as UNKNOWN*/

/* The opcode table is an array of struct m68hc11_opcode.  */
struct mc9s12x_opcode {
  const char*    name;     /* Op-code name */
  long           format;
  unsigned char  size;
  unsigned char  opcode;
  unsigned char  cycles_low;
  unsigned char  cycles_high;
  unsigned char  set_flags_mask;
  unsigned char  clr_flags_mask;
  unsigned char  chg_flags_mask;
  unsigned char  arch;
};

/* Alias definition for 68HC12.  */
struct mc9s12x_opcode_alias
{
  const char*   name;
  const char*   translation;
  unsigned char size;
  unsigned char code1;
  unsigned char code2;
};

/* The opcode table.  The table contains all the opcodes (all pages).
   You can't rely on the order.  */
extern const struct mc9s12x_opcode mc9s12x_opcodes[];
extern const int mc9s12x_num_opcodes;

/* Alias table for 68HC12.  It translates some 68HC11 insn which are not
   implemented in 68HC12 but have equivalent translations.  */
extern const struct mc9s12x_opcode_alias mc9s12x_alias[];
extern const int mc9s12x_num_alias;

#endif /* _OPCODE_M68HC11_H */
