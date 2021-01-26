// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Constants that were on top of the main file for Joe's drive; they have been
//                                       split out now for ease of tuning.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                         Joe's Drive powered by Naigon
//                         27 May 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// ====================================================================================================================
//                         Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//                         You will need libraries: EepromEX: https://github.com/thijse/Arduino-EEPROMEx
//                                                  PIDLibrary: http://playground.arduino.cc/Code/PIDLibrary
//                                                  EasyTransfer: https://github.com/madsci1016/Arduino-EasyTransfer
//
// ====================================================================================================================
// ====================================================================================================================

#ifndef __Constants_h_
#define __Constants_h_

#include "DriveSetup.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Feature flags. Uncomment to turn on a feature.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HeadTiltStabilization   // uncomment this if you want the head to stabilize on top of the body.

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ease values. Modify these to increase/decrease the quickness of motor movements.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define easeFlywheel      6.0 // Speed in which flywheel will increase/decrease during gradual movements
// S2SEase from Joe: 1.5
#define easeS2S            1.50 // Speed in which side to side moves. Higher number equates to faster movement
#define easeMsS2SA        50.00 // Length in milliseconds to reach full increment speed for ScalingEaseApplicator
#define easeMsS2SD       350.00 // Length in ms from target when ramp down is applied for ScalingEaseApplicator
#define easeDome           4.00 // Lower number means more easing when spinning
#define easeDomeServo      1.75 // Speed for dome spin in servo mode
#define easeDomeServoMsA  50.00 // Length in milliseconds to reach full increment speed for ScalingEaseApplicator
#define easeDomeServoMsD 150.00 // Length in ms from target when ramp down is applied for ScalingEaseApplicator
#define easeDomeTilt       4.00 // Lower number means more easing when moving forward and back a.k.a. slower
#define easeDomeTiltMsA   75.00 // Proportion of ease to add when starting movement for ScalingEaseApplicator
#define easeDomeTiltMsD  200.00 // Length in ms from target when ramp down is applied for ScalingEaseApplicator
#define easeDomeFR         0.50 // Lower number means more easing for the MK3 dome tilt axes.
#define easeDomeLR         1.00 // Lower number means more easing for the MK3 dome tilt axes.
#define easeTiltMK3MsA   100.00
#define easeTiltMK3MsD   400.00
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Max joystick/pot values
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MaxDrive     55
#define MaxDriveSlow 20
#define MaxDriveMed  28
#define MaxDriveFast 38

//
// Naigon - MK3 Flywheel - This value should be updated for the MK3 Flywheel, as more weight towards the outside makes
// it move higher, and it is overall more sensitive.
//
// Defines the side to side output range, ie how much it can move.
// Joe's default is 25. Max is 25.
#define MaxSideToSide 21

#define MaxDomeTiltAngle 22 // Maximum angle in which the dome will tilt. **  Max is 25  **

// Naigon: Max pot range, Joe's default is 135. This means pot will map to -135, 135
#define MaxHeadTiltPot 135
#define MaxS2SPot 135
#define MaxDomeSpinPot 180

//
// Naigon: Amount to limit the flywheel.
#define MaxFlywheelDrive 255

#define MaxDomeSpin 255

//
// Naigon: Amount that dome can spin when in servo mode. This was hardcoded inline before; I refactored to a constant
// for ease of tuning.
//
// Joe's default is 70.
#define MaxDomeSpinServo 80

#define MaxDomeSpinAuto 255 * 2 / 3
#define MaxDomeServoAuto MaxDomeSpinServo

#define MaxDomeTiltY  12      // Maximum angle to tilt the dome in the Y axis ** - MAX IS 20
#define MaxDomeTiltX  12      // Maximum angle to tilt the dome in the X axis ** - MAX IS 18
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Thresholds
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Naigon: Threshold of the pot before actually adjusting the input. Joe's default is 25
#define HeadTiltPotThresh 25
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//
// Naigon - DomeTiltFromJoystick
// Head tilt is not based on the joystick, and is relative to the difference of the max angle
#define DomeTiltAmount (MaxDomeTiltAngle - 10)

// Naigon - Head Tilt Stabilization
// Defines the number of points for the pitch and roll smoothing filter.
// Higher values make movements much smoother, at the expense of a longer delay before the drive catches up to the
// actual value.
#define PitchAndRollFilterCount 4

// Naigon - Head Tilt Stabilization
// Proportional amount of the stabilization to apply to the head tilt. Higher value means it will respond quicker at
// the expense of more jerk.
// Value should be between 0.0 and 1.0 inclusively.
#define HeadTiltPitchAndRollProportion 0.9


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auto Disable
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define S2SOutThresh   42
#define DriveOutThresh 12
// Naigon: Defines the length (in MS) for the auto disable feature to kick in.
// Joe had this hard-coded inline with a value of 3000.
#define AutoDisableMS 4000
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MK3 Head Tilt
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define dataDelay    0
#define recDelay    10  // Delay to wait after receiving data.
#define sendDelay   40  // Delay to wait after sending data.
#define domeSpeed   60  // Speed at which the servos move.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug Defines
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Uncomment at most ONE of these to debug the corresponding values.
//
//#define printRemote              // Uncomment to see values passed from controller
//#define debugS2S                 // Uncomment to see Side tilt variables, PID values, ETC.
//#define debugDrive               // Uncomment to see main drive variables, PID values, ETC.
//#define debugDomeTilt            // Uncomment to see Dome tilt variables, PID values, ETC.
//#define debugdomeRotation        // Uncomment to see Dome rotation variables, PID values, ETC.
//#define debugPSI                 // Uncomment to see PSI values.
//#define printbodyBatt            // Uncomment to see battery level
//#define printYPR                 // Uncomment to see Yaw, Pitch, and Roll
//#define printDome                // Uncomment to see the Dome's Yaw
//#define debugRSelectMillis
//#define printSoundPins
//#define debugFlywheelSpin
//#define debugSound
//#define debugAutoDisable         // Uncomment to see the autoDisable values
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// ====================================================================================================================
// PID constants
// ====================================================================================================================

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID1 is for side to side tilt
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double Pk1 = 14.0; // Joe's 13
const double Ik1 =  0.0; // Joe's 0
// Naigon - Change this value from .3 to .1 or 0 to remove shakey side to side
const double Dk1 =  0.0;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID2 is for side to side servo
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double Pk2 = 0.50; // Joe 0.5; M2 Flywheel .4
const double Ik2 = 0.00; // was .00
const double Dk2 = 0.00; // was .01


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID3 is for the main drive
// Naigon - MK3 Flywheel
//
// The following values will need to be updated if doing the MK3 flywheel and adding the balancing weights.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double Pk3 = 5.00; // Joe 5.0;
const double Ik3 = 0.00;
const double Dk3 = 0.00;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID4 is for dome tilt fwd/back
// Naigon - adjust for pid dome tilt control
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double Pk4 = 3.75; // default is 6
const double Ik4 = 1.00; // default is 0
const double Dk4 = 0.00;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID5 is for the dome spin servo
// Naigon - Animations
// It was important to tune this for animations, as the original values made the head too jerky and would cause It
// to pop off occasionally.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const double Pk5 = 2.75;
const double Ik5 = 1.00;
const double Dk5 = 0.00;

#endif // __Constants_h_
