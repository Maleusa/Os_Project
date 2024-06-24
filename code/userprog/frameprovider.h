#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include "bitmap.h"
#include "synch.h"
#define NO_MORE_VALID_PAGES -1

class FrameProvider{
	private:
		BitMap *frames;
		Semaphore *S;
	public:
		FrameProvider();
		~FrameProvider();
		int GetEmptyFrame();
		void ReleaseFrame(int index);
		int NumAvailFrame();



};

#endif
