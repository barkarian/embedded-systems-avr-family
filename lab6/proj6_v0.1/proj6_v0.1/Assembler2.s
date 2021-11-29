
/*
 * Assembler2.s
 *
 * Created: 20/11/2020 3:02:13 πμ
 *  Author: teo
 */ 
 .global reset
 reset:
	.org 0x000
	resetV:
	rjmp resetPath

	.org 0x100
	resetPath:
	 reti