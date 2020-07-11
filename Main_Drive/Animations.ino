// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Animation definitions used by the main drive.
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


// ********************************************************************************************************************
// NOTE - These includes and using statements are not actually needed for the Arduino compiler, but I added them to
// prevent intellisense errors in Visual Studio Code, as it is unable to understand that .ino files are concat
// together. Since the includes are protected by the #ifndef at the beginning, and doing using twice tested as benign,
// it causes no issues to re-include and re add using statements in what becomes the final single .ino file.
// ********************************************************************************************************************
#include "Constants.h"
#include "Enums.h"

#include "src/Libraries/NaigonAnimations/src/Animation.h"
#include "src/Libraries/NaigonAnimations/src/AnimationRunner.h"

#include "src/Libraries/NaigonSound/src/SoundPlayer.h"

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

using Naigon::NECAudio::SoundTypes;
// ********************************************************************************************************************


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

DomeMode eitherDome = DomeMode::UnspecifiedDomeSpin;
DomeMode fullSpinDome = DomeMode::FullSpinMode;
DomeMode servoDome = DomeMode::ServoMode;

// Default result that should be passed into all ScriptedAnimation instances.
// -------------------   | Drive   | S2S     | TiltFB  | TiltLR  | Spin    | Flywheel
int defaultStickVals[] = { Centered, Centered, Centered, Centered, Centered, Centered, };
DomeMode defaultDomeMode = DomeMode::UnspecifiedDomeSpin;
AnimationStep defaultResult(defaultStickVals, 6, SoundTypes::NotPlaying + 1, 0, &defaultDomeMode);

// Memory space that should be passed into all generated animations
// -------------------   | Drive   | S2S     | TiltFB  | TiltLR  | Spin    | Flywheel
int initialStickVals[] = { Centered, Centered, Centered, Centered, Centered, Centered, };
DomeMode currentDomeMode = DomeMode::UnspecifiedDomeSpin;
AnimationStep currentResult(initialStickVals, 6, SoundTypes::NotPlaying + 1, 0, &currentDomeMode);

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
    67, // Play Sound
    33, // Remaining MotorControl
};

const uint8_t domeMotorControls[] = { MotorControlId::idDomeSpin, MotorControlId::idDomeTiltFR, };
const uint16_t domeMotorControlPer[] = {                      60,                           40, };

const uint16_t millisVals[] = { 250, 350, 500, 750, 1000, 1250 };
const uint16_t millisPer[]  = {  10,  10,  30,  20,   20,   10};

const uint16_t percentDomeStickLR[] = { 0, 4, 28, 18, 18, 28, 4, 0, };
const uint16_t percentDomeServoLR[] = { 8, 12, 15, 15, 15, 15, 12, 8, };

const uint16_t percentDomeStickFR[] = { 15, 35, 25, 5, 5, 5, 5, 5, };

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

GeneratedAnimationPercents domeAnimationServoPercents(
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
    percentDomeServoLR,
    8 /* lrStickSize */,
    frStickMotorControlIds,
    frStickMotorControlIdsSize,
    65 /* pausePercent */);

// Since moving just the head is pretty basic, going with full generation here to save variable space.
GeneratedAnimation headMovementSpin(
    AnimationTarget::Bank1,
    &domeAnimationPercents,
    &fullSpinDome,
    3 /* minNumAnimationSteps */,
    2 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    4000 /* soundTimeout */,
    &currentResult);

GeneratedAnimation headMovementServo(
    AnimationTarget::Bank1,
    &domeAnimationPercents,
    &servoDome,
    3 /* minNumAnimationSteps */,
    2 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    4000 /* soundTimeout */,
    &currentResult);

