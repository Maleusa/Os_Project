// exception.cc 
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.  
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "userthread.h"

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
	static void
UpdatePC ()
{
	int pc = machine->ReadRegister (PCReg);
	machine->WriteRegister (PrevPCReg, pc);
	pc = machine->ReadRegister (NextPCReg);
	machine->WriteRegister (PCReg, pc);
	pc += 4;
	machine->WriteRegister (NextPCReg, pc);
}
// convertie un entier en chaine de charactères
// le représentant en base mod
// gérer la taille du tampon est laissé à l'utilisateur
void itoa(int val, char *s, int mod){
	int i = 0, j = 0, div = 1;
	char tmp;
	if (val < 0){
		s[i++] = '-';
		val = -val;
		j = 1;
	}
	do {
		s[i++] = '0' + ((val/div) %mod);
		div *= mod;
	} while(div <= val);
	s[i] = '\0';
	// reverse s
	i--;
	while(j < i){
		tmp = s[j];
		s[j] = s[i];
		s[i] = tmp;
		j++; i--;
	}
}

//----------------------------------------------------------------------
// do_JoinableExit
//      TODO
//----------------------------------------------------------------------
void do_JoinableExit(int arg){
	currentThread->DeleteStack(); // Free stack
	currentThread->space->DecrNbThreads();
	currentThread->space->JoinableExit(arg);
}

//----------------------------------------------------------------------
// do_Halt
//      System call used to halt the machine. If the current thread is
//      the last using the address space then halt the machine else
//      request a halt for later thread and finish.
//----------------------------------------------------------------------
void do_Halt(){
	if (currentThread->space->IsLastThread()){
		interrupt->Halt();
	}
	else {
		IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
		currentThread->space->RequestHalt();
		currentThread->Finish();
		(void)interrupt->SetLevel(oldLevel); // re-enable interrupts
	}
}

//----------------------------------------------------------------------
// do_Exit
//      System call used by threads to end the execution. If the current
//      thread is the last using the address space then finish and halt
//      if a halt is needed (requested by another thread)
//----------------------------------------------------------------------
void do_Exit(){
	if (currentThread->space->IsLastThread()){
		if (currentThread->space->IsHaltRequested()) interrupt->Halt();
		else currentThread->Finish();
	}
	else {
		IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts
		currentThread->Finish();
		(void)interrupt->SetLevel(oldLevel); // re-enable interrupts
	}
}

//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions 
//      are in machine.h.
//----------------------------------------------------------------------



