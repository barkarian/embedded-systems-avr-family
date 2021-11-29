; Replace with your application code
;
; lab2_proj.asm
;
; Created: 22/10/2020 3:25:30 μμ
; Author : teo
;


; Replace with your application code

.org $0000 ;reset
rjmp reset

;InterruptVectors
.org OVF0addr ;
rjmp timerOVF
.org URXCaddr			;ENABLE WHEN UDR READY FOR READ
rjmp USART_Receive
.org UDREaddr			;ENABLE WHEN UDR READY FOR WRITE
rjmp USART_Transmit_OK


.org 0x100
reset:
;Burned Register For Display Purposes INITIALIZATIONS
	ldi r17,0x07 ;r17 is the register that shows which segment is open and when to change X register ;
				 ;is important to initializing it at 0x07 so it points the correct memory address that we initialize
				 ;the first time it goes into loading block
	ldi r19,0x80 ;rotating Anode used inside load block with rol command
;Burned Register END
.equ ControlUsart =0x0086 ;Memory allocate for USART state in RAM
;ControlUsart(0)->State Index
;ControlUsart(>0)->States=N,A,AT,<CR>,<LF>,AT<CR>,OK
;we Store in Ram
	ldi	XL,LOW(SRAM_START)
	ldi	XH,HIGH(SRAM_START)		
	;initialization start /numbers that we want to display in the memory
	;initialization of numbers to display
	ldi r16,0x05
	st x+,r16
	ldi r16,0x06
	st x+,r16
	ldi r16,0x07
	st x+,r16
	ldi r16,0x08
	st x+,r16
	ldi r16,0x09
	st x+,r16
	ldi r16,0x01
	st x+,r16
	ldi r16,0x02
	st x+,r16
	ldi r16,0x0A
	st x+,r16
	;initialization of segment to open
	ldi r16,0b00000001 ;0
	st x+,r16
	ldi r16,0b01001111 ;1
	st x+,r16
	ldi r16,0b00010010 ;2
	st x+,r16
	ldi r16,0b00000110 ;3
	st x+,r16
	ldi r16,0b01001100 ;4
	st x+,r16
	ldi r16,0b00100100 ;5
	st x+,r16
	ldi r16,0b00100000 ;6
	st x+,r16
	ldi r16,0b00001111 ;7
	st x+,r16
	ldi r16,0b00000000 ;8
	st x+,r16
	ldi r16,0b00000100 ;9
	st x+,r16
	ldi r16,0b11111111 ;A
	st x+,r16
;Stacks and Special Register Inits
	ldi r16,8
	ldi	XL,LOW(SRAM_START)		; initialize X pointer
	add XL,r16
	;initialization over
	ldi r16, HIGH(RAMEND) ; Upper byte
	out SPH,r16 ; to stack pointer
	ldi r16, LOW(RAMEND)  ; Lower byte
	out SPL,r16 ; to stack pointer
;Init Components That Uses Interrupts
;------------------------------------
;Counter Init
	ldi r16,0x64 ; start from 100 = 256 - (156)
	;ldi r16,0xB2	;start from 178 = 256 - (78)  Wrong
	out TCNT0,r16	;setting timer
	ldi r16,(1<<TOIE0)	;mask for register 1
	out TIMSK,r16	;setting timer
	;ldi r18,0x01
	ldi r16,0x04	;cs2cs1cs0 =binary 011= 0x03
	out TCCR0,r16	;setting prescaler clk/64 AND starting clock for timer
;Counter Init END
;Init USART
	rcall USART_Init
;Init USART END
;out DDRx ;DATA DIRECTION
	ldi r16,0xff				
	out DDRC, r16
	ldi r16,0b01111111	
	out DDRB, r16
;DATA DIRECTION END		
;Interrupts Enable
	sei
										
waitloop:
	rjmp waitloop

;Interrupts Handlers
;Interrupts use in Register r16,r18,r20
;Try Not to use those Register In non Interrupted Routines as their values may lost
;----------------------------------------------------------------------------------
;TIMER
;--------------------------------------
timerOVF: ;Interrrupt Handler Timer OVF
	ldi r16,0x64 ; start from 100 = 256 - (156)
	out TCNT0,r16	;setting timer from the start so that we where as close as possible to the theoretical estimation

