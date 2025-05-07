#ifndef FINISHED_PLAYING_CMD_H
#define FINISHED_PLAYING_CMD_H

#include "Command.h"

class WaveThread;

class FinishedPlayingCmd : public Command
{
public:
	FinishedPlayingCmd() = delete;
	FinishedPlayingCmd(WaveThread* const pFinishedWaveThread);
	FinishedPlayingCmd(FinishedPlayingCmd& rhs) = delete;
	FinishedPlayingCmd& operator=(const FinishedPlayingCmd& rhs) = delete;
	~FinishedPlayingCmd() = default;

	virtual void Execute() override;

private:
	WaveThread* pFinishedWaveThread;
};

#endif
