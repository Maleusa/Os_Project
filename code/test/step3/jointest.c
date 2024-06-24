#include "syscall.h"

void parle(int i){
    SynchPutString("I'm thread ");
    SynchPutInt(i);
    SynchPutString(" and I'm happy :)");
    PutChar('\n');
    UserThreadExit(i);
}

void threadfct(void* p){
    int arg = *(int *) p;
    parle(arg);
    
}

// First simple test
int main(){
    int paramT1[5];
    paramT1[1]=1;
    paramT1[2]=2;
    paramT1[3]=3;
    paramT1[4]=4;
    
    int ptr=1;
    for (int i= 1; i<5;i++){
        
        UserThreadCreate(threadfct, &paramT1[i]);
    }
    for (int i=1; i<5;i++){
        UserThreadJoin(i,(int)&ptr);
        SynchPutString("Je suis le thread 0 et j'attendais le thread ");
        SynchPutInt(i);
        SynchPutString(" Il est sorti avec l'exit value :");
        SynchPutInt(ptr);
        PutChar('\n');
    } 
    Halt();
}
