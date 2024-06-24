#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"
#include "rwlock.h"

static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
readAvail = new Semaphore("read avail", 0);
writeDone = new Semaphore("write done", 0);
console = new Console(readFile,writeFile,ReadAvail,WriteDone,0);
readWriteLock = new RWLock("readerWriterLock");
}
SynchConsole::~SynchConsole()
{
delete console;
delete writeDone;
delete readAvail;
}
//----------------------------------------------------------------------
// SynchConsole::SynchPutChar()
// 	Write a character to the simulated display, using This::Console 
//	Takes a token once the writing is done
//  We lock the multi-threading for the putchar.
//----------------------------------------------------------------------
void SynchConsole::SynchPutChar(const char ch)
{
    readWriteLock->wLock();
    console->PutChar(ch);
    writeDone->P();
    readWriteLock->wUnlock();
}

//----------------------------------------------------------------------
// SynchConsole::SynchGetChar()
// 	Read a character from the input buffer, if there is an available character,wait on it otherwise.
//	Either return the character, or EOF if none buffered.
//  We lock the multi-threading for the getchar.
//----------------------------------------------------------------------
char SynchConsole::SynchGetChar()
{
    readWriteLock->rLock();
    readAvail->P();
    char ch = console->GetChar();
    readWriteLock->rUnlock();
    return ch;
}

//----------------------------------------------------------------------
// SynchConsole::SynchPutString()
// Write a String to the simulated display or file given as an 
// *out uses SynchConsole::SynchPutChar()
// The SynchPutString method is'nt threads safe, caracters
// can be mixed during the process.
//----------------------------------------------------------------------

void SynchConsole::SynchPutString(const char *s)
{
    int i=0;
    while(s[i]!='\0'){
        this->SynchPutChar(s[i]);
        i++;
    }

}

//----------------------------------------------------------------------
//SynchConsole::SynchPutConsole(const char *s)
//Write a string s to the simulated display or file given as an
//*out 
//Work the same way as SynchPutString for now  but is threads safe
//(We will add the put int possibility and the path name handler later)
//----------------------------------------------------------------------

void SynchConsole::SynchPutConsole(const char *s)
{
	readWriteLock->wLock();
	int i=0;
	while(s[i]!='\0'){
		console->PutChar(s[i]);
		writeDone->P();
		i++;
	}
	readWriteLock->wUnlock();
}

//----------------------------------------------------------------------
// SynchConsole::SynchGetString()
// 	Read a string from the input buffer, uses SynchConsole::SynchGetChar(),
//  ensure that the string is ending by '\0' if the user inputs an EOF or a 
//  '\0' the string will be ended at this point
//  We lock the multi-threading for the getstring, this way the user
//  is sure from the beginning to the end of his reading that the
//  string is'nt modified.
//----------------------------------------------------------------------
void SynchConsole::SynchGetString(char *s, int n)
{
    readWriteLock->rLock();
    int i=0;
    for (;i<n;i++){
        s[i]=this->SynchGetChar();
        if (s[i]==EOF || s[i]=='\0')
            break;
    }
    s[i]='\0';
    readWriteLock->rUnlock();

}

//----------------------------------------------------------------------
// SynchConsole::copyStringFromMachine()
// 	get string strating at from in MachineMemory of length < size (length exclude '\0') 
// 	and copy it at to with a '\0' as last charactere guaranteed
//  
//----------------------------------------------------------------------
void SynchConsole::copyStringFromMachine(int from, char *to, unsigned size) {
    int val;
    unsigned i =0;
    while (i<size - 1){
       machine->ReadMem(from+i,1,&val);
       if (val=='\0')
                break;
        to[i]=(char) val;
        i++;
    }
    to[i]='\0';

}


//----------------------------------------------------------------------
// SynchConsole::copyStringToMachine()
// 	get string strating at *from  of length < size (length exclude '\0') 
// 	and copy it at to in the machine adressspace with a '\0' as last charactere guaranteed
//  
//----------------------------------------------------------------------
void SynchConsole::copyStringToMachine(int to, char *from, unsigned size) {
    
    unsigned i =0;
    while (i<size - 1){
       machine->WriteMem(to+i,1,(int)from[i]);
       if (from[i]=='\0')
                break;
        
        i++;
    }
    machine->WriteMem(to+i,1,0);
    

}
