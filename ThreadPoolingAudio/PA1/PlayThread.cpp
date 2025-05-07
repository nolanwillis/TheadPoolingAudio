#include "PlayThread.h"
#include "CoordThread.h"
#include "WaveThread.h"
#include "KillThread.h"
#include "UnitTestThread.h"
#include "FillCmd.h"
#include "FinishedPlayingCmd.h"

PlayThread* PlayThread::psPlayThread = nullptr;

PlayThread::PlayThread(const char* const pName)
	:
	KillableBase(pName),
	mThread(),
	wfx{ 0 },
	waveThreadTable(new WaveThread[MAX_WAVE_THREADS]),
	inQueue()
{
	assert(waveThreadTable);
	psPlayThread = this;

	// Setup the waveform output device.
	// The sample rate. 
	this->wfx.nSamplesPerSec = 22050; 
	// Number of bits per sample of mono data. 
	this->wfx.wBitsPerSample = 16; 
	// Number of channels (i.e. mono, stereo...). 
	this->wfx.nChannels = 2; 
	// The format type. 
	this->wfx.wFormatTag = WAVE_FORMAT_PCM; 
	// Block size of the data. 
	this->wfx.nBlockAlign = (DWORD)(wfx.wBitsPerSample >> 3) * wfx.nChannels;	
	this->wfx.nAvgBytesPerSec = wfx.nBlockAlign * wfx.nSamplesPerSec;		
	this->wfx.cbSize = 0;
	// Open the waveform output device.
	MMRESULT result = waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 
		(DWORD_PTR)waveOutProc, 0, CALLBACK_FUNCTION);
	if (result != MMSYSERR_NOERROR)
	{
		fprintf(stderr, "Unable to open the waveform ouput device!\n");
		ExitProcess(1);
	}
	// Set the volume.
	waveOutSetVolume(this->hWaveOut, 0x3fff3fff);
}
PlayThread::~PlayThread()
{
	if (this->mThread.joinable())
	{
		this->mThread.join();
		Debug::out("~PlayThread: joined\n");
	}
	delete[] this->waveThreadTable;
	// Shutdown the waveform output device.
	waveOutClose(this->hWaveOut);
}

void PlayThread::Launch()
{
	if (this->mThread.joinable() == false)
	{
		this->mThread = std::thread(std::ref(*this));
	}
	else
	{
		assert(false);
	}
}
void PlayThread::operator()()
{
	START_BANNER;
	// Launch all the wave threads. 
	for (int i = 0; i < MAX_WAVE_THREADS; i++)
	{
		waveThreadTable[i].Launch();
	}
	// Add 20 fill commands to the queue for the CoordThread.
	for (int i = 0; i < MAX_WAVE_THREADS; i++)
	{
		assert(&waveThreadTable[i]);
		CoordThread::SendCommand(new FillCmd(&waveThreadTable[i]));
	}
	
	while (!KillEvent())
	{
		Command* pCmd;
		if (this->inQueue.PopFront(pCmd))
		{
			pCmd->Execute();
			delete pCmd;
		}
	}
}

HWAVEOUT& PlayThread::GetWaveHandle()
{
	PlayThread& pPlayThread = GetInstance();
	return pPlayThread.hWaveOut;
}
void PlayThread::SendCommand(Command* pCmd)
{
	assert(pCmd);
	PlayThread& pPlayThread = GetInstance();
	pPlayThread.inQueue.PushBack(pCmd);
}
void PlayThread::NotifyWaveThreadIsDone(WaveThread* pWaveThread)
{
	PlayThread& pPlayThread = GetInstance();
	WaveThread* pSelectedWaveThread = nullptr;
	int finishedCount = 0;

	// Go through the wave table, count the number of WaveThreads that have been set to 
	// be finished by the CoordThread. If there's a free wave thread save it and 
	// mark it to be sent.
	for (int i = 0; i < MAX_WAVE_THREADS; i++)
	{
		WaveThread* currWaveThread = &pPlayThread.waveThreadTable[i];
		currWaveThread->mtx.lock();
		if (currWaveThread->status == WaveThread::Status::Finished)
		{
			finishedCount++;
		}
		else if (currWaveThread->status == WaveThread::Status::Free && !pSelectedWaveThread)
		{
			pSelectedWaveThread = currWaveThread;
			currWaveThread->status = WaveThread::Status::RequestingFill;
		}
		currWaveThread->mtx.unlock();
	}

	// If the finishedCount is 19, kill the program.
	if (finishedCount == MAX_WAVE_THREADS-1)
	{
		KillThread::Kill();
	}
	// Otherwise if we found a free WaveThread, send a Fill command for it to the CoordThread. 
	else if (pSelectedWaveThread)
	{
		Debug::out("PlayThread: sending fill command for WaveThread %u\n", 
			pSelectedWaveThread->id);
		CoordThread::SendCommand(new FillCmd(pSelectedWaveThread));
	}
}

PlayThread& PlayThread::GetInstance()
{
	assert(psPlayThread);
	return *psPlayThread;
}
void PlayThread::waveOutProc(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance,
	DWORD dwParam1, DWORD dwParam2)
{
	AZUL_UNUSED_VAR(dwParam2);
	AZUL_UNUSED_VAR(hWaveOut);

	switch (uMsg)
	{
	case WOM_DONE:
	{
		// Get a pointer to the WaveThread through dwParam1.
		WAVEHDR* pWaveHeader = (WAVEHDR*)(dwParam1);
		assert(pWaveHeader);
		WaveThread* pWaveThread = (WaveThread*)pWaveHeader->dwUser;
		assert(pWaveThread);
		//Debug::out("WOM_DONE: WaveThread %u\n", pWaveThread->id);	
		// Update the wave thread now that it's finished playing.
		pWaveThread->mtx.lock();
		pWaveThread->status = WaveThread::Status::Free;
		pWaveThread->mtx.unlock();	
		pWaveThread->poDataBuffer.mtx.lock();
		pWaveThread->poDataBuffer.SetCurrSize(0);
		pWaveThread->poDataBuffer.mtx.unlock();
		Debug::out("Callback: WaveThread %u is done.\n", pWaveThread->id);
		PlayThread::SendCommand(new FinishedPlayingCmd(pWaveThread));
		break;
	}
	case WOM_CLOSE:
		Debug::out("WOM_CLOSE:\n");
		break;
	case WOM_OPEN:
		Debug::out("WOM_OPEN:\n");
		break;
	default:
		assert(false);
	}
}
