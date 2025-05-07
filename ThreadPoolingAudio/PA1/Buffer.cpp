#include "Buffer.h"

Buffer::Buffer(size_t s)
	: 
	mtx(),
	poBuffStart(new unsigned char[s]()),
	pBuffCurr(poBuffStart),
	totalSize(s),
	currSize(0)
{
	assert(poBuffStart);
}
Buffer::~Buffer()
{
	delete[] poBuffStart;
}

unsigned char* Buffer::GetRawBuffStart()
{
	assert(this->poBuffStart);
	return this->poBuffStart;
}
size_t Buffer::GetTotalSize()
{
	return this->totalSize;
}
size_t Buffer::GetCurrSize()
{
	return this->currSize;
}
void Buffer::SetCurrSize(const size_t s)
{
	assert(s <= this->totalSize);
	this->currSize = s;
}
bool Buffer::TransferDataOut(Buffer* pDstBuffer, size_t amount)
{
	assert(pDstBuffer);
	size_t adjustedAmount = 0;

	// Lock the this buffers mutex and the destination buffers mutex. 
	std::lock(this->mtx, pDstBuffer->mtx);
	std::lock_guard<std::mutex> localLck(this->mtx, std::adopt_lock);
	std::lock_guard<std::mutex> dstLck(pDstBuffer->mtx, std::adopt_lock);
	// Adjust the amount if it's more than this Buffer currently has.
	if (amount <= this->currSize)
	{
		adjustedAmount = amount;
	}
	else
	{
		adjustedAmount = this->currSize;
	}
	// Ensure the destination Buffer has room for that amount of data.
	if (amount + pDstBuffer->currSize > pDstBuffer->totalSize)
	{
		return false;
	}
	// Attempt to copy the data from src in this buffer object.
	int result = memcpy_s(
		pDstBuffer->poBuffStart,
		pDstBuffer->totalSize,
		this->pBuffCurr,
		adjustedAmount
	);
	// If the copy was successful.
	if (result == 0)
	{
		this->pBuffCurr += adjustedAmount;
		this->currSize -= adjustedAmount;
		pDstBuffer->SetCurrSize(pDstBuffer->GetCurrSize() + adjustedAmount);
		// If this buffer has transferred all of its data automatically return the 
		// current pointer to the start of the buffer.
		if (this->currSize == 0)
		{
			this->pBuffCurr = this->poBuffStart;
		}

		return true;
	}
	return false;
}
void Buffer::Reset()
{
	this->pBuffCurr = this->poBuffStart;
}
