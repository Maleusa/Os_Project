//That test is for step 3
//For that next test, step 2 is considered as
//working properly 

//For that test you need to use the -rs option with multiple seed to see if 
//errors can occurs

#include "syscall.h"

void threadfct0(){
	SynchPutConsole("Thread : 0\n");
	UserThreadExit();
}

void threadfct1(){
	SynchPutConsole("Thread : 1\n");
	UserThreadExit();
}

void threadfct2(){
	SynchPutConsole("Thread : 2\n");
	UserThreadExit();
}

void threadfct3(){
	SynchPutConsole("Thread : 3\n");
	UserThreadExit();
}

void threadfct4(){
	SynchPutConsole("Thread : 4\n");
	UserThreadExit();
}



void threadCreateMultiple(){
    int zero = 0;
    int un = 1;
	int deux = 2;
	int trois = 3;
	int quatre = 4;
	UserThreadCreate(threadfct0, &zero);
	UserThreadCreate(threadfct1, &un);
	UserThreadCreate(threadfct2, &deux);
	UserThreadCreate(threadfct3, &trois);
	UserThreadCreate(threadfct4, &quatre);
}

int main(){
    threadCreateMultiple();
    Halt();
}