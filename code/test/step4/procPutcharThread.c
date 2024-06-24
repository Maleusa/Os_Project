#include "syscall.h"

void print(char c, int n) {
	int i;
	for(i = 0; i < n; i++){
		PutChar(c+i);
	}
	PutChar('\n');
}
void putchar(void *p ){
	print('a', 4);
}
int main(){
	//ForkExec("putchar");
	int i = 0;
	UserThreadCreate(putchar, &i);
	SynchPutString("F");
	SynchPutInt(10);
	Exit(0);
}
