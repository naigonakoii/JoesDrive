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

#ifndef __NaigonBB8_Animations_h__
#define __NaigonBB8_Animations_h__

#include "Arduino.h"

namespace NaigonBB8
{

namespace AnimationConstants
{
const int RightHalf = 255 / 2;
const int LeftHalf = 512 - RightHalf;

const int RightTwoThirds = 255 / 3;
const int LeftTwoThirds = 512 - RightTwoThirds;

const int RightThreeFourths = 255 / 4;
const int LeftThreeFourths = 512 - RightThreeFourths;

const int RightFull = 0;
const int LeftFull = 512;

const int Centered = 255;

#define ForwardHalf RightHalf
#define ReverseHalf  LeftHalf

#define ForwardTwoThirds RightTwoThirds
#define ReverseTwoThirds LeftTwoThirds

#define ForwardThreeFourths RightThreeFourths
#define ReverseThreeFourths LeftThreeFourths

#define ForwardFull RightFull
#define ReverseFull LeftFull
}   //namepsace AnimationConstants

// Forward declare Animation for helper classes.
class ScriptedAnimation;
class GeneratedAnimation;

// Forward declare sound types
enum SoundTypes : int8_t;

///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that defines animation parameters for a single animation step.
///////////////////////////////////////////////////////////////////////////////////////
class AnimationState
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Instantiate a new instance of the AnimationState class.
    ///////////////////////////////////////////////////////////////////////////////////
    AnimationState(
        int drive,
        int sideToSide,
        int domeTiltFB,
        int domeTiltLR,
        int domeSpin,
        int flywheel,
        SoundTypes soundType,
        int millisOnState);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current drive stick value for this instance.
    //
    // @ret     Drive stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetDrive() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the side to side stick value for this instance.
    //
    // @ret     Side to side stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetSideToSide() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the forward/reverse dome stick value for this instance.
    //
    // @ret     Forward/reverse dome tilt stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetDomeTiltFB() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the left/right dome tilt stick value for this instance.
    //
    // @ret     Left/right dome tilt stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetDomeTiltLR() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the dome spin stick value for this instance.
    //
    // @ret     Dome spin stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetDomeSpin() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the flywheel stick value for this instance.
    //
    // @ret     Flywheel stick value in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetFlywheel() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current sound to play.
    //
    // @ret     SoundTypes value for which sound to play.
    ///////////////////////////////////////////////////////////////////////////////////
    SoundTypes GetSoundType() const;

private:
    //
    // Motor motions. These act as virtual stick controllers, as if the drive itself
    // has another remote operator.
    //
    int _drive;
    int _sideToSide;
    int _domeTiltFB;
    int _domeTiltLR;
    int _domeSpin;
    int _flywheel;

    //
    // Sound selection
    //
    SoundTypes _soundType;

    //
    // Milliseconds that this state is run for. After the time elapses, the next state
    // starts.
    //
    int _millisOnState;
    friend class ScriptedAnimation;
    friend class GeneratedAnimation;
};

///////////////////////////////////////////////////////////////////////////////////////
// @summary Enum that defines the valid animation targets. These are banks that are
//          used to logically separate the animations. That way different buttons can
//          enact upon a specific subset of all the animations. Specifically, when in
//          the two automation drive modes, only DomeAnimation can be choosen to
//          prevent weird issues from happening while driving.
///////////////////////////////////////////////////////////////////////////////////////
enum AnimationTarget : uint8_t
{
    // Special signal that an animation can be used for any purpose.
    AnyBank = 0,
    Bank1 = 1,
    Bank2 = 2,
    Bank3 = 3,
    Bank4 = 4,
};

///////////////////////////////////////////////////////////////////////////////////////
// @summary Interface for a single animation definition. All types of animations
//          implement this interface.
///////////////////////////////////////////////////////////////////////////////////////
struct IAnimation
{
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Method returns if the animation is currently running.
    //
    // @ret     true if the animation is currently running; otherwise, false.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual bool IsRunning() const = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary AnimationTarget which this animation is for.
    //
    // @ret     AnimationTarget enum value.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual AnimationTarget Target() const = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Starts the animation.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual void Start() = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Stops the animation.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual void Stop() = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary RunIteration is responsible for performing the animation for the
    //          current Arduino loop step. It should be called once per every iteration
    //          of the Arduino main loop.
    //
    //          This method can cause the animation to exit the running state; ensure
    //          to check that before using the resulting state of this method.
    //
    // @ret     Returns the state animation that should be used by the main program for
    //          the current Arduino loop iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual const AnimationState* RunIteration() = 0;
};


