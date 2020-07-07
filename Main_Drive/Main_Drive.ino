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

#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include <EasyTransfer.h>
#include <PID_v1.h> //PID loop from http://playground.arduino.cc/Code/PIDLibrary

#include "AnalogInHandler.h"
#include "Animation.h"
//#include "AnimationDefinitions.h"
#include "AnimationRunner.h"
#include "ButtonHandler.h"
#include "Constants.h"
#include "EaseApplicator.h"
#include "MotorPWM.h"
#include "SoundPlayer.h"

using namespace NaigonBB8::AnimationConstants;

using NaigonBB8::AnimationAction;
using NaigonBB8::AnimationDomeMode;

// Include each entry to allow short-hand in animation definitions.
using NaigonBB8::AnimationDomeMode::adEither;
using NaigonBB8::AnimationDomeMode::adServo;
using NaigonBB8::AnimationDomeMode::adSpin;

using NaigonBB8::AnimationRunner;
using NaigonBB8::AnimationStep;
using NaigonBB8::AnimationTarget;
using NaigonBB8::FunctionEaseApplicator;
using NaigonBB8::FunctionEaseApplicatorType;
using NaigonBB8::GeneratedAnimation;
using NaigonBB8::GeneratedAnimationPercents;
using NaigonBB8::IAnimation;
using NaigonBB8::LinearEaseApplicator;
using NaigonBB8::MotorPWM;
using NaigonBB8::ScriptedAnimation;
using NaigonBB8::SoundTypes;

// Include each entry to allow short-hand in animation definitions.
using NaigonBB8::SoundTypes::Agitated;
using NaigonBB8::SoundTypes::Chatty;
using NaigonBB8::SoundTypes::Excited;
using NaigonBB8::SoundTypes::Happy;
using NaigonBB8::SoundTypes::NotPlaying;
using NaigonBB8::SoundTypes::Sad;
using NaigonBB8::SoundTypes::Scared;

enum BodyMode : uint8_t
{
    UnknownSpeed = 0,
    Slow = 1,
    SlowWithTilt = 2,
    Servo = 3,
    ServoWithTilt = 4,
    Stationary = 5,
    Medium = 6,
    Fast = 7,
    PushToRoll = 8,
    Automated = 20,
    AutomatedServo = 21,
    AutomatedTilt = 22,
};
const BodyMode FirstSpeedEntry = BodyMode::Slow;
const BodyMode LastSpeedEntry = BodyMode::PushToRoll;
const BodyMode FirstAutomatedEntry = BodyMode::Automated;
const BodyMode LastAutomatedEntry = BodyMode::AutomatedTilt;

enum Direction : uint8_t
{
    UnknownDirection = 0,
    Forward = 1,
    Reverse = 2,
};

// Naigon - Dome Modes
// The following dome modes are inferred from the BodyMode.
enum DomeMode : uint8_t
{
    FullSpinMode = 0,
    ServoMode = 1,
};

//
// Naigon - Drive-side (Server-side) Refactor
// Body status is used as an enum to send to the remote. It is the same variable that Joe was sending; this enum just
// quantifies the values, changes the representation (ie 1 used to be body calibration), and adds the Servo value so
// the remote knows to display servo in the corner.
//
// Naigon - Dome Modes
// Through continued refactoring, BodyStatus now only represents the calibration modes. Dome is combined with the drive
// speed which is now renamed as driveMode.
enum BodyStatus : uint8_t
{
    NormalOperation = 0,
    BodyCalibration = 1,
    DomeCalibration = 2,
};

//
// Naigon - Animations
// Enum that defines the motor control specific to Joe's Drive.
// Drive | Side | DomeTFB | DomeTLR | DomeSpin | Flywhl
enum MotorControlId : uint8_t
{
    idDrive = 0,
    idSideToSide = 1,
    idDomeTiltFR = 2,
    idDomeTiltLR = 3,
    idDomeSpin = 4,
    idFlywheel = 5,
};


EasyTransfer RecRemote;
EasyTransfer SendRemote;
EasyTransfer RecIMU;

struct RECEIVE_DATA_STRUCTURE_REMOTE
{
    int ch1; //right joystick up/down
    int ch2; //right joystick left/right
    int ch3; //left joystick up/down
    int ch4; //left joystick left/right
    int ch5; //back stick left/right
    // but1 (stick 1) from Joe is selecting between dome servo and dome spin
    // Naigon - this now cycles through the combined drive mode and dome mode.
    int but1 = 1; //left select
    // but2 from Joe is audio
    // Naigon - button 2 press plays a happy/neutral sound. Holding plays a longer/sader sound
    int but2 = 1; //left button 1
    // but3 from Joe is audio
    // Naigon - button 3 press starts music, and cycles tracks. Holding stops music.
    int but3 = 1; //left button 2
    // but4 from Joe is to trigger Body/dome lighting changes
    // Naigon - Button 4 TBD
    int but4 = 1; //left button 3
    // but5 (stick 2) toggles fwd/rev
    int but5 = 0; //right select (fwd/rev)
    // but6 from Joe is for switching between drive speeds
    // Naigon - Button 6 is now TBD.
    int but6 = 1; //right button 1
    // but7 from Joe is for body calibration only currently when holding
    int but7 = 1; //right button 2
    // but8 is for select only
    int but8 = 1; //right button 3 (right select)
    int motorEnable = 1;
};
RECEIVE_DATA_STRUCTURE_REMOTE recFromRemote;

