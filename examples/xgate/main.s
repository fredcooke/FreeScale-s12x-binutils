; ----- main.s -----
; (C) 2009-2010 Sean Keys <info@powerefi.com>

; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.

; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.

; You should have received a copy of the GNU General Public License
; along with this program; if not, write to the Free Software
; Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


; ****************************************************************************
;
;	REGISTERS
;
; ****************************************************************************

PORTF   	=     	0x1005	; Port F register

.equ	SOMEVALUE, 0x01
.equ	AVALUE, 0x05

; ****************************************************************************
;
;	VARIABLES
;
; ****************************************************************************
	.sect .data
    /* Put everything below in .data section */

/***
 * VARIABLES
 * =========
 *
 * Declare three variables and initialize their contents.  Also make sure they
 * are aligned properly (half-words should start at addresses that are
 * multiples of 2, words should start at multiples of 4).
 ***/

;TODO verify
mychar:
;.byte  0x01                /* An 8-bit variable named mychar */

;.align 1                   /* Align to a 2-byte (2^1) boundary */
myshort:
.hword 0x0123               /* A 16-bit variable named myshort */

;TODO test
;.align 2                    /* Align to a 4-byte (2^2) boundary */
;myint:
;.word  0x01234567           /* A 32-bit variable named myint */

/***
 * STRINGS
 * =======
 *
 * Declare a label called "hellostring" which points to the zero-terminated
 * string "Hello World"
 ***/

hellostring:
.string "FreeEMS > MS"

/***
 * ARRAYS
 * =======
 *
 * Declare "array_initialized" with 10 32-bit words, also declare an 
 * uninitialized array called "array_uninitialized" with space for 10
 * 32-bit words (40 bytes).
 ***/

.align 2
array_initialized:
.word 5,3,6,7,2,8,7,1,9,5
array_uninitialized:
.skip 40

; ****************************************************************************
;
;	VECTORS
;
; ****************************************************************************

	.sect .vectors
	.globl vectors

vectors:
	.word def		; ffc0

; ****************************************************************************
;
;	CODE
;
; ****************************************************************************

        .sect .text

; ----------------------------------------------------------------------------
;	Interrupts
; ----------------------------------------------------------------------------

;; Default interrupt handler.
def:
;        rts


; ----------------------------------------------------------------------------
;	Main subroutine
; ----------------------------------------------------------------------------

_start:
	nop
	mov R1, R2
	or  R1, R0, R2
	com R4
	com R1, R2

loop:	
	ADC R6,R2,R2
	COM R1,R4
	asr R2,#6
	asr R3,#1
	adc R0,R0,R0
	
on:	;bang an LED'd port for everification
	nop
	COM R1,R4
	bcc 	loop

end
