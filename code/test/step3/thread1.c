#include "syscall.h"

void boucle(int i){
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
	boucle(arg);
}
int main(){
	int un = 1;
	UserThreadCreate(threadfct, &un);
}
