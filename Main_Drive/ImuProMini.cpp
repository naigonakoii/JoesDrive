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

#include "ImuProMini.h"

namespace Naigon::BB_8
{

// Declare the method that will occur below.
float updatePrevValsAndComputeAvg(float *nums, float currentVal);

ImuProMini::ImuProMini()
    : _isFirstPitchAndRoll(true)
    , _proMiniConnected(true)
{
}

float ImuProMini::Pitch() const
{
    return _pitch;
}

float ImuProMini::Roll() const
{
    return _roll;
}

float ImuProMini::FilteredPitch() const
{
    return _filteredPitch;
}

float ImuProMini::FilteredRoll() const
{
    return _filteredRoll;
}

bool ImuProMini::ProMiniConnected() const
{
    return _proMiniConnected;
}

void ImuProMini::UpdateIteration(float pitch, float roll, int imuLoop)
{
    if (imuLoop == 0)
    {
        return;
    }

    _pitch = reversePitch ? pitch * -1 : pitch;
    _roll = reverseRoll ? roll * -1 : roll;

    if (_isFirstPitchAndRoll)
    {
        // Naigon - Head Tilt Stabilization
        // Initialize the first time to the current value to prevent anomalies at startup.
        for (int i = 0; i < PitchAndRollFilterCount; i++)
        {
            _pitchPrev[i] = _pitch;
            _rollPrev[i] = _roll;
        }
        _isFirstPitchAndRoll = false;
    }

    //
    // Naigon - Head Tilt Stablilization
    // The pitch and roll are now computed as a rolling average to filter noise. This prevents jerkyness in any movements
    // based on these values.
    _filteredPitch = updatePrevValsAndComputeAvg(_pitchPrev, _pitch);
    _filteredRoll = updatePrevValsAndComputeAvg(_rollPrev, _roll);

    // After setting the values, update the status to know if the values are okay.
    CheckProMiniTime(imuLoop);
}

float updatePrevValsAndComputeAvg(float *nums, float currentVal)
{
    float sum = 0;
    nums[0] = currentVal;
    for (int i = PitchAndRollFilterCount - 1; i >= 1; i -= 1)
    {
        nums[i] = nums[i - 1];
        sum += nums[i];
    }

    return (sum + nums[0]) / (float)PitchAndRollFilterCount;
}

void ImuProMini::CheckProMiniTime(int imuLoop)
{
    if ((imuLoop == 1 && _lastLoopMillis >= 980)
        || (imuLoop < 1 && _lastLoopMillis > 3))
    {
        _lastLoopMillis = 0;
    }

    if (imuLoop > _lastLoopMillis)
    {
        _lastLoopMillis = imuLoop;
        _proMiniConnected = true;
    }
    else if (imuLoop <= _lastLoopMillis && _proMiniConnected)
    {
        _lastLoopMillis++;
    }

    if (imuLoop - _lastLoopMillis < -20 && imuLoop - _lastLoopMillis > -800)
    {
        _proMiniConnected = false;
        _lastLoopMillis = 0;
        //imuLoop = 0;
    }

    if (_lastLoopMillis >= 999)
    {
        _lastLoopMillis = 0;
    }
}

}   //namespace Naigon::BB_8
