#include "syscall.h"

int main(){
	ForkExec("putchar");
	Exit(0);
}
