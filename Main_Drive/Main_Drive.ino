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

//
// External library includes. These are included in the /ext folder, but you'll need to install them into the Arduino
// Library folder (Documents/Arduino/Libraries in Windows).
//
#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include <EasyTransfer.h>
#include <PID_v1.h> //PID loop from http://playground.arduino.cc/Code/PIDLibrary

#include "Constants.h"
#include "Enums.h"
#include "Structs.h"

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
// Animations Usings
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

using Naigon::BB_8::ImuProMini;
using Naigon::BB_8::MotorPWM;
using Naigon::BB_8::Offsets;

using Naigon::NECAudio::ISoundPlayer;
using Naigon::NECAudio::SoundTypes;
using Naigon::NECAudio::SoundMapper;
using Naigon::NECAudio::WiredSoundPlayer;

using Naigon::IO::AnalogInHandler;
using Naigon::IO::ButtonHandler;
using Naigon::IO::ButtonState;

using Naigon::Util::FunctionEaseApplicator;
using Naigon::Util::FunctionEaseApplicatorType;
using Naigon::Util::IEaseApplicator;
using Naigon::Util::LinearEaseApplicator;

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
AnalogInHandler domeTiltStickHandler(0, 512, reverseDomeTilt, -MaxDomeTiltAngle, MaxDomeTiltAngle, 2.0f);
AnalogInHandler domeRotationStickHandler(0, 512, reverseDomeSpin, -255, 255, 15.0f);
AnalogInHandler domeServoStickHandler(0, 512, reverseDomeSpin, -MaxDomeSpinServo, MaxDomeSpinServo, 15.0f);
AnalogInHandler flywheelStickHandler(0, 512, reverseFlywheel, -MaxFlywheelDrive, MaxFlywheelDrive, 50.0f);
// Pots
AnalogInHandler sideToSidePotHandler(0, 1024, reverseS2SPot, -MaxS2SPot, MaxS2SPot, 0.0f);
AnalogInHandler domeTiltPotHandler(0, 1024, reverseDomeTiltPot, -MaxHeadTiltPot, MaxHeadTiltPot, 0.0f);
AnalogInHandler domeSpinPotHandler(0, 1024, reverseDomeSpinPot, -MaxDomeSpinPot, MaxDomeSpinPot, 0.0f);
// Pointers to logical mappings. Different drive modes switch these.
AnalogInHandler *driveStickPtr = &driveStickHandlerSlow;
AnalogInHandler *sideToSideStickPtr = &sideToSideStickHandler;
AnalogInHandler *domeTiltStickPtr = &domeTiltStickHandler;
AnalogInHandler *domeSpinStickPtr = &domeRotationStickHandler;
AnalogInHandler *flywheelStickPtr = &flywheelStickHandler;


