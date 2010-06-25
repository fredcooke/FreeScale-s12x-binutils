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

#ifndef _OPCODE_MC9XGATE_H
#define _OPCODE_MC9XGATE_H

/* Flags for the definition of the 68HC11 & 68HC12 CCR.  */
//#define MC9XGATE_S_BIT     0x80	/* Stop disable */
//#define MC9XGATE_X_BIT     0x40	/* X-interrupt mask */
//#define MC9XGATE_H_BIT     0x20	/* Half carry flag */
//#define MC9XGATE_I_BIT     0x10	/* I-interrupt mask */
//#define MC9XGATE_N_BIT     0x08	/* Negative */
#define MC9XGATE_N_BIT   0x08	/* XGN - Sign Flag */
#define MC9XGATE_Z_BIT   0x04	/* XGZ - Zero Flag */
#define MC9XGATE_V_BIT   0x02	/* XGV - Overflow Flag */
#define MC9XGATE_C_BIT   0x01	/* XGC - Carry Flag */

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

#define MC9XGATE_CYCLE_V	0x01
#define MC9XGATE_CYCLE_P	0x02
#define MC9XGATE_CYCLE_r	0x04
#define MC9XGATE_CYCLE_R	0x08
#define MC9XGATE_CYCLE_w	0x10
#define MC9XGATE_CYCLE_W	0x20
#define MC9XGATE_CYCLE_A	0x40
#define MC9XGATE_CYCLE_f	0x80


/* Opcode format abbrivations */
#define MC9XGATE_INH		0x0001  /* inherent */
#define MC9XGATE_I			0x0002  /* 3-bit immediate address */
#define MC9XGATE_R_I		0x0004  /* register followed by 4/8-bit immediate address */
#define MC9XGATE_R_R		0x0008  /* register followed by a register */
#define MC9XGATE_R_R_R		0x0010  /* register followed by two registers */
#define MC9XGATE_R			0x0020  /* single register */
#define MC9XGATE_PC			0x0040  /* pc relative 10 or 11 bit */
#define MC9XGATE_R_C		0x0080
#define MC9XGATE_C_R		0x0100
#define MC9XGATE_R_P		0x0200
#define MC9XGATE_R_R_O		0x0400
#define MC9XGATE_R_R_I		0x0800
#define MC9XGATE_PCREL		0x1000  /* Instruction is a PC relative instruction */


