// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

#include "AnimationDefinitions.h"
#include "SoundPlayer.h"

namespace NaigonBB8
{

const int RightHalf = 255 / 2;
const int LeftHalf = 512 - RightHalf;

const int RightTwoThirds = 255 / 3;
const int LeftTwoThirds = 512 - RightTwoThirds;

const int RightThreeFourths = 255 / 4;
const int LeftThreeFourths = 512 - RightThreeFourths;

const int RightFull = 0;
const int LeftFull = 512;

#define ForwardHalf RightHalf
#define ReverseHalf  LeftHalf

#define ForwardTwoThirds RightTwoThirds
#define ReverseTwoThirds LeftTwoThirds

#define ForwardThreeFourths RightThreeFourths
#define ReverseThreeFourths LeftThreeFourths

#define ForwardFull RightFull
#define ReverseFull LeftFull


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// DomeAnimation
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationState headLeftTurn1State[] = {
    AnimationState(255, 255, 255, 255, LeftTwoThirds, 255, SoundTypes::NotPlaying, 250),
    AnimationState(255, 255, 255, 255, RightTwoThirds, 255, SoundTypes::NotPlaying, 250),
};

AnimationState headRightTurn1State[] = {
    AnimationState(255, 255, 255, 255, RightTwoThirds, 255, SoundTypes::NotPlaying, 250),
    AnimationState(255, 255, 255, 255, LeftTwoThirds, 255, SoundTypes::NotPlaying, 250),
};

AnimationState headLeftTurn2State[] = {
    AnimationState(255, 255, 255, 255, LeftThreeFourths, 255, SoundTypes::NotPlaying, 250),
    AnimationState(255, 255, 255, 255, 255, 255, SoundTypes::NotPlaying, 333),
    AnimationState(255, 255, 255, 255, RightThreeFourths, 255, SoundTypes::NotPlaying, 250),
};

AnimationState headRightTurn2State[] = {
    AnimationState(255, 255, 255, 255, RightThreeFourths, 255, SoundTypes::NotPlaying, 250),
    AnimationState(255, 255, 255, 255, 255, 255, SoundTypes::NotPlaying, 333),
    AnimationState(255, 255, 255, 255, LeftThreeFourths, 255, SoundTypes::NotPlaying, 250),
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Full Animations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

AnimationState TiltHeadAndLookBothWays1[] = {
    AnimationState(255, 255, ForwardFull, 255, LeftHalf, 255, SoundTypes::NotPlaying, 250),
    AnimationState(255, 255, ForwardFull, 255, 255, 255, SoundTypes::Excited, 0),
    AnimationState(255, 255, ForwardFull, 255, 255, 255, SoundTypes::NotPlaying, 200),
    AnimationState(255, 255, ForwardFull, 255, RightHalf, 255, SoundTypes::NotPlaying, 500),
    AnimationState(255, 255, ForwardFull, 255, LeftHalf, 255, SoundTypes::NotPlaying, 500),
    AnimationState(255, 255, ForwardHalf, 255, 255, 255, SoundTypes::NotPlaying, 100),
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This needs to be the number of entries in the animations array below.
/* static */ uint16_t AnimationDefinitions::NumberOfAnimations = 5;

// Array of entire animations that will be used to initialize the AnimationRunner in the main file.
/* static */ Animation AnimationDefinitions::animations[] = {
    Animation(AnimationTarget::DomeAnimation, 2, headLeftTurn1State),
    Animation(AnimationTarget::DomeAnimation, 2, headRightTurn1State),
    Animation(AnimationTarget::DomeAnimation, 3, headLeftTurn2State),
    Animation(AnimationTarget::DomeAnimation, 3, headRightTurn2State),
    Animation(AnimationTarget::FullAnimation, 6, TiltHeadAndLookBothWays1),
};

}   //namespace NaigonBB8
