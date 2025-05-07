//--------------------------------------------------------------
// Copyright 2024, Ed Keenan, all rights reserved.
//--------------------------------------------------------------

#ifndef UNIT_TEST_CONFIGURATION_H
#define UNIT_TEST_CONFIGURATION_H

// Make sure all are enabled for final submission...
//      I grade whatever is set in this file - Keenan


// only one can be enabled  PA1_A or PA1_B or PA1_C
#define PA1_A_Enable    false
#define PA1_B_Enable    false
#define PA1_C_Enable    true

#if PA1_A_Enable
#define Coord_Test       true
#define Wave_Test        false
#endif

#if PA1_B_Enable
#define Coord_Test       false
#define Wave_Test        true
#endif

#if PA1_C_Enable
#define Coord_Test       false
#define Wave_Test        false
#endif

#endif

// ---  End of File ---------------