/* 68HC11 register address offsets (range 0..0x3F or 0..64).
   The absolute address of the I/O register depends on the setting
   of the MC9XGATE_INIT register.  At init time, the I/O registers are
   mapped at 0x1000.  Address of registers is then:

   0x1000 + MC9XGATE_xxx
   TODO Update to S12X specs
*/
#define MC9XGATE_PORTA  0x00	/* Port A register */
#define MC9XGATE__RES1	0x01	/* Unused/Reserved */
#define MC9XGATE_PIOC	0x02	/* Parallel I/O Control register */
#define MC9XGATE_PORTC	0x03	/* Port C register */
#define MC9XGATE_PORTB	0x04	/* Port B register */
#define MC9XGATE_PORTCL	0x05	/* Alternate latched port C */
#define MC9XGATE__RES6	0x06	/* Unused/Reserved */
#define MC9XGATE_DDRC	0x07	/* Data direction register for port C */
#define MC9XGATE_PORTD	0x08	/* Port D register */
#define MC9XGATE_DDRD	0x09	/* Data direction register for port D */
#define MC9XGATE_PORTE	0x0A	/* Port E input register */
#define MC9XGATE_CFORC	0x0B	/* Compare Force Register */
#define MC9XGATE_OC1M	0x0C	/* OC1 Action Mask register */
#define MC9XGATE_OC1D	0x0D	/* OC1 Action Data register */
#define MC9XGATE_TCTN	0x0E	/* Timer Counter Register */
#define MC9XGATE_TCTN_H	0x0E	/* "	 "	 " High part */
#define MC9XGATE_TCTN_L	0x0F	/* "	 "	 " Low part */
#define MC9XGATE_TIC1	0x10	/* Input capture 1 register */
#define MC9XGATE_TIC1_H	0x10	/* "	 "	 " High part */
#define MC9XGATE_TIC1_L	0x11	/* "	 "	 " Low part */
#define MC9XGATE_TIC2	0x12	/* Input capture 2 register */
#define MC9XGATE_TIC2_H	0x12	/* "	 "	 " High part */
#define MC9XGATE_TIC2_L	0x13	/* "	 "	 " Low part */
#define MC9XGATE_TIC3	0x14	/* Input capture 3 register */
#define MC9XGATE_TIC3_H	0x14	/* "	 "	 " High part */
#define MC9XGATE_TIC3_L	0x15	/* "	 "	 " Low part */
#define MC9XGATE_TOC1	0x16	/* Output Compare 1 register */
#define MC9XGATE_TOC1_H	0x16	/* "	 "	 " High part */
#define MC9XGATE_TOC1_L	0x17	/* "	 "	 " Low part */
#define MC9XGATE_TOC2	0x18	/* Output Compare 2 register */
#define MC9XGATE_TOC2_H	0x18	/* "	 "	 " High part */
#define MC9XGATE_TOC2_L	0x19	/* "	 "	 " Low part */
#define MC9XGATE_TOC3	0x1A	/* Output Compare 3 register */
#define MC9XGATE_TOC3_H	0x1A	/* "	 "	 " High part */
#define MC9XGATE_TOC3_L	0x1B	/* "	 "	 " Low part */
#define MC9XGATE_TOC4	0x1C	/* Output Compare 4 register */
#define MC9XGATE_TOC4_H	0x1C	/* "	 "	 " High part */
#define MC9XGATE_TOC4_L	0x1D	/* "	 "	 " Low part */
#define MC9XGATE_TOC5	0x1E	/* Output Compare 5 register */
#define MC9XGATE_TOC5_H	0x1E	/* "	 "	 " High part */
#define MC9XGATE_TOC5_L	0x1F	/* "	 "	 " Low part */
#define MC9XGATE_TCTL1	0x20	/* Timer Control register 1 */
#define MC9XGATE_TCTL2	0x21	/* Timer Control register 2 */
#define MC9XGATE_TMSK1	0x22	/* Timer Interrupt Mask Register 1 */
#define MC9XGATE_TFLG1	0x23	/* Timer Interrupt Flag Register 1 */
#define MC9XGATE_TMSK2	0x24	/* Timer Interrupt Mask Register 2 */
#define MC9XGATE_TFLG2	0x25	/* Timer Interrupt Flag Register 2 */
#define MC9XGATE_PACTL	0x26	/* Pulse Accumulator Control Register */
#define MC9XGATE_PACNT	0x27	/* Pulse Accumulator Count Register */
#define MC9XGATE_SPCR	0x28	/* SPI Control register */
#define MC9XGATE_SPSR	0x29	/* SPI Status register */
#define MC9XGATE_SPDR	0x2A	/* SPI Data register */
#define MC9XGATE_BAUD	0x2B	/* SCI Baud register */
#define MC9XGATE_SCCR1	0x2C	/* SCI Control register 1 */
#define MC9XGATE_SCCR2	0x2D	/* SCI Control register 2 */
#define MC9XGATE_SCSR	0x2E	/* SCI Status register */
#define MC9XGATE_SCDR	0x2F	/* SCI Data (Read => RDR, Write => TDR) */
#define MC9XGATE_ADCTL	0x30	/* A/D Control register */
#define MC9XGATE_ADR1	0x31	/* A/D, Analog Result register 1 */
#define MC9XGATE_ADR2	0x32	/* A/D, Analog Result register 2 */
#define MC9XGATE_ADR3	0x33	/* A/D, Analog Result register 3 */
#define MC9XGATE_ADR4	0x34	/* A/D, Analog Result register 4 */
#define MC9XGATE__RES35	0x35
#define MC9XGATE__RES36	0x36
#define MC9XGATE__RES37	0x37
#define MC9XGATE__RES38	0x38
#define MC9XGATE_OPTION	0x39	/* System Configuration Options */
#define MC9XGATE_COPRST	0x3A	/* Arm/Reset COP Timer Circuitry */
#define MC9XGATE_PPROG	0x3B	/* EEPROM Programming Control Register */
#define MC9XGATE_HPRIO	0x3C	/* Highest priority I-Bit int and misc */
#define MC9XGATE_INIT	0x3D	/* Ram and I/O mapping register */
#define MC9XGATE_TEST1	0x3E	/* Factory test control register */
#define MC9XGATE_CONFIG	0x3F	/* COP, ROM and EEPROM enables */


