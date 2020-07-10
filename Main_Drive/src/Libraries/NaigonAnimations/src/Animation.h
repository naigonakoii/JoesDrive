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

#ifndef __NaigonBB8_Animations_h__
#define __NaigonBB8_Animations_h__

#include "Arduino.h"

namespace Naigon::Animations
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

///////////////////////////////////////////////////////////////////////////////////////
// @summary Sound Id type. Zero will be not playing; positive values represent unique
//          sound types that the drive can support.
//
//          It is recommended to create an external enum that maps these values to
//          their logical meaning.
///////////////////////////////////////////////////////////////////////////////////////
typedef uint8_t SoundId;

///////////////////////////////////////////////////////////////////////////////////////
// @summary Valid actions that the system can take. MotorControl is a special one;
//          the end user should define a separate enum that further breaks down the
//          motor control values. 
///////////////////////////////////////////////////////////////////////////////////////
enum AnimationAction : uint8_t
{
    EndAnimation = 0,
    PlaySound,
    MotorControl,
    ENDAnimationAction,
};

///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that defines animation parameters for a single animation step.
///////////////////////////////////////////////////////////////////////////////////////
class AnimationStep
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Instantiate a new instance of the AnimationStep class.
    //
    // @param   motorVals
    //          Array storage for the motor values. This array should be of size
    //          numMotorVals. The index to semantic motor mapping is an external
    //          responsibility of the caller.
    //
    //          For example, if the drive has a forward stick, turning stick, and head
    //          spin, an array like the following would be passed in:
    //
    //              { 0, 0, 0, }
    //
    //          And externally an enum like the following could be constructed:
    //
    //              enum Mapping : uint8_t { Forward = 0, Turning, DomeSpin, }
    //
    // @param   numMotorVals
    //          Number of motor values passed into this constructor.
    //
    // @param   soundType
    //          Current sound id associated with this animation step.
    //
    // @param   millisOnState
    //          Length of time in milliseconds in which this animation should be
    //          applied.
    //
    // @param   metadata
    //          Pointer to any external metadata related to this state. It is up to the
    //          caller to recast back into the appropriate type. It is recommended to
    //          use the same type for the entire program.
    ///////////////////////////////////////////////////////////////////////////////////
    AnimationStep(
        int motorVals[],
        uint8_t numMotorVals,
        SoundId soundType,
        int millisOnState,
        void *metadata);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current stick value for the specified motorId.
    //
    // @param   controlId
    //          Id in the range of zero (0) to numMotorVals - 1. It is recommended to
    //          create an external enum to map these values, but that is not a
    //          requirement of the system.
    //
    // @ret     Stick value for the specified controlId in the range of 0-512.
    ///////////////////////////////////////////////////////////////////////////////////
    int GetMotorControlValue(uint8_t controlId) const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current sound to play.
    //
    // @ret     SoundId value for which sound to play. Zero is reserved for the
    //          special meaning "Not Playing".
    ///////////////////////////////////////////////////////////////////////////////////
    SoundId GetSoundId() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the specified metadata for this step.
    //
    // @ret     Pointer to the metadata for this step.
    ///////////////////////////////////////////////////////////////////////////////////
    void* GetMetadata() const;

private:
    //
    // Motor motions. These act as virtual stick controllers, as if the drive itself
    // has another remote operator.
    //
    int *_motorControlActions;
    uint8_t _motorControlActionsSize;

    //
    // Sound selection
    //
    SoundId _soundType;

    //
    // Metadata pointer
    //
    void *_metadata;

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
    // @summary Gets the id associated with this instance.
    //
    // @ret     Id for this instance.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual uint16_t Id() const = 0;

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
    virtual const AnimationStep* RunIteration() = 0;
};


///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that defines a scripted animation. These animations are built up
//          using arrays of AnimationStep objects that are defined in the initial
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
    //          This is the size of the animationSteps array, which is how many
    //          steps the script will run.
    //
    // @param   defaultResult
    //          Animation step that will be returned if not running.
    //
    // @param   animationSteps
    //          Array of all the steps that will be executed for this script.
    ///////////////////////////////////////////////////////////////////////////////////
    ScriptedAnimation(
        AnimationTarget aTarget,
        int numOfSteps,
        const AnimationStep *defaultResult,
        const AnimationStep *animationSteps);

    //
    // IAnimation interface implementation
    //
    bool IsRunning() const;
    AnimationTarget Target() const;
    uint16_t Id() const;
    void Start();
    void Stop();
    const AnimationStep* RunIteration();

private:
    bool _isRunning;
    unsigned long _currentMillis;
    int _currentAnimationIndex;
    int _numberOfAnimationSteps;
    uint16_t _id;
    const AnimationStep *_animationSteps;
    const AnimationStep *_defaultResult;
    AnimationTarget _animationTarget;
};

