#include "syscall.h"

int main() {
	ForkExec("proc1");
	
	SynchPutString("je suis proc2");
	
	
}


