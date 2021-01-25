// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             ImuProMini
//             Receives data from an external ProMini which is hooked to its own gyro. Values are read over serial.
//             The pro mini currently runs the code directly from Joe's Drive.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             07 July 2020
// ====================================================================================================================

#ifndef __JoesDriveImuProMini_h_
#define __JoesDriveImuProMini_h_

#include "Arduino.h"
#include "Constants.h"
#include "Reverse.h"

namespace Naigon::BB_8
{

///////////////////////////////////////////////////////////////////////////////////////
// @summary Class that wraps the input received from the ProMini and applies filtering
//          along with ensuring the data is fresh.
///////////////////////////////////////////////////////////////////////////////////////
class ImuProMini
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs a new instance of the ImuProMini class.
    ///////////////////////////////////////////////////////////////////////////////////
    ImuProMini();

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current raw pitch value.
    //
    // @ret     Pitch value.
    ///////////////////////////////////////////////////////////////////////////////////
    float Pitch() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current raw roll value.
    //
    // @ret     Roll value.
    ///////////////////////////////////////////////////////////////////////////////////
    float Roll() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current filtered pitch value.
    //
    // @ret     Pitch value.
    ///////////////////////////////////////////////////////////////////////////////////
    float FilteredPitch() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current filtered roll value.
    //
    // @ret     Roll value.
    ///////////////////////////////////////////////////////////////////////////////////
    float FilteredRoll() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Returns the connection state of the ProMini. False would mean that
    //          communication has failed or isn't timely from the ProMini, and thus the
    //          drive should not be operating.
    //
    // @ret     true if the ProMini is in a good connection state; otherwise, false.
    ///////////////////////////////////////////////////////////////////////////////////
    bool ProMiniConnected() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Method that handles raw input from the ProMini each loop iteration.
    //
    // @param   pitch
    //          Raw pitch received from the ProMini.
    //
    // @param   roll
    //          Raw roll value received from the ProMini.
    //
    // @param   int imuLoop
    //          Value in milliseconds from the ProMini, telling the last time interrupt
    //          occurred.
    ///////////////////////////////////////////////////////////////////////////////////
    void UpdateIteration(float pitch, float roll, int imuLoop);

private:
    void CheckProMiniTime(int imuLoop);

    float _pitchPrev[PitchAndRollFilterCount];
    float _rollPrev[PitchAndRollFilterCount];
    float _pitch, _roll;
    float _filteredPitch, _filteredRoll;
    float _lastLoopMillis;
    bool _proMiniConnected, _isFirstPitchAndRoll;
};

}   //namespace Naigon::BB_8

#endif  //__JoesDriveImuProMini_h_
