// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive Powered by Naigon
//                         19 June 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This is part of the add-on library for Joe's Drive created by Naigon.
// ====================================================================================================================

#include "Animation.h"
#include "SoundPlayer.h"

namespace NaigonBB8
{

using namespace AnimationConstants;

const AnimationState EMPTY_RESULT(0, 0, 0, 0, 0, 0, SoundTypes::NotPlaying, 0);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animation State
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationState::AnimationState(
    int drive,
    int sideToSide,
    int domeTiltFB,
    int domeTiltLR,
    int domeSpin,
    int flywheel,
    SoundTypes soundType,
    int millisOnState)
    : _drive(drive)
    , _sideToSide(sideToSide)
    , _domeTiltFB(domeTiltFB)
    , _domeTiltLR(domeTiltLR)
    , _domeSpin(domeSpin)
    , _flywheel(flywheel)
    , _soundType(soundType)
    , _millisOnState(millisOnState)
{ }

int AnimationState::GetDrive() const { return _drive; }
int AnimationState::GetSideToSide() const { return _sideToSide; }
int AnimationState::GetDomeTiltFB() const { return _domeTiltFB; }
int AnimationState::GetDomeTiltLR() const { return _domeTiltLR; }
int AnimationState::GetDomeSpin() const { return _domeSpin; }
int AnimationState::GetFlywheel() const { return _flywheel; }
SoundTypes AnimationState::GetSoundType() const { return _soundType; }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scripted Animation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScriptedAnimation::ScriptedAnimation(AnimationTarget aTarget, int numOfSteps, const AnimationState* animationStates)
    : _numberOfAnimationSteps(numOfSteps)
    , _animationStates(animationStates)
    , _animationTarget(aTarget)
    , _isRunning(false)
    , _currentAnimationIndex(0)
    , _currentMillis(0)
{
}

bool ScriptedAnimation::IsRunning() const
{
    return _isRunning;
}

AnimationTarget ScriptedAnimation::Target() const
{
    return _animationTarget;
}

void ScriptedAnimation::Start()
{
    _isRunning = true;
    _currentMillis = -1;
    _currentAnimationIndex = -1;
}

void ScriptedAnimation::Stop()
{
    _isRunning = false;
}

const AnimationState* ScriptedAnimation::RunIteration()
{
    if (!_isRunning)
    {
        return &EMPTY_RESULT;
    }

    if (_currentAnimationIndex == -1
        || _animationStates[_currentAnimationIndex]._millisOnState == 0
        || (millis() - _currentMillis) > _animationStates[_currentAnimationIndex]._millisOnState)
    {
        _currentMillis = millis();
        _currentAnimationIndex++;
    }

    if (_currentAnimationIndex >= _numberOfAnimationSteps)
    {
        _isRunning = false;
        return &EMPTY_RESULT;
    }

    return &_animationStates[_currentAnimationIndex];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated Animation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint8_t weightedPercentBasedSelection(const int percents[], int size, uint8_t defaultValue);

enum AnimationAction : uint8_t
{
    EndAnimation = 0,
    SpinDome,
    TiltDomeFB,
    TiltDomeLR,
    SideToSide,
    Flywheel,
    PlaySound,
    ENDAnimationAction,
};

enum LeftRightStick : uint8_t
{
    FullLeft = 0,
    ThreeFourthLeft,
    TwoThirdLeft,
    OneHalfLeft,
    OneHalfRight,
    TwoThirdRight,
    ThreeFourthRight,
    FullRight,
    ENDLeftRightStick,
};

enum ForwardReverseStick : uint8_t
{
    FullForward = 0,
    ThreeFourthForward,
    TwoThirdForward,
    OneHalfForward,
    OneHalfReverse,
    TwoThirdReverse,
    ThreeFourthReverse,
    FullReverse,
    ENDForwardReverseStick,
};

enum MillisSelection : uint8_t
{
    OneHundredMS = 0,
    TwoFiftyMS,
    ThreeFiftyMS,
    FiveHundredMS,
    SevenFiftyMS,
    OneSecond,
    ENDMillis,
};

//
// Ensure these add to 100
//
const int percentTotals[] = { 5, 30, 20, 0, 10, 5, 30, };
const int percentStickLR[] { 15, 12, 8, 15, 15, 8, 12, 15, };
const int percentStickFR[] { 40, 25, 15, 5, 5, 5, 5, };
const int percentMillis[] { 5, 20, 20, 20, 20, 15, };

const int stickFRVals[] =
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

const int stickLRVals[] =
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

const int millisVals[] =
{
    100,
    250,
    350,
    500,
    750,
    1000,
};

// --------------------------------------------------------------------------------------------------------------------
// GeneratedAnimation Class Methods
// --------------------------------------------------------------------------------------------------------------------
GeneratedAnimation::GeneratedAnimation(
    AnimationTarget aTarget,
    uint8_t minNumAnimationSteps,
    uint16_t soundTimeout,
    bool allowAutoStop)
    : _animationTarget(aTarget)
    , _minNumAnimationSteps(minNumAnimationSteps)
    , _soundTimeout(soundTimeout)
    , _autoStop(allowAutoStop)
    , _currentResult(
        Centered,
        Centered,
        Centered,
        Centered,
        Centered,
        Centered,
        SoundTypes::NotPlaying,
        0)
    , _lastSoundMillis(0)
    , _isRunning(false)
    , _animationStepCount(0)
{
}

bool GeneratedAnimation::IsRunning() const
{
    return _isRunning;
}

AnimationTarget GeneratedAnimation::Target() const
{
    return _animationTarget;
}

void GeneratedAnimation::Start()
{
    Clear();
    _isRunning = true;
    _animationStepCount = 0;
}

void GeneratedAnimation::Stop()
{
    Clear();
    _isRunning = false;
}

const AnimationState* GeneratedAnimation::RunIteration()
{
    if (_animationStepCount == 0 || (millis() - _currentMillis) > _nextAnimationMS)
    {
        _currentMillis = millis();
        AutoGenerateNextState();
        _animationStepCount++;
    }

    return &_currentResult;
}

void GeneratedAnimation::AutoGenerateNextState()
{
    Clear();

    uint8_t millisBucketIndex = weightedPercentBasedSelection(
        percentMillis,
        MillisSelection::ENDMillis,
        MillisSelection::ThreeFiftyMS);

    _currentResult._millisOnState = millisVals[millisBucketIndex];

    int numConcurentActions = random(3) + 1;

    for (int i = 0; i < numConcurentActions; i++)
    {
        AnimationAction action = (AnimationAction)AutoGenerateAction();

        if (action == AnimationAction::EndAnimation && _autoStop && _animationStepCount >= _minNumAnimationSteps)
        {
            Stop();
            return;
        }
        else if (action == AnimationAction::PlaySound && (millis() - _lastSoundMillis) > _soundTimeout)
        {
            SoundTypes sound = (SoundTypes)random(SoundTypes::COUNT);

            // Don't use the types that play tracks.
            _currentResult._soundType = sound == SoundTypes::PlayTrack || sound == SoundTypes::StopTrack
                ? SoundTypes::Excited
                : sound;

            // IMPORTANT - Must set this to zero as otherwise the drive could think to play a ton of sounds in a row.
            _currentResult._millisOnState = 0;
            _lastSoundMillis = millis();
        }
        else
        {
            if (_animationTarget == AnimationTarget::DomeAnimation
                    && action != AnimationAction::SpinDome)
            {
                // Dome Animation target only supports dome spin. Give an extra 30% change to do it here.
                int spinExtra = random(100);
                if (spinExtra > 33) { continue; }
            }

            int stickAmount = AutoGenerateStickAmount(action);
            switch (action)
            {
            case AnimationAction::SpinDome:
                _currentResult._domeSpin = stickAmount;
                break;
            case AnimationAction::TiltDomeFB:
                _currentResult._domeTiltFB = stickAmount;
                break;
            case AnimationAction::TiltDomeLR:
                _currentResult._domeTiltLR = stickAmount;
                break;
            case AnimationAction::SideToSide:
                _currentResult._sideToSide = stickAmount;
                break;
            case AnimationAction::Flywheel:
                // Flywheel doesn't do much unless it is basically full blast.
                _currentResult._flywheel = stickAmount < 255 ? RightFull : LeftFull;
                break;
            }
        }
    }
}

uint8_t GeneratedAnimation::AutoGenerateAction()
{
    return weightedPercentBasedSelection(
        percentTotals,
        AnimationAction::ENDAnimationAction,
        AnimationAction::SpinDome);
}

int GeneratedAnimation::AutoGenerateStickAmount(uint8_t action)
{
    AnimationAction animationAction = (AnimationAction)action;

    if (animationAction == AnimationAction::TiltDomeFB)
    {
        uint8_t index = weightedPercentBasedSelection(
            percentStickFR,
            ForwardReverseStick::ENDForwardReverseStick,
            ForwardReverseStick::FullForward);

        return stickFRVals[index];
    }
    else
    {
        uint8_t index = weightedPercentBasedSelection(
            percentStickLR,
            LeftRightStick::ENDLeftRightStick,
            LeftRightStick::OneHalfRight);

        return stickLRVals[index];
    }
}

void GeneratedAnimation::Clear()
{
    _currentResult._drive = Centered;
    _currentResult._sideToSide = Centered;
    _currentResult._domeSpin = Centered;
    _currentResult._domeTiltFB = Centered;
    _currentResult._domeTiltLR = Centered;
    _currentResult._flywheel = Centered;
    _currentResult._soundType = SoundTypes::NotPlaying;
    _currentResult._millisOnState = 0;
}

uint8_t weightedPercentBasedSelection(const int percents[], int size, uint8_t defaultValue)
{
    // Randomly select the percentage.
    int opPercent = random(100);

    uint16_t count = 0;
    for (int j = 0; j < size; j++)
    {
        count += percents[j];
        if (opPercent < count && percents[j] > 0)
        {
            return j;
        }
    }

    return defaultValue;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


}   // namespace NaigonBB8
