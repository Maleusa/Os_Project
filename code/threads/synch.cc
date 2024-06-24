// synch.cc
//      Routines for synchronizing threads.  Three kinds of
//      synchronization routines are defined here: semaphores, locks
//      and condition variables (the implementation of the last two
//      are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      Initialize a semaphore, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//      "initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::
    Semaphore(const char *debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
//      De-allocate semaphore, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
//      Wait until semaphore value > 0, then decrement.  Checking the
//      value and decrementing must be done atomically, so we
//      need to disable interrupts before checking the value.
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------

void Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff); // disable interrupts

    while (value == 0)
    {                                         // semaphore not available
        queue->Append((void *)currentThread); // so go to sleep
        currentThread->Sleep();
    }
    value--; // semaphore available,
    // consume its value

    (void)interrupt->SetLevel(oldLevel); // re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
//      Increment semaphore value, waking up a waiter if necessary.
//      As with P(), this operation must be atomic, so we need to disable
//      interrupts.  Scheduler::ReadyToRun() assumes that threads
//      are disabled when it is called.
//----------------------------------------------------------------------

void Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL) // make thread ready, consuming the V immediately
        scheduler->ReadyToRun(thread);
    value++;
    (void)interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!.




// Not dumb anymore thanks to the rickety duo.
//----------------------------------------------------------------------
// Lock::Lock
//      Initialize a lock, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Lock::Lock(const char *debugName)
{
    name = debugName;
    lock = false;
    queue = new List; // Represents all the thread that are waiting on it
}


//----------------------------------------------------------------------
// Lock::Lock
//      De-allocate lock, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------
Lock::~Lock()
{
    delete queue;
}

//----------------------------------------------------------------------
// Lock::Acquire
//      Wait until the lock is available.
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------
void Lock::Acquire()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    while (lock != false)
    {                                         // lock not available
        queue->Append((void *)currentThread); // so go to sleep
        currentThread->Sleep();
    }
    ownerTID = currentThread->tid;     // we become the owner
    lock = true;
    (void)interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// Lock::Release
//      Free the lock, waking up a waiter if necessary.
//----------------------------------------------------------------------
void Lock::Release()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    thread = (Thread *)queue->Remove();
    if (thread != NULL) // make thread ready
        scheduler->ReadyToRun(thread);
    lock = false;
    ownerTID = -1;
    (void)interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// Lock::isHeldByCurrentThread
//      Check if the current thread held the lock. Non-atomic but
//      the test is only checking if the name of the owner is ours,
//      however, even if the owner's name changes, it can only be 
//      another one.
//----------------------------------------------------------------------
bool Lock::isHeldByCurrentThread()
{
    // return TRUE; 
    return currentThread->tid==ownerTID;
    // int held = strcmp(owner, currentThread->getName()); //Need to be replace with TID
    // return held == 0;
}







//----------------------------------------------------------------------
// Condition::Condition
//      Initialize a condition, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------
Condition::Condition(const char *debugName)
{
    name = debugName;
    queue = new List;
}


//----------------------------------------------------------------------
// Condition::Condition
//      De-allocate condition, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------
Condition::~Condition()
{
    delete queue;
}

//----------------------------------------------------------------------
// Condition::Wait
//      Release the lock and put the thread on the queue of the 
//      condition. When the thread is waking up, it gonna try to
//      acquire the lock again. 
//
//      Note that Thread::Sleep assumes that interrupts are disabled
//      when it is called.
//----------------------------------------------------------------------
void Condition::Wait(Lock *conditionLock)
{
    if(conditionLock->isHeldByCurrentThread()){
	IntStatus oldLevel = interrupt->SetLevel(IntOff);
	conditionLock->Release();
	queue->Append((void *)currentThread);
	currentThread->Sleep(); 
	conditionLock->Acquire();
	(void)interrupt->SetLevel(oldLevel);
    }
}

//----------------------------------------------------------------------
// Condition::Signal
//      Wake up one thread from the queue.
//
//----------------------------------------------------------------------
void Condition::Signal(Lock *conditionLock)
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (conditionLock->isHeldByCurrentThread())
    {
        thread = (Thread *)queue->Remove();
        if (thread != NULL){ // make thread ready, grabing the lock immediately
//            Thread *oldCurrent = currentThread;
//            currentThread = thread;
//            conditionLock->Acquire(); //The thread pushed out from the condition
//                                      //is now waiting on the lock
//                                      //Acquire make a sleep and change the currentThread
//            currentThread->Yield();

            scheduler->ReadyToRun(thread);
        }
    }
    (void)interrupt->SetLevel(oldLevel);
}


//----------------------------------------------------------------------
// Condition::Signal
//      Wake up all the threads from the queue.
//
//----------------------------------------------------------------------
void Condition::Broadcast(Lock *conditionLock)
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    if (conditionLock->isHeldByCurrentThread())
    {
        thread = (Thread *)queue->Remove();
        if (thread != NULL) // make thread ready, grabing the lock immediately
            while (thread != NULL)
            {
                scheduler->ReadyToRun(thread);
                thread = (Thread *)queue->Remove();
            }
    }
    (void)interrupt->SetLevel(oldLevel);
}
