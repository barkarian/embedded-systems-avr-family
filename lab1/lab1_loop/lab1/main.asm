;
; lab1.asm
;
; Created: 6/10/2020 2:32:58 μμ
; Author : teo
;


; Replace with your application code
start:
 LDI r16,0x14		;R16=INDEX_LOOP2=20 
					;EXECUTIONS=1CK
 loop1:				;EXEC_COUNT_LOOP1= LOOP1*4-1 + EXEC_COUNT_LOOP2
	LDI r17,0xA6	;R17=INDEX_LOOP2=166 
	
	loop2:			;EXEC_COUNT_LOOP2= 3* LOOP1 * LOOP2 -LOOP1
		DEC r17		
		BRNE loop2	;WHEN WE EXIT THE LOOP BRANCH_EXEC_COUNT=1 / ELSE =2 
	
	DEC r16			;EXEC_COUNT=1
	BRNE loop1		;WHEN WE EXIT THE LOOP BRANCH_EXEC_COUNT=1 / ELSE =2 
	
	;TOTAL_EXEC_COUNT =3*loop1*loop2 - 3*loop1 = 10020
	LDI r19,0xFF
successloop:
rjmp successloop