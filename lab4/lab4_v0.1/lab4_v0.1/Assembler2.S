
/*
 * Assembler2.S
 *
 * Created: 4/11/2020 9:58:03 μμ
 *  Author: teo
 */ 
; Replace with your application code
;
; lab2_proj.asm
;
; Created: 22/10/2020 3:25:30 μμ
; Author : teo
;


; Replace with your application code
 #include <avr/io.h>
 #define curNumberIndex 0x73;
.global ISR_timerOVF
	ISR_timerOVF: ;Interrrupt Handler Timer OVF

		ldi XH, hi8(RAMSTART);New Code For C compatibility so that anyone can mess up with X in main
		ldi XL ,curNumberIndex
		ld r17,X+
		ld r19,X
		ldi r16,0x64 ; start from 100 = 256 - (156)
		out _SFR_IO_ADDR(TCNT0), r16 ;New Code For C compatibility
		;out TCNT0,r16	;setting timer from the start so that we where as close as possible to the theoretical estimation
		
		;loading block starτ
		ldi	XL, lo8(RAMSTART)	;New Code For C compatibility
		add XL,r17				;New Code For C compatibility
		inc XL					;New Code For C compatibility
		inc r17
		cpi r17,0x08 ;gives Zflag = 1 when r17-0x07=0 ;;first time program enters here is imporetant to branch so that everything works as designed
		breq if1
		LSL r19 ;R19 Is the output for C port
		ld r18,X+ ;r18 has the value of the Number we gonna display is bsd
		rjmp endif1
		if1: 
			ldi r19,0x01;R19 Is the output for C port
			ldi r17,0x00
			subi XL,8 ; X pointer register only low part ;
			ld r18,X+ ;r18 has the value of the Number we gonna display is bsd
		endif1:
	;loading block end
	;decoder Start----------
		ldi r16,8
		ldi	XL, lo8(RAMSTART)	
		add XL,r16				; initialize X pointer to SRAM_START+8
		add XL,r18 ;r18 is the number we want to show
		ld r20,X
		;START OF OUT
		;out C port r16 close all Anodes -ADD STUFFS-
		ldi r16, 0b00000000 ;Anodes for SSDs
		out _SFR_IO_ADDR(PORTC), r16
		;out B port r20 6 down to 0 -> a down to g -ADD STUFFS-
		out _SFR_IO_ADDR(PORTA), r20
		;out C port r19 7 down to 0 -> MSDecimal(7) to LSDecimal(0) anodes -ADD STUFFS-
		out _SFR_IO_ADDR(PORTC), r19
		;-END OF OUT
		ldi XL ,curNumberIndex
		st X+,r17
		st X,r19
	;decoder End
		reti



