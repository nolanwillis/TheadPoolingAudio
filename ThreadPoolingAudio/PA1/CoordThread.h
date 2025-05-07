//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#ifndef COORD_THREAD_H
#define COORD_THREAD_H

#include "KillableBase.h"
#include "CircularData.h"
#include "Buffer.h"
#include "FileThread.h"

class WaveThread;

class CoordThread : public KillableBase
{
public:
	static const unsigned int BUFFER_SIZE = 512 * 1024;
	
	enum class ThreadStatus
	{
		Ready,
		Not_Ready
	};

	CoordThread() = delete;
	CoordThread(const char *const pName);
	CoordThread(const CoordThread &) = delete;
	CoordThread &operator = (const CoordThread &) = delete;
	~CoordThread();

	void Launch();
	void operator()();

	static size_t TransferToWaveThread(WaveThread* pWaveThread);
	static void SendCommand(Command* pCmd);

private:
	static CoordThread* psCoordThread;
	
	enum class DrainStatus
	{
		BufferA,
		BufferB
	};

	std::thread mThread;
	Buffer pDataBufferA;
	Buffer pDataBufferB;
	Buffer* pDrainBuffer;
	DrainStatus drainStatus;
	CircularData inQueue;
	ThreadStatus threadStatus;
	
	static CoordThread& GetInstance();
};

#endif

// --- End of File ---
