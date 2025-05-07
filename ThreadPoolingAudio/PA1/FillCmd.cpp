#include "FillCmd.h"
#include "CoordThread.h"
#include "WaveThread.h"

FillCmd::FillCmd(WaveThread* const pWaveThreadToFill)
	:
	pWaveThreadToFill(pWaveThreadToFill)
{
	assert(this->pWaveThreadToFill);
}

void FillCmd::Execute()
{
	CoordThread::TransferToWaveThread(pWaveThreadToFill);
}