.global ISR_USART_Receive
;------------------------
;Receive in r18 the value Of UDR 
ISR_USART_Receive:
;STATE MACHINE
 #define INITstate 0x00;
 #define TWAITstate 0x01;
 #define BYTEOUTstate 0x02;
 #define CRWAITstate 0x03;
 #define LFWAITstate 0x04;

 #define ControlUsart 0x75;
	ldi XH,0x00			//New Code For C compatibility
	ldi XL,ControlUsart
	in r18,  _SFR_IO_ADDR(UDR) ;->REAL CODE
	in r18, _SFR_IO_ADDR(UDR) ;->REAL CODE
	mov r18,r15	 ;->`TESTING CODE
	ld r20,X ;r20 Is the register with the state
	;STATE MANAGMENT
	;Jump To The Correct State Handler--
	cpi r20,INITstate ;INIT
	breq INIT
	cpi r20,TWAITstate ;T	Just waiting for T in this state
	breq TWAIT
	cpi r20,BYTEOUTstate ;ByteOut
	breq BYTEOUT
	cpi r20,CRWAITstate ;<CR>
	breq CRWAIT
	cpi r20,LFWAITstate ;<CR> when you are at <CR> you wait for <LF> print Ok and then make the state==INIT
	breq LFWAIT
	;end-------------------------------
	;Handlers=function(r18(char se ascii),r20(currentState))
	;Receive in r18 the value Of UDR
	INIT:
	cpi r18,0x41;r18==A
	breq controlif1
	cpi r18,0x4E;r18==N
	breq controlif2
	cpi r18,0x43;r18==C
	breq controlif3

	controlif1:;r18==A
		ldi r20,TWAITstate ;nextsState=TWAITstate
		rjmp return1

	controlif2:;r18==N
		;rcall clearAllBytes ;uses r18 but we dont use it from now on
		ldi	XL,lo8(RAMSTART)		; initialize X pointer
		ldi	XH,hi8(RAMSTART)		
		ldi r18,0x08
		add r18,XL ;R18=XL+8
		;whileLoop With init start
		whileLoop3:
		cp XL,r18 ;gives Zflag = 1 when r18-0x08=0 ;
		breq if3				;escape while loop
			ldi r16,0x0A		;r16=0x0A=CLOSED SEGMENTS /NO LIGHT
			st x+,r16			;store the new value also increment x for next loop x=x+1
			rjmp whileLoop3
		if3: 
		;whileLoop End
		ldi r20,BYTEOUTstate ;nextsState=BYTEOUTstate
		rjmp return1

	controlif3:;r18==C
		;rcall clearAllBytes ;uses r18 but we dont use it from now on
		ldi	XL,lo8(RAMSTART)		; initialize X pointer
		ldi	XH,hi8(RAMSTART)		
		ldi r18,0x08
		add r18,XL ;R18=XL+8
		;whileLoop With init start
		whileLoop4:
		cp XL,r18 ;gives Zflag = 1 when r18-0x08=0 ;
		breq if4				;escape while loop
			ldi r16,0x0A		;r16=0x0A=CLOSED SEGMENTS /NO LIGHT
			st x+,r16			;store the new value also increment x for next loop x=x+1
			rjmp whileLoop4
		if4: 
		;whileLoop End
		ldi r20,CRWAITstate ;CRWAITstate
		rjmp return1
	;Handler(r18(char se ascii),r20(currentState))
	rjmp return1

	TWAIT:
		ldi r20,CRWAITstate ;<CR>
	;Handler(r18(char se ascii),r20(currentState))
	rjmp return1
	
	BYTEOUT:
	cpi r18,0x0D;If inserted character = <CR>
	breq controlif4
		andi r18,0b00001111 ;Setting Mask to make the number from ASCII to BSD
							;r18 is the BSD number
		;rcall shiftAllBytesRight	;This Function Changes r20 and r18 /r18 doesn't matter to us
		shiftAllBytesRight:
		ldi	XL,lo8(RAMSTART)		; initialize X pointer
		ldi	XH,hi8(RAMSTART)		
		ldi r20,0x08
		add r20,XL ;R20=XL+8
		;whileLoop With init start
		;ldi r18,0x01
		;ld r18,x ;Init -> The first time in while Loop r16 get the right number Inserted Number
		whileLoop2:
		cp XL,r20 ;gives Zflag = 1 when r20-0x08=0 ;
		breq if2 ;escape while loop
			mov r16,r18		;r16=r18=last Byte not Yet Shifted
			inc xl			;We save next Byte 
			ld r18,-x		;we save to r18 for next loop and then go back in ram to
			st x+,r16		;store the new value also increrment x for next loop x=x+1
			rjmp whileLoop2
		if2: 
		;whileLoop End
		ldi r20,BYTEOUTstate		;But We Want to keep r20(STATE) until we save it to RAM
	rjmp return1
	controlif4:;IF CR is imported
		ldi r20,LFWAITstate
	rjmp return1

	CRWAIT:
		ldi r20,LFWAITstate ;<CR>
	rjmp return1
	
	LFWAIT:
		ldi r21,0xFF
		ldi r20,INITstate ;<CR>
		;MUST PRINT OK
		//rjmp USART_Transmit_OK
			; Wait for data to be transmitted
			rcall USART_Transmit_OK
	rjmp return1
	
	return1:
	ldi XL,ControlUsart
	st X,r20 ;Store the new State to Memory
	ldi	XL, lo8(RAMSTART) ;getting the X pointer where it was before decoder block
	add XL,r17
	reti