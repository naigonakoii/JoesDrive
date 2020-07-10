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

#include "AnimationRunner.h"
#include "Animation.h"

namespace Naigon::Animations
{

AnimationRunner::AnimationRunner(int numAnimations, IAnimation *animations[])
    : _animations(animations)
    , _currentAnimation(nullptr)
    , _numAnimations(numAnimations)
    , _numBank1(0)
    , _numBank2(0)
    , _numBank3(0)
    , _numBank4(0)
    , _current1(0)
    , _current2(0)
    , _current3(0)
    , _current4(0)
{
    for (int i = 0; i < _numAnimations; i++)
    {
        //
        // Get a count of the number of each type of animation. This is needed to randomly select one later.
        //
        if (_animations[i]->Target() == AnimationTarget::Bank1
            || _animations[i]->Target() == AnimationTarget::AnyBank)
        {
            _numBank1++;
        }

        if (_animations[i]->Target() == AnimationTarget::Bank2
            || _animations[i]->Target() == AnimationTarget::AnyBank)
        {
            _numBank2++;
        }

        if (_animations[i]->Target() == AnimationTarget::Bank3
            || _animations[i]->Target() == AnimationTarget::AnyBank)
        {
            _numBank3++;
        }

        if (_animations[i]->Target() == AnimationTarget::Bank4
            || _animations[i]->Target() == AnimationTarget::AnyBank)
        {
            _numBank4++;
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

    if (aTarget == AnimationTarget::Bank1)
    {
        r = random(_numBank1);
    }
    else if (aTarget == AnimationTarget::Bank2)
    {
        r = random(_numBank2);
    }
    else if (aTarget == AnimationTarget::Bank3)
    {
        r = random(_numBank3);
    }
    else if (aTarget == AnimationTarget::Bank4)
    {
        r = random(_numBank4);
    }

    int count = 0, i = 0;
    while (count < (r + 1))
    {
        //
        // Find the randomly selected animation from the index.
        //
        if (aTarget == AnimationTarget::AnyBank
            || _animations[i]->Target() == aTarget
            || _animations[i]->Target() == AnimationTarget::AnyBank)
        {
            count++;
        }
        if (count < (r + 1)) { i++; }
    }

    // Assign the newly selected animation and start it.
    _currentAnimation = _animations[i];
    _currentAnimation->Start();
}

void AnimationRunner::StartNextAutomation(AnimationTarget aTarget)
{
    if (_currentAnimation != nullptr && _currentAnimation->IsRunning())
    {
        // If one is already running stop it here.
        _currentAnimation->Stop();
    }

    switch(aTarget)
    {
    case AnimationTarget::Bank1:
        FindNextAndStart(aTarget, _current1);
        break;
    case AnimationTarget::Bank2:
        FindNextAndStart(aTarget, _current2);
        break;
    case AnimationTarget::Bank3:
        FindNextAndStart(aTarget, _current3);
        break;
    case AnimationTarget::Bank4:
        FindNextAndStart(aTarget, _current4);
        break;
    }
}

void AnimationRunner::FindNextAndStart(AnimationTarget aTarget, int &index)
{
    do
    {
        index++;
        if (index >= _numAnimations) { index = 0; }
    } while (_animations[index]->Target() != aTarget
        && _animations[index]->Target() != AnimationTarget::AnyBank);

    _currentAnimation = _animations[index];
    _currentAnimation->Start();
}

bool AnimationRunner::StartAnimationWithId(uint16_t animationId)
{
    for (int i = 0; i < _numAnimations; i++)
    {
        if (_animations[i]->Id() == animationId)
        {
            _currentAnimation = _animations[i];
            _currentAnimation->Start();
            return true;
        }
    }

    return false;
}

const AnimationStep* AnimationRunner::RunIteration()
{
    if (_currentAnimation != nullptr && _currentAnimation->IsRunning())
    {
        // run the next iteration of the current animation if it is running.
        return _currentAnimation->RunIteration();
    }
}

}   //namespace Naigon::Animations
