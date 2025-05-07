#ifndef WAVE_THREAD_H
#define WAVE_THREAD_H

#include "KillableBase.h"
#include "Buffer.h"

class PlayThread;

class WaveThread : public KillableBase
{
public:
	static const unsigned int BUFFER_SIZE = 2 * 1024;
	static unsigned int ID;

	enum class Status
	{
		Free,
		RequestingFill,
		CanPlay,
		Playing,
		Finished
	};

	// Data that must be mutex protected.
	std::mutex mtx;
	Status status;
	
	Buffer poDataBuffer;
	std::condition_variable canPlayCV;
	unsigned int id;

	WaveThread();
	WaveThread(const WaveThread& rhs) = delete;
	WaveThread& operator = (const WaveThread& rhs) = delete;
	~WaveThread();

	void Launch();
	void operator()();

private:
	std::thread mThread;
	// Descriptor for an audio buffer.
	WAVEHDR waveHeader;
};

#endif
