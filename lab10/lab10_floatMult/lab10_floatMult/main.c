/*
 * lab10_v0.1.c
 *
 * Created: 12/21/2020 1:14:30 AM
 * Author : teo
 */ 

#include <avr/io.h>
#define arrayWidth 3
void initTableFloat(float * A);
void init3tables(float *A,float *B,float *C);
void multiply_tables_3x3(float * A,float *B,float *C);

void init2tables(float *A,float *B){
		initTableFloat(A);
		initTableFloat(B);
}

void initTableFloat(float * A){
	for(int i=0;i<arrayWidth;i++){
		for(int j=0;j<arrayWidth;j++){
			A[i*arrayWidth+j]=i*arrayWidth+j;
		}
	}
}

void multiply_tables_3x3(float * A,float *B,float *C){
	for(int i=0;i<arrayWidth;i++){
		for(int j=0;j<arrayWidth;j++){
			float sum=0;
			for(int k=0;k<=arrayWidth;k++){
				//sum=sum +A[i][k] * B[k][j];
				sum=sum +A[i*arrayWidth+k]*B[k*arrayWidth+j];		
			}
			C[i*arrayWidth+j]=sum;
		}
	}
}


int main(void)
{
	/*--------------------------------------------------------------------
	Position of arrays stay fixed on this addresses for comparison reason
	It's OK to do this because they have smaller sizes in comparison to 
	the float arrays.
	An int contains 2 bytes and a float contains 4 bytes.
	thats why there's no danger of overlapping and overwritten with each other.
	----------------------------------------------------------------------*/
	float * A =(float *) 0x60;//start of RAM
	float * B =(float *) 0x84;//start where table A ends
	float * C =(float *) 0xA8;//start where table B ends
	init2tables(A,B);
	multiply_tables_3x3(A,B,C);
    while (1) 
    {
    }
}

