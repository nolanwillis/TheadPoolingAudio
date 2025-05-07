#include "WaveThread.h"
#include "CoordThread.h"
#include "PlayThread.h"
#include "KillThread.h"
#include "Buffer.h"
#include "UnitTestThread.h"

unsigned int WaveThread::ID = 1;

WaveThread::WaveThread()
	:
	KillableBase("--WAVE--"),
	mtx(),
	status(Status::RequestingFill),
	poDataBuffer(BUFFER_SIZE),
	canPlayCV(),
	id(WaveThread::ID),
	mThread(),
	waveHeader{ 0 } 
{
	WaveThread::ID++;
}
WaveThread::~WaveThread()
{
	if (this->mThread.joinable())
	{
		this->mThread.join();
		Debug::out("~WaveThread %u: joined\n", this->id);
	}
}

void WaveThread::Launch()
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
void WaveThread::operator()()
{
	START_BANNER;

	while (!KillEvent())
	{
		std::unique_lock<std::mutex> lck(this->mtx);
		if 
		( 
			this->canPlayCV.wait_for(lck, 0ms, 
			[&] { 
					return this->status == Status::CanPlay; 
				}
			)
		)
		{
			this->status = Status::Playing;
			// Prepare the wave header.
			memset(&this->waveHeader, 0x0, sizeof(WAVEHDR));
			this->waveHeader.dwUser = (DWORD_PTR)this;
			this->waveHeader.dwBufferLength = this->poDataBuffer.GetTotalSize();
			this->waveHeader.lpData = (LPSTR)(this->poDataBuffer.GetRawBuffStart());
			waveOutPrepareHeader(PlayThread::GetWaveHandle(), &this->waveHeader, 
				sizeof(WAVEHDR));
			// Write the wave data to the output device.
			waveOutWrite(PlayThread::GetWaveHandle(), &this->waveHeader, sizeof(WAVEHDR));
			Debug::out("WaveThread %u is playing\n", this->id);
		}
	}
}
