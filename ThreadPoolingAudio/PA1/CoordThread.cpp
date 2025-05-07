//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#include "CoordThread.h"
#include "FileThread.h"
#include "KillThread.h"
#include "WaveThread.h"

CoordThread* CoordThread::psCoordThread = nullptr;

CoordThread::CoordThread(const char *const pName)
	:
	KillableBase(pName),
	mThread(),
	pDataBufferA(CoordThread::BUFFER_SIZE),
	pDataBufferB(CoordThread::BUFFER_SIZE),
	pDrainBuffer(&this->pDataBufferA),
	drainStatus(DrainStatus::BufferA),
	inQueue(),
	threadStatus(ThreadStatus::Not_Ready)
{
	assert(pDrainBuffer);
	psCoordThread = this;
}
CoordThread::~CoordThread()
{
	if(this->mThread.joinable())
	{
		this->mThread.join();
		Debug::out("~CoordThread: joined\n");
	}
}

void CoordThread::Launch()
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
void CoordThread::operator()()
{
	START_BANNER;

	while(!KillEvent())
	{
		{	
			std::unique_lock<std::mutex> lck(FileThread::GetMutex());
			if 
			( 
				FileThread::GetCanTransferCV().wait_for(lck, 0ms,
				[&] {
						return FileThread::GetThreadStatus() == 
							FileThread::ThreadStatus::Ready;
					}
				)
			)
			{
				// If data buffer A is empty.
				if (this->pDataBufferA.GetCurrSize() == 0)
				{
					Debug::out("Start transfer to BufferA from the FileThread.\n");
					FileThread::CopyFileBufferToDest(&pDataBufferA);
					Debug::out("End transfer to BufferA from the FileThread.\n");
					// Set the status of the file thread.
					FileThread::SetThreadStatus(FileThread::ThreadStatus::Empty);
					// Now that a buffer is filled we can execute fill commands.
					this->threadStatus = CoordThread::ThreadStatus::Ready;
				}
				// If data buffer B is empty.
				else if (pDataBufferB.GetCurrSize() == 0)
				{	
					Debug::out("Start transfer to BufferB from the FileThread.\n");
					FileThread::CopyFileBufferToDest(&this->pDataBufferB);
					Debug::out("End transfer to BufferB from the FileThread.\n");
					// Set the status of the file thread.
					FileThread::SetThreadStatus(FileThread::ThreadStatus::Empty);
					// Now that a buffer is filled we can execute fill commands.
					this->threadStatus = CoordThread::ThreadStatus::Ready;
				}
			}
		}
		if (this->threadStatus == CoordThread::ThreadStatus::Ready)
		{
			Command* pCmd;
			if (this->inQueue.PopFront(pCmd))
			{
				pCmd->Execute();
				delete pCmd;
			}
		}
	}
}

size_t CoordThread::TransferToWaveThread(WaveThread* pWaveThread)
{
	assert(pWaveThread);

	CoordThread& pCoordThread = CoordThread::GetInstance();
	
	// If both buffers are empty that means we are done (this assumes the two buffers are 
	// never fully depleted pending a file load). Tell the passed in WaveThread it can no 
	// longer be used.
	if (pCoordThread.pDataBufferA.GetCurrSize() == 0 && 
		pCoordThread.pDataBufferB.GetCurrSize() == 0)
	{
		pWaveThread->mtx.lock();
		pWaveThread->status = WaveThread::Status::Finished;
		pWaveThread->mtx.unlock();
		return 0;
	}
	
	// Change the drain buffer pointer as needed.
	Debug::out("BufferA size: %zu\n", pCoordThread.pDataBufferA.GetCurrSize());
	Debug::out("BufferB size: %zu\n", pCoordThread.pDataBufferB.GetCurrSize());
	if (pCoordThread.pDataBufferA.GetCurrSize() == 0 && pCoordThread.pDataBufferB.GetCurrSize() > 0)
	{
		pCoordThread.pDrainBuffer = &pCoordThread.pDataBufferB;
		pCoordThread.drainStatus = DrainStatus::BufferB;
	}
	else if (pCoordThread.pDataBufferB.GetCurrSize() == 0 && pCoordThread.pDataBufferA.GetCurrSize() > 0)
	{
		pCoordThread.pDrainBuffer = &pCoordThread.pDataBufferA;
		pCoordThread.drainStatus = DrainStatus::BufferA;
	}
	
	// Attempt to the transfer data from the drain buffer to the WaveThread.
	std::unique_lock<std::mutex> lck(pWaveThread->mtx);
	size_t waveThreadBufferSize = 0;
	if (pWaveThread->status == WaveThread::Status::RequestingFill)
	{		
		// Get the amount to transfer.
		pWaveThread->poDataBuffer.mtx.lock();
		size_t amountToTransfer = pWaveThread->poDataBuffer.GetTotalSize();
		pWaveThread->poDataBuffer.mtx.unlock();
		// Transfer the data. 
		bool result = pCoordThread.pDrainBuffer->TransferDataOut
		(
			&pWaveThread->poDataBuffer, 
			amountToTransfer
		);
		if (result)
		{
			pWaveThread->poDataBuffer.mtx.lock();
			waveThreadBufferSize = pWaveThread->poDataBuffer.GetCurrSize();
			pWaveThread->poDataBuffer.mtx.unlock();
			if (pCoordThread.drainStatus == DrainStatus::BufferA)
			{
				Debug::out("Transferred %zu bytes from BufferA into WaveThread %u\n",
					waveThreadBufferSize, pWaveThread->id);
			}
			else
			{
				Debug::out("Transferred %zu bytes from BufferB into WaveThread %u\n",
					waveThreadBufferSize, pWaveThread->id);
			}

			if (waveThreadBufferSize > 0)
			{
				pWaveThread->status = WaveThread::Status::CanPlay;
				// Unlock the WaveThread mutex before notifying.
				lck.unlock();
				// Notify the WaveThread that it can play.
				pWaveThread->canPlayCV.notify_one();
			}
			else
			{
				// If the transfer was successful but 0 bytes were transferred.
				pWaveThread->status = WaveThread::Status::Free;
			}
		}
		// If the transfer failed set status to free.
		else
		{
			pWaveThread->status = WaveThread::Status::Free;
		}
	}

	return waveThreadBufferSize;
}
void CoordThread::SendCommand(Command* pCmd)
{
	assert(pCmd);
	CoordThread& pCoordThread = GetInstance();
	pCoordThread.inQueue.PushBack(pCmd);
}

CoordThread& CoordThread::GetInstance()
{
	assert(CoordThread::psCoordThread);
	return *psCoordThread;
}
