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

#include "EaseApplicator.h"

namespace NaigonBB8
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
// SCurveEaseApplicator
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
    return ComputeFunction(_linearApplicator.ComputeValueForCurrentIteration(target));
}

double FunctionEaseApplicator::ComputeFunction(double unscaled) const
{
    double sign = unscaled < 0.0 ? -1.0 : 1.0;

    // Scale the value to the range 0.0 to 1.0
    double x = abs(unscaled / _maxValue);

    double fx = _fType == FunctionEaseApplicatorType::SCurve
        ? ComputeS(x)
        : ComputeQ(x);

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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}   // namespace NaigonBB8