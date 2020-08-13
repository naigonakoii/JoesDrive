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
//                         Scott DeBoer
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


#include "Arduino.h"

#include "Constants.h"
#include "DriveSetup.h"
#include "Enums.h"
#include "Structs.h"

//
// External library includes. These are included in the /ext folder, but you'll need to install them into the Arduino
// Library folder (Documents/Arduino/Libraries in Windows).
//
#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include <EasyTransfer.h>
#include <PID_v1.h> //PID loop from http://playground.arduino.cc/Code/PIDLibrary

//
// Ensure HeadTiltVersion is only set to MK2_Dome or MK3_Dome.
//
#ifndef HeadTiltVersion
#error Please define 'HeadTiltVersion' in DriveSetup.h to either 'MK3_Dome' or 'MK2_Dome'.
#elif HeadTiltVersion != MK3_Dome && HeadTiltVersion != MK2_Dome
#error The define 'HeadTiltVersion' must be set to either 'MK3_Dome' or 'MK2_Dome'.
#endif

#if HeadTiltVersion == MK3_Dome
#include <VarSpeedServo.h>
#endif

//
// Ensure AudioHardware is set to an appropriate value.
//
#ifndef AudioHardware
#error Please define 'AudioHardware' in DriveSetup.h to either 'JoeSerial', 'NECWired', or 'NECWireless'.
#elif AudioHardware != JoeSerial && AudioHardware != NECWired && AudioHardware != NECWireless
#error The define 'AudioHardware' in DriveSetup.h must be set to either 'JoeSerial', 'NECWired', or 'NECWireless'.
#endif

//
// Ensure RemoteHardware is set to an appropriate value.
//
#ifndef RemoteHardware
#error Please define 'RemoteHardware' in DriveSetup.h to either 'BTRemote' or 'FeatherPair'.
#elif RemoteHardware != BTRemote && RemoteHardware != FeatherPair
#error The define 'RemoteHardware' in DriveSetup.h must be set to either 'BTRemote' or 'FeatherPair'.
#endif

#ifndef IMUSerialPort
#error Please define 'IMUSerialPort in DriveSetup.h to either 2 or 3 to match your hardware.
#elif IMUSerialPort != 2 && IMUSerialPort != 3
#error The define 'IMUSerialPort' in DriveSetup.h must be set to either 2 or 3.
#endif


//
// These are my libraries. Currently I have them living in this project, but the correct way would also be to install
// these into the Arduino libraries folder, and then update these references with angle bracket notation and just the
// file name. Example:
//
//    "src/Libraries/NaigonIO/src/AnalogInHandler.h"
//
// would become
//
//    <AnalogInHandler.h>
//
#include "src/Libraries/NaigonAnimations/src/Animation.h"
#include "src/Libraries/NaigonAnimations/src/AnimationRunner.h"
#include "src/Libraries/NaigonIO/src/AnalogInHandler.h"
#include "src/Libraries/NaigonIO/src/ButtonHandler.h"
#include "src/Libraries/NaigonUtil/src/EaseApplicator.h"
#include "src/Libraries/NaigonSound/src/SoundPlayer.h"

//
// These includes are pretty specific to not only BB-8 but Joe's Drive, and as such are just files alongside the
// project.
#include "ImuProMini.h"
#include "MotorPWM.h"
#include "Offsets.h"

//
// MK3 Head Tilt related includes
//
#if RemoteHardware == FeatherPair
#include "FeatherRemoteReceiver.h"
#endif

//
// Animations using statements
//
using namespace Naigon::Animations::AnimationConstants;
using Naigon::Animations::AnimationAction;
using Naigon::Animations::AnimationRunner;
using Naigon::Animations::AnimationStep;
using Naigon::Animations::AnimationTarget;
using Naigon::Animations::GeneratedAnimation;
using Naigon::Animations::GeneratedAnimationPercents;
using Naigon::Animations::IAnimation;
using Naigon::Animations::ScriptedAnimation;

//
// Joe's Drive Specific
//
using Naigon::BB_8::ImuProMini;
using Naigon::BB_8::MotorPWM;
using Naigon::BB_8::Offsets;

//
// MK3 Head Tilt related
//
using Naigon::BB_8::FeatherRemoteReceiver;

//
// Naigon Audio
//
using Naigon::NECAudio::ISoundPlayer;
using Naigon::NECAudio::SoundTypes;
using Naigon::NECAudio::SoundMapper;
using Naigon::NECAudio::WiredSoundPlayer;

//
// I/O Wrappers
//
using Naigon::IO::AnalogInHandler;
using Naigon::IO::ButtonHandler;
using Naigon::IO::ButtonState;

//
// Ease Applicators
//
using Naigon::Util::FunctionEaseApplicator;
using Naigon::Util::FunctionEaseApplicatorType;
using Naigon::Util::IEaseApplicator;
using Naigon::Util::LinearEaseApplicator;
using Naigon::Util::ScalingEaseApplicator;

EasyTransfer RecRemote;
EasyTransfer SendRemote;
EasyTransfer RecIMU;

RECEIVE_DATA_STRUCTURE_REMOTE recFromRemote;
SEND_DATA_STRUCTURE_REMOTE sendToRemote;
RECEIVE_DATA_STRUCTURE_IMU recIMUData;

DriveState drive;
AnimationStateVars animation;
AutoDisableState autoDisable;
AudioParams audio;


// Define the AnimationRunner instance as external, as it will be actually instanciated in the Animations.ino file.
extern AnimationRunner animationRunner;
extern uint16_t AutomatedDomeSpinId;
extern uint16_t AutomatedDomeServoId;

// Naigon - Button Handling
// NOTE - should implement for all cases where using buttons in Joe's code.
ButtonHandler button1Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button2Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button3Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button4Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button5Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button6Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button7Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button8Handler(0 /* onVal */, 1000 /* heldDuration */);

