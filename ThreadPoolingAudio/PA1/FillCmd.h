#ifndef FILL_CMD_H
#define FILL_CMD_H

#include "Command.h"

class CoordThread;
class WaveThread;

class FillCmd : public Command
{
public:
	FillCmd() = delete;
	FillCmd(WaveThread* const pWaveThreadToFill);
	FillCmd(FillCmd& rhs) = delete;
	FillCmd& operator=(const FillCmd& rhs) = delete;
	~FillCmd() = default;

	virtual void Execute() override;

private:
	WaveThread* pWaveThreadToFill;
};

#endif
