//That test is for step 2
#include "syscall.h"

void getSingleString(){
	char c [5];
    SynchGetString(c,5);
	SynchPutString(c);
}

void getMultipleString(){
    int count = 5;
    for (int i = 0; i < count; i++)
    {
	    char c [5];
        SynchGetString(c,5);
	    SynchPutString(c);
    }
}


int main(){
    getSingleString();
    getMultipleString();
    Halt();
}