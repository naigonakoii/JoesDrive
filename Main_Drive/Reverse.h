#ifndef _Reverse_h_
#define _Reverse_h_

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reverse constants from Joe
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Naigon - Analog Input Refactor
// I have made these set as true/false now for on off instead of being defined or not, as it is more clear and better
// for the analog input refactor. Enable any to true depending on your wiring and the behavior of the drive.
//
// Note
// These values have been split based on MK2 vs MK3 per what Joe's diagram for those showed. I'm hopeing this minimizes
// the amount of change that is needed here, so that the end user only needs to change the appropriate single value in
// DriveSetup.h for HeadTiltVersion.
//
#if HeadTiltVersion == MK2_Dome

#define reverseDrive        true
#define reverseDomeTilt     false
#define reverseS2S          false
#define reverseDomeSpin     false
#define reverseFlywheel     false
#define reversePitch        true
#define reverseRoll         true
#define reverseDomeTiltPot  false
#define reverseDomeSpinPot  false
#define reverseS2SPot       false
#define reverseAutoDome     true

// Naigon - Needed to prevent build breaks, but completely unused with MK2.
#define reverseDomeTiltLR   false

#else

#define reverseDrive        false
#define reverseDomeTiltFR   false
#define reverseDomeTiltLR   true
#define reverseS2S          true
#define reverseDomeSpin     true
#define reverseFlywheel     false
#define reversePitch        false
#define reverseRoll         false
#define reverseDomeTiltPot  true
#define reverseDomeSpinPot  false
#define reverseS2SPot       true
#define reverseAutoDomeX    false
#define reverseAutoDomeY    false

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif