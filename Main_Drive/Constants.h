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

//
// Update these as necessary to match your setup
//
#define enablePin 31       // Pin that provides power to motor driver enable pins
#define enablePinDome 29   // Pin that provides power to Dome motor driver enable pin
#define S2SpotPin A0       // Pin connected to side tilt potentiometer
#define readpin 34         // Pin connected to ACT on soundboard
#define soundpin1 26       // Connected to sound pin 0
#define soundpin2 28       // Connected to sound pin 1
#define soundpin3 30       // Connected to sound pin 2
#define soundpin4 32       // Connected to sound pin 3
#define soundpin5 46       // Connected to sound pin 4
#define soundpin6 44       // Connected to sound pin 5
#define fadePin A4         // Connected to + of one channel on sound board(use resistor to ground)
#define BTstatePin 33      // Connected to state pin on BT Module
#define domeTiltPotPin A1  // Connected to Potentiometer on the dome tilt mast
#define domeSpinPotPin A2  // Pin used to monitor dome spin potentiometer
#define battMonitor A3     // Pin used to monitor battery voltage
#define outputVoltage 5.2  // This is the output voltage from the Buck Converter powering the arduino
#define drivePWM1 11       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define drivePWM2 12       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM1 3          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM2 4          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM1 5     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM2 6     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM1 9     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM2 10    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM1 7 // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM2 8 // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define resistor1 121000   // Larger resisitor used on voltage divider to read battery level
#define resistor2 82000    // Smaller resisitor used on voltage divider to read battery level

//
// Naigon - NEC Audio
// Digital I/O pins that connect to my custom Naigon's Electronic Creations Igniter 3 sound player.
//
#define HappySoundPin 41
#define SadSoundPin 43
#define ExcitedSoundPin 45
#define ScaredSoundPin 47
#define ChattySoundPin 49
#define AgitatedSoundPin 51
#define PlayTrackPin 42
#define StopTrackPin 44

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Feature flags. Uncomment to turn on a feature.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define HeadTiltStabilization // uncomment this if you want the head to stabilize on top of the body.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Ease values from Joe. Modify these to increase/decrease the quickness of motor movements.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define flywheelEase 6 // Speed in which flywheel will increase/decrease during gradual movements
// S2SEase from Joe: 1.5
#define S2SEase 0.5         // Speed in which side to side moves. Higher number equates to faster movement
#define easeDome 20        // Lower number means more easing when spinning
#define easeDomeTilt 3.2   // Lower number means more easing when moving forward and back a.k.a. slower
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
// Joe's default is 25.
#define MaxSideToSide 20

#define MaxDomeTiltAngle 22 // Maximum angle in which the dome will tilt. **  Max is 25  **

// Naigon: Max pot range, Joe's default is 135. This means pot will map to -135, 135
#define MaxHeadTiltPot 135
#define MaxS2SPot 135
#define MaxDomeSpinPot 180

//
// Naigon: Amount to limit the flywheel.
#define MaxFlywheelDrive 255

//
// Naigon: Amount that dome can spin when in servo mode. This was hardcoded inline before; I refactored to a constant
// for ease of tuning.
//
// Joe's default is 70.
#define MaxDomeSpinServo 75
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
// Higher values make movements much smoother, at the expense of a longer delay before the drive catches up to the actual value.
#define PitchAndRollFilterCount 4

// Naigon - Head Tilt Stabilization
// Proportional amount of the stabilization to apply to the head tilt. Higher value means it will respond quicker at the expense of more jerk.
// Value should be between 0.0 and 1.0 inclusively.
#define HeadTiltPitchAndRollProportion 0.8


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reverse constants from Joe
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Naigon - Analog Input Refactor
// I have made these set as true/false now for on off instead of being defined or not, as it is more clear and better
// for the analog input refactor. Enable any to true depending on your wiring and the behavior of the drive.
//
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auto Disable
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define S2SOutThresh   28
#define DriveOutThresh 12
// Naigon: Defines the length (in MS) for the auto disable feature to kick in.
// Joe had this hard-coded inline with a value of 3000.
#define AutoDisableMS 4000
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
//#define printOffsets             // Uncomment to see the offsets
//#define debugRSelectMillis
//#define printOutputs
//#define printSoundPins
//#define debugFlywheelSpin
//#define debugSound
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// =====================================================================================================================================================================================================
// =====================================================================================================================================================================================================
//

#endif // __Constants_h_