class GeneratedAnimationPercents
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs an instance of the GeneratedAnimationPercents class.
    //
    // @param   actionArray
    //          Array of actions that will be supported by the animation. The size of
    //          this array must be actionSize. Note that MotorControl is broken down
    //          further by motorControlArray below.
    //
    // @param   actionPercents
    //          Percent that the corresponding action from actionArray will be executed.
    //          The size of this array must be actionSize.
    //
    // @param   actionSize
    //          Size of the actionArray and actionPercents arrays.
    //
    // @param   motorControlArray
    //          Array of all the different types of motor control stick values this
    //          animation supports.
    //
    // @param   motorControlPercents
    //          Percent that an individual motor control will be selected for this step.
    //          Note that these will add to 100%, but that's 100% AFTER MotroControl was
    //          selected for the action of the current step.
    //
    // @param   motorControlSize
    //          Size of the motorControlArray and motroControlPercents arrays.
    //
    // @param   msArray
    //          Supported milliseconds that a randomly selected animation step can run.
    //          The size of this array must be msSize.
    //
    // @param   msPercents
    //          Percent that the corresponding milliseconds will be selected as the
    //          duration from msArray. Size of this array must be msSize.
    //
    // @param   msSize
    //          The size of msArray and msPercents arrays.
    //
    // @param   frStickArray
    //          Array of supported forward/reverse movement amounts. The size of this
    //          array must be frStickSize.
    //
    // @param   frStickPercents
    //          Percent that the corresponding stick value from frStickArray will be
    //          randomly selected. The size of this array must be frStickSize.
    //
    // @param   frStickSize
    //          The size of frStickArray and frStickPercents arrays.
    //
    // @param   lrStickArray
    //          Array of supoprted left/right movement amounts. The size of this array
    //          must be lrStickSize.
    //
    // @param   lrStickPercents
    //          Percent that the corresponding stick value from lrStickArray will be
    //          randomly selected. The size of this array must be lrStickSize.
    //
    // @param   lrStickSize
    //          The size of lrStickArray and lrStickPercents arrays.
    //
    // @param   motorIdsUsingFR
    //          Array of all the motor control ids that use the FR stick.
    //
    // @param   motorIdsUsingFRSize
    //          Size of the motorIdsUsingFR array.
    //
    // @param   pausePercent
    //          Percent chance that the current randomly generated action will be a
    //          puase. Set to zero (0) to only generate non-pause actions; One hundred
    //          (100) would be to always pause.
    ///////////////////////////////////////////////////////////////////////////////////
    GeneratedAnimationPercents(
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
        const uint8_t *motorIdsUsingFR,
        uint8_t motorIdsUsingFRSize,
        uint8_t pausePercent);

private:
    const AnimationAction *_actionArray;
    const uint16_t *_actionPercents;
    const uint8_t *_controlArray;
    const uint16_t *_controlPercents;
    const uint16_t *_msArray;
    const uint16_t *_msPercents;
    const uint16_t *_frStickArray;
    const uint16_t *_frStickPercents;
    const uint16_t *_lrStickArray;
    const uint16_t *_lrStickPercents;
    const uint8_t *_motorIdsUsingFR;
    uint8_t _actionSize, _controlSize, _msSize, _frStickSize, _lrStickSize, _mFrSize;
    uint8_t _pausePercent;
    uint16_t _actionTot, _controlTot, _msTot, _frStickTot, _lrStickTot;

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
//          of AnimationStep variables, this class will have one state that it
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
    // @param   metadata
    //          Metadata that will be associated with this animation. It is up to the
    //          caller to sort out recasting back into the appropriate datatype. It is
    //          recommended to use the same type for the entire program.
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
    // @param   numSounds
    //          Number of unique sounds your drive supports. Note that a special
    //          "Not Playing" sounds needs to be accounted for; that is, if the
    //          drive supports "short" and "long" sounds, this value should be three
    //          (3) to include a "Not Playing" sound.
    //
    // @param   soundTimeout
    //          Length in milliseconds before another sound can be generated. Higher
    //          values prevent spamming the system with talking, but less overall
    //          sounds will be generated.
    //
    // @param   initialStep
    //          Memory address for the initial step. Note that this value will be
    //          modified, so do not use the same one for any scripted animations.
    ///////////////////////////////////////////////////////////////////////////////////
    GeneratedAnimation(
        AnimationTarget aTarget,
        GeneratedAnimationPercents *percents,
        void *metadata,
        uint8_t minNumAnimationSteps,
        uint8_t maxConcurentActions,
        uint8_t numSounds,
        uint16_t soundTimeout,
        AnimationStep *initialStep);

    //
    // IAnimation interface implementation
    //
    bool IsRunning() const;
    AnimationTarget Target() const;
    uint16_t Id() const;
    void Start();
    void Stop();
    const AnimationStep* RunIteration();

private:
    void AutoGenerateNextState();
    uint8_t AutoGenerateAction();
    uint8_t AutoGenerateMotorControlId();
    int AutoGenerateStickAmount(uint8_t motroControlId);
    void Clear();

    AnimationTarget _animationTarget;
    unsigned long _currentMillis, _soundTimeout, _lastSoundMillis;
    uint16_t _id;
    uint8_t _minNumAnimationSteps, _animationStepCount, _maxConcurentActions, _numSounds;
    bool _isRunning;
    AnimationStep *_currentResult;
    void *_metadata;
    GeneratedAnimationPercents *_percents;
};

}   //namepsace Naigon::Animagions

#endif //__NaigonBB8_Animations_h__
