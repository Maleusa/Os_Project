//That test is for step 2
#include "syscall.h"

void printOne(){
    SynchPutInt(42);
}

void printMultiple(){
	int i, n=5;
	for(i = 0; i < n; i++){
		SynchPutInt(i);
	}
}

int main(){
    printOne();
    printMultiple();
	Halt();
}