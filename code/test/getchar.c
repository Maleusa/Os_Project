#include "syscall.h"



int main(){
	char c = SynchGetChar();
	PutChar(c);
    Halt();
}