/* Flags of the CONFIG register (in EEPROM).  */
#define MC9XGATE_NOSEC     0x08    /* Security mode disable */
#define MC9XGATE_NOCOP     0x04    /* COP system disable */
#define MC9XGATE_ROMON     0x02    /* Enable on-chip rom */
#define MC9XGATE_EEON      0x01    /* Enable on-chip eeprom */

/* Flags of the PPROG register.  */
#define MC9XGATE_BYTE	0x10	/* Byte mode */
#define MC9XGATE_ROW       0x08	/* Row mode */
#define MC9XGATE_ERASE	0x04	/* Erase mode select (1 = erase, 0 = read) */
#define MC9XGATE_EELAT     0x02	/* EEPROM Latch Control */
#define MC9XGATE_EEPGM     0x01	/* EEPROM Programming Voltage Enable */

/* Flags of the PIOC register.  */
#define MC9XGATE_STAF	0x80	/* Strobe A Interrupt Status Flag */
#define MC9XGATE_STAI	0x40	/* Strobe A Interrupt Enable Mask */
#define MC9XGATE_CWOM	0x20	/* Port C Wire OR mode */
#define MC9XGATE_HNDS	0x10	/* Handshake mode */
#define MC9XGATE_OIN	0x08	/* Output or Input handshaking */
#define MC9XGATE_PLS	0x04	/* Pulse/Interlocked Handshake Operation */
#define MC9XGATE_EGA	0x02	/* Active Edge for Strobe A */
#define MC9XGATE_INVB	0x01	/* Invert Strobe B */

/* Flags of the SCCR1 register.  */
#define MC9XGATE_R8	0x80	/* Receive Data bit 8 */
#define MC9XGATE_T8	0x40	/* Transmit data bit 8 */
#define MC9XGATE__SCCR1_5  0x20	/* Unused */
#define MC9XGATE_M		0x10	/* SCI Character length */
#define MC9XGATE_WAKE	0x08	/* Wake up method select (0=idle, 1=addr mark) */

/* Flags of the SCCR2 register.  */
#define MC9XGATE_TIE	0x80	/* Transmit Interrupt enable */
#define MC9XGATE_TCIE	0x40	/* Transmit Complete Interrupt Enable */
#define MC9XGATE_RIE	0x20	/* Receive Interrupt Enable */
#define MC9XGATE_ILIE	0x10	/* Idle Line Interrupt Enable */
#define MC9XGATE_TE	0x08	/* Transmit Enable */
#define MC9XGATE_RE	0x04	/* Receive Enable */
#define MC9XGATE_RWU	0x02	/* Receiver Wake Up */
#define MC9XGATE_SBK	0x01	/* Send Break */

/* Flags of the SCSR register.  */
#define MC9XGATE_TDRE	0x80	/* Transmit Data Register Empty */
#define MC9XGATE_TC	0x40	/* Transmit Complete */
#define MC9XGATE_RDRF	0x20	/* Receive Data Register Full */
#define MC9XGATE_IDLE	0x10	/* Idle Line Detect */
#define MC9XGATE_OR	0x08	/* Overrun Error */
#define MC9XGATE_NF	0x04	/* Noise Flag */
#define MC9XGATE_FE	0x02	/* Framing Error */
#define MC9XGATE__SCSR_0	0x01	/* Unused */

/* Flags of the BAUD register.  */
#define MC9XGATE_TCLR	0x80	/* Clear Baud Rate (TEST mode) */
#define MC9XGATE__BAUD_6	0x40	/* Not used */
#define MC9XGATE_SCP1	0x20	/* SCI Baud rate prescaler select */
#define MC9XGATE_SCP0	0x10
#define MC9XGATE_RCKB	0x08	/* Baud Rate Clock Check (TEST mode) */
#define MC9XGATE_SCR2	0x04	/* SCI Baud rate select */
#define MC9XGATE_SCR1	0x02
#define MC9XGATE_SCR0	0x01

