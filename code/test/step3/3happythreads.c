#include "syscall.h"

void parle(int i){
    SynchPutString("I'm thread ");
    SynchPutInt(i);
    SynchPutString(" and I'm happy :)");
    PutChar('\n');
}

void threadfct(void* p){
    int arg = *(int *) p;
    parle(arg);
    UserThreadExit();
}

// First simple test
int main(){
    int paramT1 = 1;
    int paramT2 = 2;
    if (UserThreadCreate(threadfct, &paramT1) == -1 )
        Exit(-1);
    if (UserThreadCreate(threadfct, &paramT2) == -1 )
        Exit(-1);
    parle(0);
}
