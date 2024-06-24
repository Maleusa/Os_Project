#include "syscall.h"

void boucle(int i){
	for (int k = i; k > 0; k--){
		SynchPutInt(k);
	}
	PutChar('\n');
}

void threadfct(void* p){
	int arg = *(int *) p;
	boucle(arg);
	UserThreadExit();
}
int main(){
	int param = 3;
	int param2 = 5;
	UserThreadCreate(threadfct, &param);
	param2 = param + param2;
	Exit(0);
}