#define MC9XGATE_BAUD_DIV_1	(0)
#define MC9XGATE_BAUD_DIV_3	(MC9XGATE_SCP0)
#define MC9XGATE_BAUD_DIV_4	(MC9XGATE_SCP1)
#define MC9XGATE_BAUD_DIV_13	(MC9XGATE_SCP1|MC9XGATE_SCP0)

/* Flags of the SPCR register.  */
#define MC9XGATE_SPIE	0x80	/* Serial Peripheral Interrupt Enable */
#define MC9XGATE_SPE	0x40	/* Serial Peripheral System Enable */
#define MC9XGATE_DWOM	0x20	/* Port D Wire-OR mode option */
#define MC9XGATE_MSTR	0x10	/* Master Mode Select */
#define MC9XGATE_CPOL	0x08	/* Clock Polarity */
#define MC9XGATE_CPHA	0x04	/* Clock Phase */
#define MC9XGATE_SPR1	0x02	/* SPI Clock Rate Select */
#define MC9XGATE_SPR0	0x01

/* Flags of the SPSR register.  */
#define MC9XGATE_SPIF	0x80	/* SPI Transfer Complete flag */
#define MC9XGATE_WCOL	0x40	/* Write Collision */
#define MC9XGATE_MODF	0x10	/* Mode Fault */

/* Flags of the ADCTL register.  */
#define MC9XGATE_CCF	0x80	/* Conversions Complete Flag */
#define MC9XGATE_SCAN	0x20	/* Continuous Scan Control */
#define MC9XGATE_MULT	0x10	/* Multiple Channel/Single Channel Control */
#define MC9XGATE_CD	0x08	/* Channel Select D */
#define MC9XGATE_CC	0x04	/*                C */
#define MC9XGATE_CB	0x02	/*                B */
#define MC9XGATE_CA	0x01	/*                A */

/* Flags of the CFORC register.  */
#define MC9XGATE_FOC1	0x80	/* Force Output Compare 1 */
#define MC9XGATE_FOC2	0x40	/*			2 */
#define MC9XGATE_FOC3	0x20	/*			3 */
#define MC9XGATE_FOC4	0x10	/*			4 */
#define MC9XGATE_FOC5	0x08	/*			5 */

/* Flags of the OC1M register.  */
#define MC9XGATE_OC1M7	0x80	/* Output Compare 7 */
#define MC9XGATE_OC1M6	0x40	/*                6 */
#define MC9XGATE_OC1M5	0x20	/*                5 */
#define MC9XGATE_OC1M4	0x10	/*                4 */
#define MC9XGATE_OC1M3	0x08	/*                3 */

/* Flags of the OC1D register.  */
#define MC9XGATE_OC1D7	0x80
#define MC9XGATE_OC1D6	0x40
#define MC9XGATE_OC1D5	0x20
#define MC9XGATE_OC1D4	0x10
#define MC9XGATE_OC1D3	0x08

/* Flags of the TCTL1 register.  */
#define MC9XGATE_OM2	0x80	/* Output Mode 2 */
#define MC9XGATE_OL2	0x40	/* Output Level 2 */
#define MC9XGATE_OM3	0x20
#define MC9XGATE_OL3	0x10
#define MC9XGATE_OM4	0x08
#define MC9XGATE_OL4	0x04
#define MC9XGATE_OM5	0x02
#define MC9XGATE_OL5	0x01

/* Flags of the TCTL2 register.  */
#define MC9XGATE_EDG1B	0x20	/* Input Edge Capture Control 1 */
#define MC9XGATE_EDG1A	0x10
#define MC9XGATE_EDG2B	0x08	/* Input 2 */
#define MC9XGATE_EDG2A	0x04
#define MC9XGATE_EDG3B	0x02	/* Input 3 */
#define MC9XGATE_EDG3A	0x01

/* Flags of the TMSK1 register.  */
#define MC9XGATE_OC1I	0x80	/* Output Compare 1 Interrupt */
#define MC9XGATE_OC2I	0x40	/*		  2	      */
#define MC9XGATE_OC3I	0x20	/*		  3	      */
#define MC9XGATE_OC4I	0x10	/*		  4	      */
#define MC9XGATE_OC5I	0x08	/*		  5	      */
#define MC9XGATE_IC1I	0x04	/* Input Capture  1 Interrupt */
#define MC9XGATE_IC2I	0x02	/*		  2	      */
#define MC9XGATE_IC3I	0x01	/*		  3	      */

