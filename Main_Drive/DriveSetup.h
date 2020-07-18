
#ifndef __DriveSetup_h_
#define __DriveSetup_h_

#include "Constants.h"

// Set this value to either MK3_Dome or MK2_Dome depending on which version of Joe's head tilt you have.
#define HeadTiltVersion MK3_Dome

// Uncomment if you are using the older single remote that communicates via a paired bluetooth.
//#define BTRemote

// Set to either 'NECWired', 'NECWireless', or 'JoeSerial' for sound playing.
#define AudioHardware JoeSerial

#endif // __DriveSetup_h_
