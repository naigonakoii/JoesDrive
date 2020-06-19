#ifndef __Naigon_MotorPWM_
#define __Naigon_MotorPWM_

#include "Arduino.h"

class MotorPWM
{
public:
  MotorPWM(int pwmPin1, int pwmPin2, int maxInput, int minOutput);

  void WritePWM(int output, int input);
  void WriteZeros();

private:
  int _pwmPin1, _pwmPin2;
  int _maxInput, _minOutput;
};

#endif //__Naigon_MotorPWM_