/* Flags of the TFLG1 register.  */
#define MC9XGATE_OC1F	0x80	/* Output Compare 1 Flag */
#define MC9XGATE_OC2F	0x40	/*		  2	 */
#define MC9XGATE_OC3F	0x20	/*		  3	 */
#define MC9XGATE_OC4F	0x10	/*		  4	 */
#define MC9XGATE_OC5F	0x08	/*		  5	 */
#define MC9XGATE_IC1F	0x04	/* Input Capture  1 Flag */
#define MC9XGATE_IC2F	0x02	/*		  2	 */
#define MC9XGATE_IC3F	0x01	/*		  3	 */

/* Flags of Timer Interrupt Mask Register 2 (TMSK2).  */
#define MC9XGATE_TOI       0x80    /* Timer Overflow Interrupt Enable */
#define MC9XGATE_RTII      0x40    /* RTI Interrupt Enable */
#define MC9XGATE_PAOVI     0x20    /* Pulse Accumulator Overflow Interrupt En. */
#define MC9XGATE_PAII      0x10    /* Pulse Accumulator Interrupt Enable */
#define MC9XGATE_PR1       0x02    /* Timer prescaler */
#define MC9XGATE_PR0       0x01    /* Timer prescaler */
#define MC9XGATE_TPR_1     0x00    /* " " prescale div 1 */
#define MC9XGATE_TPR_4     0x01    /* " " prescale div 4 */
#define MC9XGATE_TPR_8     0x02    /* " " prescale div 8 */
#define MC9XGATE_TPR_16    0x03    /* " " prescale div 16 */

/* Flags of Timer Interrupt Flag Register 2 (MC9XGATE_TFLG2).  */
#define MC9XGATE_TOF       0x80    /* Timer overflow bit */
#define MC9XGATE_RTIF      0x40    /* Read time interrupt flag */
#define MC9XGATE_PAOVF     0x20    /* Pulse accumulator overflow Interrupt flag */
#define MC9XGATE_PAIF      0x10    /* Pulse accumulator Input Edge " " " */

/* Flags of Pulse Accumulator Control Register (PACTL).  */
#define MC9XGATE_DDRA7     0x80    /* Data direction for port A bit 7 */
#define MC9XGATE_PAEN      0x40    /* Pulse accumulator system enable */
#define MC9XGATE_PAMOD     0x20    /* Pulse accumulator mode */
#define MC9XGATE_PEDGE     0x10    /* Pulse accumulator edge control */
#define MC9XGATE_RTR1      0x02    /* RTI Interrupt rates select */
#define MC9XGATE_RTR0      0x01    /* " " " " */

/* Flags of the Options register.  */
#define MC9XGATE_ADPU      0x80    /* A/D Powerup */
#define MC9XGATE_CSEL      0x40    /* A/D/EE Charge pump clock source select */
#define MC9XGATE_IRQE      0x20    /* IRQ Edge/Level sensitive */
#define MC9XGATE_DLY       0x10    /* Stop exit turn on delay */
#define MC9XGATE_CME       0x08    /* Clock Monitor enable */
#define MC9XGATE_CR1       0x02    /* COP timer rate select */
#define MC9XGATE_CR0       0x01    /* COP timer rate select */

/* Flags of the HPRIO register.  */
#define MC9XGATE_RBOOT	0x80	/* Read Bootstrap ROM */
#define MC9XGATE_SMOD	0x40	/* Special Mode */
#define MC9XGATE_MDA	0x20	/* Mode Select A */
#define MC9XGATE_IRV	0x10	/* Internal Read Visibility */
#define MC9XGATE_PSEL3	0x08	/* Priority Select */
#define MC9XGATE_PSEL2	0x04
#define MC9XGATE_PSEL1	0x02
#define MC9XGATE_PSEL0	0x01

/* Some insns used by gas to turn relative branches into absolute ones.  */
#define MC9XGATE_BRA	0x20
#define MC9XGATE_JMP	0x7e
#define MC9XGATE_BSR	0x8d
#define MC9XGATE_JSR	0xbd
#define MC9XGATE_JMP2       0x06
#define MC9XGATE_BSR2       0x07
#define MC9XGATE_JSR2       0x16

