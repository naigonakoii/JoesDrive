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

#include "MotorPWM.h"

namespace NaigonBB8
{

MotorPWM::MotorPWM(int pwmPin1, int pwmPin2, int maxInput, int minOutput)
  : _pwmPin1(pwmPin1)
  , _pwmPin2(pwmPin2)
  , _maxInput(maxInput)
  , _minOutput(minOutput)
{
}

void MotorPWM::WritePWM(int output, int input)
{
  if (output <= -_minOutput
    && (input > -_maxInput || _maxInput == 0))
  {
    analogWrite(_pwmPin1, 0);
    analogWrite(_pwmPin2, abs(output));
  }
  else if (output >= _minOutput
    && (input < _maxInput || _maxInput == 0))
  {
    analogWrite(_pwmPin2, 0);
    analogWrite(_pwmPin1, abs(output));
  }
  else
  {
    WriteZeros();
  }
}

void MotorPWM::WriteZeros()
{
  analogWrite(_pwmPin1, 0);
  analogWrite(_pwmPin2, 0);
}

} // namespace NaigonBB8
