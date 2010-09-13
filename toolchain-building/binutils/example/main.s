; ----- main.s -----
; (C) 2002-2003 Aurelien Jarno <aurel32@debian.org>

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
PORTE   	=    	0x100A	; Port E register
INIT		=	0x103D 	; RAM, nd I/O Mapping Register


; ****************************************************************************
;
;	VARIABLES
;
; ****************************************************************************
	.sect .data


; ****************************************************************************
;
;	VECTORS
;
; ****************************************************************************

	.sect .vectors
	.globl vectors

vectors:
	.word def		; ffc0
	.word def		; ffc2
	.word def		; ffc4
	.word def		; ffc6
	.word def		; ffc8
	.word def		; ffca
	.word def		; ffcc
	.word def		; ffce
	.word def		; ffd0
	.word def		; ffd2
	.word def		; ffd4

	;; SCI
	.word def		; ffd6

	;; SPI
	.word def		; ffd8
	.word def		; ffda (PAII)
	.word def		; ffdc (PAOVI)
	.word def		; ffde (TOI)

	;; Timer Output Compare
	.word def		; ffe0
	.word def		; ffe2
	.word def		; ffe4
	.word def		; ffe6
	.word def		; ffe8

	;; Timer Input compare
	.word def		; ffea
	.word def		; ffec
	.word def		; ffee

	;;  Misc
	.word def		; fff0 (RTII)
	.word def		; fff2 (IRQ)
	.word def		; fff4 (XIRQ)
	.word def		; fff6 (SWI)
	.word def		; fff8 (ILL)
	.word def		; fffa (COP Failure)
	.word def		; fffc (COP Clock monitor)
	.word _start		; fffe (reset)



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
        rti


; ----------------------------------------------------------------------------
;	Main subroutine
; ----------------------------------------------------------------------------

_start:
	ldaa	#0x01		; Map RAM to $0000
	staa	INIT		; and registers to $1000
	lds	_stack		; Set stack pointer


; Simply copy the state of PE0 to PF0
loop:	
	ldaa	PORTE
	anda	#0x01
	tsta
	beq	off
	bra	on

off:
	ldaa	#0x00
	staa	PORTF
	bra	loop

on:
	ldaa	#0x01
	staa	PORTF
	bra 	loop

	end
