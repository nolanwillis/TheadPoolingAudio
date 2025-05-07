//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#include <conio.h>

#include "FileThread.h"
#include "CoordThread.h"
#include "File.h"
#include "KillThread.h"

using namespace Azul;

FileThread* FileThread::psFileThread = nullptr;

FileThread::FileThread(const char* const pName)
	:
	KillableBase(pName),
	mThread(),
	poFileName(new char[FILE_NAME_SIZE]),
	numFilesRead(0),
	canTransferCV(),
	pDataBuffer(BUFFER_SIZE),
	mtx(),
	threadStatus(ThreadStatus::Empty)
{
	assert(poFileName);
	psFileThread = this;
}
FileThread::~FileThread()
{
	if(this->mThread.joinable())
	{
		this->mThread.join();
		Debug::out("~FileThread: joined\n");
	}
	delete[] poFileName;
}

void FileThread::Launch()
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
void FileThread::operator()()
{
	START_BANNER;
	while(!KillEvent())
	{
		{
			std::unique_lock<std::mutex> lck(this->mtx);
			if (this->numFilesRead < MAX_NUM_WAVE_FILES &&
				this->threadStatus == ThreadStatus::Empty)
			{
				// Load the file into FileThread's data buffer. 
				char* pName = this->GetFileName(this->numFilesRead);
				// Unlock the FileThreads mutex during the file load so we don't block other 
				// threads from accessing FileThread's data.
				lck.unlock();
				this->LoadFile(pName);
				// Change the status of the FileThread.
				lck.lock();
				this->threadStatus = ThreadStatus::Ready;
				lck.unlock();
				this->numFilesRead++;
				// Notify the CoordThread. 
				this->canTransferCV.notify_one();
			}
		}
		// Here is the one sleep that is allowed.
		std::this_thread::sleep_for(200ms);
	}
}

size_t FileThread::CopyFileBufferToDest(Buffer* pDest)
{
	assert(pDest);
	// Get reference to the singleton(ish) FileThread class. 
	FileThread& pfileThread = FileThread::GetInstance();
	// Copy the data from the FileThread's data Buffer to pDest.
	pfileThread.pDataBuffer.TransferDataOut(pDest, pDest->GetTotalSize());
	return pDest->GetCurrSize();
}
void FileThread::SetThreadStatus(FileThread::ThreadStatus inStatus)
{
	FileThread& pFileThread = GetInstance();
	pFileThread.threadStatus = inStatus;
}
FileThread::ThreadStatus FileThread::GetThreadStatus()
{
	FileThread& pFileThread = GetInstance();
	return pFileThread.threadStatus;
}
std::mutex& FileThread::GetMutex()
{
	FileThread& pFileThread = GetInstance();
	return pFileThread.mtx;
}
std::condition_variable& FileThread::GetCanTransferCV()
{
	FileThread& pFileThread = GetInstance();
	return pFileThread.canTransferCV;
}

FileThread &FileThread::GetInstance()
{
	assert(FileThread::psFileThread);
	return *psFileThread;
}
void FileThread::LoadFile(const char* const pFileName)
{
	File::Handle fh;
	File::Error err;

	assert(pFileName);

	Debug::out("%s load start <-- \n", pFileName);

	pDataBuffer.mtx.lock();

	// Get the size of the file.
	err = File::Open(fh, pFileName, File::Mode::READ);
	assert(err == File::Error::SUCCESS);
	err = File::Seek(fh, File::Position::END, 0);
	assert(err == File::Error::SUCCESS);
	DWORD size;
	err = File::Tell(fh, size);
	assert(err == File::Error::SUCCESS);

	// Write the data from the file into the poBuffer object's unsigned char buffer.
	err = File::Seek(fh, File::Position::BEGIN, 0);
	assert(err == File::Error::SUCCESS);
	err = File::Read(fh, pDataBuffer.GetRawBuffStart(), size);
	assert(err == File::Error::SUCCESS);
	err = File::Close(fh);
	assert(err == File::Error::SUCCESS);

	// Update metadata.
	pDataBuffer.SetCurrSize(size);

	pDataBuffer.mtx.unlock();
	
	Debug::out("%s load end --> %d bytes \n", pFileName, pDataBuffer.GetCurrSize());
}
char* FileThread::GetFileName(int index)
{
	sprintf_s(poFileName, FILE_NAME_SIZE, "wave_%d.wav", index);
	return poFileName;
}
