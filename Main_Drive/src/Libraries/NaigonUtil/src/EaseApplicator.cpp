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
//             19 June 2020
// ====================================================================================================================


#include "EaseApplicator.h"

namespace Naigon::Util
{

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// LinearEaseApplicator
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LinearEaseApplicator::LinearEaseApplicator(double initialValue, double incrementAmount)
    : _value(initialValue)
    , _incrementAmount(incrementAmount)
{
}

double LinearEaseApplicator::GetValue() const
{
    return _value;
}

double LinearEaseApplicator::GetIncrement() const
{
    return _incrementAmount;
}

double LinearEaseApplicator::ComputeValueForCurrentIteration(double target)
{
    if (abs(target - _value) < _incrementAmount)
    {
        // Target is very close, directly set it.
        _value = target;
    }
    else if (target > _value)
    {
        _value += _incrementAmount;
    }
    else
    {
        _value -= _incrementAmount;
    }

    return _value;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FunctionEaseApplicator
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FunctionEaseApplicator::FunctionEaseApplicator(
    double initialValue,
    double maxValue,
    double incrementAmount,
    FunctionEaseApplicatorType fType)
    : _linearApplicator(initialValue, incrementAmount)
    , _fType(fType)
    , _maxValue(maxValue)
{
}

double FunctionEaseApplicator::GetMaxValue() const
{
    return _maxValue;
}

double FunctionEaseApplicator::GetValue() const
{
    return ComputeS(_linearApplicator.GetValue());
}

double FunctionEaseApplicator::GetIncrement() const
{
    return _linearApplicator.GetIncrement();
}

double FunctionEaseApplicator::ComputeValueForCurrentIteration(double target)
{
    return ComputeFunction(_linearApplicator.ComputeValueForCurrentIteration(target), target);
}

double FunctionEaseApplicator::ComputeFunction(double unscaled, double target) const
{
    double sign = unscaled < 0.0 ? -1.0 : 1.0;

    // Scale the value to the range 0.0 to 1.0
    double x = abs(unscaled / _maxValue);

    double fx;
    
    if(_fType == FunctionEaseApplicatorType::SCurve)
    {
        fx = ComputeS(x);
    }
    else if (_fType == FunctionEaseApplicatorType::Quadratic)
    {
        fx = ComputeQ(x);
    }
    else
    {
        fx = ComputeReverseQ(x);
    }

    // Unscale to the original range, and reapply the sign.
    return fx * _maxValue * sign;
}

double FunctionEaseApplicator::ComputeS(double x) const
{
    return x < 0.5
        ? 2.0 * x * x
        : -2.0 * (x - 1.0) * (x - 1.0) + 1.0;
}

double FunctionEaseApplicator::ComputeQ(double x) const
{
    return x * x;
}

double FunctionEaseApplicator::ComputeReverseQ(double x) const
{
    return -1.0 * (x - 1.0) * (x - 1.0) + 1.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ScalingEaseApplicator
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MIN_DECREMENT_MULT 1.0 / 100.0

ScalingEaseApplicator::ScalingEaseApplicator(
    double initialValue,
    double maxIncrement,
    double accelerationMs,
    double decelerationMs,
    uint16_t periodLength)
    : _value(initialValue)
    , _maxIncrement(maxIncrement)
    , _accelerationMs(accelerationMs)
    , _decelerationMs(decelerationMs)
    , _periodLength(periodLength)
    , _currentIncrement(0)
{
    _accelerationIncrement = periodLength * maxIncrement / accelerationMs;
    _decelerationIncrement = 0;
}

double ScalingEaseApplicator::GetValue() const
{
    return _value;
}

double ScalingEaseApplicator::GetIncrement() const
{
    return _currentIncrement;
}

double ScalingEaseApplicator::ComputeValueForCurrentIteration(double target)
{
    if (abs(target - _value) < _currentIncrement)
    {
        // Target is very close, directly set it.
        _value = target;
        // Reset the increment now that motion stopped
        _currentIncrement = 0.0;
        _decelerationIncrement = 0.0;
        return _value;
    };

    double msToTarget = (double)abs(target - _value) / _currentIncrement * (double)_periodLength;

    if (msToTarget < _decelerationMs)
    {
        _decelerationIncrement = _decelerationIncrement == 0.0
            ? _periodLength * _maxIncrement / _decelerationMs
            : _decelerationIncrement;

        _currentIncrement = _currentIncrement <= (_maxIncrement * MIN_DECREMENT_MULT)
            ? _currentIncrement - _decelerationIncrement
            : _currentIncrement;
    }
    else if (_currentIncrement < _maxIncrement)
    {
        _currentIncrement += _accelerationIncrement;
    }

    if (target > _value)
    {
        _value += _currentIncrement;
    }
    else
    {
        _value -= _currentIncrement;
    }

    return _value;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}   // namespace Naigon::Util
