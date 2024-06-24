// addrspace.cc 
//      Routines to manage address spaces (executing user programs).
//
//      In order to run a user program, you must:
//
//      1. link with the -N -T 0 option 
//      2. run coff2noff to convert the object file to Nachos format
//              (Nachos object code format is essentially just a simpler
//              version of the UNIX executable object code format)
//      3. load the NOFF file into the Nachos file system
//              (if you haven't implemented the file system yet, you
//              don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.
#include <stdexcept>
#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

#include <strings.h>		/* for bzero */

//----------------------------------------------------------------------
// SwapHeader
//      Do little endian to big endian conversion on the bytes in the 
//      object file header, in case the file was generated on a little
//      endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

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

//----------------------------------------------------------------------
// ReadAtVirtual
// 	read the disk at position and write in Memspace
//
// 	at the moment, pageTable and numPages are unused
//----------------------------------------------------------------------
static void ReadAtVirtual(OpenFile *executable, int virtualaddr, int numBytes, int position, TranslationEntry *pageTable, unsigned numPages) {
	char buff[numBytes];

	// save pageTable in machine
	TranslationEntry *oldPageTable;
	unsigned oldNumPages;
	oldPageTable = machine->pageTable;
	oldNumPages = machine->pageTableSize;

	machine->pageTable = pageTable;
	machine->pageTableSize = numPages;

	// read in disk
	executable->ReadAt(buff, numBytes, position);
	// write in MemSpace
	int written = 0;
	int i = 0;
	while (written < numBytes){
		machine->WriteMem(virtualaddr + i, 1, (int) buff[written]);
		written ++;
		i++;
	}
	
	// restore pageTable in machine
	machine->pageTable = oldPageTable;
	machine->pageTableSize = oldNumPages;
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
//      Create an address space to run a user program.
//      Load the program from a file "executable", and set everything
//      up so that we can start executing user instructions.
//
//      Assumes that the object code file is in NOFF format.
//
//      First, set up the translation from program memory to physical 
//      memory.  For now, this is really simple (1:1), since we are
//      only uniprogramming, and we have a single unsegmented page table
//
//      "executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace (OpenFile * executable) {
	NoffHeader noffH;
	unsigned int i, size;
	validPage=TRUE;
	//reading the file header
	executable->ReadAt ((char *) &noffH, sizeof (noffH), 0);
	if ((noffH.noffMagic != NOFFMAGIC) &&
			(WordToHost (noffH.noffMagic) == NOFFMAGIC))
		SwapHeader (&noffH);
	ASSERT (noffH.noffMagic == NOFFMAGIC);
	
	// how big is address space?
	size = noffH.code.size + noffH.initData.size + noffH.uninitData.size + UserStackSize;	// we need to increase the size
	// to leave room for the stack
	numPages = divRoundUp (size, PageSize);
	size = numPages * PageSize;

	// TODO
	// changer ça pour un truc plus fin
	ASSERT (numPages <= NumPhysPages);	// check we're not trying
	// to run anything too big --
	// at least until we have
	// virtual memory

	DEBUG ('a', "Initializing address space, num pages %d, size %d\n",
			numPages, size);
	
	// first, set up the translation 
	pageTable = new TranslationEntry[numPages];
	for (i = 0; i < numPages; i++)
	{
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		int z = frameProvider->GetEmptyFrame();
		if (z<0) {
			validPage=false;
			DEBUG('x',"CECI EST MON SECOND MESSAGE DE DEBUG");
			
			} // we set our flag to false if one of our physical page is invalid
		pageTable[i].physicalPage=z;
		pageTable[i].valid = TRUE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;	// if the code segment was entirely on 
		// a separate page, we could set its 
		// pages to be read-only
	}
	// then, copy in the code and data segments into memory
	if (noffH.code.size > 0 && validPage) {
		DEBUG ('a', "Initializing code segment, at 0x%x, size %d\n",
				noffH.code.virtualAddr, noffH.code.size);
		ReadAtVirtual(executable, noffH.code.virtualAddr,
				noffH.code.size, noffH.code.inFileAddr,
				pageTable, numPages);
		//	  executable->ReadAt (&(machine->mainMemory[noffH.code.virtualAddr]),
		//			      noffH.code.size, noffH.code.inFileAddr);
	} if (noffH.initData.size > 0 && validPage) {
		DEBUG ('a', "Initializing data segment, at 0x%x, size %d\n",
				noffH.initData.virtualAddr, noffH.initData.size);
		ReadAtVirtual(executable, noffH.initData.virtualAddr,
				noffH.initData.size, noffH.initData.inFileAddr,
				pageTable, numPages);
		//	  executable->ReadAt (&
		//			      (machine->mainMemory
		//			       [noffH.initData.virtualAddr]),
		//			      noffH.initData.size, noffH.initData.inFileAddr);
	}

	// thread management 
	lastTidGiven=0; //initialize thread counter at 0 since main thread does not count
	// one thread is already associated since the address space is created
	nbThreads =1;

	// Init of the Bitmap stacks list
	int nbStacks = UserStackSize / (NbPagePerThread * PageSize);
	stacks = new BitMap(nbStacks);
	stacks->Mark(0); // We mark first bit because this is the main thread stack
    //init the list of joinable
    firstJoinable=new joinable_t;
    firstJoinable->tid=0;
    firstJoinable->returnAddr=new List();
    firstJoinable->threadsToStart=new List();
    firstJoinable->zombie=false;
	firstJoinable->next=NULL;    
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
//      Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace ()
{
	// delete pageTable;
	for (unsigned i = 0; i < numPages; i++){
		if(pageTable[i].physicalPage>=0)
			frameProvider->ReleaseFrame(pageTable[i].physicalPage);
	}
	delete [] pageTable;
	// delete Joinable list
	joinable_t *toDel = firstJoinable;
	joinable_t *next = firstJoinable;
	while(toDel != NULL) {
		next = toDel->next;
		delete toDel->returnAddr;
		delete toDel->threadsToStart;
		delete toDel;
		toDel = next;

	}
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
//      Set the initial values for the user-level register set.
//
//      We write these directly into the "machine" registers, so
//      that we can immediately jump to user code.  Note that these
//      will be saved/restored into the currentThread->userRegisters
//      when this thread is context switched out.
//----------------------------------------------------------------------

	void
AddrSpace::InitRegisters ()
{
	int i;

	for (i = 0; i < NumTotalRegs; i++)
		machine->WriteRegister (i, 0);

	// Initial program counter -- must be location of "Start"
	machine->WriteRegister (PCReg, 0);

	// Need to also tell MIPS where next instruction is, because
	// of branch delay possibility
	machine->WriteRegister (NextPCReg, 4);

	// Set the stack register to the end of the address space, where we
	// allocated the stack; but subtract off a bit, to make sure we don't
	// accidentally reference off the end!
	machine->WriteRegister (StackReg, numPages * PageSize - 16);
	DEBUG ('a', "Initializing stack register to %d\n",
			numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
//      On a context switch, save any machine state, specific
//      to this address space, that needs saving.
//
//      For now, nothing!
//----------------------------------------------------------------------

	void
AddrSpace::SaveState ()
{
	pageTable = machine->pageTable;
	numPages = machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
//      On a context switch, restore the machine state so that
//      this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

	void
AddrSpace::RestoreState ()
{
	machine->pageTable = pageTable;
	machine->pageTableSize = numPages;
}

void AddrSpace::AllocateNewStack(int index) {
	machine->WriteRegister(StackReg, (numPages - NbPagePerThread * index) * PageSize - 16);
}

void AddrSpace::DeAllocateStack(int index) {
	stacks->Clear(index);
}

int AddrSpace::ReserveStack(){
	return stacks->Find();
}

void AddrSpace::AddSleepingThreads() {
	sleepingThreadsList->Append(currentThread);
}

void AddrSpace::FinishAllSleepingThreads() {
	Thread *t;
	while (!sleepingThreadsList->IsEmpty()){
		t = (Thread *) sleepingThreadsList->Remove();
		t->Finish();
	}
}

void AddrSpace::RequestHalt() {
	haltRequested = true;
}

bool AddrSpace::IsLastThread() {
	return nbThreads == 0;
}

bool AddrSpace::IsHaltRequested() {
	return haltRequested;
}

void AddrSpace::IncrNbThreads() {
	nbThreads++;
}

void AddrSpace::DecrNbThreads() {
	nbThreads--;
}

int AddrSpace::GetNextTid(){
    return lastTidGiven++;
}

int AddrSpace::do_Wait(int tidtowait,int returnvalueAddr){
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    joinable_t* waitingStruct=firstJoinable;
    bool exist=FALSE;
    while(waitingStruct->tid!=tidtowait && waitingStruct->next!=NULL){
        if(waitingStruct->tid==tidtowait){
            exist=TRUE;
            break;
        }
        else {
            waitingStruct=waitingStruct->next;
        }
    }
    if(waitingStruct->tid==tidtowait) exist=TRUE;
    if(!exist) return-1;
    int ret=currentThread->tid;
    waitingStruct->threadsToStart->Append(currentThread);
    waitingStruct->returnAddr->Append((void*)returnvalueAddr);
    if(waitingStruct->zombie){
		if (returnvalueAddr!=NO_RETURN_ADDRESS)
        machine->WriteMem(returnvalueAddr,1,waitingStruct->exitValue);
        return 0;
    } 
    currentThread->Sleep();
    (void)interrupt->SetLevel(oldLevel);
    return ret;
}

void AddrSpace::JoinableExit(int exitVal){
		
        int id=currentThread->tid;
        if(id==0) return;
		joinable_t* toWakeUp=firstJoinable;
        
        while (toWakeUp->tid!=id&&toWakeUp->next!=NULL)
        {
            
            toWakeUp=toWakeUp->next;
        }
        if (toWakeUp==NULL) return;
        toWakeUp->zombie=true;
        toWakeUp->exitValue=exitVal;
		if(toWakeUp->returnAddr!=NULL)
		{	
        while(!toWakeUp->returnAddr->IsEmpty()){
            int addr=(int)toWakeUp->returnAddr->Remove();
			if (addr!=NO_RETURN_ADDRESS)
            machine->WriteMem(addr,4,exitVal);
        }
		}
        if(toWakeUp->threadsToStart!=NULL)
		{	
        while(!toWakeUp->threadsToStart->IsEmpty()){
             IntStatus oldLevel = interrupt->SetLevel(IntOff);
             Thread *thread = (Thread *)toWakeUp->threadsToStart->Remove();
             if (thread != NULL) // make thread ready, grabing the lock immediately
                scheduler->ReadyToRun(thread);
            (void)interrupt->SetLevel(oldLevel);
        }
		}     
        
}
void AddrSpace::threadFinish(int returnValue){
	DecrNbThreads();
	JoinableExit(returnValue);
}

void AddrSpace::newJoinableThread(int newTid){
	if (newTid==0) return;
    joinable_t* last=firstJoinable ;

	if (currentThread->space==NULL) return;
    while (last->next!=NULL)
    {
        last=last->next;
    }
    if(last==NULL){
    firstJoinable=new joinable_t;
    firstJoinable->tid=0;
    firstJoinable->returnAddr=new List();
    firstJoinable->threadsToStart=new List();
    firstJoinable->zombie=false;       
    }else{
        
    last->next=new joinable_t;
    last->next->tid=newTid;
    last->next->returnAddr=new List();
    last->next->threadsToStart=new List();   
    last->next->next=NULL;
    last->next->zombie=false;
    }  
}

void AddrSpace::ExitAllThreads(){
	return;
}

unsigned int AddrSpace::GetPid(){
	return this->pid;
}


void AddrSpace::SetPid(unsigned int pidtoset){
	this->pid=pidtoset;
}

bool AddrSpace::CheckPagesValidity(){
	return validPage;
}
