// addrspace.h 
//      Data structures to keep track of executing user programs 
//      (address spaces).
//
//      For now, we don't keep any information about address spaces.
//      The user level CPU state is saved and restored in the thread
//      executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "translate.h"
#include "list.h"
#include  "bitmap.h"

#define UserStackSize		4608	// increase this as necessary!
#define NbPagePerThread     3       // Size of the stack of a thread

#define NO_RETURN_ADDRESS -2
typedef struct joinable_s{
	int tid;
	int exitValue;
	List * threadsToStart;
	List * returnAddr;
	bool zombie;
	joinable_s* next;
}joinable_t;

class AddrSpace
{
	public:
		AddrSpace (OpenFile * executable);	// Create an address space,
		// initializing it with the program
		// stored in the file "executable"
		~AddrSpace ();		// De-allocate an address space

		void InitRegisters ();	// Initialize user-level CPU registers,
		// before jumping to user code

		void SaveState ();		// Save/restore address space-specific
		void RestoreState ();	// info on a context switch
		int ReserveStack();     // reserve a stack position for a thread,
		// return -1 si there is no more space available
		void AllocateNewStack(int index); // allocate the stack number index
		// to the current thread
		void DeAllocateStack(int index); // deallocate the stack number index

		// only use this to finish a thread 
		void threadFinish(int returnValue);

		// Functions using Sleeping Threads List
		void AddSleepingThreads();
		void FinishAllSleepingThreads();
		void RequestHalt();
		bool IsHaltRequested();
		bool IsLastThread(); // Return true if there is only one thread remaining
		void IncrNbThreads();
		void DecrNbThreads();
		int GetNextTid();
		int do_Wait(int tidtowait,int returnvalueAddr);
		void newJoinableThread(int newTid);
		void JoinableExit(int exitVal);
		unsigned int GetPid();
		void SetPid(unsigned int pidtoset);
		bool CheckPagesValidity();
		bool validPage;
	private:
		
		unsigned int pid; // Unique id of a process PID
		List *sleepingThreadsList; // List of all threads that are currently sleeping waiting to be finished/halted
		joinable_t * firstJoinable;

		void ExitAllThreads();
		TranslationEntry * pageTable;	// Assume linear page table translation
		// for now!
		unsigned int numPages;	// Number of pages in the virtual 
		// address space

		bool haltRequested = false; // True iff a thread requested an halt

		int nbThreads; // Number of active threads associated to this adress space

		int lastTidGiven; //Number of thread created in this adresse space
		// Address space stack gestion
		BitMap *stacks; // A bitmap representing the state of the stacks
		// there is UserStackSize / (nbPagePerThread * PageSize) stacks available

};

#endif // ADDRSPACE_H
