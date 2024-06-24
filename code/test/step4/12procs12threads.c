#include "syscall.h"

#define NB_PROCS 1000

int main(){
    int pid[NB_PROCS];
    // Creation of NB_PROCS processes
    for (int i=0; i < NB_PROCS; i++){
        // Each process create NB_THREADS
        // and wait for it
        pid[i]=ForkExec("step4/dadwaiting");
    }

    SynchPutString("Waitin...\n");
    for (int i = 0; i <NB_PROCS; i++){
        if(pid[i]>=0){
            UserProcJoin(pid[i],-2);
            SynchPutString("Process ");
            SynchPutInt(pid[i]);

            SynchPutString(" Joined\n");
        }

    }
    SynchPutString("All Process Joined, GOODBYE ! \n");
    //Halt();
}