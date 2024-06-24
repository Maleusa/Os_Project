#include "processtable.h"
#include "system.h"
#include "noff.h"
#include <stdexcept>

ProcessTable::ProcessTable(){
	// TODO start at 0
	countProcess = 1;
	S = new Semaphore("processTableSem", 1);
	pidCounter = 0;
	
}

ProcessTable::~ProcessTable(){
}

void startUserProc(int arg){
	//Do the doobly do
	
	// init of regs
	currentThread->space->InitRegisters();	
	currentThread->space->RestoreState();	
	// run user program
	machine->Run();
}

static void
SwapHeader (NoffHeader * noffH)
{
	noffH->noffMagic = WordToHost (noffH->noffMagic);
	noffH->code.size = WordToHost (noffH->code.size);
	noffH->code.virtualAddr = WordToHost (noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost (noffH->code.inFileAddr);
	noffH->initData.size = WordToHost (noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost (noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost (noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost (noffH->uninitData.size);
	noffH->uninitData.virtualAddr =
		WordToHost (noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost (noffH->uninitData.inFileAddr);
}
bool roomleft(OpenFile * executable){
	NoffHeader noffH;
	unsigned int  size;
	
	executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
			(WordToHost (noffH.noffMagic) == NOFFMAGIC))
		SwapHeader (&noffH);
	ASSERT (noffH.noffMagic == NOFFMAGIC);
	
	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
	// to leave room for the stack
	unsigned int numPages = divRoundUp (size, PageSize);
	size = numPages * PageSize;
	if (numPages>=(unsigned int)frameProvider->NumAvailFrame()) return false;
	else return true;
}



int ProcessTable::createProcess(char * filename){
	S->P();
	OpenFile *executable = fileSystem->Open(filename);
	if (!roomleft(executable)) {
		S->V();
		return NEW_PROCESS_FAIL;
	}
	Thread *t = new Thread(filename);
	// create new AddrSpace on filename
	
	if (executable == NULL){
		printf("Unable to open file %s\n", filename);
		return -1;
	}
		AddrSpace *newSpace = new AddrSpace(executable);
	
	DEBUG('x',"ceci est mon bool : %d",newSpace->validPage);
	if (newSpace->validPage==false){ //we check the validity of the process
			
			delete newSpace;
			DEBUG('x',"CECI EST UN TEST DE DEBUG\n");
			t->Finish();
			
			delete executable;
			S->V();
			return NEW_PROCESS_FAIL;
	}
	delete executable; // close file
	t->space = newSpace;
	// incr counter

	S->V();
	incrProcess();
	t->Fork(startUserProc, -1);
	pidCounter++;
	t->space->SetPid(pidCounter);
	NewJoinableProcess(pidCounter);
	return pidCounter;
}

void ProcessTable::deleteProcess(AddrSpace *space){
	delete space;
	decrProcess();
}

void ProcessTable::incrProcess(){
	S->P();
	countProcess += 1;
	S->V();
}

void ProcessTable::decrProcess(){
	S->P();
	countProcess -= 1 ;	
	S->V();
}

bool ProcessTable::noMoreProc(){
	bool res;
	S->P();
	res = countProcess;
	S->V();
	return res == 0;
}

int ProcessTable::NewJoinableProcess(int newPid){
	joinable_process_t *newProcess= new joinable_process_t;
	newProcess->exitvalue=-1;
	newProcess->exitvalueadd=-1;
	newProcess->joinablepid=newPid;
	newProcess->next=NULL;
	newProcess->waitingThread=NULL;
	newProcess->zombieState=false;
	
	if(first==NULL)
		first=newProcess;
	else{
		joinable_process_t *lastProcess=first;

		while (lastProcess->next!=NULL)
		{
			lastProcess=lastProcess->next;
		}
		lastProcess->next=newProcess;
		
	}
	return 0;
}

int ProcessTable::do_WaitProcessExit(unsigned int pidtowait,int returnvalueaddr){
		if (first==NULL)
			return INVALID_PID;
		joinable_process_t * waitingstructure= first;
		joinable_process_t * previouswaitingstructure=NULL;
		while(waitingstructure->next!=NULL){
			previouswaitingstructure=waitingstructure;
			waitingstructure=waitingstructure->next;
			if (waitingstructure->joinablepid==pidtowait) break;
		}
		if (waitingstructure->joinablepid!=pidtowait)
			return INVALID_PID;
		if (waitingstructure->zombieState){
			if(returnvalueaddr>0)
				machine->WriteMem(returnvalueaddr,4,waitingstructure->exitvalue);
			if(previouswaitingstructure==NULL)
				first=waitingstructure->next;
			else{
				previouswaitingstructure->next=waitingstructure->next;
			}
			delete waitingstructure;
			return JOIN_SUCCESFULL;

		}
		if (waitingstructure->waitingThread!=NULL)
			return PID_ALREADY_JOINED;
		else{
			waitingstructure->waitingThread=currentThread;
			waitingstructure->exitvalueadd=returnvalueaddr;
			IntStatus oldLevel = interrupt->SetLevel(IntOff);
			currentThread->Sleep();
			(void)interrupt->SetLevel(oldLevel);
			return JOIN_SUCCESFULL;
    
		}
		return INVALID_PID;
}


void ProcessTable::do_JoinableProcessExit(int n_exitvalue){
	joinable_process_t * waitingstructure= first;
	joinable_process_t * previouswaitingstructure=NULL;
	while(waitingstructure->next!=NULL ){
		previouswaitingstructure=waitingstructure;
		waitingstructure=waitingstructure->next;
		if (waitingstructure->joinablepid==currentThread->space->GetPid())
			break;
		}
	if(waitingstructure->waitingThread!=NULL){

		if(waitingstructure->exitvalueadd>0)
			machine->WriteMem(waitingstructure->exitvalueadd,4,n_exitvalue);
		IntStatus oldLevel = interrupt->SetLevel(IntOff);
		scheduler->ReadyToRun(waitingstructure->waitingThread);
		waitingstructure->waitingThread=NULL;
		if(previouswaitingstructure==NULL)
				first=waitingstructure->next;
			else{
				previouswaitingstructure->next=waitingstructure->next;
			}
		delete waitingstructure;
		(void)interrupt->SetLevel(oldLevel);
		return;
		}
	waitingstructure->exitvalue=n_exitvalue;
	waitingstructure->zombieState=true;
}