// Naigon - Ease Applicator
// Refactor code to use the new PWM driver.
MotorPWM drivePwm(drivePWM1, drivePWM2, 0, 2);
MotorPWM sideToSidePWM(s2sPWM1, s2sPWM2, MaxSideToSide, 1);
MotorPWM headTiltPWM(domeTiltPWM1, domeTiltPWM2, HeadTiltPotThresh, 0);
MotorPWM domeSpinPWM(domeSpinPWM1, domeSpinPWM2, 0, 20);
MotorPWM domeServoPWM(domeSpinPWM1, domeSpinPWM2, 0, 4);
MotorPWM flywheelPWM(flywheelSpinPWM1, flywheelSpinPWM2, 0, 35);
// Naigon - Ease Applicator
// Refactor the code to use the new IEaseApplicator instances.
//
FunctionEaseApplicator driveApplicatorSlow(0.0, 20.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorMed(0.0, 28.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorHigh(0.0, 38.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorWiggle(0.0, 5.0, 0.1, FunctionEaseApplicatorType::Quadratic);
// Naigon - Ease Applicator: for the drive this pointer will always be the one in use.
FunctionEaseApplicator *driveApplicatorPtr = &driveApplicatorSlow;
FunctionEaseApplicator sideToSideEaseApplicator(0.0, MaxSideToSide, S2SEase, FunctionEaseApplicatorType::SCurve);
FunctionEaseApplicator domeTiltEaseApplicator(0.0, MaxDomeTiltAngle, easeDomeTilt, FunctionEaseApplicatorType::SCurve);
FunctionEaseApplicator domeServoEaseApplicator(0.0, MaxDomeSpinServo, easeDomeServo, FunctionEaseApplicatorType::SCurve);
FunctionEaseApplicator domeSpinEaseApplicator(0.0, 255, easeDome, FunctionEaseApplicatorType::SCurve);
LinearEaseApplicator flywheelEaseApplicator(0.0, flywheelEase);
// Use different dome spin ease under automation to prevent it from being to jerky which can cause the head to pop loose.
FunctionEaseApplicator automatedDomeSpinEaseApplicator(0.0, 255, easeDomeAuto, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator automatedDomeServoEaseApplicator(0.0, MaxDomeSpinServo, easeDomeServoAuto, FunctionEaseApplicatorType::Quadratic);

ImuProMini imu;
Offsets offsets;

#ifdef WirelessSound
// In the future if an XBee is hooked to the Arduino Mega the hard-wired pins will not be needed.
#else
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

// For the voltage divider.
float R1 = resistor1;
float R2 = resistor2;
unsigned long lastBatteryUpdate;

// Drive motor
int driveSpeed;

unsigned long lastLoopMillis;

int BTstate = 0;

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

    // Naigon - I changed Serial3 for the IMU, so swap these in code.
    RecRemote.begin(details(recFromRemote), &Serial1);
    SendRemote.begin(details(sendToRemote), &Serial1);
    RecIMU.begin(details(recIMUData), &Serial3);

    pinMode(enablePin, OUTPUT);     // enable pin
    pinMode(enablePinDome, OUTPUT); // enable pin for dome spin
    pinMode(BTstatePin, INPUT);     // BT paired status

    pinMode(readpin, INPUT_PULLUP); // read stat of Act on Soundboard
    pinMode(soundpin1, OUTPUT);     // play sound from pin 0 on Soundboard
    pinMode(soundpin2, OUTPUT);     // play sound from pin 1 on Soundboard
    pinMode(soundpin3, OUTPUT);     // play sound from pin 2 on Soundboard
    pinMode(soundpin4, OUTPUT);     // play sound from pin 3 on Soundboard
    pinMode(soundpin5, OUTPUT);     // play sound from pin 4 on Soundboard
    pinMode(soundpin6, OUTPUT);     // play sound from pin 4 on Soundboard

#ifdef WirelessSound
    // TODO - put the wireless sound setup here.
#else
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
#endif

    soundPlayer->ClearSounds();

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

    if (millis() - lastLoopMillis >= 20)
    {
        sendAndReceive();
        BTenable();

        updateInputHandlers();
        setOffsetsAndSaveToEEPROM();
        updateBodyMode();
        reverseDirection();

        updateAnimations();
        handleSounds();
        readVin();

        bodyCalib();
        domeCalib();
        //debugRoutines();

        movement();
        lastLoopMillis = millis();
    }
}

// ====================================================================================================================
// Functions
// ====================================================================================================================

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send and receive all data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendAndReceive()
{
    RecRemote.receiveData();
    SendRemote.sendData();
    imu.UpdateIteration(recIMUData.pitch, recIMUData.roll, recIMUData.IMUloop);
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
void BTenable()
{
    BTstate = digitalRead(BTstatePin); //read whether BT is connected

    if (recFromRemote.motorEnable == 0 && BTstate == 1)
    { //if motor enable switch is on and BT connected, turn on the motor drivers
        autoDisableMotors();
        digitalWrite(enablePinDome, HIGH);
    }
    else if (recFromRemote.motorEnable == 1 || BTstate == 0)
    { //if motor enable switch is off OR BT disconnected, turn off the motor drivers
        digitalWrite(enablePin, LOW);
        digitalWrite(enablePinDome, LOW);
        // TODO - I think there was a bug here, as in Joe's code this was 0, but from rest of code 0 seems to mean
        // false.
        autoDisable.isAutoDisabled = true;
        autoDisable.autoDisableDoubleCheck = 0;
    }
}

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

    int dr, s, dt, ds, fl;

    if (sendToRemote.bodyMode == BodyMode::PushToRoll)
    {
        // Naigon - Safe Joystick Button Toggle: Refactored here due to Analog Joystick Refactor
        // Safe mode will keep the default 255 and will not read from the actual sticks.
        dr = s = dt = ds = fl = 255;
    }
    else
    {
        dr = drive.IsStationary ? 255 : recFromRemote.Joy1Y;
        s = animation.IsAutomation ? recFromRemote.Joy2X : recFromRemote.Joy1X;
        dt = animation.IsAutomation ? 255 : recFromRemote.Joy2Y;
        ds = animation.IsAutomation ? 255 : recFromRemote.Joy2X;
        fl = drive.IsStationary ? recFromRemote.Joy1X : recFromRemote.Joy3X;
    }

    // Joysticks
    driveStickPtr->UpdateState(dr);
    sideToSideStickPtr->UpdateState(s);
    domeTiltStickPtr->UpdateState(dt);
    domeSpinStickPtr->UpdateState(ds);
    flywheelStickPtr->UpdateState(fl);

    // Pots
    domeTiltPotHandler.UpdateState(analogRead(domeTiltPotPin));
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

    // Naigon - Dome Modes
    // The dome servo or normal state is now parsed from the bodyMode.
    DomeMode lastMode = drive.CurrentDomeMode;
    drive.CurrentDomeMode = sendToRemote.bodyMode == BodyMode::Servo
        || sendToRemote.bodyMode == BodyMode::ServoWithTilt
        || sendToRemote.bodyMode == BodyMode::AutomatedServo
            ? DomeMode::ServoMode
            : DomeMode::FullSpinMode;

    if (drive.CurrentDomeMode != lastMode && animation.IsAutomation)
    {
        // Switched automation modes, so stop the current animation to get the correct new one.
        animationRunner.StopCurrentAnimation();
    }

    // Naigon - Analog Input Refactor
    // Swap the rotation handler depending on if the drive is in servo mode or not.
    domeSpinStickPtr = recFromRemote.motorEnable == 0 && drive.CurrentDomeMode == DomeMode::ServoMode
        ? &domeServoStickHandler
        : &domeRotationStickHandler;
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
    // I've had some pretty catastropic issues where I accidently hit reverse when driving and didn't realize it. This
    // is because the reverse is pressing the drive stick.
    //
    // To prevent that, I'm only going to accept the input when joysticks are below a threshold.
    if (button5Handler.GetState() == ButtonState::Pressed
        && !driveStickPtr->HasMovement()
        && !sideToSideStickPtr->HasMovement()
        && !domeTiltStickPtr->HasMovement()
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
        sendToRemote.bodyMode = animation.IsAutomation
            ? FirstAutomatedEntry
            : FirstSpeedEntry;
    }

    //
    // Now that we know the drive is enabled, set an animation based on button inputs or the current mode.
    //
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

        // Set the dome mode this animation requires.
        // TODO - Refactor animations to not have DomeMode, but a pointer to state.
        animation.AnimatedDomeMode = *static_cast<DomeMode*>(aStep->GetMetadata());
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
/*
void sounds() {
  if ((recFromRemote.but2 == 0 || recFromRemote.but7 == 0) && recFromRemote.but8 == 1 && readPinState == 1 && BTstate ==1 && soundState == 0 && playSound == 0) {   
    playSound = 1;
  }///*else if (recFromRemote.but2 == 1 && recFromRemote.but7 == 1 && playSound == 0) {
    //soundState=0;
    //digitalWrite((soundPins[0]), HIGH);
    //digitalWrite((soundPins[1]), HIGH);
    //digitalWrite((soundPins[2]), HIGH);
    //digitalWrite((soundPins[3]), HIGH);
    //digitalWrite((soundPins[4]), HIGH);
  //}//
          
  if ((recFromRemote.but3 == 0) && (readPinState == 1) && (BTstate ==1)) {
    musicState = 1;
    musicStateMillis = millis();
    digitalWrite(soundpin6, LOW);
  }
  else if (recFromRemote.but3 == 1) {
    digitalWrite(soundpin6, HIGH);
  }

  if(playSound == 1) {
    randSoundPin = random(0, 5);
    digitalWrite((soundPins[randSoundPin]), LOW);
    soundMillis = millis();
    playSound = 2;
  }
  else if(playSound == 2 && (millis() - soundMillis > 200)) {
    digitalWrite((soundPins[randSoundPin]), HIGH);
    playSound = 0;
  }
}
*/
void handleSounds()
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calibration methods
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bodyCalib()
{
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
            (int)domeTiltPotHandler.GetMappedValue());
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
    if (recFromRemote.motorEnable == 0 && BTstate == 1 && imu.ProMiniConnected())
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
            sideTilt(&sideToSideEaseApplicator);
            mainDrive(driveApplicatorPtr);
            domeTilt(&domeTiltEaseApplicator);
            flywheelSpin(&flywheelEaseApplicator);
        }
    }
    else
    {
        turnOffAllTheThings(true /*disableDrive*/);
    }

    // Naigon - Animation
    // Animation can override the dome spin mode.
    //
    DomeMode currentDomeMode = animationRunner.IsRunning() && animation.AnimatedDomeMode != DomeMode::UnspecifiedDomeSpin
        ? animation.AnimatedDomeMode
        : drive.CurrentDomeMode;

    if ((currentDomeMode == DomeMode::ServoMode || drive.IsDomeCentering)
        && !drive.AutoDisable
        && recFromRemote.motorEnable == 0)
    {
        IEaseApplicator *domeEaseApplicatorPtr = animationRunner.IsRunning() || drive.IsDomeCentering
            ? &automatedDomeServoEaseApplicator
            : &domeServoEaseApplicator;
        domeSpinServo(domeEaseApplicatorPtr);
    }
    else if (currentDomeMode == DomeMode::FullSpinMode || drive.AutoDisable || recFromRemote.motorEnable == 1)
    {
        IEaseApplicator *domeEaseApplicatorPtr = animationRunner.IsRunning() && recFromRemote.motorEnable == 0
            ? &automatedDomeSpinEaseApplicator
            : &domeSpinEaseApplicator;
        domeSpin(domeEaseApplicatorPtr);
    }

    // Naigon - Animations
    // Stop forcing the head servo mode now that it is back into position.
    if (drive.IsDomeCentering && abs(domeServoVals.output) < 15) { drive.IsDomeCentering = false; }
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
void domeTilt(IEaseApplicator *easeApplicatorPtr)
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

// ------------------------------------------------------------------------------------
// Dome spin - Manual
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
void domeSpinServo(IEaseApplicator *easeApplicatorPtr)
{
    int ch4Servo = (int)domeSpinStickPtr->GetMappedValue();

    domeServoVals.input = sendToRemote.bodyDirection == Direction::Forward
        ? (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpinPotOffset() - 180
        : (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpinPotOffset();

    if (domeServoVals.input < -180)
    {
        domeServoVals.input += 360;
    }
    else if (domeServoVals.input > 180)
    {
        domeServoVals.input -= 360;
    }

    domeServoVals.setpoint = constrain(
        easeApplicatorPtr->ComputeValueForCurrentIteration(ch4Servo),
        -MaxDomeSpinServo,
        MaxDomeSpinServo);
    PID5.Compute();

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
    if ((requireBT == true && BTstate != 1)
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
    if (!offsets.AreValuesLoaded())
    {
        offsets.UpdateOffsets(
            imu.Pitch(),
            imu.Roll(),
            (int)sideToSidePotHandler.GetMappedValue(),
            (int)domeTiltPotHandler.GetMappedValue());

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
