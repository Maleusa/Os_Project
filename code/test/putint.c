#include "syscall.h"

void print(char c, int n) {
	int i;
	for(i = 0; i < n; i++){
		SynchPutInt(i);
	}
	PutChar('\n');
}

int main(){
	print('a', 4);
	Halt();
}