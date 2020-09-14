// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a
// copy of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Enums that were on the top of the Main_Drive.ino file; they have been split
//                                       out to minimize the size of that file.
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                         Joe's Drive powered by Naigon
//                         27 May 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// ====================================================================================================================
//                         Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//                         You will need libraries: EepromEX: https://github.com/thijse/Arduino-EEPROMEx
//                                                  PIDLibrary: http://playground.arduino.cc/Code/PIDLibrary
//                                                  EasyTransfer: https://github.com/madsci1016/Arduino-EasyTransfer
//
// ====================================================================================================================
// ====================================================================================================================

#ifndef __MainDrive_Enums_h
#define __MainDrive_Enums_h

enum BodyMode : uint8_t
{
    UnknownSpeed = 0,
    Slow = 1,
    SlowWithTilt = 2,
    Servo = 3,
    ServoWithTilt = 4,
    Stationary = 5,
    Medium = 6,
    Fast = 7,
    PushToRoll = 8,
    Automated = 20,
    AutomatedServo = 21,
    AutomatedTilt = 22,
};
const BodyMode FirstSpeedEntry = BodyMode::Slow;
const BodyMode LastSpeedEntry = BodyMode::PushToRoll;
const BodyMode FirstAutomatedEntry = BodyMode::Automated;
const BodyMode LastAutomatedEntry = BodyMode::AutomatedTilt;

enum Direction : uint8_t
{
    UnknownDirection = 0,
    Forward = 1,
    Reverse = 2,
};

// Naigon - Dome Modes
// The following dome modes are inferred from the BodyMode.
enum DomeMode : uint8_t
{
    UnspecifiedDomeSpin = 0,
    FullSpinMode = 1,
    ServoMode = 2,
};

//
// Naigon - Drive-side (Server-side) Refactor
// Body status is used as an enum to send to the remote. It is the same variable that Joe was sending; this enum just
// quantifies the values, changes the representation (ie 1 used to be body calibration), and adds the Servo value so
// the remote knows to display servo in the corner.
//
// Naigon - Dome Modes
// Through continued refactoring, BodyStatus now only represents the calibration modes. Dome is combined with the drive
// speed which is now renamed as driveMode.
enum BodyStatus : uint8_t
{
    NormalOperation = 0,
    BodyCalibration = 1,
    DomeCalibration = 2,
    JoystickCalibration = 3,
    SaveJoystickValues = 4,
};

//
// Naigon - Animations
// Enum that defines the motor control specific to Joe's Drive.
// Drive | Side | DomeTFB | DomeTLR | DomeSpin | Flywhl
enum MotorControlId : uint8_t
{
    idDrive = 0,
    idSideToSide = 1,
    idDomeTiltFR = 2,
    idDomeTiltLR = 3,
    idDomeSpin = 4,
    idFlywheel = 5,
};

#endif // __MainDrive_Enums_h
