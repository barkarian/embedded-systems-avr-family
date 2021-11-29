;
; lab2_proj.asm
;
; Created: 13/10/2020 6:08:42 μμ
; Author : teo
;


; Replace with your application code

.org $0000 ;reset
rjmp reset

.org OVF0addr ;
rjmp timerOVF

.org 0x100
reset:
ldi r17,0x07 ;r17 is the register that shows which segment is open and when to change X register ;
			 ;is important to initializing it at 0x07 so it points the correct memory address that we initialize
			 ;the first time it goes into loading block
ldi r19,0x80 ;rotating Anode used inside load block with rol command

;we Store in Ram
ldi	XL,LOW(SRAM_START)		; initialize X pointer
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
ldi r16,0x03
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

ldi r16,8
ldi	XL,LOW(SRAM_START)		; initialize X pointer
add XL,r16
;initialization over


ldi r16, HIGH(RAMEND) ; Upper byte
out SPH,r16 ; to stack pointer

ldi r16, LOW(RAMEND)  ; Lower byte
out SPL,r16 ; to stack pointer

sei
ldi r16,0x64 ; start from 100 = 256 - (156)
;ldi r16,0xB2	;start from 178 = 256 - (78)  Wrong
out TCNT0,r16	;setting timer

ldi r16,(1<<TOIE0)	;mask for register 1
out TIMSK,r16	;setting timer

;ldi r18,0x01
ldi r16,0x04	;cs2cs1cs0 =binary 011= 0x03
out TCCR0,r16	;setting prescaler clk/64 AND starting clock for timer

;out DDRx
ldi r16,0xff				
out DDRC, r16
ldi r16,0b01111111			
out DDRB, r16											


waitloop:
	rjmp waitloop

timerOVF: ;Interrrupt Handler
	ldi r16,0x64 ; start from 100 = 256 - (156)
	out TCNT0,r16	;setting timer from the start so that we where as close as possible to the theoretical estimation
	
	;loading block start
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
	;Loading block gives us in r18 the Number we gonna display is bsd AND at R17 the memory location of The Decimal Degit we display AND in r19 the Port C outputs
	;dont touch r17,r19,through all the program
	;dont touch r18 until you succesfully decode it and get it out
	 
	
	;decoder Start
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