struct SEND_DATA_STRUCTURE_REMOTE
{
    double bodyBatt = 0.0;
    double domeBattSend;
    uint8_t bodyStatus = 0;
    uint8_t bodyMode = 0;
    uint8_t bodyDirection = 0;
};
SEND_DATA_STRUCTURE_REMOTE sendToRemote;

struct RECEIVE_DATA_STRUCTURE_IMU
{
    float IMUloop;
    float pitch;
    float roll;
};
RECEIVE_DATA_STRUCTURE_IMU recIMUData;

//
// Naigon - Head Tilt Stabilization
// State for the IMU data. sendAndReceive() is responsible for fetching these.
// To keep the head tilt from being jerky, do some filtering on the pitch and roll over time.
// TODO - Probably should put the filtering on the Arduino Pro Mini that is attached to the IMU.
//
struct IMUData
{
    float Pitch;
    float PitchPrev[PitchAndRollFilterCount];
    float Roll;
    float RollPrev[PitchAndRollFilterCount];
    bool IsFirstPitchAndRoll = true;
};
IMUData imu;

//
// State that persists over multiple Arduino loop() calls for the checkMiniTime method.
//
struct IMUState
{
    float LastLoopMillis;
    int MiniStatus;
};
IMUState imuState;

//
// Dirve state
// State consumed by the movement() method and each motor submethod
//
struct DriveState
{
    // Naigon - Stationary/Wiggle Mode
    // Setting this to true will put the drive in wiggle mode.
    bool IsStationary = false;

    // Setting this will force the dome into servo mode just until the head is centered. This resets animations.
    bool IsDomeCentering = false;

    // Joe - Dome servo or normal mode (was int servoMode in Joe's code)
    DomeMode CurrentDomeMode = DomeMode::FullSpinMode;

    // Joe - Allow motors to power down if droid is sitting still.
    bool AutoDisable;
};
DriveState drive;

//
// Naigon - Animations
// State consumed by the runAnimations() method.
//
struct AnimationStateVars
{
    // Indicates if the drive is in an automated mode; all modes can run automation via a button press, but in an
    // automated mode the animation is run continuously while driving around.
    bool IsAutomation = false;

    // Indicates if any animation is running. This is needed to know when to force the drive into Servo mode when the
    // animationRunner indicates an animation has stopped.
    bool IsAnimationRunning = false;

    // Animations can specify if they are specific for dome spin, dome servo, or if it does not matter. Mark it here
    // so that the mode selection logic can choose the appropriate value.
    AnimationDomeMode AnimatedDomeMode;
};
AnimationStateVars animation;

//
// State that is used by the auto disable method that needs to persist over multiple loop() calls.
//
struct AutoDisableState
{
    unsigned long autoDisableMotorsMillis = 0;
    int autoDisableDoubleCheck;
    unsigned long autoDisableDoubleCheckMillis = 0;
    bool forcedMotorEnable = false;
    bool isAutoDisabled = true;
};
AutoDisableState autoDisable;

//
// Pot and gyro offsets that are used by the movement() methods. These values are also stored into the EEPROM to
// persist even after the drive is shut down.
//
struct MotionOffsets
{
    // Joe's EEPROM vars
    float Pitch;
    float Roll;
    int S2SPot;
    int DomeTiltPot;
    int DomeSpin;
};
MotionOffsets offsets;

//
// Joe's Audio Player
//
struct AudioParams
{
    int readPinState = 1;
    int randSoundPin;
    int soundState;
    int musicState;
    unsigned long musicStateMillis = 0;
    int soundPins[4] = { soundpin1, soundpin2, soundpin3, soundpin4 };
};
AudioParams audio;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Shared Animation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const uint16_t stickFRVals[] =
{
    ForwardFull,
    ForwardThreeFourths,
    ForwardTwoThirds,
    ForwardHalf,
    ReverseHalf,
    ReverseTwoThirds,
    ReverseThreeFourths,
    ReverseFull,
};

const uint16_t stickLRVals[] =
{
    LeftFull,
    LeftThreeFourths,
    LeftTwoThirds,
    LeftHalf,
    RightHalf,
    RightTwoThirds,
    RightThreeFourths,
    RightFull,
};

