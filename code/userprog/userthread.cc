#include "userthread.h"
#include "thread.h"
#include "system.h"
#include "syscall.h"

extern void itoa(int val, char *s, int mod);

typedef struct serial {
	int indexStack;
	int f;
	int args;
	int tid;
} funAndArg;

int stackNewThreadOffsett = PageSize * 3;

static void startUserThread(int f){
	funAndArg *s;
	s = (funAndArg *) f;
	// init of regs

	s->tid=currentThread->space->GetNextTid();

	currentThread->space->newJoinableThread(s->tid);
	
	currentThread->space->IncrNbThreads();
	currentThread->space->InitRegisters();	
	currentThread->space->RestoreState();	
	// la fonction _start se lance (PC = 0)
	machine->WriteRegister(PCReg, 0);
	machine->WriteRegister(NextPCReg, 4);
	// parametre pour lancer startThread dans _start
	machine->WriteRegister(5, 1);
	// parametre a utiliser dans startThread
	machine->WriteRegister(4, s->args);
	machine->WriteRegister(6, s->f);
	// placer StackReg
	currentThread->space->AllocateNewStack(s->indexStack);
	//set TID
	currentThread->tid=s->tid;
	// run user program
	machine->Run();
}


// extern void do_UserThreadExit(){
// 	currentThread->DeleteStack(); // Free stack
// 	AddrSpace * space = currentThread->space;
// 	space->DecrNbThreads();
// 	if (space->IsLastThread()){
// 		//space->FinishAllSleepingThreads();
// 		if (space->IsHaltRequested()) interrupt->Halt();
// 		else currentThread->Finish();
// 	}
// 	else {
// 		IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

//         space->AddSleepingThreads();
// 		currentThread->Finish();
// 		(void)interrupt->SetLevel(oldLevel); // re-enable interrupts
// 	}
// }

extern int do_UserThreadCreate(int f, int args){
	funAndArg *s = new funAndArg ;
	if ( (s->indexStack = currentThread->space->ReserveStack()) < 0 ) return -1 ; // no more stack available
	//currentThread->stackID = s->indexStack;
	s->f = f;
	s->args = args;
	char nameT[10];
	itoa(s->indexStack, nameT, 10);
	// create a Nachos thread
	Thread *t = new Thread(nameT);
	t->stackID=s->indexStack;
	t->tid=s->tid;
	t->Fork(startUserThread, (int) s);
	if (t == NULL) return -1; // if the thread creation has not succeed
	if (t->space == NULL) { // if the space is missed allocated
		delete t;
		return -2;
	}
	return s->tid;
}
