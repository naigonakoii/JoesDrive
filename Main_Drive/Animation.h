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

#ifndef __NaigonBB8_Animations_h__
#define __NaigonBB8_Animations_h__

#include "Arduino.h"

namespace NaigonBB8
{

// Forward declare sound types
enum SoundTypes : int8_t;

// Get methods return this value if the animation does not specific action for that
// drive.
#define NOT_RUNNING -2048

// Forward declare Animation for helper classes.
class Animation;

///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that defines animation parameters for a single animation step.
///////////////////////////////////////////////////////////////////////////////////////
class AnimationState
{
public:
    AnimationState(
        int drive,
        int sideToSide,
        int domeTiltFB,
        int domeTiltLR,
        int domeSpin,
        int flywheel,
        SoundTypes soundType,
        int millisOnState);

    int GetDrive() const;
    int GetSideToSide() const;
    int GetDomeTiltFB() const;
    int GetDomeTiltLR() const;
    int GetDomeSpin() const;
    int GetFlywheel() const;
    SoundTypes GetSoundType() const;

private:
    //
    // Motor motions. These act as virtual stick controllers, as if the drive itself
    // has another remote operator.
    //
    int _drive = NOT_RUNNING;
    int _sideToSide = NOT_RUNNING;
    int _domeTiltFB = NOT_RUNNING;
    int _domeTiltLR = NOT_RUNNING;
    int _domeSpin = NOT_RUNNING;
    int _flywheel = NOT_RUNNING;

    SoundTypes _soundType;

    //
    // Milliseconds that this state is run for. After the time elapses, the next state
    // starts.
    //
    int _millisOnState;
    friend class Animation;
};

enum AnimationTarget : uint8_t
{
    // Special signal that an animation can be used for any purpose.
    AnyAnimation = 0,
    DomeAnimation = 1,
    FullAnimation = 2,
};

class Animation
{
public:
    Animation(AnimationTarget aTarget, int numOfSteps, const AnimationState* animationStates);
    bool IsRunning() const;
    AnimationTarget Target() const;

    void Start();
    void Stop();

    const AnimationState* RunIteration();

private:
    bool _isRunning;
    int _currentAnimationIndex;
    unsigned long _currentMillis;
    int _numberOfAnimationSteps;
    const AnimationState* _animationStates;
    AnimationTarget _animationTarget;
};

class AnimationRunner
{
public:
    AnimationRunner(int numAnimations, const Animation* animations);

    bool IsRunning() const;

    void StopCurrentAnimation();
    const AnimationState* RunIteration();
    void SelectAndStartAnimation(AnimationTarget aTarget);

private:
    const Animation* _animations;
    const Animation* _currentAnimation;
    int _numOfHeadAnimations, _numOfFullAnimations, _numAnimations;
};

}   //namepsace NaigonBB8

#endif //__NaigonBB8_Animations_h__
