/*
 * lab4_v0.1.c
 *
 * Created: 4/11/2020 1:56:20 μμ
 * Author : teo
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define FOSC 10000000
#define BAUD 9600
#define UBRR FOSC/16/BAUD-1

extern void ISR_timerOVF(void);
extern void ISR_USART_Receive(void);


ISR(TIMER0_OVF_vect){
	ISR_timerOVF();
}
ISR(USART_RXC_vect){
	ISR_USART_Receive();
}


void INIT(){
	//memory map for data
	unsigned char *displayNumbers=(unsigned char *) 0x60;
	displayNumbers[0]=(unsigned char)0x05;
	displayNumbers[1]=(unsigned char)0x06;
	displayNumbers[2]=(unsigned char)0x07;
	displayNumbers[3]=(unsigned char)0x08;
	displayNumbers[4]=(unsigned char)0x09;
	displayNumbers[5]=(unsigned char)0x00;
	displayNumbers[6]=(unsigned char)0x01;
	displayNumbers[7]=(unsigned char)0x02;
	unsigned char *seven_segment_decoder=(unsigned char *) 0x68;
	seven_segment_decoder[0]=(unsigned char)0b00000001; //0
	seven_segment_decoder[1]=(unsigned char)0b01001111; //1
	seven_segment_decoder[2]=(unsigned char)0b00010010; //2
	seven_segment_decoder[3]=(unsigned char)0b00000110; //3
	seven_segment_decoder[4]=(unsigned char)0b01001100; //4
	seven_segment_decoder[5]=(unsigned char)0b00100100; //5
	seven_segment_decoder[6]=(unsigned char)0b00100000; //6
	seven_segment_decoder[7]=(unsigned char)0b00001111; //7
	seven_segment_decoder[8]=(unsigned char)0b00000000; //8
	seven_segment_decoder[9]=(unsigned char)0b00000100; //9
	seven_segment_decoder[10]=(unsigned char)0b11111111; //A
	unsigned char *curNumberIndex=(unsigned char *) 0x73; //r17 value
	*curNumberIndex=(unsigned char)0x07;
	unsigned char *ringCounter=(unsigned char *) 0x74; //r17 value
	*ringCounter=(unsigned char)0x80;
	
	//masks for ports a and c
	//i just realise in the past labs i use port B as my port A
	//so i fixed this in this one
	DDRC=(unsigned char) 0xff;
	DDRA=(unsigned char)0b01111111;
	asm("nop");


    unsigned int ubrr= (unsigned int)UBRR;

    USART_Init(ubrr);
    TIMER0_Init();
	sei();
	return;
}

void USART_Init( unsigned int ubrr)
{
    unsigned char *curState=(unsigned char *) 0x75;//set state init
	*curState=(unsigned char)0x00;
	/* Set baud rate */
    UBRRH = (unsigned char)(ubrr>>8);
    UBRRL = (unsigned char)ubrr;
    /*Setting 0*/
    UCSRA=(unsigned char)0x00;
    /* Enable receiver and transmitter */
    UCSRB = (1<<RXCIE)|(0<<UDRIE)|(1<<RXEN)|(1<<TXEN);
    /* Set frame format: 8data, 2stop bit */
    UCSRC = (0<<URSEL)|(0<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);
}

void TIMER0_Init(void){
    
    TCNT0= (unsigned char ) 100;
    TIMSK= (1<<TOIE0); //mask on
    TCCR0= (1<<CS02); //prescaller cs2cs1cs0 was 0x03 now 0x04 for nearly 30hz 
}


void USART_Transmit( unsigned char data )
{
/* Wait for empty transmit buffer */
while ( !( UCSRA & (1<<UDRE)) )
;
/* Put data into buffer, sends the data */
TCNT2 = data;
}

void USART_Transmit_OK( unsigned char data )
{
USART_Transmit((unsigned char)0x4F); //O
USART_Transmit((unsigned char)0x4B); //K
USART_Transmit((unsigned char)0x0D); //<CR>
USART_Transmit((unsigned char)0x0A); //<LF>
}


int main(void)
{
	INIT();
	while (1)
	{
	}
}
