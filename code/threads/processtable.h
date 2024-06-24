#ifndef PROCESSTABLE_H

#define PROCESSTABLE_H
#include "synch.h"
#include "thread.h"

#define PID_ALREADY_JOINED -2
#define INVALID_PID -1
#define JOIN_SUCCESFULL 0
#define NEW_PROCESS_FAIL -1


typedef struct joinable_process_s{
	unsigned int joinablepid;
	int exitvalue;
	int exitvalueadd;
	bool zombieState;
	Thread * waitingThread;
	joinable_process_s * next;
}joinable_process_t;

class ProcessTable{
	private:
		unsigned int countProcess;
		Semaphore *S;
		unsigned int pidCounter;
		joinable_process_t * first;
	public:
		ProcessTable();
		~ProcessTable();
		int createProcess(char * filename);
		void deleteProcess(AddrSpace *space);
		// number of process
		void incrProcess();
		void decrProcess();
		bool noMoreProc();
		void decrProcess(int i);
		int NewJoinableProcess(int newPid);
		int do_WaitProcessExit(unsigned int pidtowait,int returnvalueaddr);
		void do_JoinableProcessExit(int exitvalue);
};
#endif
