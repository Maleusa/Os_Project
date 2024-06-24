#include "syscall.h"



int main(){
	char c [128];
    SynchGetString(c,128);
	SynchPutString(c);
    Exit(0);
}