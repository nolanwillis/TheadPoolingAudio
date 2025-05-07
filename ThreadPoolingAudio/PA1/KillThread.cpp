//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#include <conio.h>

#include "KillThread.h"

KillThread::KeyPress KillThread::keyPress;

KillThread::KillThread(const char *const pName)
	: 
	KillableBase(pName),
	mThread()
{
	// ThreadCount
}

void KillThread::Kill()
{
	keyPress.cv.notify_one();
}

void KillThread::KeyPressEvent()
{
	_getch();
	this->keyPress.cv.notify_one();
}

void KillThread::WaitForKeyPress()
{
	std::unique_lock<std::mutex> lock_key(this->keyPress.mtx);
	this->keyPress.cv.wait(lock_key);

	Debug::out("key pressed <----  \n");
}

void KillThread::SignalThreadsToDie()
{
	// kill them now
	KillableBase::posKillShare->prom.set_value();
}

void KillThread::operator()()
{
	START_BANNER;

	this->WaitForKeyPress();

	this->SignalThreadsToDie();

	TC.WaitUntilThreadDone();
}

KillThread::~KillThread()
{
	if(this->mThread.joinable())
	{
		this->mThread.join();
		Debug::out("~KillThread: joined\n");
	}
}

void KillThread::Launch()
{
	if(this->mThread.joinable() == false)
	{
		this->mThread = std::thread(std::ref(*this));
	}
	else
	{
		assert(false);
	}
}

// --- End of File ---
