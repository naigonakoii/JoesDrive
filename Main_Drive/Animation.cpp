#include "Animation.h"
#include "SoundPlayer.h"

namespace NaigonBB8
{

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
// Animation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Animation::Animation(AnimationTarget aTarget, int numOfSteps, const AnimationState* animationStates)
    : _numberOfAnimationSteps(numOfSteps)
    , _animationStates(animationStates)
    , _animationTarget(aTarget)
    , _isRunning(false)
    , _currentAnimationIndex(0)
    , _currentMillis(0)
{ }

bool Animation::IsRunning() const { return _isRunning; }

AnimationTarget Animation::Target() const { return _animationTarget; }

void Animation::Start()
{
    _isRunning = true;
    _currentMillis = -1;
    _currentAnimationIndex = -1;
}

void Animation::Stop() { _isRunning = false; }

const AnimationState* Animation::RunIteration()
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
// Animation Runner
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationRunner::AnimationRunner(int numAnimations, const Animation* animations)
    : _animations(animations)
    , _currentAnimation(nullptr)
    , _numAnimations(numAnimations)
    , _numOfHeadAnimations(0)
    , _numOfFullAnimations(0)
{
    for (int i = 0; i < _numAnimations; i++)
    {
        //
        // Get a count of the number of each type of animation. This is needed to randomly select one later.
        //
        if (animations[i].Target() == AnimationTarget::FullAnimation
            || animations[i].Target() == AnimationTarget::AnyAnimation)
        {
            _numOfFullAnimations++;
        }

        if (animations[i].Target() == AnimationTarget::DomeAnimation
            || animations[i].Target() == AnimationTarget::AnyAnimation)
        {
            _numOfHeadAnimations++;
        }
    }
}

bool AnimationRunner::IsRunning() const
{
    // Null check here since first call will be null.
    return _currentAnimation != nullptr ? _currentAnimation->IsRunning() : false;
}

void AnimationRunner::StopCurrentAnimation()
{
    if (IsRunning())
    {
        _currentAnimation->Stop();
    }
}

void AnimationRunner::SelectAndStartAnimation(AnimationTarget aTarget)
{
    if (_currentAnimation != nullptr && _currentAnimation->IsRunning())
    {
        // If one is already running stop it here.
        _currentAnimation->Stop();
    }

    int r = random(_numAnimations);

    if (aTarget == AnimationTarget::DomeAnimation)
    {
        r = random(_numOfHeadAnimations);
    }
    else if (aTarget == AnimationTarget::FullAnimation)
    {
        r = random(_numOfFullAnimations);
    }

    int count = 0, i = 0;
    while (count < (r + 1))
    {
        //
        // Find the randomly selected animation from the index.
        //
        if (aTarget == AnimationTarget::AnyAnimation
            || _animations[i].Target() == aTarget
            || _animations[i].Target() == AnimationTarget::AnyAnimation)
        {
            count++;
        }
        if (count < (r + 1)) { i++; }
    }

    // Assign the newly selected animation and start it.
    _currentAnimation = &_animations[i];
    _currentAnimation->Start();
}

const AnimationState* AnimationRunner::RunIteration()
{
    if (_currentAnimation != nullptr && _currentAnimation->IsRunning())
    {
        // run the next iteration of the current animation if it is running.
        return _currentAnimation->RunIteration();
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}   // namespace NaigonBB8
