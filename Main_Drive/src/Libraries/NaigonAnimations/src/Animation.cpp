// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Droids Powered by Naigon
//                         19 June 2020
//                         Scott DeBoer
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This is part of the add-on library for droids and robotics. I created it for Joe's Drive but it can be adapted to
// and motor-controled robotic application.
// ====================================================================================================================

#include "Animation.h"

namespace Naigon::Animations
{

using namespace AnimationConstants;

uint16_t GlobalId = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Animation State
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationStep::AnimationStep(
    int motorVals[],
    uint8_t numMotorVals,
    SoundId soundType,
    int millisOnState,
    void *metadata)
    : _motorControlActions(motorVals)
    , _motorControlActionsSize(numMotorVals)
    , _soundType(soundType)
    , _millisOnState(millisOnState)
    , _metadata(metadata)
{
}

void* AnimationStep::GetMetadata() const { return _metadata; }
SoundId AnimationStep::GetSoundId() const { return _soundType; }
int AnimationStep::GetMotorControlValue(uint8_t controlId) const { return _motorControlActions[controlId]; }
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Scripted Animation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ScriptedAnimation::ScriptedAnimation(
    AnimationTarget aTarget,
    int numOfSteps,
    const AnimationStep *defaultResult,
    const AnimationStep *animationSteps)
    : _numberOfAnimationSteps(numOfSteps)
    , _defaultResult(defaultResult)
    , _animationSteps(animationSteps)
    , _animationTarget(aTarget)
    , _isRunning(false)
    , _currentAnimationIndex(0)
    , _currentMillis(0)
    , _id(GlobalId++)
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

uint16_t ScriptedAnimation::Id() const
{
    return _id;
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

const AnimationStep* ScriptedAnimation::RunIteration()
{
    if (!_isRunning)
    {
        return _defaultResult;
    }

    if (_currentAnimationIndex == -1
        || _animationSteps[_currentAnimationIndex]._millisOnState == 0
        || (millis() - _currentMillis) > _animationSteps[_currentAnimationIndex]._millisOnState)
    {
        _currentMillis = millis();
        _currentAnimationIndex++;
    }

    if (_currentAnimationIndex >= _numberOfAnimationSteps)
    {
        _isRunning = false;
        return _defaultResult;
    }

    return &_animationSteps[_currentAnimationIndex];
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
    const uint8_t *motorControlArray,
    const uint16_t *motorControlPercents,
    uint8_t motorControlSize,
    const uint16_t *msArray,
    const uint16_t *msPercents,
    uint8_t msSize,
    const uint16_t *frStickArray,
    const uint16_t *frStickPercents,
    uint8_t frStickSize,
    const uint16_t *lrStickArray,
    const uint16_t *lrStickPercents,
    uint8_t lrStickSize,
    const uint8_t *motorIdsUsingFr,
    uint8_t motorIdsUsingFRSize,
    uint8_t pausePercent)
    : _actionArray(actionArray)
    , _actionPercents(actionPercents)
    , _actionSize(actionSize)
    , _controlArray(motorControlArray)
    , _controlPercents(motorControlPercents)
    , _controlSize(motorControlSize)
    , _msArray(msArray)
    , _msPercents(msPercents)
    , _msSize(msSize)
    , _frStickArray(frStickArray)
    , _frStickPercents(frStickPercents)
    , _frStickSize(frStickSize)
    , _lrStickArray(lrStickArray)
    , _lrStickPercents(lrStickPercents)
    , _lrStickSize(lrStickSize)
    , _motorIdsUsingFR(motorIdsUsingFr)
    , _mFrSize(motorIdsUsingFRSize)
    , _pausePercent(pausePercent)
    , _actionTot(0)
    , _controlTot(0)
    , _msTot(0)
    , _frStickTot(0)
    , _lrStickTot(0)
{
    for (uint16_t i = 0; i < _actionSize; i++)
    {
        _actionTot += _actionPercents[i];
    }

    for (uint16_t i = 0; i < _controlSize; i++)
    {
        _controlTot += _controlPercents[i];
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
    void *metadata,
    uint8_t minNumAnimationSteps,
    uint8_t maxConcurentActions,
    uint8_t numSounds,
    uint16_t soundTimeout,
    AnimationStep *initialStep)
    : _animationTarget(aTarget)
    , _percents(percents)
    , _metadata(metadata)
    , _minNumAnimationSteps(minNumAnimationSteps)
    , _maxConcurentActions(maxConcurentActions)
    , _numSounds(numSounds)
    , _soundTimeout(soundTimeout)
    , _currentResult(initialStep)
    , _lastSoundMillis(0)
    , _isRunning(false)
    , _animationStepCount(0)
    , _id(GlobalId++)
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

uint16_t GeneratedAnimation::Id() const
{
    return _id;
}

void GeneratedAnimation::Start()
{
    Clear();
    _isRunning = true;
    _animationStepCount = 0;
    _lastSoundMillis = millis() - _soundTimeout - 1;
}

void GeneratedAnimation::Stop()
{
    Clear();
    _isRunning = false;
}

const AnimationStep* GeneratedAnimation::RunIteration()
{
    if (_animationStepCount == 0 || (millis() - _currentMillis) > _currentResult->_millisOnState)
    {
        _currentMillis = millis();
        AutoGenerateNextState();
        _animationStepCount++;
    }

    return _currentResult;
}

void GeneratedAnimation::AutoGenerateNextState()
{
    Clear();

    // Set to the dome mode of this class.
    _currentResult->_metadata = _metadata;

    uint8_t millisBucketIndex = weightedPercentBasedSelection(
        _percents->_msPercents,
        _percents->_msSize,
        _percents->_msTot /* totPercent */,
        _percents->_msSize / 2);

    _currentResult->_millisOnState = _percents->_msArray[millisBucketIndex];

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
            // Assumed that end user included the special "Not Playing" sound, so no need to add one (1).
            _currentResult->_soundType = (SoundId)random(_numSounds);

            // IMPORTANT - Must set this to zero as otherwise the drive could think to play a ton of sounds in a row.
            _currentResult->_millisOnState = 0;
            _lastSoundMillis = millis();
        }
        else if (action == AnimationAction::MotorControl)
        {
            // Select the motor control from the passed in percents.
            uint8_t motorControlId = AutoGenerateMotorControlId();
            int stickAmount = AutoGenerateStickAmount(motorControlId);

            _currentResult->_motorControlActions[motorControlId] = stickAmount < 255 ? RightFull : LeftFull;
        }
    }
}

uint8_t GeneratedAnimation::AutoGenerateAction()
{
    uint8_t index = weightedPercentBasedSelection(
        _percents->_actionPercents,
        _percents->_actionSize,
        _percents->_actionTot,
        AnimationAction::PlaySound);

    return _percents->_actionArray[index];
}

uint8_t GeneratedAnimation::AutoGenerateMotorControlId()
{
    uint8_t index = weightedPercentBasedSelection(
        _percents->_controlPercents,
        _percents->_controlSize,
        _percents->_controlTot,
        0);

    return _percents->_controlArray[index];
}

int GeneratedAnimation::AutoGenerateStickAmount(uint8_t motorControlId)
{
    bool usesFrStick = false;
    for (uint8_t i = 0; i < _percents->_mFrSize; i++)
    {
        usesFrStick = _percents->_motorIdsUsingFR[i] == motorControlId;
        if (usesFrStick) { continue; }
    }

    if (usesFrStick)
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
    for (uint8_t i = 0; i < _currentResult->_motorControlActionsSize; i++)
    {
        _currentResult->_motorControlActions[i] = Centered;
    }

    _currentResult->_soundType = 0;
    _currentResult->_millisOnState = 0;
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

}   // namespace Naigon::Animations
