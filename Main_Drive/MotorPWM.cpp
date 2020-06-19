#include "MotorPWM.h"

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