/* Instruction code pages. Code page 1 is the default.  */
/*#define	MC9XGATE_OPCODE_PAGE1	0x00*/
#define	MC9XGATE_OPCODE_PAGE2	0x18
#define	MC9XGATE_OPCODE_PAGE3	0x1A
#define	MC9XGATE_OPCODE_PAGE4	0xCD


/* 68HC11 operands formats as stored in the MC9XGATE_opcode table.  These
   flags do not correspond to anything in the 68HC11 or 68HC12.
   They are only used by GAS to recognize operands.  */
#define MC9XGATE_OP_NONE         ""        /* No operand AKA Inherint*/
#define MC9XGATE_OP_INH			 ""
#define MC9XGATE_OP_TRI			 "r,r,r"
#define MC9XGATE_OP_DYA			 "r,r"
#define MC9XGATE_OP_IMM8		 "r,i8"
#define MC9XGATE_OP_IMM4         "r,i4"
#define MC9XGATE_OP_IMM3		 "i3"
#define MC9XGATE_OP_MON			 "r"
#define MC9XGATE_OP_MON_R_C		 "r,c"
#define MC9XGATE_OP_MON_C_R		 "c,r"
#define MC9XGATE_OP_MON_R_P		 "r,p"
#define MC9XGATE_OP_IDR			 "r,r,r"
#define MC9XGATE_OP_IDO5		 "r,r,i5"
#define MC9XGATE_OP_REL9		 "b9"
#define MC9XGATE_OP_REL10		 "b10"


#define MC9XGATE_OP_DIRECT       0x0001   /* Page 0 addressing:   *<val-8bits>  */
#define MC9XGATE_OP_IMM16_REMOVE        0x0009   /* THERE IS NO IMM16 16 bits immediat:    #<val-16bits> */
#define MC9XGATE_OP_IND16        0x0010   /* Indirect abs:        <val-16>      */
#define MC9XGATE_OP_IND16_P2     0x0020   /* Second parameter indirect abs.     */
//#define MC9XGATE_OP_MON          0x0040   /* Monadic Addressing (MON) was Register operand 1                 */
//#define MC9XGATE_OP_DYA          0x0080   /* Dyadic Addressing (DYA) was Register operand 2                 */
//#define MC9XGATE_OP_TRI			 0x0100	  /* Triadic Addressing (TRI)									*/

//#define MC9XGATE_OP_IDO5         0x0200   /* Indirect IX:         <val-8>,x     */
//#define MC9XGATE_OP_IDR			 0x0400	  /* Index Register plus Register Offset (IDR) */
//#define MC9XGATE_OP_IY           0x0100   /* Indirect IY:         <val-8>,y     */
#define MC9XGATE_OP_IDX          0x0200   /* Indirect: N,r N,[+-]r[+-] N:5-bits */  //was SAVE
#define MC9XGATE_OP_IDX_1        0x0400   /* N,r N:9-bits  */
#define MC9XGATE_OP_IDX_2        0x0800   /* N,r N:16-bits */
#define MC9XGATE_OP_D_IDX        0x1000   /* Indirect indexed: [D,r] */
#define MC9XGATE_OP_D_IDX_2      0x2000   /* [N,r] N:16-bits */
#define MC9XGATE_OP_PAGE         0x4000   /* Page number */
#define MC9XGATE_OP_MASK         0x07FFF
#define MC9XGATE_OP_BRANCH       0x00008000 /* Branch, jsr, call */
#define MC9XGATE_OP_BITMASK      0x00010000 /* Bitmask:             #<val-8>    */
#define MC9XGATE_OP_JUMP_REL_9     0x00020000 /* Relative Addressing 9 Bit Wide (REL9) was Pc-Relative:         <val-8>     */
#define MC9XGATE_OP_JUMP_REL_10   0x00040000 /* Relative Addressing 10 Bit Wide (REL10) was Pc-relative:         <val-10>    */
#define MC9XGATE_OP_PAGE1        0x0000
#define MC9XGATE_OP_PAGE2        0x00080000 /* Need a page2 opcode before       */
#define MC9XGATE_OP_PAGE3        0x00100000 /* Need a page3 opcode before       */
#define MC9XGATE_OP_PAGE4        0x00200000 /* Need a page4 opcode before       */
#define MC9XGATE_MAX_OPERANDS    3     /* Max operands: brset <dst> <mask> <b> */

