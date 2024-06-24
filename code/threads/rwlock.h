// rwlock.h
//      Data structure for a reader writer lock
//      Use a lock and a semaphore from synch.h
//      While a writer is in the critical section
//      every other threads (writers and readers) are blocked.
//      While a reader is in the critical section
//      only the readers can access the section. indeed, the 
//      writers are blocked.
// Copyright (c) 2023 The rickety duo.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// rwlock.h -- synchronization primitives.

#ifndef RWLOCK_H
#define RWLOCK_H
#include "synch.h"

class RWLock
{
    public:
        RWLock (const char *debugName); 
        ~RWLock ();		// de-allocate RWLock
        const char *getName ()
        {
        return name;
        }				// debugging assist

        void wLock ();			// these are the only operations on a rwlock
        void rLock ();			// they are all *atomic*
        void wUnlock ();        // wLock gonna lock every access and rLock gonna
        void rUnlock ();        // lock only for writers

    private:
        const char *name;		// useful for debugging

        int nbReaders;          // the implementation choice gonna be
        Condition * writer;     // on the .cc file wich gonna define :
                                // readers first, writers first, etc.
        Lock * lock;		
};
#endif // RWLOCK_H