// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Main drive mechanism using Ardino Mega with secondary Arduino Pro Mini IMU
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

// ********************************************************************************************************************
// NOTE - These includes, using statements, and extrn variable definitions are not actually needed for the Arduino
// compiler, but I added them to prevent intellisense errors in Visual Studio Code, as it is unable to understand that
// .ino files are concat together. Since the includes are protected by the #ifndef at the beginning, and doing using
// twice tested as benign, it causes no issues to re-include and re add using statements in what becomes the final
// single .ino file.
// ********************************************************************************************************************
#include "Arduino.h"

#include "Constants.h"
#include "DriveSetup.h"
#include "Structs.h"

#include "src/Libraries/NaigonIO/src/AnalogInHandler.h"

#include "ImuProMini.h"
#include "Offsets.h"
#if AudioHardware == JoeSerial
#include "JoeSerialAudio.h"
#endif

using Naigon::BB_8::ImuProMini;
using Naigon::BB_8::Offsets;
#if AudioHardware == JoeSerial
using Naigon::BB_8::JoeSerialAudio;
#endif
using Naigon::IO::AnalogInHandler;

extern AnalogInHandler *driveStickPtr;
extern AnalogInHandler *sideToSideStickPtr;
extern AnalogInHandler *domeTiltStickPtr;
extern AnalogInHandler *domeTiltStickLRPtr;
extern AnalogInHandler *domeSpinStickPtr;
extern AnalogInHandler *flywheelStickPtr;

extern AnalogInHandler sideToSidePotHandler;
extern AnalogInHandler domeSpinPotHandler;

#if RemoteHardware == FeatherPair
extern SEND_DATA_STRUCTURE_FEATHER sendToRemote;
#else
extern SEND_DATA_STRUCTURE_REMOTE sendToRemote;
#endif

extern ImuProMini imu;
extern Offsets offsets;
#if AudioHardware == JoeSerial
extern JoeSerialAudio audio;
#endif
extern PIDVals s2sTiltVals;
extern PIDVals s2sServoVals;
extern PIDVals driveVals;
#if HeadTiltVersion == MK2_Dome
extern PIDVals domeTiltVals;
#endif
extern PIDVals domeServoVals;
#if HeadTiltVersion == MK3_Dome
extern int servoLeft, servoRight;
#endif
extern int currentDomeSpeed;
extern int flywheelRotation;

extern AutoDisableState autoDisable;
// ********************************************************************************************************************


