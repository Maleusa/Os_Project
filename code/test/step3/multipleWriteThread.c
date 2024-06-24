#include "syscall.h"

void boucleA(){
	SynchPutConsole("Normalement la phrase est toujours complete ");
}

void threadfct(void* p){
//	int arg = *(int *) p;
	boucleA();
	UserThreadExit();
}
int main(){
	int un = 1;
	int deux = 2;
	int trois = 3;
	int quatre = 4;
	UserThreadCreate(threadfct, &un);
	UserThreadCreate(threadfct, &deux);
	UserThreadCreate(threadfct, &trois);
	UserThreadCreate(threadfct, &quatre);
	boucleA(0);
}
