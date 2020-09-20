// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Structs used by the main drive; split out for potential reuse and shrinking
//                         the main file size.
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

#ifndef __MainDrive_Structs_h
#define __MainDrive_Structs_h

#include "Arduino.h"
#include "Constants.h"
#include "Enums.h"

struct RECEIVE_DATA_STRUCTURE_REMOTE
{
    int Joy1Y; //right joystick up/down
    int Joy1X; //right joystick left/right
    int Joy2Y; //left joystick up/down
    int Joy2X; //left joystick left/right
    int Joy3X; //back stick left/right
    int Joy4X;
    // but1 (stick 1) from Joe is selecting between dome servo and dome spin
    // Naigon - this now cycles through the combined drive mode and dome mode.
    uint8_t but1 = 1; //left select
    // but2 from Joe is audio
    // Naigon - button 2 press plays a happy/neutral sound. Holding plays a longer/sader sound
    uint8_t but2 = 1; //left button 1
    // but3 from Joe is audio
    // Naigon - button 3 press starts music, and cycles tracks. Holding stops music.
    uint8_t but3 = 1; //left button 2
    // but4 from Joe is to trigger Body/dome lighting changes
    // Naigon - Button 4 TBD
    uint8_t but4 = 1; //left button 3
    // but5 (stick 2) toggles fwd/rev
    uint8_t but5 = 0; //right select (fwd/rev)
    // but6 from Joe is for switching between drive speeds
    // Naigon - Button 6 is now TBD.
    uint8_t but6 = 1; //right button 1
    // but7 from Joe is for body calibration only currently when holding
    uint8_t but7 = 1; //right button 2
    // but8 is for select only
    uint8_t but8 = 1; //right button 3 (right select)
    uint8_t motorEnable = 1;
};

struct SEND_DATA_STRUCTURE_REMOTE
{
    double bodyBatt = 0.0;
    double domeBattSend;
    uint8_t bodyStatus = 0;
    uint8_t bodyMode = 0;
    uint8_t bodyDirection = 0;
};

//
// Naigon - Serial Audio
// The body now sends everything in one struct, but on the Feather 32u4 it will split out the data to go to the
// remote vs the dome.
struct SEND_DATA_STRUCTURE_FEATHER
{
    double bodyBatt = 0.0;
    double domeBattSend;
    uint8_t bodyStatus = 0;
    uint8_t bodyMode = 0;
    uint8_t bodyDirection = 0;
    uint8_t psi = 0;
};

struct RECEIVE_DATA_STRUCTURE_IMU
{
    float IMUloop;
    float pitch;
    float roll;
};

//
// Dirve state
// State consumed by the movement() method and each motor submethod
//
struct DriveState
{
    // Naigon - Stationary/Wiggle Mode
    // Setting this to true will put the drive in wiggle mode.
    bool IsStationary = false;

    // Setting this will force the dome into servo mode just until the head is centered. This resets animations.
    bool IsDomeCentering = false;

    // Joe - Dome servo or normal mode (was int servoMode in Joe's code)
    DomeMode CurrentDomeMode = DomeMode::FullSpinMode;

    // Save the previous of each type to switch back to previous when toggling between dome modes.
    BodyMode PreviousAnimationMode;
    BodyMode PreviousNormalMode;

    // Naigon - MK3 Head Tilt
    // The head needs to move initially at the beginning to ensure the X will not go past the end stops.
    bool HasDomeMoved;

    // Joe - Allow motors to power down if droid is sitting still.
    bool AutoDisable;

    // Naigon - MK3 Head Tilt
    // This variable indicates if the remote connection state is good. This will either be the BT or feather.
    bool IsConnected;
};

//
// Naigon - Animations
// State consumed by the runAnimations() method.
//
struct AnimationStateVars
{
    // Indicates if the drive is in an automated mode; all modes can run automation via a button press, but in an
    // automated mode the animation is run continuously while driving around.
    bool IsAutomation = false;

    // Indicates if any animation is running. This is needed to know when to force the drive into Servo mode when the
    // animationRunner indicates an animation has stopped.
    bool IsAnimationRunning = false;

    // Stores the dome mode specified by the animation.
    DomeMode AnimationDomeMode;

    // Flags if this animation wants the reduced range stick for safety.
    bool UseReducedDomeStick;
};

//
// State that is used by the auto disable method that needs to persist over multiple loop() calls.
//
struct AutoDisableState
{
    unsigned long autoDisableMotorsMillis = 0;
    int autoDisableDoubleCheck;
    unsigned long autoDisableDoubleCheckMillis = 0;
    bool forcedMotorEnable = false;
    bool isAutoDisabled = true;
};

struct PIDVals
{
    double input;
    double setpoint;
    double output;
};

// Naigon - Animations
// This is the bag that is passed into the Animation State's metadata.
// This is basically a struct but since C++ doesn't support struct init like C99 I went
// with a very basic class to have a constructor.
class AnimationMetadata
{
public:
    AnimationMetadata(DomeMode m, bool s)
        : domeMode(m)
        , useReducedStick(s)
    { };
    DomeMode domeMode;
    bool useReducedStick;
};

#endif // __MainDrive_Structs_h
