;
; lab1_Timer.asm
;
; Created: 7/10/2020 6:34:38 μμ
; Author : teo
;


; Replace with your application code

.org $0000 ;reset
rjmp reset

.org OVF0addr ;
rjmp timerOVF

.org 0x100
reset:
ldi r16, HIGH(RAMEND) ; Upper byte
out SPH,r16 ; to stack pointer

ldi r16, LOW(RAMEND)  ; Lower byte
out SPL,r16 ; to stack pointer

sei
ldi r16,0x64 ; start from 100 = 256 - (156)
;ldi r16,0xB2	;start from 178 = 256 - (78)  Wrong
out TCNT0,r16	;setting timer

ldi r17,(1<<TOIE0)	;mask for register 1
out TIMSK,r17	;setting timer

;ldi r18,0x01
ldi r18,0x03	;cs2cs1cs0 =binary 011= 0x03
out TCCR0,r18	;setting prescaler clk/64 AND starting clock for timer


waitloop:
	rjmp waitloop

timerOVF: ;Interrrupt Handler
	ldi r19,0xFF ;EXIT
	ldi r18,0x00	;cs2cs1cs0 =binary 000= 0x00
	out TCCR0,r18	;setting prescaler to not count at all 
	reti
	





