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


#ifndef __AnalogInHandler_h_
#define __AnalogInHandler_h_

#include "Arduino.h"

namespace Naigon::IO
{

class AnalogInHandler
{
public:
    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Constructs an instance of the AnalogInHandler class.
    //
    //          The Analog In handler is responsible for remapping analog input from one
    //          range into the appropriate output range.
    //
    // @param   minInput
    //          Minimum value that is possible for the input.
    //
    // @param   maxInput
    //          Max value possible for the input.
    //
    // @param   isInputReversed
    //          If true, this implies that the increasing input mapps to decreasing output.
    //
    // @param   minOutput
    //          Minimum possible value that the output will be remapped. A call to
    //          'GetMappedValue()' will not return a value less than this.
    //
    // @param   maxOutput
    //          Maximum possible value that the output will be remapped. A call to
    //          'GetMappedValue()' will not return a value less than this.
    //
    // @param   movementThreshold
    //          Amount the value can vary before it is considered that there is input.
    //          This amount is applied to output range so scale accordingly.
    ///////////////////////////////////////////////////////////////////////////////////
    AnalogInHandler(
        int minInput,
        int maxInput,
        bool isInputReversed,
        float minOutput,
        float maxOutput,
        float movementThreshold);

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Indicates whether the stick has any movement or if it is just floating
    //          near center.
    //
    // @ret     'true' if there is stick movement beyound the threshold; otherwise,
    //          'false'.
    ///////////////////////////////////////////////////////////////////////////////////
    bool HasMovement() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Gets the current mapped value associated with the input. Repeated
    //          calls to this method will not change the underlying value. The value
    //          takes into account if this instance is reversed or not.
    //
    // @ret     Mapped value for the output since the last call to
    //          'UpdateInput(int)'.
    ///////////////////////////////////////////////////////////////////////////////////
    float GetMappedValue() const;

    ///////////////////////////////////////////////////////////////////////////////////
    // @summary Updates the value based on the passed in analog input. This method
    //          should be called once per 'loop()'.
    //
    // @param   inputValue
    //          Current analog input value for this loop iteration.
    //
    // @ret     Returns the computed value from this iteration.
    ///////////////////////////////////////////////////////////////////////////////////
    float UpdateState(int inputValue);

private:
    float _value, _minOutput, _maxOutput, _lowThresh, _highThresh;
    int _minInput, _maxInput;
    bool _isInputReversed;
};

}   //namespace Naigon::IO

#endif //__AnalogInHandler_h_