#define MC9XGATE_ACC_OFFSET      0x00400000 /* A,r B,r D,r			    */
#define MC9XGATE_ACC_IND         0x00800000 /* [D,r]			    */
#define MC9XGATE_PRE_INC         0x01000000 /* n,+r   n = -8..8		    */
#define MC9XGATE_PRE_DEC         0x02000000 /* n,-r			    */
#define MC9XGATE_POST_INC        0x04000000 /* n,r+			    */
#define MC9XGATE_POST_DEC        0x08000000 /* n,r-			    */
#define MC9XGATE_INDEXED_IND     0x10000000 /* [n,r]  n = 16-bits		    */
#define MC9XGATE_INDEXED         0x20000000 /* n,r    n = 5, 9 or 16-bits	    */
#define MC9XGATE_OP_IDX_P2       0x40000000

/* Markers to identify some instructions.  */
#define MC9XGATE_OP_EXG_MARKER   0x01000000 /* exg r1,r2 */
#define MC9XGATE_OP_TFR_MARKER   0x02000000 /* tfr r1,r2 */
#define MC9XGATE_OP_SEX_MARKER   0x04000000 /* sex r1,r2 */

#define MC9XGATE_OP_EQ_MARKER    0x80000000 /* dbeq/ibeq/tbeq */
#define MC9XGATE_OP_DBCC_MARKER  0x04000000 /* dbeq/dbne */
#define MC9XGATE_OP_IBCC_MARKER  0x02000000 /* ibeq/ibne */
#define MC9XGATE_OP_TBCC_MARKER  0x01000000

#define MC9XGATE_OP_TRAP_ID      0x80000000 /* trap #N */

#define MC9XGATE_OP_HIGH_ADDR    0x01000000 /* Used internally by gas.  */
#define MC9XGATE_OP_LOW_ADDR     0x02000000

#define MC9XGATE_BANK_VIRT 0x010000
#define MC9XGATE_BANK_MASK 0x00003fff
#define MC9XGATE_BANK_BASE 0x00008000
#define MC9XGATE_BANK_SHIFT 14
#define MC9XGATE_BANK_PAGE_MASK 0x0ff


/* CPU identification.  */
#define cpu6811 0x01
#define cpu6812 0x02
#define cpumc9xgate 0x06  /* was 0x4 somewhere it is bit-wised to 0x06 otherwise objdump will see it as UNKNOWN*/

/* The opcode table is an array of struct mc9xgate_opcode.  */
struct mc9xgate_opcode {
	char*    name;     /* Op-code name  was const*/
	char*    constraints; /* was const*/
	char*    format;   /* format string was const*/
	unsigned int sh_format; /* shorthand format */
	unsigned int   size;   /* size in bytes , was in words */
	unsigned int   bin_opcode;  /* binary opcode with operands masked off */
	unsigned char  cycles_min; /* minimum cpu cycles needed */
	unsigned char  cycles_max; /* maximum cpu cycles needed */
	unsigned char  set_flags_mask; /* CCR flags set */
	unsigned char  clr_flags_mask; /* CCR flags cleared */
	unsigned char  chg_flags_mask; /* CCR flags changed */
	unsigned char  arch; /* cpu type, may always be mc9s12 only */
};

//struct mc9xgate_opcode
//{
//  char *        name;
//  char *        constraints;
//  int           insn_size;		/* In words.  */
//  int           isa;
//  unsigned int  bin_opcode;
//};

/* Alias definition for 68HC12.  */
struct mc9xgate_opcode_alias
{
  const char*   name;
  const char*   translation;
  unsigned char size;
  unsigned char code1;
  unsigned char code2;
};

/* The opcode table.  The table contains all the opcodes (all pages).
   You can't rely on the order.  */
extern struct mc9xgate_opcode mc9xgate_opcodes[];
extern const int mc9xgate_num_opcodes;
//extern struct mc9xgate_opcode_desc;

/* Alias table for 68HC12.  It translates some 68HC11 insn which are not
   implemented in 68HC12 but have equivalent translations.  */
extern const struct mc9xgate_opcode_alias mc9xgate_alias[];
extern const int mc9xgate_num_alias;

#endif /* _OPCODE_MC9XGATE_H */