// Default result that should be passed into all ScriptedAnimation instances.
// -------------------   | Drive   | S2S     | TiltFB  | TiltLR  | Spin    | Flywheel
int defaultStickVals[] = { Centered, Centered, Centered, Centered, Centered, Centered, };
AnimationStep defaultResult(defaultStickVals, 6, SoundTypes::NotPlaying + 1, AnimationDomeMode::adEither, 0);

// Memory space that should be passed into all generated animations
// -------------------   | Drive   | S2S     | TiltFB  | TiltLR  | Spin    | Flywheel
int initialStickVals[] = { Centered, Centered, Centered, Centered, Centered, Centered, };
AnimationStep currentResult(initialStickVals, 6, SoundTypes::NotPlaying + 1, AnimationDomeMode::adEither, 0);

// Array of all motor control Ids that use front/reverse stick.
const uint8_t frStickMotorControlIds[] = { MotorControlId::idDomeTiltFR, };
const uint8_t frStickMotorControlIdsSize = 1;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Continuous generated animations for Automated mode - Bank 1
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const AnimationAction domeActions[] = {
    AnimationAction::PlaySound,
    AnimationAction::MotorControl,
};

const uint16_t domeActPer[] =
{
    55, // Play Sound
    45, // Remaining MotorControl
};

const uint8_t domeMotorControls[] = { MotorControlId::idDomeSpin, MotorControlId::idDomeTiltFR, };
const uint16_t domeMotorControlPer[] = {                      60,                           40, };

const uint16_t millisVals[] = { 100, 250, 350, 500, 750, 1000, };
const uint16_t millisPer[]  = {  10,  20,  20,  25,  15,   10, };

const uint16_t percentDomeStickLR[] { 8, 12,  15, 15, 15, 15, 12, 8, };

const uint16_t percentDomeStickFR[] { 35, 25, 15, 5, 5, 5, 5, 5, };

GeneratedAnimationPercents domeAnimationPercents(
    domeActions,
    domeActPer,
    2 /* actionSize */,
    domeMotorControls,
    domeMotorControlPer,
    2,
    millisVals,
    millisPer,
    6 /* msSize */,
    stickFRVals,
    percentDomeStickFR,
    8 /* frStickSize */,
    stickLRVals,
    percentDomeStickLR,
    8 /* lrStickSize */,
    frStickMotorControlIds,
    frStickMotorControlIdsSize,
    65 /* pausePercent */);

// Since moving just the head is pretty basic, going with full generation here to save variable space.
GeneratedAnimation headMovement(
    AnimationTarget::Bank1,
    &domeAnimationPercents,
    AnimationDomeMode::adEither,
    3 /* minNumAnimationSteps */,
    2 /* maxConcurentActions */,
    NaigonBB8::SoundTypesNumTalking,
    8000 /* soundTimeout */,
    &currentResult);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scripted Animations for Button 4 Press - Bank2.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------  Drive   | Side    | DomeTFB    | DomeTLR | DomeSpin | Flywhl  --------------------------------- 
int bank2A1Step01[6] = { Centered, Centered, ForwardFull, Centered, LeftHalf,  Centered, };
int bank2A1Step02[6] = { Centered, Centered, ForwardFull, Centered, Centered,  Centered, };
int bank2A1Step03[6] = { Centered, Centered, ForwardFull, Centered, Centered,  Centered, };
int bank2A1Step04[6] = { Centered, Centered, ForwardFull, Centered, RightHalf, Centered, };
int bank2A1Step05[6] = { Centered, Centered, ForwardFull, Centered, LeftHalf,  Centered, };
int bank2A1Step06[6] = { Centered, Centered, ForwardHalf, Centered, Centered,  Centered, };
AnimationStep tiltHeadAndLookBothWays1State[] = {
    // --------- MotorVals | nVal | SoundId     | Dome  |   MS
    AnimationStep(bank2A1Step01, 6, NotPlaying + 1, adSpin, 250),
    AnimationStep(bank2A1Step02, 6, Excited + 1,    adSpin,   0),
    AnimationStep(bank2A1Step03, 6, NotPlaying + 1, adSpin, 200),
    AnimationStep(bank2A1Step04, 6, NotPlaying + 1, adSpin, 500),
    AnimationStep(bank2A1Step05, 6, NotPlaying + 1, adSpin, 500),
    AnimationStep(bank2A1Step06, 6, NotPlaying + 1, adSpin, 100),
};
ScriptedAnimation tiltHeadAndLookBothWays1(AnimationTarget::Bank2, 6, &defaultResult, tiltHeadAndLookBothWays1State);

