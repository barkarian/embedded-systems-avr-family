/*
 * lab7n8_v0.1.c
 *
 * Created: 28/11/2020 2:08:57 ??
 * Author : teo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
void TIMER0_Init(void);
void TIMER0_Init(void);
void EXTERNAL_INTERRUPTS_Init(void);

//INTERRUPTS
ISR(TIMER0_OVF_vect){
	TCNT0= (unsigned char ) 100;
	unsigned char *lastInputMem = (unsigned char *)0x60;
	unsigned char *stableSignalCounterMem = (unsigned char *)0x61;
	unsigned char volatile bitA,nbitA;
	//Getting lastInput and stableSignalCounter from Memory
	unsigned char lastInput = *lastInputMem;
	unsigned char stableSignalCounter = *stableSignalCounterMem;
	//Getting Current Inputs Stable or Unstable
	//Abit
	bitA=PINA>>PINA0;
	bitA=bitA & 0x01;
	//A'bit
	nbitA=PINA>>PINA1;
	nbitA=nbitA & 0x01;
	//LOGIC
	if( stableSignalCounter==0x00 && (lastInput != bitA || lastInput != (~nbitA & 0x01)) ){
		stableSignalCounter++;
	}
	else if(stableSignalCounter==0x0A){
		stableSignalCounter = 0x00;
		*lastInputMem=bitA;
		PORTA = bitA<<PA2;
	}
	else if(stableSignalCounter>0x00){
		stableSignalCounter++;
	}
	*stableSignalCounterMem=stableSignalCounter;
	
	
}

//External Interrupt Handlers(PD4 is Output)

ISR(INT0_vect){//PD2 is INT0->SET (FALLING EDGE MEANS PD4=0)
	PORTD=0<<PD4;
}

ISR(INT1_vect){//PD3 is INT1->RESET (FALLING EDGE MEANS PD4=1)
	PORTD|=1<<PD4;
}


void INIT(){
	//MEMORY MAP DATA
	unsigned char * lastInputMem =(unsigned char *)0x60;
	unsigned char * stableSignalCounterMem =(unsigned char *)0x61;
	*lastInputMem=0X00;
	*stableSignalCounterMem=0x00;
	
	//SETTING DATA DIRECTION REGISTERS
	DDRA=(unsigned char)0b01111100;	//PA0 PA1 PINs PA2			->Sampling
	DDRD=(unsigned char)0b11110011; //PD2(INT0) PD3(INT1) PINs	->Interrupt sensitive
	//SETTING TIMER
	TIMER0_Init();
	//SETTING EXTERNAL INTERRUPTS
	EXTERNAL_INTERRUPTS_Init();
	//ENABLE INTERRUPTS
	sei();
	return;
}


void TIMER0_Init(void){
	TCNT0= (unsigned char ) 100;
	TIMSK= (1<<TOIE0);	//mask on
	TCCR0= 0x03;		// Prescaler Bits cs2cs1cs0 was 0x04 now 0x03 for nearly 1ms
}

void EXTERNAL_INTERRUPTS_Init(void){
	unsigned char * QMem =(unsigned char *)0x62;
	*QMem=0x00;
	GICR|=(1<<INT0)|(1<<INT1);
	MCUCR |= 0b00001010; //isc11,isc10 and isc01,isc00 both set to -> 01 (Any logical change will trigger them)
}



int main(void)
{
	INIT();
	while (1){}
}