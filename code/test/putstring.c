#include "syscall.h"

void print(const char *c) {
	SynchPutString(c);
}

int main(){
	const char s[] = "salut mec 123456789 123456789 123456789 123456789 123456789 ";
	print(s);
	Halt();
}
