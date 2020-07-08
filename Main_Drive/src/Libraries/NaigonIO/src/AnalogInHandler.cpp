// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//             AnalogInHandler
//             Library for wrapping Arduino analog ins to a reduced output range. Part of the NaigonIO library.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             Scott DeBoer
//             28 June 2020
// ====================================================================================================================

#include "AnalogInHandler.h"
#include "ArduinoFMath.h"

namespace Naigon::IO
{

AnalogInHandler::AnalogInHandler(
    int minInput,
    int maxInput,
    bool isInputReversed,
    float minOutput,
    float maxOutput,
    float movementThreshold)
    : _minInput(minInput)
    , _maxInput(maxInput)
    , _minOutput(minOutput)
    , _maxOutput(maxOutput)
    , _isInputReversed(isInputReversed)
{
    float center = ((maxOutput - minOutput) / 2.0) + minOutput;
    _lowThresh = center - movementThreshold;
    _highThresh = center + movementThreshold;
}

bool AnalogInHandler::HasMovement() const
{
    return _value > _highThresh || _value < _lowThresh;
}

float AnalogInHandler::GetMappedValue() const
{
    return _value;
}

float AnalogInHandler::UpdateState(int inputValue)
{
    // Ensure that the input value does not exceed the specified bounds.
    inputValue = constrain(inputValue, _minInput, _maxInput);

    float minOutput = _isInputReversed ? _maxOutput : _minOutput;
    float maxOutput = _isInputReversed ? _minOutput : _maxOutput;

    float adjustedOutput = mapf((float)inputValue, (float)_minInput, (float)_maxInput, minOutput, maxOutput);
    _value = constrain(adjustedOutput, _minOutput, _maxOutput);
    return _value;
}

}   //namespace Naigon::IO