;Loading block gives us in r18 the Number we gonna display is bsd AND at R17 the memory location of The Decimal Degit we display AND in r19 the Port C outputs
;dont touch r17,r19,through all the program
;dont touch r18 until you succesfully decode it and get it out
;loading block start-----
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
	ldi	XL, LOW(SRAM_START)	
	add XL,r16				; initialize X pointer to SRAM_START+8
	add XL,r18 ;r18 is the number we want to show
	ld r20,X
	;START OF OUT
	;out C port r16 close all Anodes -ADD STUFFS-
	ldi r16, 0b00000000 ;Anodes for SSDs
	out PORTC, r16
	;out B port r20 6 down to 0 -> a down to g -ADD STUFFS-
	out PORTB, r20
	;out C port r19 7 down to 0 -> MSDecimal(7) to LSDecimal(0) anodes -ADD STUFFS-
	out PORTC, r19
	;-END OF OUT
	ldi	XL, LOW(SRAM_START) ;getting the X pointer where it was before decoder block
	add XL,r17
	inc XL					;inc because we used "ld r18,X+" so X was left one count after sram_start+r17 
;decoder End
	reti
;USART RECEIVE
;------------------------
;Receive in r18 the value Of UDR
USART_Receive:
;STATE MACHINE
.equ	INITstate		= 0x00
.equ	TWAITstate		= 0x01
.equ	BYTEOUTstate	= 0x02
.equ	CRWAITstate		= 0x03
.equ	LFWAITstate		= 0x04
;FinishOperation;0x01->CR;0x02->LFBeforeOk
	ldi XL,ControlUsart
	;REMEMBER that r17,r19 In Use BURNED
	in r18, UDR ;->REAL CODE
	in r18, UDR ;->REAL CODE
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
		rcall clearAllBytes ;uses r18 but we dont use it from now on
		ldi r20,BYTEOUTstate ;nextsState=BYTEOUTstate
		rjmp return1
	controlif3:;r18==C
		rcall clearAllBytes ;uses r18 but we dont use it from now on
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
		rcall shiftAllBytesRight	;This Function Changes r20 and r18 /r18 doesn't matter to us
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
		rcall USART_Transmit_OK
	rjmp return1
	
	return1:
	ldi XL,ControlUsart
	st X,r20 ;Store the new State to Memory
	ldi	XL, LOW(SRAM_START) ;getting the X pointer where it was before decoder block
	add XL,r17
	reti

	USART_Transmit_OK:
	; Wait for data to be transmitted
	sbis UCSRA, UDRE
	rjmp USART_Transmit_OK
	; Transmit data
	ldi r16,0x4F ;r16=O (from OK)
	;out UDR, r18
	out TCNT2, r16 ; replaced
	USART_Transmit_K:
	sbis UCSRA, UDRE
	rjmp USART_Transmit_K
	ldi r16,0x4B ;r16=K (from OK)
	;out UDR, r18
	out TCNT2, r16 ; replaced

	USART_Transmit_CR:
	sbis UCSRA, UDRE
	rjmp USART_Transmit_CR
	ldi r16,0x0D ;r16=<CR>
	;out UDR, r18
	out TCNT2, r16 ; replaced
	USART_Transmit_LF:
	sbis UCSRA, UDRE
	rjmp USART_Transmit_LF
	ldi r16,0x0A ;r16=<LF>
	;out UDR, r18
	out TCNT2, r16 ; replaced
	ret

;FUNCTIONS
;------------------------------------------------------------------
	;Before you use this function keep your X Register In Memory 
	;r18 The Inserted Byte -> From USART for this exercise
	shiftAllBytesRight:
		ldi	XL,LOW(SRAM_START)		; initialize X pointer
		ldi	XH,HIGH(SRAM_START)		
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
	ret

	;Before you use this function keep your X Register In Memory 
	clearAllBytes:
		ldi	XL,LOW(SRAM_START)		; initialize X pointer
		ldi	XH,HIGH(SRAM_START)		
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
		ret		


	;USART FUNCTIONS
	USART_Init:
	; Set baud rate
	ldi r16,0x00
	out UBRRH, r17
	ldi r16,0x40
	out UBRRL, r16
	ldi r16, 0x00
	out UCSRA,r16
	; Enable receiver and transmitter AND enaBLE interrupts for them
	ldi r16, (1<<RXCIE)|(0<<UDRIE)|(1<<RXEN)|(1<<TXEN)
	out UCSRB,r16
	; Set frame format: 8data, 1stop bit,8 char
	ldi r16, (0<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0)
	out UCSRC,r16
	ret
