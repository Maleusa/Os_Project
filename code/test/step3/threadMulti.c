#include "syscall.h"

void boucleA(int i){
	SynchPutString("Thread : ");
	SynchPutInt(i);
	SynchPutString("\n count [ ");
	for (int k = i; k > 0; k--){
		SynchPutInt(k);
	}
	PutChar(']');
	PutChar('\n');
}

void threadfct(void* p){
	int arg = *(int *) p;
	boucleA(arg);
	UserThreadExit();
}
int main(){
	int un = 1;
	int deux = 2;
	int trois = 3;
	int quatre = 4;
	//int p=0;
	UserThreadCreate(threadfct, &un);
	UserThreadCreate(threadfct, &deux);
	UserThreadCreate(threadfct, &trois);
	UserThreadCreate(threadfct, &quatre);
	boucleA(0);
	//Halt();
}
