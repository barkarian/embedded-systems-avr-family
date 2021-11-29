/*
 * lab7n8_v0.1.c
 *
 * Created: 28/11/2020 2:08:57 πμ
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
#define Sstate 0x05
#define Qstate 0x06

#define sliceCounterSize 0x02 //just for testing Set to 0x19=25 for 100ms 
#define totalProcesses 0x03 //With Current Solution Till 8 Processes

void findNextProcess();
unsigned char sliceCounterFlag();
unsigned char maskAndSetBit(unsigned char input);

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
	
	//SLICE COUNTER
	unsigned char *processesFlagsMem=(unsigned char *) 0x7A;//3FlagBits
	unsigned char *sliceFlagMem=(unsigned char *) 0x78;
	unsigned char processesFlags=*processesFlagsMem;
	
	unsigned char sliceFlag = sliceCounterFlag();
	//even if we got a sliceFlag we don't want to change Process if we don't have one
	//Because we gonna stuck in while loop of findNextProcess	
	if(sliceFlag==0x01 && processesFlags!=0){
		findNextProcess();
	}
	else if(sliceFlag==0x01){
		*sliceFlagMem=0x00;		
	}
	
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
	unsigned char volatile input=UDR;
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
	unsigned char compProcessFlag;
	unsigned char *processesFlagsMem=(unsigned char *) 0x7A;
	unsigned char processesFlags = *processesFlagsMem;
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
			}else if (input=='S'){
			*stateMem=Sstate;	
			}else if(input=='Q'){
			*stateMem=Qstate;
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
		case Sstate:		//S state wait for number 1-3
		compProcessFlag=maskAndSetBit(input);
		*processesFlagsMem=processesFlags|compProcessFlag;
		*stateMem=CRWAITstate;
		break;
		case Qstate:		//LF Just wait for number 1-3
		compProcessFlag=maskAndSetBit(input);
		compProcessFlag=~compProcessFlag;
		*processesFlagsMem=processesFlags & compProcessFlag;
		*stateMem=CRWAITstate;
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
	//MEMORY MAP DATA
	unsigned char *displayNumbers=(unsigned char *) 0x60;
	displayNumbers[0]=(unsigned char)0x05;
	displayNumbers[1]=(unsigned char)0x05;
	displayNumbers[2]=(unsigned char)0x05;
	displayNumbers[3]=(unsigned char)0x05;
	displayNumbers[4]=(unsigned char)0x05;
	displayNumbers[5]=(unsigned char)0x05;
	displayNumbers[6]=(unsigned char)0x05;
	displayNumbers[7]=(unsigned char)0x05;
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
	//USART DATA
	//0x75 is curState
	//ox76 is Data in from USART
	//PROCESSES DATA/SLICE COUNTER
	unsigned char *sliceCounter=(unsigned char *) 0x77;
	unsigned char *sliceFlag=(unsigned char *) 0x78;
	unsigned char *curProcess=(unsigned char *) 0x79;
	unsigned char *processesFlags=(unsigned char *) 0x7A;//3FlagBits
	*sliceCounter=0x00;//0 to 24
	*sliceFlag=0x00;//0 or 1
	*curProcess=0x00;//0 or 1 or 2
	*processesFlags=0b00000000; //Running Processes :1,3 
	//PROCESSES
	unsigned char * ringCounterMem1=(unsigned char *) 0x7B;
	unsigned char * ringCounterMem2=(unsigned char *) 0x7B;
	unsigned char *zerosOnesMem=(unsigned char *) 0x7D;
	*ringCounterMem1=0x01;
	*ringCounterMem2=0x01;
	*zerosOnesMem=0x00;
	
	
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


//Updates sliceCounter and returns sliceFlag
unsigned char sliceCounterFlag(){
	//MEMORY LOADING
	unsigned char *sliceCounterMem=(unsigned char *) 0x77;
	unsigned char *sliceFlagMem=(unsigned char *) 0x78;
	unsigned char sliceCounter=*sliceCounterMem;
	//LOGIC
	if(sliceCounter==sliceCounterSize-1){
		*sliceFlagMem=1;//MEMORY STORING
		sliceCounter=0;
	}else{
		sliceCounter++;
	}
	*sliceCounterMem=sliceCounter;//MEMORY STORING
	return *sliceFlagMem;
}

//Finds next process and store it in RAM(curProcess location)
void findNextProcess(){
	//MEMORY LOADING
	unsigned char *curProcessMem=(unsigned char *) 0x79;
	unsigned char *processesFlagsMem=(unsigned char *) 0x7A;//3FlagBits
	unsigned char processesFlags=*processesFlagsMem;
	unsigned char curProcess=*curProcessMem;
	//LOCAL VARIABLES
	unsigned char nextProcessFlag=0x00;
	unsigned char nextProcessFlagComp;
	//LOGIC
	while(nextProcessFlag==0){
		curProcess++;
		if(curProcess==totalProcesses){
			curProcess=0x00;
		}
		nextProcessFlagComp=1<<curProcess;
		nextProcessFlag=processesFlags & nextProcessFlagComp;
	}
	//MEMORY STORING
	*curProcessMem=curProcess;
}

//Decoder from usart to bitwise processFlag 
unsigned char maskAndSetBit(unsigned char input){
	unsigned char maskedInput=input & 0x0F;
	unsigned char compProcessFlag;
	compProcessFlag = 1<<(maskedInput-1);
	return compProcessFlag;
}

//PROCESS1
//0x01 → 0x02 → ... → 0x80→ 0x01 → ...
void ringCounterRightProc(){
	unsigned char * ringCounterMem=(unsigned char *) 0x7B;
	unsigned char ringCounter=* ringCounterMem;
	if(ringCounter==0x80){
		ringCounter=0x01;
	}else{
		ringCounter=ringCounter<<1;
	}
	PORTB=ringCounter;
	*ringCounterMem=ringCounter;
}
//PROCESS2
//0x80 → 0x40 → ... → 0x01→ 0x80 → ...
void ringCounterLeftProc(){
	unsigned char *ringCounterMem=(unsigned char *) 0x7C;
	unsigned char ringCounter=* ringCounterMem;
	if(ringCounter==0x80){
		ringCounter=0x01;
		}else{
		ringCounter=ringCounter>>1;
	}
	PORTB=ringCounter;
	*ringCounterMem=ringCounter;
}

//PROCESS3
//0x80 → 0x40 → ... → 0x01→ 0x80 → ...
void zerosToOnesAndBack(){	//makes 0x00 → 0xFF → 0x00 → ...
	unsigned char *valueMem=(unsigned char *) 0x7D;
	unsigned char value=* valueMem;
	if(value==0x00){
		value=0xFF;
		}else{
		value=0x00;
		}
	PORTB=value;
	*valueMem=value;
}




int main(void)
{
	INIT();
	unsigned char *sliceFlagMem=(unsigned char *) 0x78;
	unsigned char *curProcessMem=(unsigned char *) 0x79;
	unsigned char volatile sliceFlag;
	unsigned char volatile curProcess=*curProcessMem;
	while (1)
	{
	if(sliceFlag==0x01){
			//run code
			curProcess=*curProcessMem;
			switch(curProcess){
				case 0x00:
				asm("call ringCounterRightProc");
				break;
				case 0x01:
				asm("call ringCounterLeftProc");
				break;
				case 0x02:
				asm("call zerosToOnesAndBack");
				break;
			}
		*sliceFlagMem=0x00;
		}
	sliceFlag = *sliceFlagMem;
	}
}