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
#define INITstate 0x00
#define TWAITstate 0x01
#define BYTEOUTstate 0x02
#define CRWAITstate 0x03
#define LFWAITstate 0x04

//extern void ISR_timerOVF(void);
//extern void ISR_USART_Receive(void);

//INTERRUPTS
ISR(TIMER0_OVF_vect){
	unsigned char curNumberIndex,ringCounter;
	unsigned char curNumber;
	//get from memory
	unsigned char * numberArray=(unsigned char *)0x60;
	unsigned char * ssdArray=(unsigned char *)0x68;
	unsigned char *tmp=(unsigned char *) 0x73;		//used for setting ringCounter and curNumberIndex
	curNumberIndex=*tmp;
	tmp++;
	ringCounter=*tmp;
	//set counter for next interrupt
	TCNT0=0x64;
	//program logic
	if(curNumberIndex<7){
		curNumberIndex++;
		ringCounter = ringCounter<<1;	
	}
	else{
		curNumberIndex=0;
		ringCounter=0b00000001;
	}
	//display changes
	PORTC=0x00;//close display
	curNumber=numberArray[curNumberIndex];//getting number
	PORTA=ssdArray[curNumber];//setting up SSD
	PORTC=ringCounter;//open display's right anode
	
	//set updated data in memory
	tmp=(unsigned char *) 0x73;
	*tmp=curNumberIndex;
	tmp++;
	*tmp=ringCounter;
}

ISR(USART_RXC_vect){
	//get from memory
	unsigned char *stateMem=(unsigned char *) 0x75;
	//get input from usart
	unsigned char input=UDR;
	input=UDR;
	
	//For simulation purposes
	unsigned char *simData =0x76;
	__asm__(
	"ldi XH,0x00\n"
	"ldi XL,0x76\n"
	"st X,r15\n"
	"nop\n"
	);
	input=*simData;
	
	//FSM
	switch (*stateMem){
    case INITstate:			//A or N or C just Came
		if(input=='A'){
			*stateMem=TWAITstate;
		}else if (input=='N'){
			clearAllNumbers();
			*stateMem=BYTEOUTstate;
		}else if(input=='C'){
			clearAllNumbers();
			*stateMem=CRWAITstate;
		}
      break;
    case TWAITstate:		//T Just Came
		*stateMem=CRWAITstate;
      // statements
      break;
	case BYTEOUTstate:		//BYTE or CR Just Came
		if(input==0x0D){//<CR> ASCII code is 0x0D
			*stateMem=LFWAITstate;
		}else{				//NUMBER Just Came
			add_number_UDR(input);
			// *stateMem Does Not Change
		}
	  break;
	case CRWAITstate:		//CR Just Came
		*stateMem=LFWAITstate;
      break;
    case LFWAITstate:		//LF Just Came
		USART_Transmit_OK();
		*stateMem=INITstate;
      break;
    default:
		break;
	}
}

//numberArray[0 to 8] = 0x0A ->which means closed screens after SSD_decode
void clearAllNumbers(){
	unsigned char * numberArray=(unsigned char *)0x60;
	for(unsigned char i=0;i<=7;i++){
		numberArray[i]=(unsigned char)0x0A;//closed all segments number
	}
}

//add input to start of numberArray and shifts all the other numbers
void add_number_UDR(unsigned char input){
	unsigned char * numberArray=(unsigned char *)0x60;
	unsigned char placeholder,insertedValue;
	insertedValue=input&0x0F;
	for(unsigned char i=0;i<=7;i++){
		placeholder=numberArray[i];
		numberArray[i]=insertedValue;
		insertedValue=placeholder;
	}	
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
	*curState=(unsigned char)INITstate;
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

void USART_Transmit_OK()
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
