// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         File contains specific animation definitions for all animations known to the system.
//                         The individual definitions go in the corresponding cpp file; that allows only the final
//                         top-level array to be exposed to the main file.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                         Joe's Drive powered by Naigon
//                         27 May 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// ====================================================================================================================
// ====================================================================================================================

#ifndef __AnimationDefinitions_h_
#define __AnimationDefinitions_h_

#include "Arduino.h"
#include "Animation.h"

namespace NaigonBB8
{

///////////////////////////////////////////////////////////////////////////////////////
// @summary Static definitions required to construct an AnimationRunner instance.
///////////////////////////////////////////////////////////////////////////////////////
class AnimationDefinitions
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Number of animations that are defined.
    ///////////////////////////////////////////////////////////////////////////////////
    static uint16_t NumberOfAnimations;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary The array of animations that should be passed into the
    //          AnimationRunner instance.
    ///////////////////////////////////////////////////////////////////////////////////
    static Animation animations[];
};

}   //namespace NaigonBB8

#endif  //__AnimationDefinitions_h_
