//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#ifndef KILL_THREAD_H
#define KILL_THREAD_H

#include "KillableBase.h"

class Command;

// Macros to clean up code even more
class KillThread : public KillableBase
{
private:
	struct  KeyPress
	{
		std::mutex				 mtx;
		std::condition_variable  cv;
	};

public:

	KillThread(const char *const pName);

	KillThread() = delete;
	KillThread(const KillThread &) = delete;
	KillThread &operator = (const KillThread &) = delete;
	~KillThread();

	static void Kill();

	void KeyPressEvent();
	void WaitForKeyPress();
	void SignalThreadsToDie();
	void Launch();

	void operator()();

	// data
	std::thread mThread;
	static KeyPress keyPress;
};

#endif

// --- End of File ---