// Implementations of extrn variables
uint16_t AutomatedDomeSpinId = headMovementSpin.Id();
uint16_t AutomatedDomeServoId = headMovementServo.Id();
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scripted Animations for Button 4 Press - Bank2.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// -------------------  Drive   | Side    | DomeTFB     | DomeTLR | DomeSpin | Flywhl  --------------------------------- 
int bank2A1Step01[6] = { Centered, Centered, ForwardFull, Centered, LeftFull,  Centered, };
int bank2A1Step02[6] = { Centered, Centered, ForwardFull, Centered, Centered,  Centered, };
int bank2A1Step03[6] = { Centered, Centered, ForwardFull, Centered, Centered,  Centered, };
int bank2A1Step04[6] = { Centered, Centered, ForwardFull, Centered, RightFull, Centered, };
int bank2A1Step05[6] = { Centered, Centered, ForwardFull, Centered, LeftFull,  Centered, };
int bank2A1Step06[6] = { Centered, Centered, ForwardHalf, Centered, Centered,  Centered, };
AnimationStep tiltHeadAndLookBothWays1State[] = {
    // --------- MotorVals | nVal | SoundId                   | MS | Metadata
    AnimationStep(bank2A1Step01, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A1Step02, 6, SoundTypes::Excited + 1,      0, &servoDome),
    AnimationStep(bank2A1Step03, 6, SoundTypes::NotPlaying + 1, 350, &servoDome),
    AnimationStep(bank2A1Step04, 6, SoundTypes::NotPlaying + 1, 600, &servoDome),
    AnimationStep(bank2A1Step05, 6, SoundTypes::NotPlaying + 1, 600, &servoDome),
    AnimationStep(bank2A1Step06, 6, SoundTypes::NotPlaying + 1, 350, &servoDome),
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
    // --------- MotorVals | nVal | SoundId                   | MS | Metadata
    AnimationStep(bank2A2Step01, 6, SoundTypes::Chatty + 1,       0, &servoDome),
    AnimationStep(bank2A2Step02, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step03, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step04, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step05, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step06, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step07, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step08, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
    AnimationStep(bank2A2Step09, 6, SoundTypes::NotPlaying + 1, 500, &servoDome),
};
ScriptedAnimation tiltHeadOppositeWays1(AnimationTarget::Bank2, 9, &defaultResult, tiltHeadOppositeWays1State);

// NOTE: The flywheel is somewhat backwards because it is upsidedown on the remote. so even though dome and flywheel
// are the same direction in code, they will spin in opposite directions, which is intended for this animation.
// -----------------     Drive   | Side    | DomeTFB | DomeTLR | DomeSpin | Flywhl  -----------------------------------
int bank2A3Step01[6] = { Centered, Centered, Centered, Centered, LeftHalf,  LeftFull, };
int bank2A3Step02[6] = { Centered, Centered, Centered, Centered, RightFull, RightFull, };
AnimationStep flywheelSpin1State[] = {
    // --------- MotorVals | nVal | SoundId                   | MS  | Metadata
    AnimationStep(bank2A3Step01, 6, SoundTypes::NotPlaying + 1,  500, &fullSpinDome),
    AnimationStep(bank2A3Step02, 6, SoundTypes::NotPlaying + 1, 3000, &fullSpinDome),
};
ScriptedAnimation flywheelSpin1(AnimationTarget::Bank2, 2, &defaultResult, flywheelSpin1State);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated and scripted animations for Button 6 press: Flywheel, no S2S - Bank3
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
    15, // Flywheel
};

const uint16_t bank3MillisVals[] = { 250, 350, 500, 750, 1000, 1250, 1500, };
const uint16_t bank3MillisPer[]  = {   4,  12,  24,  24,   18,   12,    5, };

const uint16_t bank3PercentDomeStickLR[] { 0, 12,  18, 20, 20, 18, 12, 0, };
const uint16_t bank3PercentDomeServoLR[] { 12, 12, 16, 10, 10, 16, 12, 12, };

const uint16_t bank3PercentDomeStickFR[] { 10, 40, 25, 5, 5, 5, 5, 5, };

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

