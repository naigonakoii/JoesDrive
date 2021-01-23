// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Pins that map to the physical wiring to the main Arduino.
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

#ifndef __Pins_H_
#define __Pins_H_

#include "DriveSetup.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Pin Mapping
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if HeadTiltVersion == MK2_Dome

// PIN settings for MK2_Dome
//
#define enablePin 31         // Pin that provides power to motor driver enable pins
#define enablePinDome 29     // Pin that provides power to Dome motor driver enable pin
#define S2SpotPin A0         // Pin connected to side tilt potentiometer
#define readpin 34           // Pin connected to ACT on soundboard
#define fadePin A4           // Connected to + of one channel on sound board(use resistor to ground)
#define BTstatePin 33        // Connected to state pin on BT Module
#define domeTiltPotPin A1    // Connected to Potentiometer on the dome tilt mast
#define domeSpinPotPin A2    // Pin used to monitor dome spin potentiometer
#define battMonitor A3       // Pin used to monitor battery voltage
#define outputVoltage 5.2    // This is the output voltage from the Buck Converter powering the arduino
#define drivePWM1 11         // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define drivePWM2 12         // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM1 3            // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM2 4            // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM1 5       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM2 6       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM1 9       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM2 10      // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM1 7   // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM2 8   // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define resistor1 121000     // Larger resistor used on voltage divider to read battery level
#define resistor2 82000      // Smaller resistor used on voltage divider to read battery level

#else

// MK3 values
//
#define enablePinS2S 31       // Pin that provides power to motor driver enable pins
#define enablePin 29          // Pin that provides power to motor driver enable pins
#define enablePinDome 33      // Pin that provides power to Dome motor driver enable pin
#define S2SpotPin A0          // Pin connected to side tilt potentiometer 
#define ACTpin 35             // Pin connected to ACT on soundboard
#define fadePin A2            // Connected to + of one channel on sound board(use resistor to ground)
#define domeSpinPotPin A4     // Pin used to monitor dome spin potentiometer
#define battMonitor A3        // Pin used to monitor battery voltage
#define outputVoltage 5.2     // This is the output voltage from the Buck Converter powering the arduino
#define drivePWM1 12          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define drivePWM2 13          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM1 6             // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM2 7             // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM1 10       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM2 11       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM1 8    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed 
#define flywheelSpinPWM2 9    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed 
#define leftDomeTiltServo  4  // Signal pin for the left dome tilt servo 
#define rightDomeTiltServo 5  // Signal pin for the right dome tilt servo
#define resetPin 40
#define SFX_RST 37            // Reset pin for the Adafruit audio player
#define resistor1 151000      // Larger resisitor used on voltage divider to read battery level
#define resistor2 82000       // Smaller resisitor used on voltage divider to read battery level

#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Joe's Wired Sound Player settings (NOT SUPPORTED)
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define soundpin1 26          // Connected to sound pin 0
#define soundpin2 28          // Connected to sound pin 1
#define soundpin3 30          // Connected to sound pin 2
#define soundpin4 32          // Connected to sound pin 3
#define soundpin5 46          // Connected to sound pin 4
#define soundpin6 44          // Connected to sound pin 5
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Naigon - NEC Audio
// Digital I/O pins that connect to my custom Naigon's Electronic Creations Igniter 3 sound player.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define HappySoundPin 41
#define SadSoundPin 43
#define ExcitedSoundPin 45
#define ScaredSoundPin 47
#define ChattySoundPin 49
#define AgitatedSoundPin 51
#define PlayTrackPin 42
#define StopTrackPin 44
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //__Pins_H_