// ------------------------------------------------------------------------------------
// Debug Output Naming Convention:
//
//   R     - IMU Roll
//   P     - IMU Pitch
//   fR    - IMU Filtered Roll
//   fP    - IMU Filtered Pitch
//   o     - Offset prefix; ie oR = Roll Offset
//   I     - PID input value
//   S     - PID setpoint value
//   O     - PID output value
//   Pot   - relevant potentiometer output
//   Joy   - relevant joystick current value
//   E     - relevant ease current value
// ------------------------------------------------------------------------------------
void debugRoutines()
{
    //
    // Uncomment " #Define " in Constants.h
    //

    #ifdef debugDrive
    Serial.print(F(" Joy: "));
    Serial.print(driveStickPtr->GetMappedValue());
    Serial.print(F(", S3: "));
    Serial.print(driveVals.setpoint);
    Serial.print(F(", I3: "));
    Serial.print(driveVals.input);
    Serial.print(F(", O3: "));
    Serial.println(driveVals.output);
    #endif

    #ifdef debugS2S
    Serial.print(F("Joy: "));
    Serial.print(sideToSideStickPtr->GetMappedValue());
    Serial.print(F(", R: "));
    Serial.print(imu.Roll());
    Serial.print(F(", oR: "));
    Serial.print(offsets.RollOffset());
    Serial.print(F(", Pot: "));
    Serial.print(sideToSidePotHandler.GetMappedValue());
    Serial.print(F(", oPot: "));
    Serial.print(offsets.SideToSidePotOffset());
    Serial.print(F(", I1: "));
    Serial.print(s2sTiltVals.input);
    Serial.print(F(", S1: "));
    Serial.print(s2sTiltVals.setpoint);
    Serial.print(F(", O1: "));
    Serial.println(s2sTiltVals.output);
    #endif

    #ifdef debugDomeTilt
    Serial.print(F(" JoyY: "));
    Serial.print(domeTiltStickPtr->GetMappedValue());
    Serial.print(F(", JoyX: "));
    Serial.print(domeTiltStickLRPtr->GetMappedValue());

    Serial.print(F(", P"));
    Serial.print(imu.Pitch());
    Serial.print(F(", fP"));
    Serial.print(imu.FilteredPitch());
    Serial.print(F(", oP"));
    Serial.print(offsets.PitchOffset());

    #if HeadTiltVersion == MK3_Dome
        Serial.print(F(", R"));
        Serial.print(imu.Roll());
        Serial.print(F(", fR"));
        Serial.print(imu.FilteredRoll());
        Serial.print(F(", oR"));
        Serial.print(offsets.RollOffset());
    #endif

    #if HeadTiltVersion == MK2_Dome
        Serial.print(F(" I4 :"));
        Serial.print(domeTiltVals.input);
        Serial.print(F(" S4 :"));
        Serial.print(domeTiltVals.setpoint);
        Serial.print(F(" O4 :"));
        Serial.println(domeTiltVals.output);
    #else
        Serial.print(F(", sL"));
        Serial.print(servoLeft);
        Serial.print(F(", sR"));
        Serial.print(servoRight);
    #endif
    Serial.println();
    #endif

    #ifdef debugdomeRotation
    // NOTE: This only debugs with non-server dome spin modes per Joe's original code. I will look at improveing it later.
    Serial.print(F(" pot: "));
    Serial.print(domeSpinPotHandler.GetMappedValue());
    Serial.print(F(", out: "));
    Serial.println(currentDomeSpeed);
    #endif

    #ifdef debugSound && SoundHardware == NaigonWired
    Serial.print(F(" playing sound: "));
    Serial.println(soundPlayer->SoundTypeCurrentlyPlaying());
    #endif

    #ifdef debugPSI
    Serial.print(F("psiVal: "));
    Serial.print(sendToRemote.psi);
    Serial.print(F(", isPlaying: "));
    Serial.print(audio.IsPlaying());
    Serial.println();
    #endif

    #ifdef printbodyBatt
    Serial.print(F(" Vin: "));
    Serial.println(sendToRemote.bodyBatt);
    #endif

    #ifdef printYPR
    //Serial.print(F(" Yaw: "));
    //Serial.print(yaw);
    Serial.print(F(" R: "));
    Serial.print(imu.Roll());
    Serial.print(F(", fR: "));
    Serial.print(imu.FilteredRoll());
    Serial.print(F(", oR: "));
    Serial.print(offsets.RollOffset());


    Serial.print(F(",  P: "));
    Serial.println(imu.Pitch());
    Serial.print(F(",  fP: "));
    Serial.println(imu.FilteredPitch());
    Serial.print(F(", oP: "));
    Serial.println(offsets.PitchOffset());
    #endif

    #ifdef printDome
    //Serial.print(F(" Dome Yaw: "));
    //Serial.print(recFromDome.domeYaw);
    //Serial.print(F(" Dome Batt: "));
    //Serial.println(recFromDome.domeBatt);
    #endif

    #ifdef printRemote
    Serial.print(F("  Remote: "));
    Serial.print(recFromRemote.Joy1Y);
    Serial.print(" , ");
    Serial.print(recFromRemote.Joy1X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy2Y);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy2X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy3X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy4X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but1);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but2);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but3);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but4);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but5);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but6);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but7);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but8);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.motorEnable);
    Serial.println();
    #endif

    #ifdef debugRSelectMillis
    //Serial.print(" currentMillisBodyCalib: ");
    //Serial.print(currentMillisBodyCalib);
    Serial.print(" setCalibMillis: ");
    Serial.print(setCalibMillis);
    Serial.print(" motorEnable: ");
    Serial.print(recFromRemote.motorEnable);
    Serial.print(" bodyCalibState: ");
    Serial.print(bodyCalibState);
    Serial.print(" bodyStatus: ");
    Serial.print(sendToRemote.bodyStatus);
    Serial.print(" countdown: ");
    Serial.print(countdown);
    #endif

    #ifdef printSoundPin && AudioHardware == JoeWired
    Serial.print(F(" Pin1: "));
    Serial.print(digitalRead(soundpin1));
    Serial.print(F(" Pin2: "));
    Serial.print(digitalRead(soundpin2));
    Serial.print(F(" Pin3: "));
    Serial.print(digitalRead(soundpin3));
    Serial.print(F(" Pin4: "));
    Serial.print(digitalRead(soundpin4));
    Serial.print(F(" Pin5: "));
    Serial.print(digitalRead(soundpin5));
    Serial.print(F(" Pin6: "));
    Serial.print(digitalRead(soundpin6));
    Serial.print(F(" soundState: "));
    Serial.print(soundState);
    Serial.print(F(" readPinState: "));
    Serial.print(digitalRead(readpin));
    Serial.print(F(" randSoundPin: "));
    Serial.println(randSoundPin);
    #endif

    #ifdef debugFlywheelSpin
    Serial.print(F(" Joy: "));
    Serial.print(flywheelStickPtr->GetMappedValue());
    Serial.print(F(" flywheelRotation: "));
    Serial.println(flywheelRotation);
    #endif

    #ifdef debugAutoDisable
    Serial.print(F("isAutoDisabled: "));
    Serial.print(autoDisable.isAutoDisabled);
    Serial.print(", forcedMotorEnable: ");
    Serial.print(autoDisable.forcedMotorEnable);
    Serial.println();
    #endif

    #ifdef debugCalibration
    Serial.print(F("bodyStatus: "));
    Serial.print(sendToRemote.bodyStatus);
    Serial.println();
    #endif
}