// Naigon - Analog Input Refactor
// Refactor code similar to ButtonHandler to have analog input handlers for joystick axis and pots.
AnalogInHandler driveStickHandlerSlow(0, 512, reverseDrive, -MaxDriveSlow, MaxDriveSlow, 2.0f);
AnalogInHandler driveStickHandlerMed(0, 512, reverseDrive, -MaxDriveMed, MaxDriveMed, 2.0f);
AnalogInHandler driveStickHandlerFast(0, 512, reverseDrive, -MaxDriveFast, MaxDriveFast, 2.0f);
AnalogInHandler sideToSideStickHandler(0, 512, reverseS2S, -MaxSideToSide, MaxSideToSide, 2.0f);
#if HeadTiltVersion == MK2_Dome
AnalogInHandler domeTiltStickHandler(0, 512, reverseDomeTilt, -MaxDomeTiltAngle, MaxDomeTiltAngle, 2.0f);
#else
AnalogInHandler domeTiltStickHandlerFR(0, 512, reverseDomeTilt, -MaxDomeTiltX, MaxDomeTiltX, 2.0f);
#endif
AnalogInHandler domeTiltStickHandlerLR(0, 512, reverseDomeTilt, -MaxDomeTiltY, MaxDomeTiltY, 2.0f);
AnalogInHandler domeSpinStickHandler(0, 512, reverseDomeSpin, -MaxDomeSpin, MaxDomeSpin, 15.0f);
AnalogInHandler domeSpinAutoStickHandler(0, 512, reverseDomeSpin, -MaxDomeSpinAuto, MaxDomeSpinAuto, 15.0f);
AnalogInHandler domeServoStickHandler(0, 512, reverseDomeSpin, -MaxDomeSpinServo, MaxDomeSpinServo, 15.0f);
AnalogInHandler domeServoAutoStickHandler(0, 512, reverseDomeSpin, -MaxDomeServoAuto, MaxDomeServoAuto, 15.0f);
AnalogInHandler flywheelStickHandler(0, 512, reverseFlywheel, -MaxFlywheelDrive, MaxFlywheelDrive, 50.0f);
// Pots
AnalogInHandler sideToSidePotHandler(0, 1024, reverseS2SPot, -MaxS2SPot, MaxS2SPot, 0.0f);
AnalogInHandler domeSpinPotHandler(0, 1024, reverseDomeSpinPot, -MaxDomeSpinPot, MaxDomeSpinPot, 0.0f);
#if HeadTiltVersion == MK2_Dome
AnalogInHandler domeTiltPotHandler(0, 1024, reverseDomeTiltPot, -MaxHeadTiltPot, MaxHeadTiltPot, 0.0f);
#endif
// Pointers to logical mappings. Different drive modes switch these.
AnalogInHandler *driveStickPtr = &driveStickHandlerSlow;
AnalogInHandler *sideToSideStickPtr = &sideToSideStickHandler;
#if HeadTiltVersion == MK2_Dome
AnalogInHandler *domeTiltStickPtr = &domeTiltStickHandler;
#else
AnalogInHandler *domeTiltStickPtr = &domeTiltStickHandlerFR;
#endif
AnalogInHandler *domeTiltStickLRPtr = &domeTiltStickHandlerLR;
AnalogInHandler *domeSpinStickPtr = &domeSpinStickHandler;
AnalogInHandler *flywheelStickPtr = &flywheelStickHandler;


// Naigon - Ease Applicator
// Refactor code to use the new PWM driver.
//
MotorPWM drivePwm(drivePWM1, drivePWM2, 0, 2);
MotorPWM sideToSidePWM(s2sPWM1, s2sPWM2, MaxSideToSide, 1);
MotorPWM domeSpinPWM(domeSpinPWM1, domeSpinPWM2, 0, 20);
MotorPWM domeServoPWM(domeSpinPWM1, domeSpinPWM2, 0, 4);
MotorPWM flywheelPWM(flywheelSpinPWM1, flywheelSpinPWM2, 0, 35);
#if HeadTiltVersion == MK2_Dome
MotorPWM headTiltPWM(domeTiltPWM1, domeTiltPWM2, HeadTiltPotThresh, 0);
#endif

