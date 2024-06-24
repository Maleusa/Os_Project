#include "syscall.h"

int main() {
	ForkExec("proc0");
	SynchPutString("je suis proc1");
}