// ----------------     Drive   | Side    | DomeTFB    | DomeTLR | DomeSpin    | Flywhl   -----------------------------
int bank2A2Step01[6] = { Centered, Centered, Centered,    Centered, LeftTwoThirds, Centered, };
int bank2A2Step02[6] = { Centered, Centered, ForwardFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step03[6] = { Centered, Centered, ReverseFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step04[6] = { Centered, Centered, ForwardFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step05[6] = { Centered, Centered, ReverseFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step06[6] = { Centered, Centered, ForwardFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step07[6] = { Centered, Centered, ReverseFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step08[6] = { Centered, Centered, ForwardFull, Centered, LeftTwoThirds, Centered, };
int bank2A2Step09[6] = { Centered, Centered, ReverseFull, Centered, LeftTwoThirds, Centered, };
AnimationStep tiltHeadOppositeWays1State[] = {
    // --------- MotorVals | nVal | SoundId       | Dome  |   MS
    AnimationStep(bank2A2Step01, 6, Chatty + 1,     adServo,   0),
    AnimationStep(bank2A2Step02, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step03, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step04, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step05, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step06, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step07, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step08, 6, NotPlaying + 1, adServo, 500),
    AnimationStep(bank2A2Step09, 6, NotPlaying + 1, adServo, 500),
};
ScriptedAnimation tiltHeadOppositeWays1(AnimationTarget::Bank2, 9, &defaultResult, tiltHeadOppositeWays1State);

// NOTE: The flywheel is somewhat backwards because it is upsidedown on the remote. so even though dome and flywheel
// are the same direction in code, they will spin in opposite directions, which is intended for this animation.
// ----------------     Drive   | Side    | DomeTFB | DomeTLR | DomeSpin | Flywhl  ------------------------------------
int bank2A3Step01[6] = { Centered, Centered, Centered, Centered, LeftHalf,  LeftFull, };
int bank2A3Step02[6] = { Centered, Centered, Centered, Centered, RightFull, RightFull, };
AnimationStep flywheelSpin1State[] = {
    // --------- MotorVals | nVal | SoundId       | Dome  |   MS
    AnimationStep(bank2A3Step01, 6, NotPlaying + 1, adSpin,  500),
    AnimationStep(bank2A3Step02, 6, NotPlaying + 1, adSpin, 3000),
};
ScriptedAnimation flywheelSpin1(AnimationTarget::Bank2, 2, &defaultResult, flywheelSpin1State);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated and scripted animations for Button 6 press - Bank3
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const AnimationAction bank3DomeActions[] = {
    AnimationAction::EndAnimation,
    AnimationAction::PlaySound,
    AnimationAction::MotorControl,
};

const uint16_t bank3DomeActPer[] =
{
    20, // End animation
    40, // Play Sound
    40, // Motor Control
};

const uint8_t bank3MotorControlIds[] =
{
    MotorControlId::idDomeSpin,
    MotorControlId::idDomeTiltFR,
    MotorControlId::idFlywheel,
};

const uint16_t bank3MotorControlPer[] =
{
    38, // Dome Spin
    37, // Tilt Dome
    25, // Flywheel
};

const uint16_t bank3MillisVals[] = { 100, 250, 350, 500, 750, 1000, 1500, };
const uint16_t bank3MillisPer[]  = {  7,   18,  20,  25,  15,   10,    5, };

const uint16_t bank3PercentDomeStickLR[] { 8, 12,  15, 15, 15, 15, 12, 8, };

const uint16_t bank3PercentDomeStickFR[] { 35, 25, 15, 5, 5, 5, 5, 5, };

GeneratedAnimationPercents bank3Percents(
    bank3DomeActions,
    bank3DomeActPer,
    3 /* actionSize */,
    bank3MotorControlIds,
    bank3MotorControlPer,
    3 /* motorControlSize */,
    bank3MillisVals,
    bank3MillisPer,
    7 /* msSize */,
    stickFRVals,
    bank3PercentDomeStickFR,
    8 /* frStickSize */,
    stickLRVals,
    bank3PercentDomeStickLR,
    8 /* lrStickSize */,
    frStickMotorControlIds,
    frStickMotorControlIdsSize,
    15 /* pausePercent */);

// Since moving just the head is pretty basic, going with full generation here to save variable space.
GeneratedAnimation bank3Servo(
    AnimationTarget::Bank3,
    &bank3Percents,
    AnimationDomeMode::adServo,
    4 /* minNumAnimationSteps */,
    4 /* maxConcurentActions */,
    NaigonBB8::SoundTypesNumTalking,
    8000 /* soundTimeout */,
    &currentResult);

GeneratedAnimation bank3Spin(
    AnimationTarget::Bank3,
    &bank3Percents,
    AnimationDomeMode::adSpin,
    4 /* minNumAnimationSteps */,
    4 /* maxConcurentActions */,
    NaigonBB8::SoundTypesNumTalking,
    8000 /* soundTimeout */,
    &currentResult);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animation Runner
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Array of entire animations that will be used to initialize the AnimationRunner in the main file.
const int NumAnimations = 6;
IAnimation *animations[] =
{
    // Bank 1
    &headMovement,

    // Bank 2
    &tiltHeadAndLookBothWays1,
    &tiltHeadOppositeWays1,
    &flywheelSpin1,

    // Bank 3
    &bank3Servo,
    &bank3Spin,
};

// Naigon - Animations
AnimationRunner animationRunner(NumAnimations, animations);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Naigon - Button Handling
// NOTE - should implement for all cases where using buttons in Joe's code.
ButtonHandler button1Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button2Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button3Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button4Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button5Handler(0 /* onVal */, 2000 /* heldDuration */);
ButtonHandler button6Handler(0 /* onVal */, 2000 /* heldDuration */);
ButtonHandler button7Handler(0 /* onVal */, 2000 /* heldDuration */);
ButtonHandler button8Handler(0 /* onVal */, 2000 /* heldDuration */);

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
FunctionEaseApplicator domeServoEaseApplicator(0.0, MaxDomeSpinServo, 5, FunctionEaseApplicatorType::SCurve);
// Naigon - Ease Applicator: motors/modes where it free spins use the normal linear applicator.
LinearEaseApplicator domeSpinEaseApplicator(0.0, easeDome);
LinearEaseApplicator flywheelEaseApplicator(0.0, flywheelEase);

#ifdef WirelessSound
// In the future if an XBee is hooked to the Arduino Mega the hard-wired pins will not be needed.
#else
//
// Naigon - NEC Audio.
//
NaigonBB8::SoundMapper mapper(
    HappySoundPin,
    SadSoundPin,
    ExcitedSoundPin,
    ScaredSoundPin,
    ChattySoundPin,
    AgitatedSoundPin,
    PlayTrackPin,
    StopTrackPin);
NaigonBB8::ISoundPlayer *soundPlayer;
#endif

SoundTypes forcedSoundType = SoundTypes::NotPlaying;

// Save
int SaveToEEPROM;
float R1 = resistor1;
float R2 = resistor2;

// Drive motor
int driveSpeed;

unsigned long lastLoopMillis;

int BTstate = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID1 is for side to side tilt
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk1 = 32.0; // Joe's 13
double Ik1 = 0.0;
// Naigon - Change this value from .3 to .1 or 0 to remove shakey side to side
double Dk1 = 0.0;
double Setpoint1, Input1, Output1;

PID PID1(&Input1, &Output1, &Setpoint1, Pk1, Ik1, Dk1, DIRECT);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID2 is for side to side servo
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk2 = 1.00; // Joe 0.5; M2 Flywheel .4
double Ik2 = 0.00; // was .00
double Dk2 = 0.01; // was .01
double Setpoint2, Input2, Output2;

PID PID2(&Input2, &Output2, &Setpoint2, Pk2, Ik2, Dk2, DIRECT); // PID Setup - S2S stability

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PID3 is for the main drive
// Naigon - MK3 Flywheel
//
// The following values will need to be updated if doing the MK3 flywheel and adding the balancing weights.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk3 = 5.0; // Joe 5.0;
double Ik3 = 0;
double Dk3 = 0;
double Setpoint3, Input3, Output3;

PID PID3(&Input3, &Output3, &Setpoint3, Pk3, Ik3, Dk3, DIRECT); // Main drive motor

//PID4 is for dome tilt fwd/back
// Naigon - adjust for pid dome tilt control
double Pk4 = 6; // default is 6
double Ik4 = 0;
double Dk4 = 0.05;
double Setpoint4, Input4, Output4;

PID PID4(&Input4, &Output4, &Setpoint4, Pk4, Ik4, Dk4, DIRECT);

double Setpoint5a;

//PID5 is for the dome spin servo
double Kp5 = 4, Ki5 = 0, Kd5 = 0;
double Setpoint5, Input5, Output5;

PID PID5(&Input5, &Output5, &Setpoint5, Kp5, Ki5, Kd5, DIRECT);

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
    soundPlayer = new NaigonBB8::WiredSoundPlayer(mapper, 200);
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

    // *********  Read offsets from EEPROM  **********
    offsets.Pitch = EEPROM.readFloat(0);
    offsets.Roll = EEPROM.readFloat(4);
    offsets.S2SPot = EEPROM.readInt(8);
    offsets.DomeTiltPot = EEPROM.readInt(12);
    offsets.DomeSpin = EEPROM.readInt(16);

    if (abs(offsets.Roll) + abs(offsets.Pitch) + abs(offsets.S2SPot) + abs(offsets.DomeTiltPot) == 0)
    {
        setOffsetsONLY();
    }

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
        checkMiniTime();
        BTenable();

        updateInputHandlers();
        updateBodyMode();
        reverseDirection();

        updateAnimations();
        handleSounds();
        psiVal();
        readVin();

        bodyCalib();
        domeCalib();
        debugRoutines();

        setOffsetsAndSaveToEEPROM();
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

    if (recIMUData.IMUloop == 0)
    {
        return;
    }

    imu.Pitch = reversePitch ? recIMUData.pitch * -1 : recIMUData.pitch;
    imu.Roll = reverseRoll ? recIMUData.roll * -1 : recIMUData.roll;

    if (imu.IsFirstPitchAndRoll)
    {
        // Naigon - Head Tilt Stabilization
        // Initialize the first time to the current value to prevent anomilies at startup.
        for (int i = 0; i < 4; i++)
        {
            imu.PitchPrev[i] = imu.Pitch;
            imu.RollPrev[i] = imu.Roll;
        }
        imu.IsFirstPitchAndRoll = false;
    }

    //
    // Naigon - Head Tilt Stablilization
    // The pitch and roll are now computed as a rolling average to filter noise. This prevents jerkyness in any movements
    // based on these values.
    imu.Pitch = updatePrevValsAndComputeAvg(imu.PitchPrev, imu.Pitch);
    imu.Roll = updatePrevValsAndComputeAvg(imu.RollPrev, imu.Roll);
}

/* static */
float updatePrevValsAndComputeAvg(float *nums, float currentVal)
{
    float sum = 0;
    nums[0] = currentVal;
    for (int i = PitchAndRollFilterCount - 1; i >= 1; i -= 1)
    {
        nums[i] = nums[i - 1];
        sum += nums[i];
    }

    return (sum + nums[0]) / (float)PitchAndRollFilterCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MPU6050 stuff
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkMiniTime()
{
    if ((recIMUData.IMUloop == 1 && imuState.LastLoopMillis >= 980)
        || (recIMUData.IMUloop < 1 && imuState.LastLoopMillis > 3))
    {
        imuState.LastLoopMillis = 0;
    }

    if (recIMUData.IMUloop > imuState.LastLoopMillis)
    {
        imuState.LastLoopMillis = recIMUData.IMUloop;
        imuState.MiniStatus = 1;
    }
    else if (recIMUData.IMUloop <= imuState.LastLoopMillis && imuState.MiniStatus != 0)
    {
        imuState.LastLoopMillis++;
    }

    if (recIMUData.IMUloop - imuState.LastLoopMillis < -20 && recIMUData.IMUloop - imuState.LastLoopMillis > -800)
    {
        imuState.MiniStatus = 0;
        imuState.LastLoopMillis = 0;
        recIMUData.IMUloop = 0;
    }

    if (imuState.LastLoopMillis >= 999)
    {
        imuState.LastLoopMillis = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PSI Value
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void psiVal()
{
    // Naigon - Method not needed since dome communicates via XBee to audio sub-board currently.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read voltage in
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readVin()
{
    // Naigon
    // I've been having issues where this reads a constant value of 12.86 regardless of my battery. So I'm going to
    // watch it for a while and try to deduce what is wrong.
    //
    // DO NOT TAKE THIS CHANGE
    //
    sendToRemote.bodyBatt = ((analogRead(battMonitor) * outputVoltage) / 1024.0) / (R2 / (R1 + R2));
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
        dr = drive.IsStationary ? 255 : recFromRemote.ch1;
        s = animation.IsAutomation ? recFromRemote.ch4 : recFromRemote.ch2;
        dt = animation.IsAutomation ? 255 : recFromRemote.ch3;
        ds = animation.IsAutomation ? 255 : recFromRemote.ch4;
        fl = drive.IsStationary ? recFromRemote.ch1 : recFromRemote.ch5;
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
    drive.CurrentDomeMode = sendToRemote.bodyMode == BodyMode::Servo
        || sendToRemote.bodyMode == BodyMode::ServoWithTilt
        || sendToRemote.bodyMode == BodyMode::AutomatedServo
            ? DomeMode::ServoMode
            : DomeMode::FullSpinMode;

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
    // To prevent that, I'm only going to accept the input when the ch1 and ch2 are below a threshold.
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
    else if (button6Handler.GetState() == ButtonState::Pressed)
    {
        animationRunner.SelectAndStartAnimation(AnimationTarget::Bank3);
        animation.IsAnimationRunning = true;
    }
    else if (animation.IsAutomation && !animationRunner.IsRunning())
    {
        animationRunner.SelectAndStartAnimation(AnimationTarget::Bank1);
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
        animation.AnimatedDomeMode = aStep->GetDomeMode();
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
        SaveToEEPROM = 1;
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
        setDomeSpinOffset();
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
    if (recFromRemote.motorEnable == 0 && BTstate == 1 && imuState.MiniStatus != 0)
    {
        unsigned long currentMillis = millis();

        if (sendToRemote.bodyMode == BodyMode::PushToRoll)
        {
            // Naigon - Safe Mode
            // Only move the main drive as s2s and dome could be compromised (flywheel unneeded).
            // The active stabilization will allow pushing the ball to the desired orientation for access.
            mainDrive();
            turnOffAllTheThings(false /*disableDrive*/);
        }
        else
        {
            // Normal modes do all the things.
            sideTilt();
            mainDrive();
            domeTilt();
            flywheelSpin();
        }
    }
    else
    {
        turnOffAllTheThings(true /*disableDrive*/);
    }

    // Naigon - Animation
    // Animation can override the dome spin mode.
    //
    DomeMode currentDomeMode = drive.CurrentDomeMode;
    if (animationRunner.IsRunning() && animation.AnimatedDomeMode != AnimationDomeMode::adEither)
    {
        currentDomeMode = animation.AnimatedDomeMode == AnimationDomeMode::adServo
            ? DomeMode::ServoMode
            : DomeMode::FullSpinMode;
    }

    if ((currentDomeMode == DomeMode::ServoMode || drive.IsDomeCentering)
        && !drive.AutoDisable
        && recFromRemote.motorEnable == 0)
    {
        domeSpinServo();
    }
    else if (currentDomeMode == DomeMode::FullSpinMode || drive.AutoDisable || recFromRemote.motorEnable == 1)
    {
        domeSpin();
    }

    // Naigon - Animations
    // Stop forcing the head servo mode now that it is back into position.
    if (drive.IsDomeCentering && abs(Output5) < 15) { drive.IsDomeCentering = false; }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motor Drivers
//
// The following group of functions are for driving the output to the motors.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------
// Main drive
// ------------------------------------------------------------------------------------
void mainDrive()
{
    // Naigon - Stationary/Wiggle Mode
    // When in wiggle/stationary mode, don't use the joystick to move at all.
    int joystickDrive = (int)driveStickPtr->GetMappedValue();

    Setpoint3 = constrain(
        driveApplicatorPtr->ComputeValueForCurrentIteration(joystickDrive),
        -MaxDrive,
        MaxDrive);

    Input3 = (imu.Pitch + offsets.Pitch); // - domeOffset;
    // domeTiltOffset used to keep the ball from rolling when dome is tilted front/back

    PID3.Compute();
    writeMotorPwm(drivePwm, Output3, 0 /*input*/, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Side to Side
// ------------------------------------------------------------------------------------

//
//s2s left joystick goes from 0(LEFT) to 512(RIGHT)
//The IMU roll should go DOWN as it tilts to the right, and UP as it tilts to the left
//The side to side pot should go UP as the ball tilts left, and LOW as it tilts right
//
void sideTilt()
{
    int joystickS2S = (int)sideToSideStickPtr->GetMappedValue();

    // Setpoint will increase/decrease by S2SEase each time the code runs until it matches the joystick. This slows the side to side movement.
    Setpoint2 = constrain(
        sideToSideEaseApplicator.ComputeValueForCurrentIteration(joystickS2S),
        -MaxSideToSide,
        MaxSideToSide);

    int S2Spot = (int)sideToSidePotHandler.GetMappedValue();
    Input2 = imu.Roll + offsets.Roll;
    
    PID2.Compute(); //PID2 is used to control the 'servo' control of the side to side movement.

    Input1 = S2Spot + offsets.S2SPot;
    Setpoint1 = map(
        constrain(Output2, -MaxSideToSide, MaxSideToSide),
        -MaxSideToSide,
        MaxSideToSide,
        MaxSideToSide,
        -MaxSideToSide);
    PID1.Compute(); //PID1 is for side to side stabilization
    writeMotorPwm(sideToSidePWM, Output1, Input1, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome tilt
// ------------------------------------------------------------------------------------
void domeTilt()
{
    //
    //The joystick will go from 0(Forward) to 512(Back).
    //The pot will get HIGH as it moves back, and LOW as it moves forward
    //

    BodyMode bodyM = (BodyMode)sendToRemote.bodyMode;
    // speedDomeTilt offsets the dome based on the main drive to tilt it in the direction of movement.
    // Naigon - Dome Modes: Add the tilt based on the main stick if the mode uses tilt.
    double speedDomeTilt =
        (Setpoint3 >= 3 || Setpoint3 <= -3)
        && (bodyM == BodyMode::SlowWithTilt || bodyM == BodyMode::ServoWithTilt)
            ? (double)driveStickPtr->GetMappedValue() * (double)DomeTiltAmount / (double)driveSpeed
            : 0.0;

#ifdef HeadTiltStabilization
    // Naigon - Head Tilt Stabilization
    // Calculate the pitch to input into the head tilt input in order to keep it level.
    // Naigon - TODO: once the ease applicator is created, use it here to increment to pitch adjust.
    int pitchAdjust = sendToRemote.bodyMode != BodyMode::PushToRoll
        ? (imu.Pitch + offsets.Pitch) * HeadTiltPitchAndRollProportion
        : 0;
#else
    int pitchAdjust = 0;
#endif

    int domeTiltPot = (int)domeTiltPotHandler.GetMappedValue() + offsets.DomeTiltPot;

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

    Input4 = domeTiltPot + (imu.Pitch + offsets.Pitch);
    Setpoint4 = domeTiltEaseApplicator.ComputeValueForCurrentIteration(joystickDome);
    Setpoint4 = constrain(Setpoint4, -MaxDomeTiltAngle, MaxDomeTiltAngle);
    PID4.Compute();

    writeMotorPwm(headTiltPWM, Output4, domeTiltPot, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin - Manual
// ------------------------------------------------------------------------------------
void domeSpin()
{
    int domeRotation = (int)domeSpinStickPtr->GetMappedValue();

    int currentDomeSpeed = constrain(
        domeSpinEaseApplicator.ComputeValueForCurrentIteration(domeRotation),
        -255,
        255);

    // Joe has always allowed the dome to spin regardless of whether the motors were enabled or not, which I like.
    writeMotorPwm(domeSpinPWM, currentDomeSpeed, 0 /*input*/, true /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin - Servo
// ------------------------------------------------------------------------------------
void domeSpinServo()
{
    int ch4Servo = (int)domeSpinStickPtr->GetMappedValue();

    Input5 = sendToRemote.bodyDirection == Direction::Forward
        ? (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpin - 180
        : (int)domeSpinPotHandler.GetMappedValue() + offsets.DomeSpin;

    if (Input5 < -180)
    {
        Input5 += 360;
    }
    else if (Input5 > 180)
    {
        Input5 -= 360;
    }

    Setpoint5 = constrain(
        domeServoEaseApplicator.ComputeValueForCurrentIteration(ch4Servo),
        -MaxDomeSpinServo,
        MaxDomeSpinServo);
    PID5.Compute();

    writeMotorPwm(domeServoPWM, Output5, 0, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Flywheel spin
// ------------------------------------------------------------------------------------
void flywheelSpin()
{
    // Naigon - Stationary/Wiggle Mode
    // When in stationary mode, use the drive stick as the flywheel, as the drive is disabled.
    int ch5PWM = (int)flywheelStickPtr->GetMappedValue();

    int flywheelRotation = constrain(
        flywheelEaseApplicator.ComputeValueForCurrentIteration(ch5PWM),
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
// Disable droid
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turnOffAllTheThings(bool includingDrive)
{
    //disables all PIDS and movement. This is to avoid any sudden jerks when re-enabling motors.
    Input2 = 0;
    Setpoint2 = 0;
    Output2 = 0;
    Input1 = 0;
    Setpoint1 = 0;
    Output1 = 0;
    Input4 = 0;
    Setpoint4 = 0;
    Output4 = 0;

    if (includingDrive)
    {
        Input3 = 0;
        Setpoint3 = 0;
        Output3 = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set offsets and save to EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsAndSaveToEEPROM()
{
    // Naigon - TODOD
    // Joe has this only save one thing per iteration. I'm not sure if that is due to needing to keep getting update
    // remote values, but it would be a lot better to just save everything in one go.
    //

    if (SaveToEEPROM == 0)
    {
        return;
    }

    if (SaveToEEPROM == 1)
    {
        offsets.Pitch = imu.Pitch * -1;
        EEPROM.writeFloat(0, offsets.Pitch);
        SaveToEEPROM = 2;
    }
    else if (SaveToEEPROM == 2)
    {
        offsets.Roll = imu.Roll * -1;
        EEPROM.writeFloat(4, offsets.Roll);
        SaveToEEPROM = 3;
    }
    else if (SaveToEEPROM == 3)
    {
        offsets.S2SPot = 0 - (int)sideToSidePotHandler.GetMappedValue();
        EEPROM.writeInt(8, offsets.S2SPot);
        SaveToEEPROM = 4;
    }
    else if (SaveToEEPROM == 4)
    {
        offsets.DomeTiltPot = 0 - (int)domeTiltPotHandler.GetMappedValue();
        EEPROM.writeInt(12, offsets.DomeTiltPot);
        SaveToEEPROM = 0;
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
        //playSound = 1;
    }
}

void setDomeSpinOffset()
{
    // Naigon - TODO: Figure out why it is mapped from 180, -180 as opposed to -180, 180, and change it so that this
    // can be moved to using GetMappedForwardValue.
    offsets.DomeSpin = sendToRemote.bodyDirection == Direction::Reverse
        ? 180 - (int)domeSpinPotHandler.GetMappedValue()
        : 0 - (int)domeSpinPotHandler.GetMappedValue();

    EEPROM.writeInt(16, offsets.DomeSpin);
    // delay(200);
    sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    //playSound = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set offsets ONLY; this is used if nothing is stored in EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsONLY()
{
    offsets.Pitch = 0 - imu.Pitch;
    offsets.Roll = 0 - imu.Roll;
    offsets.S2SPot = 0 - (map(analogRead(S2SpotPin), 0, 1024, -135, 135));
    offsets.DomeTiltPot = 0 - (map(analogRead(domeTiltPotPin), 0, 1024, -MaxHeadTiltPot, MaxHeadTiltPot));
    //delay(200);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auto disable motors
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void autoDisableMotors()
{
    double output1A = abs(Output1);
    double output3A = abs(Output3);

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