// Naigon - Ease Applicator
// Refactor the code to use the new IEaseApplicator instances.
//
FunctionEaseApplicator driveApplicatorSlow(0.0, 20.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorMed(0.0, 28.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorHigh(0.0, 38.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorWiggle(0.0, 5.0, 0.1, FunctionEaseApplicatorType::Quadratic);
// Naigon - Ease Applicator: for the drive this pointer will always be the one in use.
FunctionEaseApplicator *driveApplicatorPtr = &driveApplicatorSlow;
ScalingEaseApplicator sideToSideEase(0.0, easeS2S, easeMsS2SA, easeMsS2SD, 20);
ScalingEaseApplicator domeServoEase(0.0, easeDomeServo, easeDomeServoMsA, easeDomeServoMsD, 20);
LinearEaseApplicator domeSpinEase(0.0, easeDome);
LinearEaseApplicator flywheelEase(0.0, easeFlywheel);
#if HeadTiltVersion == MK2_Dome
ScalingEaseApplicator domeTiltEase(0.0, easeDomeTilt, easeDomeTiltMsA, easeDomeTiltMsD, 20);
#else
ScalingEaseApplicator domeTiltEaseFR(0.0, easeDomeMK3, easeDomeTiltMsA, easeDomeTiltMsD, 20);
ScalingEaseApplicator domeTiltEaseLR(0.0, easeDomeMK3, easeDomeTiltMsA, easeDomeTiltMsD, 20);
#endif

ImuProMini imu;
Offsets offsets;

#if AudioHardware == NECWired
//
// Naigon - NEC Audio.
//
SoundMapper mapper(
    HappySoundPin,
    SadSoundPin,
    ExcitedSoundPin,
    ScaredSoundPin,
    ChattySoundPin,
    AgitatedSoundPin,
    PlayTrackPin,
    StopTrackPin);
ISoundPlayer *soundPlayer;
#endif

SoundTypes forcedSoundType = SoundTypes::NotPlaying;

#if RemoteHardware == FeatherPair
// For feather remotes.
FeatherRemoteReceiver featherRemotes(recDelay);
#endif

#if HeadTiltVersion == MK3_Dome
VarSpeedServo leftServo;
VarSpeedServo rightServo;
#endif

// For the voltage divider.
float R1 = resistor1;
float R2 = resistor2;
unsigned long lastBatteryUpdate;

// Drive motor
int driveSpeed;

unsigned long lastLoopMillis;

PIDVals s2sTiltVals;
PID PID1(&s2sTiltVals.input, &s2sTiltVals.output, &s2sTiltVals.setpoint, Pk1, Ik1, Dk1, DIRECT);

PIDVals s2sServoVals;
PID PID2(&s2sServoVals.input, &s2sServoVals.output, &s2sServoVals.setpoint, Pk2, Ik2, Dk2, DIRECT); // PID Setup - S2S stability

PIDVals driveVals;
PID PID3(&driveVals.input, &driveVals.output, &driveVals.setpoint, Pk3, Ik3, Dk3, DIRECT); // Main drive motor

PIDVals domeTiltVals;
PID PID4(&domeTiltVals.input, &domeTiltVals.output, &domeTiltVals.setpoint, Pk4, Ik4, Dk4, DIRECT);

PIDVals domeServoVals;
PID PID5(&domeServoVals.input, &domeServoVals.output, &domeServoVals.setpoint, Pk5, Ik5, Dk5, DIRECT);

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial3.begin(115200);

    #if HeadTiltVersion == MK3_Dome
    leftServo.attach(leftDomeTiltServo);
    rightServo.attach(rightDomeTiltServo);
    leftServo.write(95,50, false); 
    rightServo.write(90, 50, false);
    #endif

    // Naigon - I changed Serial3 for the IMU, so swap these in code.
    RecRemote.begin(details(recFromRemote), &Serial1);
    SendRemote.begin(details(sendToRemote), &Serial1);

    #if IMUSerialPort == 3
    RecIMU.begin(details(recIMUData), &Serial3);
    #else
    RecIMU.begin(details(recIMUData), &Serial2);
    #endif

    pinMode(enablePin, OUTPUT);     // enable pin
    pinMode(enablePinDome, OUTPUT); // enable pin for dome spin

    #if RemoteHardware == BTRemote
    pinMode(BTstatePin, INPUT);     // BT paired status
    #elseif RemoteHardware == FeatherPair
    
    #endif

    #if AudioHardware == JoeWired
    pinMode(readpin, INPUT_PULLUP); // read stat of Act on Soundboard
    pinMode(soundpin1, OUTPUT);     // play sound from pin 0 on Soundboard
    pinMode(soundpin2, OUTPUT);     // play sound from pin 1 on Soundboard
    pinMode(soundpin3, OUTPUT);     // play sound from pin 2 on Soundboard
    pinMode(soundpin4, OUTPUT);     // play sound from pin 3 on Soundboard
    pinMode(soundpin5, OUTPUT);     // play sound from pin 4 on Soundboard
    pinMode(soundpin6, OUTPUT);     // play sound from pin 4 on Soundboard
    #elif AudioHardware == NECWired
    digitalWrite(soundpin6, HIGH);
    digitalWrite(soundpin5, HIGH);
    digitalWrite(soundpin4, HIGH);
    digitalWrite(soundpin3, HIGH);
    digitalWrite(soundpin2, HIGH);
    digitalWrite(soundpin1, HIGH);

    pinMode(HappySoundPin, OUTPUT);
    pinMode(SadSoundPin, OUTPUT);
    pinMode(ExcitedSoundPin, OUTPUT);
    pinMode(ScaredSoundPin, OUTPUT);
    pinMode(ChattySoundPin, OUTPUT);
    pinMode(AgitatedSoundPin, OUTPUT);
    pinMode(PlayTrackPin, OUTPUT);
    pinMode(StopTrackPin, OUTPUT);

    // Setup the soundPlayer as the one with a wired interface.
    soundPlayer = new WiredSoundPlayer(mapper, 200);
    soundPlayer->ClearSounds();
    #endif

    // *********** PID setup ***********

    PID1.SetMode(AUTOMATIC); // PID Setup -  S2S SERVO
    PID1.SetOutputLimits(-255, 255);
    PID1.SetSampleTime(15);

    PID2.SetMode(AUTOMATIC); // PID Setup -  S2S Stability
    PID2.SetOutputLimits(-255, 255);
    PID2.SetSampleTime(15);

    PID3.SetMode(AUTOMATIC); // PID Setup - main drive motor
    PID3.SetOutputLimits(-255, 255);
    PID3.SetSampleTime(15);

    PID4.SetMode(AUTOMATIC); // PID Setup - dome tilt
    PID4.SetOutputLimits(-255, 255);
    PID4.SetSampleTime(15);

    PID5.SetMode(AUTOMATIC);
    PID5.SetOutputLimits(-255, 255); // PID Setup - dome spin 'servo'
    PID5.SetSampleTime(15);

    // Load the values stored from the EEPROM.
    offsets.LoadOffsetsFromMemory();

    // Always startup on slow speed.
    sendToRemote.bodyMode = BodyMode::Slow;

    drive.PreviousNormalMode = BodyMode::Slow;
    drive.PreviousAnimationMode = BodyMode::AutomatedServo;

    // TODO - could make this based on when a button is pressed or a connection to make it more random.
    randomSeed(millis());
}

//     ================================================================================================================
//     ================================================================================================================
//     =====                                            LOOP, bruh!                                               =====
//     ================================================================================================================
//     ================================================================================================================
void loop()
{
    RecIMU.receiveData();

    #if RemoteHardware == FeatherPair
    featherRemotes.UpdateIteration(&RecRemote);
    #endif

    if (millis() - lastLoopMillis >= 20)
    {
        sendAndReceive();

        updateInputHandlers();
        setOffsetsAndSaveToEEPROM();
        updateBodyMode();
        reverseDirection();

        updateAnimations();
        sounds();
        readVin();

        bodyCalib();
        domeCalib();
        //debugRoutines();

        movement();
        lastLoopMillis = millis();
    }

    #if RemoteHardware == FeatherPair
    sendDriveData();
    #endif
}

// ====================================================================================================================
// Functions
// ====================================================================================================================

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send and receive all data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendAndReceive()
{
    #if RemoteHardware == BTRemote
    RecRemote.receiveData();
    SendRemote.sendData();
    BTenable();
    #endif

    imu.UpdateIteration(recIMUData.pitch, recIMUData.roll, recIMUData.IMUloop);
}

void sendDriveData()
{
    if (featherRemotes.ReceivedData())
    {
        SendRemote.sendData();
        delay(5);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read voltage in
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readVin()
{
    if(millis() - lastBatteryUpdate >= 15000){
      lastBatteryUpdate = millis();
      sendToRemote.bodyBatt = ((analogRead(battMonitor) * outputVoltage) / 1024.0) / (R2 / (R1 + R2));
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bluetooth enable
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if RemoteHardware == BTRemote
void BTenable()
{

    int btState = digitalRead(BTstatePin); //read whether BT is connected
    drive.IsConnected = btState == 1;

    if (recFromRemote.motorEnable == 0 && btState == 1)
    { //if motor enable switch is on and BT connected, turn on the motor drivers
        autoDisableMotors();
        digitalWrite(enablePinDome, HIGH);
    }
    else if (recFromRemote.motorEnable == 1 || btState == 0)
    { //if motor enable switch is off OR BT disconnected, turn off the motor drivers
        digitalWrite(enablePin, LOW);
        digitalWrite(enablePinDome, LOW);
        // TODO - I think there was a bug here, as in Joe's code this was 0, but from rest of code 0 seems to mean
        // false.
        autoDisable.isAutoDisabled = true;
        autoDisable.autoDisableDoubleCheck = 0;
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update values for the current iteration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateInputHandlers()
{
    // Naigon - Safe Joystick Button Toggle
    // This method needs to be before body mode selection. However, this method depends on the body mode. This just
    // means that the values will be one iteration behind the body mode, which is okay in my opinion.
    //

    // Update button state for the rest of the functions.
    button1Handler.UpdateState(recFromRemote.but1);
    button2Handler.UpdateState(recFromRemote.but2);
    button3Handler.UpdateState(recFromRemote.but3);
    button4Handler.UpdateState(recFromRemote.but4);
    button5Handler.UpdateState(recFromRemote.but5);
    button6Handler.UpdateState(recFromRemote.but6);
    button7Handler.UpdateState(recFromRemote.but7);
    button8Handler.UpdateState(recFromRemote.but8);

    int dr, s, dt, ds, fl, lr;

    if (sendToRemote.bodyMode == BodyMode::PushToRoll)
    {
        // Naigon - Safe Joystick Button Toggle: Refactored here due to Analog Joystick Refactor
        // Safe mode will keep the default 255 and will not read from the actual sticks.
        dr = s = dt = ds = fl = lr = 255;
    }
    else
    {
        #if HeadTiltVersion == MK3_Dome
        dr = drive.IsStationary ? 255 : recFromRemote.Joy1Y;
        s = animation.IsAutomation ? recFromRemote.Joy2X : recFromRemote.Joy1X;
        dt = animation.IsAutomation ? 255 : recFromRemote.Joy2Y;
        lr = animation.IsAutomation ? 255 : recFromRemote.Joy2X;
        ds = animation.IsAutomation ? 255 : recFromRemote.Joy4X;
        fl = drive.IsStationary ? recFromRemote.Joy1X : recFromRemote.Joy3X;
        #else
        dr = drive.IsStationary ? 255 : recFromRemote.Joy1Y;
        s = animation.IsAutomation ? recFromRemote.Joy2X : recFromRemote.Joy1X;
        dt = animation.IsAutomation ? 255 : recFromRemote.Joy2Y;
        lr = 255;
        ds = animation.IsAutomation ? 255 : recFromRemote.Joy2X;
        fl = drive.IsStationary ? recFromRemote.Joy1X : recFromRemote.Joy3X;
        #endif
    }

    // Joysticks
    driveStickPtr->UpdateState(dr);
    sideToSideStickPtr->UpdateState(s);
    domeSpinStickPtr->UpdateState(ds);
    flywheelStickPtr->UpdateState(fl);
    domeTiltStickPtr->UpdateState(dt);
    domeTiltStickLRPtr->UpdateState(lr);

    // Pots
    #if HeadTiltVersion == MK2_Dome
    domeTiltPotHandler.UpdateState(analogRead(domeTiltPotPin));
    #endif

    domeSpinPotHandler.UpdateState(analogRead(domeSpinPotPin));
    sideToSidePotHandler.UpdateState(analogRead(S2SpotPin));
}

void updateBodyMode()
{
    //
    // Increment runs through automation and normal modes separately.
    //
    if (animation.IsAutomation)
    {
        incrementBodyModeToggle(FirstAutomatedEntry, LastAutomatedEntry);
    }
    else
    {
        incrementBodyModeToggle(FirstSpeedEntry, LastSpeedEntry);
    }

    if (sendToRemote.bodyMode == BodyMode::Medium)
    {
        driveApplicatorPtr = &driveApplicatorMed;
        driveStickPtr = &driveStickHandlerMed;
    }
    else if (sendToRemote.bodyMode == BodyMode::Fast)
    {
        driveApplicatorPtr = &driveApplicatorHigh;
        driveStickPtr = &driveStickHandlerFast;
    }
    else if (sendToRemote.bodyMode == BodyMode::Stationary)
    {
        // For safety, set the drive speed back to slow, even though the stick shouldn't use it.
        driveApplicatorPtr = &driveApplicatorWiggle;
        driveStickPtr = &driveStickHandlerSlow;
    }
    else
    {
        // For all other modes, just default to the slow speed as it honestly works the best.
        driveApplicatorPtr = &driveApplicatorSlow;
        driveStickPtr = &driveStickHandlerSlow;
    }

    driveSpeed = driveApplicatorPtr->GetMaxValue();

    // Indicate if the droid is in stationary mode only when in the stationary state.
    drive.IsStationary = sendToRemote.bodyMode == BodyMode::Stationary;

    DomeMode lastDomeMode = drive.CurrentDomeMode;
    // Naigon - Dome Modes
    // The dome servo or normal state is now parsed from the bodyMode.
    drive.CurrentDomeMode = sendToRemote.bodyMode == BodyMode::Servo
        || sendToRemote.bodyMode == BodyMode::ServoWithTilt
        || sendToRemote.bodyMode == BodyMode::AutomatedServo
            ? DomeMode::ServoMode
            : DomeMode::FullSpinMode;

    if (animation.IsAutomation && lastDomeMode != drive.CurrentDomeMode)
    {
        animationRunner.StopCurrentAnimation();
    }

    // Naigon
    // When switching into servo mode, mark that it centering. This will prevent the spin from being too high.
    drive.IsDomeCentering = lastDomeMode != DomeMode::ServoMode && drive.CurrentDomeMode == DomeMode::ServoMode
        ? true
        : drive.IsDomeCentering;

    // Naigon - Analog Input Refactor
    // Swap the rotation handler depending on if the drive is in servo mode or not.
    domeSpinStickPtr = recFromRemote.motorEnable == 0
        && (drive.CurrentDomeMode == DomeMode::ServoMode || drive.IsDomeCentering)
        ? &domeServoStickHandler
        : &domeSpinStickHandler;

    // Naigon - Automation
    // Swap the stick handler for automation as well as very fast dome spin during automation can result in head pops.
    //
    if (animationRunner.IsRunning() && animation.UseReducedDomeStick)
    {
        domeSpinStickPtr = drive.CurrentDomeMode == DomeMode::FullSpinMode
            ? &domeSpinAutoStickHandler
            : &domeServoAutoStickHandler;
    }
}

void incrementBodyModeToggle(uint8_t firstEntry, uint8_t lastEntry)
{
    if (button1Handler.GetState() != ButtonState::Pressed
        || sendToRemote.bodyStatus != BodyStatus::NormalOperation
        // Naigon - Safe Joystick Button Toggle
        || driveStickPtr->HasMovement()
        || sideToSideStickPtr->HasMovement()
        || flywheelStickPtr->HasMovement()
        || domeTiltStickPtr->HasMovement()
        || domeTiltStickLRPtr->HasMovement()
        || domeSpinStickPtr->HasMovement())
    {
        // Only increment when the button was pressed and released, and the joystick was (mostly) centered.
        return;
    }

    //
    // Naigon - Drive-side (Server-side) Refactor
    // This method was taken directly from the remote code and ported here. In general, the drive should do all state
    // changes and should be the state "master", while the remote ect should just send commands. This will allow
    // secondary controllers to operate on the drive and the drive can maintain a state.
    //
    sendToRemote.bodyMode =
        sendToRemote.bodyMode >= lastEntry || sendToRemote.bodyMode == BodyMode::UnknownSpeed
            ? firstEntry
            : sendToRemote.bodyMode + 1;
}

void reverseDirection()
{
    // Naigon - Safe Joystick Button Toggle.
    //
    // I've had some pretty catastropic issues where I accidentally hit reverse when driving and didn't realize it.
    // This is because the reverse is pressing the drive stick.
    //
    // To prevent that, I'm only going to accept the input when joysticks are below a threshold.
    if (button5Handler.GetState() == ButtonState::Pressed
        && !driveStickPtr->HasMovement()
        && !sideToSideStickPtr->HasMovement()
        && !domeTiltStickPtr->HasMovement()
        && !domeTiltStickLRPtr->HasMovement()
        && !domeSpinStickPtr->HasMovement()
        && !flywheelStickPtr->HasMovement())
    {
        sendToRemote.bodyDirection = sendToRemote.bodyDirection == Direction::Forward
            ? Direction::Reverse
            : Direction::Forward;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update animations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateAnimations()
{
    //
    // First, turn off any animations that are running if the drive was disabled, and abandon since animations are only
    // allowed if the drive is enabled.
    //
    if (recFromRemote.motorEnable != 0)
    {
        animationRunner.StopCurrentAnimation();
        return;
    }

    bool stopAutomation = false;

    //
    // Next, see if automation mode is toggled in or out
    //
    if (button8Handler.GetState() == ButtonState::Pressed)
    {
        // Button 6 toggles animations. Mark if leaving the mode as a stop of running animations is required there.
        stopAutomation = animation.IsAutomation;
        animation.IsAutomation = animation.IsAutomation ? false : true;

        if (animation.IsAutomation)
        {
            drive.PreviousNormalMode = (BodyMode)sendToRemote.bodyMode;
            sendToRemote.bodyMode = drive.PreviousAnimationMode;
        }
        else
        {
            drive.PreviousAnimationMode = (BodyMode)sendToRemote.bodyMode;
            sendToRemote.bodyMode = drive.PreviousNormalMode;
        }
    }

    //
    // Now that current animations are handled, set an animation based on button inputs or the current mode.
    if (button4Handler.GetState() == ButtonState::Pressed)
    {
        // Since Bank2 contains all the scripted exact ones, it's important to do those in order. That allows knowing
        // when one will come, and prevents duplication since they are canned.
        animationRunner.StartNextAutomation(AnimationTarget::Bank2);
        animation.IsAnimationRunning = true;
    }
    else if (button4Handler.GetState() == ButtonState::Held)
    {
        animationRunner.SelectAndStartAnimation(AnimationTarget::Bank4);
        animation.IsAnimationRunning = true;
    }
    else if (button6Handler.GetState() == ButtonState::Pressed)
    {
        animationRunner.SelectAndStartAnimation(AnimationTarget::Bank3);
        animation.IsAnimationRunning = true;
    }
    else if (animation.IsAutomation && !animationRunner.IsRunning())
    {
        uint16_t id = drive.CurrentDomeMode == DomeMode::ServoMode
            ? AutomatedDomeServoId
            : AutomatedDomeSpinId;
        animationRunner.StartAnimationWithId(id);
        animation.IsAnimationRunning = true;
    }
    else if (stopAutomation && animationRunner.IsRunning())
    {
        animationRunner.StopCurrentAnimation();
    }

    //
    // Next, actually handle any updates from currently running animations.
    //
    if (animationRunner.IsRunning())
    {
        const AnimationStep* aStep = animationRunner.RunIteration();

        if (!domeSpinStickPtr->HasMovement())
        {
            // Dome spin is allowed when driving as it is pretty safe.
            domeSpinStickPtr->UpdateState(aStep->GetMotorControlValue(MotorControlId::idDomeSpin));
        }
        if (!domeTiltStickPtr->HasMovement()
            && !driveStickPtr->HasMovement())
        {
            domeTiltStickPtr->UpdateState(aStep->GetMotorControlValue(MotorControlId::idDomeTiltFR));
        }
        if (!domeTiltStickLRPtr->HasMovement()
            && !driveStickPtr->HasMovement())
        {
            domeTiltStickLRPtr->UpdateState(aStep->GetMotorControlValue(MotorControlId::idDomeTiltLR));
        }
        if (!sideToSideStickPtr->HasMovement()
            && !driveStickPtr->HasMovement())
        {
            sideToSideStickPtr->UpdateState(aStep->GetMotorControlValue(MotorControlId::idSideToSide));
        }
        if (!flywheelStickPtr->HasMovement()
            && !driveStickPtr->HasMovement())
        {
            // TODO - Consider sliding the scale if flywheel values are too low.
            flywheelStickPtr->UpdateState(aStep->GetMotorControlValue(MotorControlId::idFlywheel));
        }
        forcedSoundType = (SoundTypes)(((int8_t)aStep->GetSoundId()) - 1);

        // Extract the metadata
        AnimationMetadata *metadata = static_cast<AnimationMetadata*>(aStep->GetMetadata());
        animation.AnimationDomeMode = metadata->domeMode;
        animation.UseReducedDomeStick = metadata->useReducedStick;

        if (metadata->domeMode == DomeMode::ServoMode && drive.CurrentDomeMode != DomeMode::ServoMode)
        {
            // Make the dome as centering so that when it enters an animation it doesn't spin too quickly.
            drive.IsDomeCentering = true;
        }
    }
    else if (animation.IsAnimationRunning)
    {
        // There was an active animation running, but now it is no longer running. Temporarily run head servo mode
        // until the head is back into position.
        drive.IsDomeCentering = drive.CurrentDomeMode == DomeMode::ServoMode ? false : true;
        animation.IsAnimationRunning = false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sounds
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sounds()
{
    #if AudioHardware == JoeSerial
    // TODO - Joe's sound player.
    #else
    soundsNEC();
    #endif
}

#if AudioHardware == JoeSerial
#else
void soundsNEC()
{
    //
    // Naigon - NEC Audio
    // This method handles sending sound to the custom Naigon's Electronic Creations gen 3 BB-8 sound player.
    //
    bool played = false;

    if (forcedSoundType != SoundTypes::NotPlaying)
    {
        soundPlayer->PlaySound(forcedSoundType);
        played = true;
    }
    else if (
        (button2Handler.GetState() == ButtonState::Pressed || button2Handler.GetState() == ButtonState::Held)
        && soundPlayer->SoundTypeCurrentlyPlaying() == SoundTypes::NotPlaying)
    {
        // Button will only do a sound that is deemed "happy".
        int randomType = random(0, 3) * 2;
        if (button2Handler.GetState() == ButtonState::Held)
        {
            randomType += 1;
        }
        soundPlayer->PlaySound((SoundTypes)randomType);
        played = true;
    }
    else if (button3Handler.GetState() == ButtonState::Pressed
        && soundPlayer->TrackTypeCurrentlyPlaying() == SoundTypes::NotPlaying)
    {
        soundPlayer->PlaySound(SoundTypes::PlayTrack);
        played = true;
    }
    else if (button3Handler.GetState() == ButtonState::Held
        && soundPlayer->TrackTypeCurrentlyPlaying() != SoundTypes::StopTrack)
    {
        soundPlayer->PlaySound(SoundTypes::StopTrack);
        played = true;
    }

    if (!played)
    {
        // If nothing was played, then update this loop with nothing.
        soundPlayer->PlaySound(SoundTypes::NotPlaying);
    }
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calibration methods
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bodyCalib()
{
    #if HeadTiltVersion == MK2_Dome
    int domeTiltFR = (int)domeTiltPotHandler.GetMappedValue();
    #else
    int domeTiltFR = 0;
    #endif

    if (sendToRemote.bodyStatus == BodyStatus::NormalOperation
        && button8Handler.GetState() == ButtonState::Held
        && button7Handler.GetState() == ButtonState::NotPressed)
    {
        sendToRemote.bodyStatus = BodyStatus::BodyCalibration;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::BodyCalibration
        && button8Handler.GetState() == ButtonState::Pressed)
    {
        offsets.UpdateOffsets(
            imu.Pitch(),
            imu.Roll(),
            (int)sideToSidePotHandler.GetMappedValue(),
            domeTiltFR);
    }
    else if (sendToRemote.bodyStatus == BodyStatus::BodyCalibration
        && button7Handler.GetState() == ButtonState::Pressed)
    {
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dome calibration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void domeCalib()
{
    if (sendToRemote.bodyStatus == BodyStatus::NormalOperation
        && button1Handler.GetState() == ButtonState::Held)
    {
        sendToRemote.bodyStatus = BodyStatus::DomeCalibration;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::DomeCalibration
        && button8Handler.GetState() == ButtonState::Pressed)
    {
        offsets.UpdateDomeOffset(
            (int)domeSpinPotHandler.GetMappedValue(),
            sendToRemote.bodyDirection == Direction::Reverse);
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::DomeCalibration
        && button7Handler.GetState() == ButtonState::Pressed)
    {
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motor movement
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void movement()
{
    if (recFromRemote.motorEnable == 0 && drive.IsConnected && imu.ProMiniConnected())
    {
        unsigned long currentMillis = millis();

        if (sendToRemote.bodyMode == BodyMode::PushToRoll)
        {
            // Naigon - Safe Mode
            // Only move the main drive as s2s and dome could be compromised (flywheel unneeded).
            // The active stabilization will allow pushing the ball to the desired orientation for access.
            mainDrive(driveApplicatorPtr);
            turnOffAllTheThings(false /*disableDrive*/);
        }
        else
        {
            // Normal modes do all the things.
            sideTilt(&sideToSideEase);
            mainDrive(driveApplicatorPtr);

            #if HeadTiltVersion == MK2_Dome
            domeTiltMK2(&domeTiltEase);
            #else
            domeTiltMK3(&domeTiltEaseFR, &domeTiltEaseLR);
            #endif

            flywheelSpin(&flywheelEase);
        }
    }
    else
    {
        turnOffAllTheThings(true /*disableDrive*/);
    }

    DomeMode currentDomeMode =
        animationRunner.IsRunning() && animation.AnimationDomeMode != DomeMode::UnspecifiedDomeSpin
            ? animation.AnimationDomeMode
            : drive.CurrentDomeMode;

    if ((currentDomeMode == DomeMode::ServoMode || drive.IsDomeCentering)
        && !drive.AutoDisable
        && recFromRemote.motorEnable == 0)
    {
        domeSpinServo(&domeServoEase, drive.IsDomeCentering);
    }
    else if (currentDomeMode == DomeMode::FullSpinMode || drive.AutoDisable || recFromRemote.motorEnable == 1)
    {
        domeSpin(&domeSpinEase);
    }

    // Naigon - Animations
    // Stop forcing the head servo mode now that it is back into position.
    if (drive.IsDomeCentering && abs(domeServoVals.output) < 2)
    {
        drive.IsDomeCentering = false;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motor Drivers
//
// The following group of functions are for driving the output to the motors.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------
// Main drive
// ------------------------------------------------------------------------------------
void mainDrive(IEaseApplicator *easeApplicatorPtr)
{
    // Naigon - Stationary/Wiggle Mode
    // When in wiggle/stationary mode, don't use the joystick to move at all.
    int joystickDrive = (int)driveStickPtr->GetMappedValue();

    driveVals.setpoint = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(joystickDrive),
        -MaxDrive,
        MaxDrive);

    driveVals.input = (imu.Pitch() + offsets.PitchOffset()); // - domeOffset;
    // domeTiltOffset used to keep the ball from rolling when dome is tilted front/back

    PID3.Compute();
    writeMotorPwm(drivePwm, driveVals.output, 0 /*input*/, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Side to Side
// ------------------------------------------------------------------------------------

//
//s2s left joystick goes from 0(LEFT) to 512(RIGHT)
//The IMU roll should go DOWN as it tilts to the right, and UP as it tilts to the left
//The side to side pot should go UP as the ball tilts left, and LOW as it tilts right
//
void sideTilt(IEaseApplicator *easeApplicatorPtr)
{
    int joystickS2S = (int)sideToSideStickPtr->GetMappedValue();

    // Setpoint will increase/decrease by S2SEase each time the code runs until it matches the joystick. This slows the side to side movement.
    s2sServoVals.setpoint = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(joystickS2S),
        -MaxSideToSide,
        MaxSideToSide);

    int S2Spot = (int)sideToSidePotHandler.GetMappedValue();
    s2sServoVals.input = imu.Roll() + offsets.RollOffset();
    
    PID2.Compute(); //PID2 is used to control the 'servo' control of the side to side movement.

    s2sTiltVals.input = S2Spot + offsets.SideToSidePotOffset();
    s2sTiltVals.setpoint = map(
        constrain(s2sServoVals.output, -MaxSideToSide, MaxSideToSide),
        -MaxSideToSide,
        MaxSideToSide,
        MaxSideToSide,
        -MaxSideToSide);
    PID1.Compute(); //PID1 is for side to side stabilization
    writeMotorPwm(sideToSidePWM, s2sTiltVals.output, s2sTiltVals.input, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome tilt
// ------------------------------------------------------------------------------------
#if HeadTiltVersion == MK2_Dome
void domeTiltMK2(IEaseApplicator *easeApplicatorPtr)
{
    //
    //The joystick will go from 0(Forward) to 512(Back).
    //The pot will get HIGH as it moves back, and LOW as it moves forward
    //

    BodyMode bodyM = (BodyMode)sendToRemote.bodyMode;
    // speedDomeTilt offsets the dome based on the main drive to tilt it in the direction of movement.
    // Naigon - Dome Modes: Add the tilt based on the main stick if the mode uses tilt.
    double speedDomeTilt =
        (driveVals.setpoint >= 3 || driveVals.setpoint <= -3)
        && (bodyM == BodyMode::SlowWithTilt || bodyM == BodyMode::ServoWithTilt)
            ? (double)driveStickPtr->GetMappedValue() * (double)DomeTiltAmount / (double)driveSpeed
            : 0.0;

#ifdef HeadTiltStabilization
    // Naigon - Head Tilt Stabilization
    // Calculate the pitch to input into the head tilt input in order to keep it level.
    // Naigon - TODO: once the ease applicator is created, use it here to increment to pitch adjust.
    int pitchAdjust = sendToRemote.bodyMode != BodyMode::PushToRoll
        ? (imu.Pitch() + offsets.PitchOffset()) * HeadTiltPitchAndRollProportion
        : 0;
#else
    int pitchAdjust = 0;
#endif

    int domeTiltPot = (int)domeTiltPotHandler.GetMappedValue() + offsets.DomeTiltPotOffset();

    // Naigon - Dome Automation
    // Dome tilt is completely controlled by automation.
    int ch3Val = (int)domeTiltStickPtr->GetMappedValue();

    // Naigon: BUG
    // Joe's code had a bug here; you need to subtract within the constrain, otherwise driving can cause this value to go
    // outside the bounds and really bad things happen like the drive locking and losing the head.
    int joystickDome = constrain(
        ch3Val - (int)speedDomeTilt - pitchAdjust,
        -MaxDomeTiltAngle,
        MaxDomeTiltAngle); // Reading the stick for angle -40 to 40

    domeTiltVals.input = domeTiltPot + (imu.Pitch() + offsets.PitchOffset());
    domeTiltVals.setpoint = easeApplicatorPtr->ComputeValueForCurrentIteration(joystickDome);
    domeTiltVals.setpoint = constrain(domeTiltVals.setpoint, -MaxDomeTiltAngle, MaxDomeTiltAngle);
    PID4.Compute();

    writeMotorPwm(headTiltPWM, domeTiltVals.output, domeTiltPot, false /*requireBT*/, false /*requireMotorEnable*/);
}
#else
void domeTiltMK3(IEaseApplicator *easeApplicatorFRPtr, IEaseApplicator *easeApplicatorLRPtr)
{
    #ifdef HeadTiltStabilization
    // Naigon - Head Tilt Stabilization
    // Calculate the pitch to input into the head tilt input in order to keep it level.
    // Naigon - TODO: once the ease applicator is created, use it here to increment to pitch adjust.
    int pitchAdjust = sendToRemote.bodyMode != BodyMode::PushToRoll
        ? (imu.Pitch() + offsets.PitchOffset()) * HeadTiltPitchAndRollProportion
        : 0;

    int rollAdjust = sendToRemote.bodyMode != BodyMode::PushToRoll
        ? (imu.Roll() + offsets.RollOffset()) * HeadTiltPitchAndRollProportion
        : 0;
    #else
    int pitchAdjust = 0;
    #endif

    int joyX = constrain(
        domeTiltStickPtr->GetMappedValue(),
        -MaxDomeTiltX,
        MaxDomeTiltX);

    int joyY = constrain(
        domeTiltStickLRPtr->GetMappedValue(),
        -MaxDomeTiltY,
        MaxDomeTiltY);

    //
    // This block of code changes the pitch based on if the drive is rolling or not.
    // TODO - Figure out if this is needed with testing, as my work to add the tilt may fix it.
    //
    //if(Setpoint3 >= 2 || Setpoint3 <= -2)
    //{
    //    Joy2YPitch = Joy2YDirection + pitch;
    //}
    //else
    //{
    //    Joy2YPitch = Joy2YDirection - pitchOffset;
    //}

    int joy2XEaseMap = easeApplicatorFRPtr->ComputeValueForCurrentIteration(joyX);
    int joy2YEaseMap = easeApplicatorFRPtr->ComputeValueForCurrentIteration(joyY);

    int joy2Ya, joy2XLowOffset, joy2XHighOffset;
    if(joy2YEaseMap < 0)
    {
        joy2Ya = map(joy2YEaseMap, -20, 0, 70, 0);
        joy2XLowOffset = map(joy2Ya, 1, 70, -15, -50);
        joy2XHighOffset = map(joy2Ya, 1, 70, 30, 20);
    }
    else if(joy2YEaseMap > 0)
    {
        joy2Ya = map(joy2YEaseMap, 0, 24, 0, -80); 
        joy2XLowOffset = map(joy2Ya, -1, -80, -15, 10);
        joy2XHighOffset = map(joy2Ya, -1, -80, 30, 90);
    }
    else
    {
        joy2Ya = 0;
    }

    int joy2XLowOffsetA, joy2XHighOffsetA, servoLeft, servoRight;

    if(joy2XEaseMap > 0)
    {
        joy2XLowOffsetA = map(joy2XEaseMap, 0, 18, 0, joy2XLowOffset);
        joy2XHighOffsetA = map(joy2XEaseMap, 0, 18, 0, joy2XHighOffset);
        servoLeft = joy2Ya + joy2XHighOffsetA;
        servoRight = joy2Ya + joy2XLowOffsetA;
    }
    else if(joy2XEaseMap < 0)
    {
        joy2XLowOffsetA = map(joy2XEaseMap, -18, 0, joy2XLowOffset, 0);
        joy2XHighOffsetA = map(joy2XEaseMap, -18, 0, joy2XHighOffset, 0);
        servoRight = joy2Ya + joy2XHighOffsetA;
        servoLeft = joy2Ya + joy2XLowOffsetA;
    }
    else
    {
        joy2XHighOffsetA = 0;
        joy2XLowOffsetA = 0; 
        servoRight = joy2Ya;
        servoLeft = joy2Ya;
    }
   
    leftServo.write(constrain(map(servoLeft, -90, 90, 0, 180),0, 180) + 5, domeSpeed, false); 
    rightServo.write(constrain(map(servoRight,-90, 90, 180, 0), 0, 180), domeSpeed, false);
}
#endif

// ------------------------------------------------------------------------------------
// Dome spin - Full Spin
// ------------------------------------------------------------------------------------
void domeSpin(IEaseApplicator *easeApplicatorPtr)
{
    int domeRotation = (int)domeSpinStickPtr->GetMappedValue();

    int currentDomeSpeed = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(domeRotation),
        -255,
        255);

    // Joe has always allowed the dome to spin regardless of whether the motors were enabled or not, which I like.
    writeMotorPwm(domeSpinPWM, currentDomeSpeed, 0 /*input*/, true /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin - Servo
// ------------------------------------------------------------------------------------
void domeSpinServo(IEaseApplicator *easeApplicatorPtr, bool isCentering)
{
    int ch4Servo = (int)domeSpinStickPtr->GetMappedValue();

    domeServoVals.input = sendToRemote.bodyDirection == Direction::Forward
        ? (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpinPotOffset() - 180
        : (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpinPotOffset();

    if (domeServoVals.input <= -180)
    {
        domeServoVals.input += 360;
    }
    else if (domeServoVals.input >= 180)
    {
        domeServoVals.input -= 360;
    }

    domeServoVals.setpoint = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(ch4Servo),
        -MaxDomeSpinServo,
        MaxDomeSpinServo);
    PID5.Compute();

    // Naigon - Animations
    // To prevent the head from spinning really fast at the end of an animation when re-centering, scale the pwm speed.
    domeServoVals.output = isCentering ? domeServoVals.output * 3 / 4 : domeServoVals.output;

    writeMotorPwm(domeServoPWM, domeServoVals.output, 0, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Flywheel spin
// ------------------------------------------------------------------------------------
void flywheelSpin(IEaseApplicator *easeApplicatorPtr)
{
    // Naigon - Stationary/Wiggle Mode
    // When in stationary mode, use the drive stick as the flywheel, as the drive is disabled.
    int ch5PWM = (int)flywheelStickPtr->GetMappedValue();

    int flywheelRotation = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(ch5PWM),
        -255,
        255);

    writeMotorPwm(flywheelPWM, flywheelRotation, 0 /*input*/, true /*requireBT*/, true /*requireMotorEnable*/);
}

void writeMotorPwm(MotorPWM &motorPwm, int output, int input, bool requireBT, bool requireMotorEnable)
{
    if ((requireBT == true && !drive.IsConnected)
        || (requireMotorEnable == true && recFromRemote.motorEnable != 0))
    {
        motorPwm.WriteZeros();
    }
    else
    {
        motorPwm.WritePWM(output, input);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Write to EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsAndSaveToEEPROM()
{
    #if HeadTiltVersion == MK2_Dome
    int domeTiltPot = (int)domeTiltPotHandler.GetMappedValue();
    #else
    int domeTiltPot = 0;
    #endif

    if (!offsets.AreValuesLoaded())
    {
        offsets.UpdateOffsets(
            imu.Pitch(),
            imu.Roll(),
            (int)sideToSidePotHandler.GetMappedValue(),
            domeTiltPot);

        offsets.UpdateDomeOffset(
            (int)domeSpinPotHandler.GetMappedValue(),
            sendToRemote.bodyDirection == Direction::Reverse);
    }

    offsets.WriteOffsets();

    if (!offsets.NeedsWrite())
    {
        // Update the body status back to normal once all is saved.
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Disable droid
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turnOffAllTheThings(bool includingDrive)
{
    //disables all PIDS and movement. This is to avoid any sudden jerks when re-enabling motors.
    s2sServoVals.input = 0;
    s2sServoVals.setpoint = 0;
    s2sServoVals.output = 0;
    s2sTiltVals.input = 0;
    s2sTiltVals.setpoint = 0;
    s2sTiltVals.output = 0;
    domeTiltVals.input = 0;
    domeTiltVals.setpoint = 0;
    domeTiltVals.output = 0;

    if (includingDrive)
    {
        driveVals.input = 0;
        driveVals.setpoint = 0;
        driveVals.output = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auto disable motors
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void autoDisableMotors()
{
    double output1A = abs(s2sTiltVals.output);
    double output3A = abs(driveVals.output);

    if (
        !driveStickPtr->HasMovement()
        && !sideToSideStickPtr->HasMovement()
        // Naigon - Head Tilt Stabilization: Add a bit more tolerance here since the stabilization adds to the stick.
        && !domeTiltStickPtr->HasMovement()
        && !domeTiltStickLRPtr->HasMovement()
        && !domeSpinStickPtr->HasMovement()
        && !flywheelStickPtr->HasMovement()
        && (!autoDisable.isAutoDisabled))
    {
        autoDisable.autoDisableMotorsMillis = millis();
        autoDisable.isAutoDisabled = true;
    }
    else if (
        driveStickPtr->HasMovement()
        || sideToSideStickPtr->HasMovement()
        // Naigon - Head Tilt Stabilization: Add a bit more tolerance here since the stabilization adds to the stick.
        || domeTiltStickPtr->HasMovement()
        || domeTiltStickLRPtr->HasMovement()
        || domeSpinStickPtr->HasMovement()
        || flywheelStickPtr->HasMovement()
        || autoDisable.forcedMotorEnable == true)
    {
        autoDisable.isAutoDisabled = false;
        digitalWrite(enablePin, HIGH);
        autoDisable.autoDisableDoubleCheck = 0;
        drive.AutoDisable = false;
        autoDisable.forcedMotorEnable = false;
    }

    if (autoDisable.isAutoDisabled
        && (millis() - autoDisable.autoDisableMotorsMillis) >= (unsigned long)AutoDisableMS
        && (output1A <= S2SOutThresh && output3A <= DriveOutThresh))
    {
        digitalWrite(enablePin, LOW);
        drive.AutoDisable = true;
    }
    else if (output1A > 50 || output3A > 20)
    {
        autoDisable.isAutoDisabled = false;
        digitalWrite(enablePin, HIGH);
        autoDisable.autoDisableDoubleCheck = 0;
        drive.AutoDisable = false;
    }
    else if ((output1A > S2SOutThresh || output3A > DriveOutThresh) && autoDisable.autoDisableDoubleCheck == 0)
    {
        autoDisable.autoDisableDoubleCheckMillis = millis();
        autoDisable.autoDisableDoubleCheck = 1;
    }
    else if ((autoDisable.autoDisableDoubleCheck == 1) && (millis() - autoDisable.autoDisableDoubleCheckMillis >= 100))
    {
        if (output1A > S2SOutThresh || output3A > DriveOutThresh)
        {
            autoDisable.isAutoDisabled = false;
            digitalWrite(enablePin, HIGH);
            autoDisable.autoDisableDoubleCheck = 0;
            drive.AutoDisable = false;
        }
        else
        {
            autoDisable.autoDisableDoubleCheck = 0;
        }
    }
}
