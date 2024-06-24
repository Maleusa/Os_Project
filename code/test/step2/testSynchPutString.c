//That test is for step 2
#include "syscall.h"

void printOneChar(){
    SynchPutString("a");
}

void printOneChain(){
    SynchPutString("\nIt's a longer chain.\n");
}

void printMultiple(){
	int i, n=5;
    const char* c = "Some multiple chain.\n";
	for(i = 0; i < n; i++){
		SynchPutString(c);
	}
}

void printSpecialChar(){
    PutChar('\n');
}

int main(){
    printOneChar();
    printOneChain();
    printMultiple();
	Halt();
}