void ExceptionHandler (ExceptionType which) {
	int type = machine->ReadRegister (2);
	int arg1 = machine->ReadRegister (4);
	int arg2 = machine->ReadRegister (5);
	int returnvalue = 0;

	// used for some exceptions
	char str[MAX_STRING_SIZE];

	if (which == SyscallException){
		switch(type){
			//EXIT SysCall : Put back into r2 the exit return value
			case SC_Exit:
				DEBUG ('e', "Exit, initiated by user program.");
				// notify the addresseSpace
				currentThread->space->threadFinish(arg1);
				// delete stack
				currentThread->DeleteStack();
				// check if last to Exit
				if (currentThread->space->IsLastThread()){

					processTable->do_JoinableProcessExit(arg1);
					// Exit processus
					processTable->deleteProcess(currentThread->space);
					// Halt is last process ended
					if (processTable->noMoreProc()) {
						interrupt->Halt();
					} 
				}
				// if not halted sleep
				currentThread->Finish();
    				// not reached
				//returnvalue = arg1;
//				do_JoinableExit(arg1);	
//				do_Exit();
				break;
				//Halt Syscall : Shutdown cleanly a user program
			case SC_Halt:
				DEBUG ('e', "Shutdown, initiated by user program.\n");
				do_JoinableExit(arg1);	
				do_Halt();
				break;
			case SC_Yield:
				currentThread->Yield();
				break;
				//PutChar Syscall : Put a character that is located in r4 in the current
				// out stream be it a file or stdout
			case SC_PutChar:
				DEBUG ('e', "PutChar, initiated by user program.\n");
				synchconsole->SynchPutChar((char) arg1);
				break;
				//SynchPutString Syscall : Put the string located in the adress pointed by r4
				// int the current out stream be it a file or stdout 
			case SC_SynchPutString:
				DEBUG ('e', "SynchPutString, initiated by user program. \n");
				synchconsole->copyStringFromMachine(arg1,str,MAX_STRING_SIZE);
				synchconsole->SynchPutString(str);
				break;
				//SynchGetChar Syscall : get a char from the current in stream
				// and put it in the return register r2	
			case SC_SynchGetChar:
				DEBUG ('e', "SynchGetChar, initiated by user program. \n");
				returnvalue = (int) synchconsole->SynchGetChar();
				break;
				//SynchGetString Syscall : get a string from the current in stream
				// and put it in the correct address space in the simulation(starting at the adresse pointed in r4)
				// TODO : add maybe a return value of the size of the string added (minus '\0'?)
			case SC_SynchGetString:
				DEBUG ('e', "SynchGetString, initiated by user program. \n");
				synchconsole->SynchGetString(str,arg2);
				synchconsole->copyStringToMachine(arg1, str, arg2);
				break;
				// SynchPutInt Syscall : get an int from the register r4, converts it to a string
				// and put it in the current out stream be it a file or stdout
			case SC_SynchPutInt:
				DEBUG ('e', "SynchPutInt, initiated by user program. \n");
				itoa(arg1, str, 10);
				synchconsole->SynchPutString(str);
				break;
			case SC_UserThreadCreate:
				DEBUG ('e', "UserthreadCreate, initiated by user program. \n");
				returnvalue = do_UserThreadCreate(arg1, arg2);
				break;
			case SC_UserThreadExit:
				DEBUG ('e', "UserThreadExit, initiated by user program. \n");
				// notify the addresseSpace
				currentThread->space->threadFinish(arg1);
				// delete stack
				currentThread->DeleteStack();
				// check if last to Exit
				if (currentThread->space->IsLastThread()){
					processTable->do_JoinableProcessExit(arg1);
					// Exit processus
					processTable->deleteProcess(currentThread->space);
					// Halt is last process ended
					if (processTable->noMoreProc()) {
						interrupt->Halt();
					}
				}
				// if not halted sleep
				currentThread->Finish();
    				// not reached
				//do_JoinableExit(arg1);			
				//do_Exit();
				break;
			case SC_UserThreadJoin:
				DEBUG ('e', "UserThreadJoin, initiated by user program. \n");
				returnvalue=currentThread->space->do_Wait(arg1,arg2);				
				break;
			case SC_ForkExec:
				DEBUG ('e', "ForkExec, initiated by user program. \n");
				synchconsole->copyStringFromMachine(arg1,str,MAX_STRING_SIZE);
				returnvalue = processTable->createProcess(str);
				processTable->NewJoinableProcess(returnvalue);
				break;
			case SC_SynchPutConsole:
				DEBUG ('e', "SynchPutConsole, initiated by user program. \n");
				synchconsole->copyStringFromMachine(arg1,str,MAX_STRING_SIZE);
				synchconsole->SynchPutConsole(str);
				break;
			case SC_UserProcJoin:
				DEBUG ('e', "Join, initiated by user program. \n");
				returnvalue=processTable->do_WaitProcessExit(arg1,arg2);
				break;

				// this is the default syscall atm make the machine go down in flames 
				// we need to change this !
			default:
				printf ("Unexpected user mode exception %d %d\n", which, type);
				ASSERT (FALSE);
		}
		UpdatePC ();
		machine->WriteRegister (2, returnvalue); // write of the value of returnvalue in reg2 
	}
}
