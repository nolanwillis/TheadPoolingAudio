//--------------------------------------------------------------
// Copyright 2023, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#ifndef FILE_THREAD_H
#define FILE_THREAD_H

#include "KillableBase.h"
#include "Buffer.h"

class FileThread : public KillableBase
{
public:
	static const unsigned int BUFFER_SIZE = 512 * 1024;
	static const unsigned int FILE_NAME_SIZE = 12;
	static const int MAX_NUM_WAVE_FILES = 23;

	enum class ThreadStatus
	{
		Empty, 
		Ready, 
		Uninitialized
	};
	
	FileThread() = delete;
	FileThread(const char *const pName);
	FileThread(const FileThread& rhs) = delete;
	FileThread &operator = (const FileThread& rhs) = delete;
	~FileThread();

	void Launch();
	void operator()();

	// Copies the data from FileThread's Buffer into a destination Buffer. 
	static size_t CopyFileBufferToDest(Buffer* pDest);
	static std::mutex& GetMutex();
	static FileThread::ThreadStatus GetThreadStatus();
	static void SetThreadStatus(FileThread::ThreadStatus inStatus);
	static std::condition_variable& GetCanTransferCV();

private:
	static FileThread* psFileThread;
	
	std::thread mThread;
	char* poFileName;
	int numFilesRead;
	std::condition_variable canTransferCV;
	Buffer pDataBuffer;

	// Data the must be protected by a mutex.
	std::mutex mtx;
	ThreadStatus threadStatus;
	
	static FileThread& GetInstance();
	// Loads the given file into poBuffer's unsigned char buffer.
	void LoadFile(const char* const pFileName);
	// Writes the file name to the poFileName buffer and returns a pointer to poFileName.
	char* GetFileName(int index);
};

#endif

// --- End of File ---
