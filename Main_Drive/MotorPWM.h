// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive Powered by Naigon
//                         19 June 2020
//                         Scott DeBoer
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// This is part of the add-on library for Joe's Drive created by Naigon.
// ====================================================================================================================

#ifndef __Naigon_MotorPWM_
#define __Naigon_MotorPWM_

#include "Arduino.h"

namespace Naigon::BB_8
{

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

} // namespace Naigon::BB_8
#endif // __Naigon_MotorPWM_
