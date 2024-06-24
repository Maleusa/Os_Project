#include "syscall.h"

#define NB_T 180

void play(){
    SynchPutString("=D\n");
}

int main(){
    //int ptr = 1;
    int tid_filston;
    int tids[NB_T];
    for (int j=0; j < NB_T; j++){
        tid_filston = UserThreadCreate(play, 0);
        tids[j] = tid_filston;
    }
    for (int j=0; j < NB_T; j++){
        if(tids[j]>0)
        UserThreadJoin(tids[j], -2);
    }
    SynchPutString("Process : All kids joined\n");
}
