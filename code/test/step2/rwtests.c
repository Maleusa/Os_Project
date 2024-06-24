    #include "syscall.h"

void writeInNumberThread(int i){
    SynchPutString("I'm thread ");
    SynchPutInt(i);
    SynchPutString(" and I'm happy :)");
    PutChar('\n');
}

void writeIn(char* c){
    SynchPutString(c);
    PutChar('\n');
}

void readIn(char* c, int n){
    // SynchGetChar();
    SynchGetString(c, n);
}

void threadfct(void* p){
    int nb = *(int*) p;
    char* c = "hello world";
    writeInNumberThread(nb);
    readIn(c,9);
    writeIn(c);
    UserThreadExit(0);
}

// First simple test
int main(){
    int paramT1 = 1;
    if (UserThreadCreate(threadfct, &paramT1) == -1 )
        Exit(-1);
    char* c = "hello world";
    writeInNumberThread(0);
    readIn(c,9);
    writeIn(c);
    // Halt();
}