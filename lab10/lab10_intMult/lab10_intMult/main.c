/*
 * lab10_v0.1.c
 *
 * Created: 12/21/2020 1:14:30 AM
 * Author : teo
 */ 

#include <avr/io.h>
#define arrayWidth 3
void initTableInt(int * A);
void init3tables(int *A,int *B,int *C);
void multiply_tables_3x3(int * A,int *B,int *C);

void init2tables(int *A,int *B){
		initTableInt(A);
		initTableInt(B);
}

void initTableInt(int * A){
	for(int i=0;i<arrayWidth;i++){
		for(int j=0;j<arrayWidth;j++){
			A[i*arrayWidth+j]=i*arrayWidth+j;
		}
	}
}

void multiply_tables_3x3(int * A,int *B,int *C){
	for(int i=0;i<arrayWidth;i++){
		for(int j=0;j<arrayWidth;j++){
			int sum=0;
			for(int k=0;k<=arrayWidth;k++){
				sum=sum +A[i*arrayWidth+k]*B[k*arrayWidth+j];	//sum=sum +A[i][k] * B[k][j];	
			}
			C[i*arrayWidth+j]=sum;
		}
	}
}


int main(void)
{
	/*---------------------------------------------------------------------
	Position of arrays stay fixed on this addresses for comparison reason
	It's OK to do this because they have smaller sizes in comparison to 
	the float arrays.
	An int contains 2 bytes and a float contains 4 bytes.
	thats why there's no danger of overlapping and overwritten with each other.
	----------------------------------------------------------------------*/
	int * A =(int *) 0x60;
	int * B =(int *) 0x84;
	int * C =(int *) 0xA8;
	init2tables(A,B);
	multiply_tables_3x3(A,B,C);
    while (1) 
    {
    }
}

