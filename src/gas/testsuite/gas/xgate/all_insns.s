# Example of XGATE instructions
	.sect .text
_start:
L0: 	adc r1, r2, r3
L1:	bcc END_CODE
L2:	add r4, r5, r6
L3:	add r7 , #225
L4:	addh r1, 255
L5:	addl r2, #255
L6:	add  r4, 8004
L7:	and r3, r4, r5
L8:	and r1, #0x8004
L9: 	add r5, END_CODE
L10:    and r7, END_CODE
L11:	and r4, #65281
L12:    andl r3, #01
L13:    andh r6, #255
L14:    asr r0, #3
L15:    asr r1, r2
L16:    bcc END_CODE
L17:    bcs END_CODE
L18:    beq END_CODE
L19:    bfext r3, r4, r5
L20:    bffo r6, r7
L21:    bfins r0, r1, r2
L22:    bfinsi r3, r4, r5
L23:    bfinsx r6, r7, r0
L24:    bge END_CODE
L25:    bgt END_CODE
L26:    bhi END_CODE
L27:    bhs END_CODE
L28:    bith r1, #32
L29:    bitl r2, #0
L30:    ble  END_CODE
L31:    blo END_CODE
L32:    bls END_CODE
L33:	blt END_CODE
L24:	bmi END_CODE
L25:    bne END_CODE
L26:    bpl END_CODE
L27:    bra END_CODE
L28:    brk
L29:    bvc END_CODE
L30:    bvs END_CODE
L31:    cmp r1, r2
L32:    cmpl r3, #255
L33:    com r4, r5
L34:    cpc r6, r7
L35:    cmp r1, #65535
L36:    cpch r2, #255
L37:    csem #4
L38:    csem r5
L39:    csl r6, #11
L40:    csl r7, r0
L41:    csr r1, #2
L42:    csr r2, r3
L43:    jal r4
L44:    ldb r5, (r6, #20)
L45:    ldb r7, (r0, r1+)
L46:    ldb r7, (r0, -r1)
L47:    ldb r0, (r0, r0)
L48:    ldh r1, #255
L49:    ldl r2, #255
L50:	ldd r3, END_CODE
L51:    ldw r4, (r5, #20)
L52:    ldw r5, (r6, r7+)
L53:    ldw r5, (r6, -r7)
L54:    ldw r1, (r2, r4)
L55:    lsl r1, #4
L56:    lsl r2, r3
L57:    lsr r4, #5
L58:    lsr r5, r6
L59:    mov r6, r7
L60:    neg r1, r2
L61:    nop
L62:    or r1, r2, r3
L63:    orh r4, #255
L64:    orl r5, #255
L65:    par r6
L66:    rol r7, #6
L67:    rol r1, r2
L68:    ror r3, #5
L69:    ror r4, r5
L70:    rts
L71:    sbc r1, r2, r3
L72:    ssem #4
L73:    ssem r1
L74:    sex r2
L75:    sif
L76:    sif r4
L77:    stb r5, (r6, #5)
L78:    stb r0, (r0, r0+)
L79:    stb r0, (r0, -r0)
L80:    stb r2, (r0, r0)
L81:    stw r1, (r2, #16)
L82:    stw r1, (r2, r3+)
L83:    stw r1, (r2, -r3)
L84:    stw r2, (r3 ,r4)
L85:    sub r3, r4, r6
L86:    sub r4, #65535
L87:    subh r5, #255
L88:    subl r6, #255
L89:    tfr r7, pc
L90:    tfr r7,ccr
L91:    tfr ccr, r7
L92:    tst r1
L93:    xnor r1, r2, r3
L94:    xnorh r4, #255
L95:    xnorl r5, #255  
END_CODE:

