#ifndef BUFFER_H
#define BUFFER_H

struct Buffer
{
public:
	Buffer(size_t size);
	Buffer() = delete;
	Buffer(const Buffer&) = delete;
	Buffer& operator = (const Buffer&) = delete;
	~Buffer();

	unsigned char* GetRawBuffStart();
	size_t GetTotalSize();
	size_t GetCurrSize();
	void SetCurrSize(const size_t s);
	// Transfers the data from this Buffer into a destination Buffer. Includes correct
	// mutex locking for ease of use.
	bool TransferDataOut(Buffer* pOther, size_t amount);
	// Resets the data buffer of this Buffer object. 
	void Reset();
	
	// This mutex is to protect all the data in the Buffer. Multiple threads can be 
	// modifying Buffers at once.
	std::mutex     mtx;

private:
	// Pointer to the data buffer.
	unsigned char* poBuffStart;
	// Pointer to the current position in the data buffer.
	unsigned char* pBuffCurr;
	size_t         totalSize;
	size_t         currSize;
};

#endif