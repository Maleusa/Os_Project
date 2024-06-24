//That test is for step 3
//For that next test, step 2 is considered as
//working properly 

#include "syscall.h"
void boucle(int i){
	SynchPutConsole("Thread : ");
	SynchPutInt(i);
    SynchPutConsole("\n");
}

void threadfct(void* p){
	int arg = *(int *) p;
	boucle(arg);
	UserThreadExit();
}
void threadCreateOne(){
    int zero = 0;
	UserThreadCreate(threadfct, &zero);
}

void threadCreateMultiple(){
    int un = 1;
	int deux = 2;
	int trois = 3;
	int quatre = 4;
	UserThreadCreate(threadfct, &un);
	UserThreadCreate(threadfct, &deux);
	UserThreadCreate(threadfct, &trois);
	UserThreadCreate(threadfct, &quatre);
}

int main(){
	threadCreateOne();
    threadCreateMultiple();
    Halt();
}