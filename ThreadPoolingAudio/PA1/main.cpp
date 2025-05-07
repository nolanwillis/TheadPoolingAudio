//----------------------------------------------------------------------------
// Copyright 2024, Ed Keenan, all rights reserved.
//----------------------------------------------------------------------------

#include "KillThread.h"
#include "FileThread.h"
#include "CoordThread.h"
#include "PlayThread.h"
#include "File.h"

#include "UnitTestThread.h"

using namespace Azul;

int main()
{
	START_BANNER_MAIN("-Main-");

	UnitTestThread tUnitTest("--UNIT--");
	tUnitTest.Launch();

	// ----------------------------------------------------------------
	// setup objects
	//-----------------------------------------------------------------
	KillThread tKill("--KILL--");
	FileThread tFile("--FILE--");
	CoordThread tCoord("--COORD--");
	PlayThread tPlay("--PLAY--");

	tKill.Launch();
	tFile.Launch();
	tCoord.Launch();
	tPlay.Launch();

	// Key press
	//tKill.KeyPressEvent();
}

// --- End of File ---
