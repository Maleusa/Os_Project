#include "frameprovider.h"
#include "system.h"

FrameProvider::FrameProvider(){
	frames = new BitMap(NumPhysPages);
	S = new Semaphore("frameProviderSem", 1);
}

FrameProvider::~FrameProvider(){
	delete frames;
}

int FrameProvider::GetEmptyFrame(){
	S->P();
	int frame = frames->Find();
	if (frame<0) return NO_MORE_VALID_PAGES;
	DEBUG ('p', "page %i given\n", frame);
	bzero(machine->mainMemory + frame*PageSize, PageSize);
	S->V();
	return frame;	
}

void FrameProvider::ReleaseFrame(int index){
	S->P();
	frames->Clear(index);
	DEBUG ('p', "page index %i released\n", index);
	S->V();
}

int FrameProvider::NumAvailFrame(){
	int res;
	S->P();
	res = frames->NumClear();
	S->V();
	return res;
}
