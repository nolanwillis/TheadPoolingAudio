#include "FinishedPlayingCmd.h"
#include "PlayThread.h"
#include "WaveThread.h"

FinishedPlayingCmd::FinishedPlayingCmd(WaveThread* const pFinishedWaveThread)
	: 
	pFinishedWaveThread(pFinishedWaveThread)
{
	assert(this->pFinishedWaveThread);
}

void FinishedPlayingCmd::Execute()
{
	PlayThread::NotifyWaveThreadIsDone(this->pFinishedWaveThread);
}
