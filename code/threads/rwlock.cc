// rwlock.cc 
//      An implementtaion of the reader/writer lock.
//      His goal is to keep the reading access while  
//      a thread is reading and to block every other      
//      access while a thread is writing.
// 
//
// Copyright (c) 2023 The rickety duo.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// rwlock.h -- synchronization primitives.

#include "rwlock.h"
#include "system.h"

//----------------------------------------------------------------------
// RWLock::RWLock
//      Initialize a reader/writer lock, so that it can be used for synchronization.
//
//      "debugName" is an arbitrary name, useful for debugging.
//----------------------------------------------------------------------

RWLock::
RWLock (const char *debugName)
{
    name = debugName;

    nbReaders = 0;
    lock = new Lock(debugName);
    writer = new Condition(debugName);
}

//----------------------------------------------------------------------
// RWLock::RWLock
//      De-allocate rwlock, when no longer needed.  Assume no one
//      is still waiting on the semaphore!
//----------------------------------------------------------------------

RWLock::~RWLock ()
{
    delete lock;
    delete writer;
}

//----------------------------------------------------------------------
// RWLock::wLock
//      Lock the critical section with a write lock :
//      No other thread can access to the locked section
//----------------------------------------------------------------------

void
RWLock::wLock ()
{
    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    lock->Acquire();
    while (nbReaders!=0) 
    {   //As long as there is readers in the section
        writer->Wait(lock); 
    }
    (void) interrupt->SetLevel (oldLevel);
}

//----------------------------------------------------------------------
// RWLock::rLock
//      Lock the critical section with a read lock :
//      Only readers can access to the locked section
//----------------------------------------------------------------------

void
RWLock::rLock ()
{
    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    lock->Acquire();
    nbReaders++;
    lock->Release(); //We release the lock, this way anyone can try to access the section
    (void) interrupt->SetLevel (oldLevel);
}



// The two following methods wUnlock and rUnlock will also
// define the priority and possible starvation because they
// are responsible for the order of signal/broadcast
//
// In the current implementation, starvation is possible because
// a writer which is signaled is put on wait again, on the lock.
// The starvation is managed by the implementation of the lock :
// A FIFO implementation.


//----------------------------------------------------------------------
// RWLock::wUnlock
//      Free the lock of the critical section while a write lock 
//      was on it :
//      Anyone can try to access it.
//----------------------------------------------------------------------

void
RWLock::wUnlock ()
{
    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    writer->Signal(lock); //We signal a potential waiting thread
    lock->Release();
    (void) interrupt->SetLevel (oldLevel);
}

//----------------------------------------------------------------------
// RWLock::rUnlock
//      Free the lock of the critical section while a read lock 
//      was on it :
//      Anyone can try to access it.
//----------------------------------------------------------------------

void
RWLock::rUnlock ()
{
    IntStatus oldLevel = interrupt->SetLevel (IntOff);
    lock->Acquire();
    nbReaders--;
//    if(nbReaders==0) //We signal only if we are the last reader for less system time's 
    writer->Signal(lock);
    lock->Release();
    (void) interrupt->SetLevel (oldLevel);
}
