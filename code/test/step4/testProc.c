#include "syscall.h"

int main(){
	ForkExec("putchar");
	SynchPutString("new Process lancé.");
	Exit(0);
}
