//That test is for step 2
#include "syscall.h"

void printOne(){
    PutChar('a');
}

void printMultiple(){
	int i, n=5;
    char c = 'b';
	for(i = 0; i < n; i++){
		PutChar(c+i);
	}
}

void printSpecialChar(){
    PutChar('\n');
}

int main(){
    printOne();
    printMultiple();
    printSpecialChar();
	Halt();
}