GeneratedAnimationPercents bank3ServoPercents(
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
    bank3PercentDomeServoLR,
    8 /* lrStickSize */,
    frStickMotorControlIds,
    frStickMotorControlIdsSize,
    15 /* pausePercent */);

// Since moving just the head is pretty basic, going with full generation here to save variable space.
GeneratedAnimation bank3Servo(
    AnimationTarget::Bank3,
    &bank3ServoPercents,
    &servoDome,
    4 /* minNumAnimationSteps */,
    4 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    1500 /* soundTimeout */,
    &currentResult);

GeneratedAnimation bank3Spin(
    AnimationTarget::Bank3,
    &bank3Percents,
    &fullSpinDome,
    4 /* minNumAnimationSteps */,
    2 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    1500 /* soundTimeout */,
    &currentResult);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated animations for Button 6 hold: Flywheel and S2S - Bank4
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const AnimationAction bank4DomeActions[] = {
    AnimationAction::EndAnimation,
    AnimationAction::PlaySound,
    AnimationAction::MotorControl,
};

const uint16_t bank4DomeActPer[] =
{
    20, // End animation
    47, // Play Sound
    33, // Motor Control
};

const uint8_t bank4MotorControlIds[] =
{
    MotorControlId::idDomeSpin,
    MotorControlId::idDomeTiltFR,
    MotorControlId::idFlywheel,
    MotorControlId::idSideToSide,
};

const uint16_t bank4MotorControlPer[] =
{
    30, // Dome Spin
    30, // Tilt Dome
    20, // Flywheel
    20, // Side to Side
};

const uint16_t bank4MillisVals[] = { 250, 350, 500, 750, 1000, 1200, 1500, };
const uint16_t bank4MillisPer[]  = {   5,  15,  25,  25,   15,   10,    5, };

const uint16_t bank4PercentDomeStickLR[] { 4, 14,  16, 16, 16, 16, 14, 4, };

const uint16_t bank4PercentDomeStickFR[] { 35, 25, 15, 5, 5, 5, 5, 5, };

GeneratedAnimationPercents bank4Percents(
    bank4DomeActions,
    bank4DomeActPer,
    3 /* actionSize */,
    bank4MotorControlIds,
    bank4MotorControlPer,
    4 /* motorControlSize */,
    bank4MillisVals,
    bank4MillisPer,
    7 /* msSize */,
    stickFRVals,
    bank4PercentDomeStickFR,
    8 /* frStickSize */,
    stickLRVals,
    bank4PercentDomeStickLR,
    8 /* lrStickSize */,
    frStickMotorControlIds,
    frStickMotorControlIdsSize,
    15 /* pausePercent */);

// Since moving just the head is pretty basic, going with full generation here to save variable space.
GeneratedAnimation bank4Servo(
    AnimationTarget::Bank4,
    &bank4Percents,
    &servoDome,
    4 /* minNumAnimationSteps */,
    4 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    1500 /* soundTimeout */,
    &currentResult);

GeneratedAnimation bank4Spin(
    AnimationTarget::Bank4,
    &bank4Percents,
    &fullSpinDome,
    4 /* minNumAnimationSteps */,
    3 /* maxConcurentActions */,
    Naigon::NECAudio::SoundTypesNumTalking,
    1500 /* soundTimeout */,
    &currentResult);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animation Runner
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Array of entire animations that will be used to initialize the AnimationRunner in the main file.
const int NumAnimations = 9;
IAnimation *animations[] =
{
    // Bank 1
    &headMovementSpin,
    &headMovementServo,

    // Bank 2
    &tiltHeadAndLookBothWays1,
    &tiltHeadOppositeWays1,
    &flywheelSpin1,

    // Bank 3
    &bank3Servo,
    &bank3Spin,

    // Bank 4
    &bank4Servo,
    &bank4Spin,
};

// Naigon - Animations
AnimationRunner animationRunner(NumAnimations, animations);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
