// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive Powered by Naigon
//                         19 June 2020
//                         Scott DeBoer
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

// Private methods which can be considered like private statics off GeneratedAnimationPercents.
uint8_t weightedPercentBasedSelection(const uint16_t percents[], uint16_t size, uint16_t totPercent, uint8_t defaultValue);

// --------------------------------------------------------------------------------------------------------------------
// GeneratedAnimationPercents Class Methods
// --------------------------------------------------------------------------------------------------------------------
GeneratedAnimationPercents::GeneratedAnimationPercents(
    const AnimationAction *actionArray,
    const uint16_t *actionPercents,
    uint8_t actionSize,
    const uint16_t *msArray,
    const uint16_t *msPercents,
    uint8_t msSize,
    const uint16_t *frStickArray,
    const uint16_t *frStickPercents,
    uint8_t frStickSize,
    const uint16_t *lrStickArray,
    const uint16_t *lrStickPercents,
    uint8_t lrStickSize,
    uint8_t pausePercent)
    : _actionArray(actionArray)
    , _actionPercents(actionPercents)
    , _actionSize(actionSize)
    , _msArray(msArray)
    , _msPercents(msPercents)
    , _msSize(msSize)
    , _frStickArray(frStickArray)
    , _frStickPercents(frStickPercents)
    , _frStickSize(frStickSize)
    , _lrStickArray(lrStickArray)
    , _lrStickPercents(lrStickPercents)
    , _lrStickSize(lrStickSize)
    , _pausePercent(pausePercent)
    , _actionTot(0)
    , _msTot(0)
    , _frStickTot(0)
    , _lrStickTot(0)
{
    for (uint16_t i = 0; i < _actionSize; i++)
    {
        _actionTot += _actionPercents[i];
    }

    for (uint16_t i = 0; i < _msSize; i++)
    {
        _msTot += _msPercents[i];
    }

    for (uint16_t i = 0; i < _frStickSize; i++)
    {
        _frStickTot += _frStickPercents[i];
    }

    for (uint16_t i = 0; i < _lrStickSize; i++)
    {
        _lrStickTot += _lrStickPercents[i];
    }
}

// --------------------------------------------------------------------------------------------------------------------
// GeneratedAnimation Class Methods
// --------------------------------------------------------------------------------------------------------------------
GeneratedAnimation::GeneratedAnimation(
    AnimationTarget aTarget,
    GeneratedAnimationPercents *percents,
    uint8_t minNumAnimationSteps,
    uint8_t maxConcurentActions,
    uint16_t soundTimeout)
    : _animationTarget(aTarget)
    , _percents(percents)
    , _minNumAnimationSteps(minNumAnimationSteps)
    , _maxConcurentActions(maxConcurentActions)
    , _soundTimeout(soundTimeout)
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
    if (_animationStepCount == 0 || (millis() - _currentMillis) > _currentResult._millisOnState)
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
        _percents->_msPercents,
        _percents->_msSize,
        _percents->_msTot /* totPercent */,
        _percents->_msSize / 2);

    _currentResult._millisOnState = _percents->_msArray[millisBucketIndex];

    if (random(101) < _percents->_pausePercent)
    {
        // Pause is just a cleared state for a specific duration, so we are done.
        return;
    }

    int numConcurentActions = random(_maxConcurentActions) + 1;

    for (int i = 0; i < numConcurentActions; i++)
    {
        AnimationAction action = (AnimationAction)AutoGenerateAction();

        if (action == AnimationAction::EndAnimation && _animationStepCount >= _minNumAnimationSteps)
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
    uint8_t index = weightedPercentBasedSelection(
        _percents->_actionPercents,
        _percents->_actionSize,
        _percents->_actionTot,
        AnimationAction::SpinDome);

    return _percents->_actionArray[index];
}

int GeneratedAnimation::AutoGenerateStickAmount(uint8_t action)
{
    AnimationAction animationAction = (AnimationAction)action;

    if (animationAction == AnimationAction::TiltDomeFB)
    {
        uint8_t index = weightedPercentBasedSelection(
            _percents->_frStickPercents,
            _percents->_frStickSize,
            _percents->_frStickTot,
            0);

        return _percents->_frStickArray[index];
    }
    else
    {
        uint8_t index = weightedPercentBasedSelection(
            _percents->_lrStickPercents,
            _percents->_lrStickSize,
            _percents->_lrStickTot,
            _percents->_lrStickSize / 2);

        return _percents->_lrStickArray[index];
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

uint8_t weightedPercentBasedSelection(const uint16_t percents[], uint16_t size, uint16_t totPercent, uint8_t defaultValue)
{
    // Randomly select the percentage.
    int opPercent = random(totPercent);

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