///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that defines a scripted animation. These animations are built up
//          using arrays of AnimationState objects that are defined in the initial
//          program. These allow the perfect animations to be defined, but are space
//          intensive which is why I have other types of animations that are not
//          completely defined in advance but rather use heuristics.
///////////////////////////////////////////////////////////////////////////////////////
class ScriptedAnimation : public IAnimation
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs a new instance of the ScriptedAnimation class.
    //
    // @param   aTarget
    //          Target in which this instacne is grouped with.
    //
    // @param   numOfSteps
    //          This is the size of the animationStates array, which is how many
    //          steps the script will run.
    //
    // @param   animationStates
    //          Array of all the steps that will be executed for this script.
    ///////////////////////////////////////////////////////////////////////////////////
    ScriptedAnimation(
        AnimationTarget aTarget,
        int numOfSteps,
        const AnimationState* animationStates);

    //
    // IAnimation interface implementation
    //
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


///////////////////////////////////////////////////////////////////////////////////////
// @summary Valid actions that could be generated by GeneratedAnimation.
///////////////////////////////////////////////////////////////////////////////////////
enum AnimationAction : uint8_t
{
    Pause = 0,
    EndAnimation,
    SpinDome,
    TiltDomeFB,
    TiltDomeLR,
    SideToSide,
    Flywheel,
    PlaySound,
    ENDAnimationAction,
};

class GeneratedAnimationPercents
{
public:
    GeneratedAnimationPercents(
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
        uint8_t lrStickSize);

private:
    const AnimationAction *_actionArray;
    const uint16_t *_actionPercents;
    const uint16_t *_msArray;
    const uint16_t *_msPercents;
    const uint16_t *_frStickArray;
    const uint16_t *_frStickPercents;
    const uint16_t *_lrStickArray;
    const uint16_t *_lrStickPercents;
    uint8_t _actionSize, _msSize, _frStickSize, _lrStickSize;
    uint16_t _actionTot, _msTot, _frStickTot, _lrStickTot;

    friend class GeneratedAnimation;
};

///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that is used to generate animation scripts according to heuristics
//          passed into the class.
//
//          While the ScriptedAnimation class above allows for fine-tuning perfect
//          animation sequences, it comes a the cost of eating a lot of the limited
//          variable memory space of the system. Thus, to prevent having to script
//          many small but similar animations, this class instead can be used.
//
//          The basic idea of the class is that instead of getting a passed in array
//          of AnimationState variables, this class will have one state that it
//          generates based on some canned rules and some parameters passed into the
//          class. It then returns that state until the randomly selected time has
//          expired, and it will create a new state again using the same rules to
//          randomly reconfigure the state. This simulates an animation array being
//          passed in, and allows for my animations without taking a lot of variable
//          memory space.
///////////////////////////////////////////////////////////////////////////////////////
class GeneratedAnimation : public IAnimation
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Construts a new instace of the GeneratedAnimation class.
    //
    // @param   aTarget
    //          AnimationTarget for whcih this animation is applicapable.
    //
    // @param   GeneratedAnimationPercents
    //          Pointer to the percents class that will be associated with this
    //          instance. These percents define how likely actions, motion directions,
    //          and the lengths for each animation, allowing non-uniform random
    //          distributions to define the generation.
    //
    // @param   minNumAnimationSteps
    //          Defines the minimum number of steps the animation should run before it
    //          can stop. Ignored if allowAutoStop is set to false.
    //
    // @param   maxConcurentActions
    //          Maximum number of concurent actions that can happen at once. Higher
    //          values mean it is more likely for multiple actions to occur at the
    //          same time.
    //
    // @param   soundTimeout
    //          Length in milliseconds before another sound can be generated. Higher
    //          values prevent spamming the system with talking, but less overall
    //          sounds will be generated.
    //
    // @param   allowAutoStop
    //          If true, the animation will be able to stop itself after at least
    //          minNumAnimationSteps have been completed. If false, the animation runs
    //          indefinitely until it is manually stopped by calling the Stop() method.
    ///////////////////////////////////////////////////////////////////////////////////
    GeneratedAnimation(
        AnimationTarget aTarget,
        GeneratedAnimationPercents *percents,
        uint8_t minNumAnimationSteps,
        uint8_t maxConcurentActions,
        uint16_t soundTimeout,
        bool allowAutoStop);

    //
    // IAnimation interface implementation
    //
    bool IsRunning() const;
    AnimationTarget Target() const;
    void Start();
    void Stop();
    const AnimationState* RunIteration();

private:
    void AutoGenerateNextState();
    uint8_t AutoGenerateAction();
    int AutoGenerateStickAmount(uint8_t action);
    void Clear();

    AnimationTarget _animationTarget;
    unsigned long _currentMillis, _soundTimeout, _lastSoundMillis;
    uint8_t _minNumAnimationSteps, _animationStepCount, _maxConcurentActions;
    bool _isRunning, _autoStop;
    AnimationState _currentResult;
    GeneratedAnimationPercents *_percents;
};

}   //namepsace NaigonBB8

#endif //__NaigonBB8_Animations_h__
