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


#ifndef __NAIGON_Ease_Applicator_h_
#define __NAIGON_Ease_Applicator_h_

#include "Arduino.h"

namespace Naigon::Util
{

struct IEaseApplicator
{
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Get the current value represented of this class.
    //
    // @ret     Current value.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual double GetValue() const = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Get the current increment set for this class.
    //
    // @ret     Current increment.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual double GetIncrement() const = 0;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Update the current instance based on the new target. This method needs
    //          to be called at each loop iteration.
    //
    // @param   target
    //          Target value for this iteration.
    //
    // @ret     Returns the current value after computation for the current iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    virtual double ComputeValueForCurrentIteration(double target) = 0;
};


class LinearEaseApplicator : public IEaseApplicator
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs and instance of the LinearEaseApplicator class.
    //
    // @param   initialValue
    //          Initial value for this instance.
    //
    // @param   incrementAmount
    //          Amount the current value will be incremented each iteration.
    //
    // @ret     Returns the current value after computation for the current iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    LinearEaseApplicator(double intialValue, double incrementAmount);

    //
    // IEaseApplicator methods
    //
    double GetValue() const;
    double GetIncrement() const;
    double ComputeValueForCurrentIteration(double target);

private:
    double _value;
    double _incrementAmount;
};


enum FunctionEaseApplicatorType : uint8_t
{
    Quadratic = 0,
    SCurve = 1,
    ReverseQuadratic = 2,
};

class FunctionEaseApplicator : public IEaseApplicator
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs an instance of the FunctionEaseApplicator class.
    //
    //          This class applies an S-curve from 0 to 1 inclusively. That value is
    //          then multiplied by the maxValue to get the result.
    //
    //          The S-Curve function is a piece-wise of two quadratic functions:
    //
    //              f(x) = 2x^2,            if 0 <= x < 0.5
    //              f(x) = -2(x-1)^2 + 1,   if 0.5 <= x <= 1.0
    //
    //          The quadratic function is a simple quadratic
    //
    //              f(x) = x^2
    //
    //          Finally, there is a reverse quadratic, which is another simple equation
    //
    //              f(x) = -(x-1)^2 + 1
    //
    //          x above is internally stored in a LinearEaseApplicator instance, Arduino
    //          that value is fed into the s-function.
    //
    // @param   initialValue
    //          Initial value for this instance.
    //
    // @param   maxValue
    //          Max value for this instance; this is the amount f(x) above will be
    //          multiplied by when returning the actual result.
    //
    // @param   incrementAmount
    //          The amount the current instance will be incremented each iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    FunctionEaseApplicator(
        double initialValue,
        double maxValue,
        double incrementAmount,
        FunctionEaseApplicatorType fType);

    double GetMaxValue() const;

    //
    // IEaseApplicator methods
    //
    double GetValue() const;
    double GetIncrement() const;
    double ComputeValueForCurrentIteration(double target);

private:
    double ComputeFunction(double unscaled, double target) const;
    double ComputeS(double x) const;
    double ComputeQ(double x) const;
    double ComputeReverseQ(double x) const;
    double _maxValue;
    LinearEaseApplicator _linearApplicator;
    FunctionEaseApplicatorType _fType;
};

class ScalingEaseApplicator : public IEaseApplicator
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs a new instance of the ScalingEaseApplicator class.
    //
    //          This class applies scaling to the increment value over time to prevent
    //          rough starts and stops of the motors.
    //
    // @param   initialValue
    //          Initial internal value.
    //
    // @param   maxIncrement
    //          This is increment value. Actual increment varies over time but will
    //          always be less than or equal to this value.
    //
    // @param   accelerationMs
    //          Length in milliseconds after movement started that the ramp up occurs.
    //          Actual ramp up will be less if the total time to the target is less
    //          than accelerationMs + decelationMs.
    //
    // @param   decelerationMs
    //          Length in milliseconds that the deceleration will occur.
    //
    // @param   periodLength
    //          Interval in milliseconds between calls to the method
    //          ComputeValueForCurrentIteration.
    ///////////////////////////////////////////////////////////////////////////////////
    ScalingEaseApplicator(
        double initialValue,
        double maxIncrement,
        double accelerationMs,
        double decelationMs,
        uint16_t periodLength);

    //
    // IEaseApplicator methods
    //
    double GetValue() const;
    double GetIncrement() const;
    double ComputeValueForCurrentIteration(double target);

private:
    double _value, _maxIncrement, _currentIncrement;
    double _accelerationMs, _decelerationMs;
    double _accelerationIncrement, _decelerationIncrement;
    uint16_t _periodLength;
};

}   // namespace Naigon::Util

#endif  // #define __NAIGON_Ease_Applicator_h_
