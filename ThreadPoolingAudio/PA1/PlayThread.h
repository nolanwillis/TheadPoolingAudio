#ifndef PLAY_THREAD_H
#define PLAY_THREAD_H

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "KillableBase.h"
#include "CircularData.h"

class WaveThread;

class PlayThread : public KillableBase
{
public:
	static const unsigned int MAX_WAVE_THREADS = 20;

	PlayThread() = delete;
	PlayThread(const char* const pName);
	PlayThread(const PlayThread&) = delete;
	PlayThread& operator = (const PlayThread&) = delete;
	~PlayThread();

	void Launch();
	void operator()();

	static HWAVEOUT& GetWaveHandle();
	static void SendCommand(Command* pCmd);
	static void NotifyWaveThreadIsDone(WaveThread* pWaveThread);

private:
	static PlayThread* psPlayThread;
	
	std::thread mThread;
	// Handle for the waveform output device.
	HWAVEOUT hWaveOut;  
	// Descriptor for the format of the audio data.  
	WAVEFORMATEX wfx;  
	WaveThread* waveThreadTable;
	CircularData inQueue;

	static PlayThread& GetInstance();	
	// Callback function invoked when the waveform output device is out of data.
	static void CALLBACK waveOutProc(HWAVEOUT hWaveOut, UINT uMsg,
		DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
};

#endif

// --- End of File ---
