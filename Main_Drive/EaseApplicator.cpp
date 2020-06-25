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
SCurveEaseApplicator::SCurveEaseApplicator(double initialValue, double maxValue, double incrementAmount)
    : _linearApplicator(initialValue, incrementAmount)
    , _maxValue(maxValue)
{
}

double SCurveEaseApplicator::GetMaxValue() const
{
    return _maxValue;
}

double SCurveEaseApplicator::GetValue() const
{
    return ComputeS(_linearApplicator.GetValue());
}

double SCurveEaseApplicator::GetIncrement() const
{
    return _linearApplicator.GetIncrement();
}

double SCurveEaseApplicator::ComputeValueForCurrentIteration(double target)
{
    return ComputeS(_linearApplicator.ComputeValueForCurrentIteration(target));
}

double SCurveEaseApplicator::ComputeS(double x) const
{
    double sign = x < 0.0 ? -1.0 : 1.0;

    x = abs(x / _maxValue);

    double fx = x < 0.5
        ? 2.0 * x * x
        : -2.0 * (x - 1.0) * (x - 1.0) + 1.0;
    
    return fx * _maxValue * sign;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}   // namespace NaigonBB8