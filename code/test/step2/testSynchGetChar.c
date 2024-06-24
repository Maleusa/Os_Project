//That test is for step 2
#include "syscall.h"

void getSingleChar(){
	char c = SynchGetChar();
	PutChar(c);
}

void getMultipleChar(){
    int count = 5;
    for (int i = 0; i < count; i++)
    {
	    char c = SynchGetChar();
	    PutChar(c);
    }
}


int main(){
    getSingleChar();
    getMultipleChar();
    Halt();
}