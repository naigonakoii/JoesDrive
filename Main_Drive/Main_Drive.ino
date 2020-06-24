// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive - Main drive mechanism using Ardino Mega with secondary Arduino Pro Mini IMU
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

//
// Update these as necessary to match your setup
//
#define enablePin 31       // Pin that provides power to motor driver enable pins
#define enablePinDome 29   // Pin that provides power to Dome motor driver enable pin
#define S2SpotPin A0       // Pin connected to side tilt potentiometer
#define readpin 34         // Pin connected to ACT on soundboard
#define soundpin1 26       // Connected to sound pin 0
#define soundpin2 28       // Connected to sound pin 1
#define soundpin3 30       // Connected to sound pin 2
#define soundpin4 32       // Connected to sound pin 3
#define soundpin5 46       // Connected to sound pin 4
#define soundpin6 44       // Connected to sound pin 5
#define fadePin A4         // Connected to + of one channel on sound board(use resistor to ground)
#define BTstatePin 33      // Connected to state pin on BT Module
#define domeTiltPotPin A1  // Connected to Potentiometer on the dome tilt mast
#define easeDome 20        // Lower number means more easing when spinning
#define easeDomeTilt 3.5   // Lower number means more easing when moving forward and back a.k.a. slower
#define domeSpinPot A2     // Pin used to monitor dome spin potentiometer
#define battMonitor A3     // Pin used to monitor battery voltage
#define outputVoltage 5.2  // This is the output voltage from the Buck Converter powering the arduino
#define drivePWM1 11       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define drivePWM2 12       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM1 3          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM2 4          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM1 5     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM2 6     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM1 9     // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM2 10    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM1 7 // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM2 8 // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define resistor1 121000   // Larger resisitor used on voltage divider to read battery level
#define resistor2 82000    // Smaller resisitor used on voltage divider to read battery level

//
// Naigon - NEC Audio
// Digital I/O pins that connect to my custom Naigon's Electronic Creations Igniter 3 sound player.
//
#define HappySoundPin 41
#define SadSoundPin 43
#define ExcitedSoundPin 45
#define ScaredSoundPin 47
#define ChattySoundPin 49
#define AgitatedSoundPin 51
#define PlayTrackPin 42
#define StopTrackPin 44

#define flywheelEase 3 // Speed in which flywheel will increase/decrease during gradual movements
// S2SEase from Joe: 1.5
#define S2SEase 0.5         // Speed in which side to side moves. Higher number equates to faster movement
#define MaxDomeTiltAngle 22 // Maximum angle in which the dome will tilt. **  Max is 25  **

//#define TiltDomeForwardWhenDriving      // uncomment this if you want to tilt the dome forward when driving.
#define HeadTiltStabilization // uncomment this if you want the head to stabilize on top of the body.

//
// Naigon - Flywheel MK3
// Lighter flywheel means drive moves more before the ball starts to roll. This value is
// factored out as a double to allow tuning here.
//
// This value should be between 0.0 and 1.0 exclusively.
// Joe's default is .05
#define DomeTiltAmount 0.06

#define reverseDrive    // uncomment if your drive joystick is reversed
#define reverseDomeTilt // uncomment if your dome tilt joystick is reversed
//#define reverseS2S                      // uncomment if your side to side joystick is reversed
//#define reverseDomeSpin                 // uncomment if your dome spin joystick is reversed
//#define reverseFlywheel                 // uncomment if your flywheel joystick is reversed

#define reversePitch // reverse Pitch. Test this by moving the drive by hand; the weight/stabilization should move WITH you, NOT AGAINST.
#define reverseRoll  // reverse Roll. Test this by moving the drive by hand; the weight/stabilization should move WITH you, NOT AGAINST.

//#define reverseDomeTiltPot
//#define reverseDomeSpinPot
//#define reverseS2SPot

#ifdef reverseDomeTilt
#define revDome1
#define revDome2 *-1
#else
#define revDome1 *-1
#define revDome2
#endif

// Naigon: Max pot range, Joe's default is 135. This means pot will map to -HeadTiltPotMax, HeadTiltPotMax
#define HeadTiltPotMax 135
// Naigon: Threshold of the pot before actually adjusting the input. Joe's default is 25
#define HeadTiltPotThresh 25
// Naigon: Amount to limit the flywheel when in stationary mode. At full 255, the drive when spun up takes a while to
// slow and respond to the second direction; this allows it to do quicker moves for animatronics, at the expense of not
// being able to spin as much.
#define FlywheelStationaryMax 215
// Naigon: Amount to limit the flywheel in normal drive modes. Full 255 can "lock" the droid for a half second or so.
// To prevent I just cap a bit more than full blast.
#define FlywheelDriveMax 245
// Naigon: Default drive speeds
// Joe had 55, 75, ?
// I found his defaults TOO powerful so toned it down a bit.
#define DRIVE_SPEED_SLOW 45
#define DRIVE_SPEED_MEDIUM 55
#define DRIVE_SPEED_HIGH 75
//
// Naigon - MK3 Flywheel - This value should be updated for the MK3 Flywheel, as more weight towards the outside makes
// it move higher, and it is overall more sensitive.
//
// Defines the side to side output range, ie how much it can move.
// Joe's default is 25.
#define SideToSideMax 20

//
// Naigon - Safe Joystick Button Toggle
//
// Defines the amount that is considered to cause real movement. When the stick is above this value, no press from the
// stick will be registered.
#define JoystickMinMovement 10

// Naigon: Defines the length (in MS) for the auto disable feature to kick in.
// Joe had this hard-coded inline with a value of 3000.
#define AutoDisableMS 4000

// Naigon - Head Tilt Stabilization
// Defines the number of points for the pitch and roll smoothing filter.
// Higher values make movements much smoother, at the expense of a longer delay before the drive catches up to the actual value.
#define PitchAndRollFilterCount 4

// Naigon - Head Tilt Stabilization
// Proportional amount of the stabilization to apply to the head tilt. Higher value means it will respond quicker at the expense of more jerk.
// Value should be between 0.0 and 1.0 inclusively.
#define HeadTiltPitchAndRollProportion 0.8

//
// Debug Defines
// Uncomment at most ONE of these to debug the corresponding values.
//
//#define printRemote              // Uncomment to see values passed from controller
//#define debugS2S                 // Uncomment to see Side tilt variables, PID values, ETC.
//#define debugDrive               // Uncomment to see main drive variables, PID values, ETC.
//#define debugDomeTilt            // Uncomment to see Dome tilt variables, PID values, ETC.
//#define debugdomeRotation        // Uncomment to see Dome rotation variables, PID values, ETC.
//#define debugPSI                 // Uncomment to see PSI values.
//#define printbodyBatt            // Uncomment to see battery level
//#define printYPR                 // Uncomment to see Yaw, Pitch, and Roll
//#define printDome                // Uncomment to see the Dome's Yaw
//#define printOffsets             // Uncomment to see the offsets
//#define debugRSelectMillis
//#define printOutputs
//#define printSoundPins
//#define debugFlywheelSpin
//#define debugSound

// =====================================================================================================================================================================================================
// =====================================================================================================================================================================================================
//

#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include "Arduino.h"
#include <EasyTransfer.h>
#include "ButtonHandler.h"
#include "SoundPlayer.h"
#include "Animations.h"
#include "MotorPWM.h"

using NaigonBB8::MotorPWM;

EasyTransfer RecRemote;
EasyTransfer SendRemote;
EasyTransfer RecIMU;

struct RECEIVE_DATA_STRUCTURE_REMOTE
{
    int ch1; //right joystick up/down
    int ch2; //right joystick left/right
    int ch3; //left joystick up/down
    int ch4; //left joystick left/right
    int ch5; //flywheel
    // but1 (stick 1) from Joe is selecting between dome servo and dome spin
    int but1 = 1; //left select
    // but2 from Joe is audio
    int but2 = 1; //left button 1
    // but3 from Joe is audio
    int but3 = 1; //left button 2
    // but4 from Joe is to trigger dome effects?
    int but4 = 1; //left button 3
    // but5 (stick 2) toggles fwd/rev
    int but5 = 0; //right select (fwd/rev)
    // but6 from Joe is for switching between modes
    int but6 = 1; //right button 1
    // but7 from Joe is for body calibration only currently when holding
    int but7 = 1; //right button 2
    // but8 is for select only
    int but8 = 1; //right button 3 (right select)
    int motorEnable = 1;
};

struct SEND_DATA_STRUCTURE_REMOTE
{
    double bodyBatt = 0.0;
    double domeBattSend;
    int bodyStatus = 0;
    uint8_t bodySpeed;
    uint8_t bodyDirection;
};

struct RECEIVE_DATA_STRUCTURE_IMU
{
    float IMUloop;
    float pitch;
    float roll;
};

enum SpeedToggle : uint8_t
{
    UnknownSpeed = 0,
    Slow = 1,
    Medium = 2,
    Fast = 3,
    Stationary = 4,
    PushToRoll = 5,
};
SpeedToggle LastSpeedEntry = SpeedToggle::PushToRoll;

enum Direction : uint8_t
{
    UnknownDirection = 0,
    Forward = 1,
    Reverse = 2,
};

//
// Naigon - Drive-side (Server-side) Refactor
// Body status is used as an enum to send to the remote. It is the same variable that Joe was sending; this enum just
// quantifies the values, changes the representation (ie 1 used to be body calibration), and adds the Servo value so
// the remote knows to display servo in the corner.
enum BodyStatus
{
    Default = 0,
    Servo = 1,
    BodyCalibration = 101,
    DomeCalibration = 102,
};

// Naigon - Head Tilt Stabilization
// To keep the head tilt from being jerky, do some filtering on the pitch and roll over time.
float pitch;
float pitchPrev[PitchAndRollFilterCount];
float roll;
float rollPrev[PitchAndRollFilterCount];
bool isFirstPitchAndRoll = true;

RECEIVE_DATA_STRUCTURE_REMOTE recFromRemote;
SEND_DATA_STRUCTURE_REMOTE sendToRemote;
RECEIVE_DATA_STRUCTURE_IMU recIMUData;

#include <PID_v1.h> //PID loop from http://playground.arduino.cc/Code/PIDLibrary

// Naigon - Stationary/Wiggle Mode
bool IsStationary = false;

// Naigon - Animations (in progress)
AnimateYes animate;
AnimationState animationState;

// Naigon - Button Handling
// NOTE - should implement for all cases where using buttons in Joe's code.
ButtonHandler button1Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button2Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button3Handler(0 /* onVal */, 1000 /* heldDuration */);
ButtonHandler button5Handler(0 /* onVal */, 2000 /* heldDuration */);
ButtonHandler button6Handler(0 /* onVal */, 2000 /* heldDuration */);

// Naigon
// Refactor code to use the new PWM driver.
MotorPWM drivePwm(drivePWM1, drivePWM2, 0, 2);
MotorPWM sideToSidePWM(s2sPWM1, s2sPWM2, SideToSideMax, 1);
MotorPWM headTiltPWM(domeTiltPWM1, domeTiltPWM2, HeadTiltPotThresh, 0);
MotorPWM domeSpinPWM(domeSpinPWM1, domeSpinPWM2, 0, 20);
MotorPWM flywheelPWM(flywheelSpinPWM1, flywheelSpinPWM2, 0, 10);

int ch4Servo; //left joystick left/right when using servo mode
int currentDomeSpeed;
int domeRotation;

int fadeVal = 0;
int readPinState = 1;

int soundPins[] = {soundpin1, soundpin2, soundpin3, soundpin4};
int randSoundPin;
int soundState;
int musicState;
int autoDisableState;
unsigned long musicStateMillis = 0;

unsigned long autoDisableMotorsMillis = 0;
int autoDisableDoubleCheck;
unsigned long autoDisableDoubleCheckMillis = 0;
int autoDisable;
bool forcedMotorEnable = false;

unsigned long lastLoopMillis;
float lastIMUloop;
int MiniStatus;

int flywheelRotation;

int SaveToEEPROM;

float R1 = resistor1;
float R2 = resistor2;

int joystickDrive;

int ch5PWM;

int driveSpeed = DRIVE_SPEED_SLOW;
int driveAccel;
// the speedArray is used to create an S curve for the 'throttle' of bb8
int speedArray[] = {0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 7, 7, 8, 8, 9,
                    9, 10, 10, 11, 12, 12, 13, 13, 14, 15, 16, 16, 17, 18, 19, 19, 20, 21, 22, 23, 24, 25, 26, 26, 27, 28, 29, 30,
                    31, 32, 33, 33, 34, 35, 36, 37, 37, 38, 39, 40, 40, 41, 42, 42, 43, 44, 44, 45, 45, 46, 46, 47, 47, 48, 48, 49,
                    49, 50, 50, 50, 51, 51, 51, 52, 52, 52, 52, 53, 53, 53, 53, 54, 54, 54, 54, 54, 55, 55, 55, 55, 55};

int joystickS2S;

int joystickDome;
double domeTiltOffset;
int domeTiltPot;

int domeSpinOffset;
int servoMode;

float countdown;

int domeServo = 0;

int S2Spot;

int BTstate = 0;
int speedDomeTilt = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID1 is for side to side tilt
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk1 = 22; // was 13
double Ik1 = 0;
// Naigon - Change this value from .3 to .1 or 0 to remove shakey side to side
double Dk1 = 0.0;
double Setpoint1, Input1, Output1;

PID PID1(&Input1, &Output1, &Setpoint1, Pk1, Ik1, Dk1, DIRECT);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PID2 is for side to side servo
// Naigon - MK3 Flywheel
//
// The following values need tuning if moving to the MK3 flywheel.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk2 = 1.00; // Joe 0.5; M2 Flywheel .4
double Ik2 = 0.00; // was .00
double Dk2 = 0.01; // was .01
double Setpoint2, Input2, Output2;

PID PID2(&Input2, &Output2, &Setpoint2, Pk2, Ik2, Dk2, DIRECT); // PID Setup - S2S stability

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//PID3 is for the main drive
// Naigon - MK3 Flywheel
//
// The following values will need to be updated if doing the MK3 flywheel and adding the balancing weights.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double Pk3 = 5.0; // Joe 5.0;
double Ik3 = 0;
double Dk3 = 0;
double Setpoint3, Input3, Output3;

PID PID3(&Input3, &Output3, &Setpoint3, Pk3, Ik3, Dk3, DIRECT); // Main drive motor

//PID4 is for dome tilt fwd/back
// Naigon - adjust for pid dome tilt control
double Pk4 = 6; // default is 6
double Ik4 = 0;
double Dk4 = 0.05;
double Setpoint4, Input4, Output4;

PID PID4(&Input4, &Output4, &Setpoint4, Pk4, Ik4, Dk4, DIRECT);

double Setpoint5a;

//PID5 is for the dome spin servo
double Kp5 = 4, Ki5 = 0, Kd5 = 0;
double Setpoint5, Input5, Output5, Output5a;

PID PID5(&Input5, &Output5, &Setpoint5, Kp5, Ki5, Kd5, DIRECT);

long setCalibMillis;

float pitchOffset;
float rollOffset;
int potOffsetS2S;
int domeTiltPotOffset;

int bodyCalibState = 0;

#ifdef WirelessSound
// In the future if an XBee is hooked to the Arduino Mega the hard-wired pins will not be needed.
#else
//
// Naigon - NEC Audio.
//
NaigonBB8::SoundMapper mapper(
    HappySoundPin,
    SadSoundPin,
    ExcitedSoundPin,
    ScaredSoundPin,
    ChattySoundPin,
    AgitatedSoundPin,
    PlayTrackPin,
    StopTrackPin);
NaigonBB8::ISoundPlayer *soundPlayer;
#endif

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup()
{
    Serial.begin(115200);
    Serial1.begin(115200);
    Serial3.begin(115200);

    // Naigon - I changed Serial3 for the IMU, so swap these in code.
    RecRemote.begin(details(recFromRemote), &Serial1);
    SendRemote.begin(details(sendToRemote), &Serial1);
    RecIMU.begin(details(recIMUData), &Serial3);

    pinMode(enablePin, OUTPUT);     // enable pin
    pinMode(enablePinDome, OUTPUT); // enable pin for dome spin
    pinMode(BTstatePin, INPUT);     // BT paired status

    pinMode(readpin, INPUT_PULLUP); // read stat of Act on Soundboard
    pinMode(soundpin1, OUTPUT);     // play sound from pin 0 on Soundboard
    pinMode(soundpin2, OUTPUT);     // play sound from pin 1 on Soundboard
    pinMode(soundpin3, OUTPUT);     // play sound from pin 2 on Soundboard
    pinMode(soundpin4, OUTPUT);     // play sound from pin 3 on Soundboard
    pinMode(soundpin5, OUTPUT);     // play sound from pin 4 on Soundboard
    pinMode(soundpin6, OUTPUT);     // play sound from pin 4 on Soundboard

#ifdef WirelessSound
    // TODO - put the wireless sound setup here.
#else
    digitalWrite(soundpin6, HIGH);
    digitalWrite(soundpin5, HIGH);
    digitalWrite(soundpin4, HIGH);
    digitalWrite(soundpin3, HIGH);
    digitalWrite(soundpin2, HIGH);
    digitalWrite(soundpin1, HIGH);

    pinMode(HappySoundPin, OUTPUT);
    pinMode(SadSoundPin, OUTPUT);
    pinMode(ExcitedSoundPin, OUTPUT);
    pinMode(ScaredSoundPin, OUTPUT);
    pinMode(ChattySoundPin, OUTPUT);
    pinMode(AgitatedSoundPin, OUTPUT);
    pinMode(PlayTrackPin, OUTPUT);
    pinMode(StopTrackPin, OUTPUT);

    // Setup the soundPlayer as the one with a wired interface.
    soundPlayer = new NaigonBB8::WiredSoundPlayer(mapper, 200);
#endif

    soundPlayer->ClearSounds();

    // *********** PID setup ***********

    PID1.SetMode(AUTOMATIC); // PID Setup -  S2S SERVO
    PID1.SetOutputLimits(-255, 255);
    PID1.SetSampleTime(15);

    PID2.SetMode(AUTOMATIC); // PID Setup -  S2S Stability
    PID2.SetOutputLimits(-255, 255);
    PID2.SetSampleTime(15);

    PID3.SetMode(AUTOMATIC); // PID Setup - main drive motor
    PID3.SetOutputLimits(-255, 255);
    PID3.SetSampleTime(15);

    PID4.SetMode(AUTOMATIC); // PID Setup - dome tilt
    PID4.SetOutputLimits(-255, 255);
    PID4.SetSampleTime(15);

    PID5.SetMode(AUTOMATIC);
    PID5.SetOutputLimits(-255, 255); // PID Setup - dome spin 'servo'
    PID5.SetSampleTime(15);

    // *********  Read offsets from EEPROM  **********
    pitchOffset = EEPROM.readFloat(0);
    rollOffset = EEPROM.readFloat(4);
    potOffsetS2S = EEPROM.readInt(8);
    domeTiltPotOffset = EEPROM.readInt(12);
    domeSpinOffset = EEPROM.readInt(16);

    if (abs(rollOffset) + abs(pitchOffset) + abs(potOffsetS2S) + abs(domeTiltPotOffset) == 0)
    {
        setOffsetsONLY();
    }

    // Always startup on slow speed.
    sendToRemote.bodySpeed = SpeedToggle::Slow;
}

//     ================================================================================================================
//     ================================================================================================================
//     =====                                            LOOP, bruh!                                               =====
//     ================================================================================================================
//     ================================================================================================================
void loop()
{
    RecIMU.receiveData();

    if (millis() - lastLoopMillis >= 20)
    {
        sendAndReceive();
        checkMiniTime();

        // Update button state for the rest of the functions.
        button1Handler.UpdateState(recFromRemote.but1);
        button2Handler.UpdateState(recFromRemote.but2);
        button3Handler.UpdateState(recFromRemote.but3);
        button5Handler.UpdateState(recFromRemote.but5);
        button6Handler.UpdateState(recFromRemote.but6);

        //sounds();
        handleSounds();
        psiVal();
        readVin();
        BTenable();
        setDriveSpeed();
        reverseDirection();
        bodyCalib();
        updateAnimations();
        movement();
        domeCalib();
        lastLoopMillis = millis();
    }
}

// ====================================================================================================================
// Functions
// ====================================================================================================================

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Send and receive all data
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendAndReceive()
{
    RecRemote.receiveData();
    SendRemote.sendData();

    if (recIMUData.IMUloop != 0)
    {

#ifdef reversePitch
        pitch = recIMUData.pitch * -1;
#else
        pitch = recIMUData.pitch;
#endif

#ifdef reverseRoll
        roll = recIMUData.roll * -1;
#else
        roll = recIMUData.roll;
#endif
    }

    if (isFirstPitchAndRoll == true)
    {
        // Naigon - Head Tilt Stabilization
        // Initialize the first time to the current value to prevent anomilies at startup.
        for (int i = 0; i < 4; i++)
        {
            pitchPrev[i] = pitch;
            rollPrev[i] = roll;
        }
        isFirstPitchAndRoll = false;
    }

    //
    // Naigon - Head Tilt Stablilization
    // The pitch and roll are now computed as a rolling average to filter noise. This prevents jerkyness in any movements
    // based on these values.
    pitch = updatePrevValsAndComputeAvg(pitchPrev, pitch);
    roll = updatePrevValsAndComputeAvg(rollPrev, roll);
}

float updatePrevValsAndComputeAvg(float *nums, float currentVal)
{
    float sum = 0;
    nums[0] = currentVal;
    for (int i = PitchAndRollFilterCount - 1; i >= 1; i -= 1)
    {
        nums[i] = nums[i - 1];
        sum += nums[i];
    }

    return (sum + nums[0]) / (float)PitchAndRollFilterCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MPU6050 stuff
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void checkMiniTime()
{
    if (recIMUData.IMUloop == 1 && lastIMUloop >= 980)
    {
        lastIMUloop = 0;
    }
    else if (recIMUData.IMUloop < 1 && lastIMUloop > 3)
    {
        lastIMUloop = 0;
    }

    if (recIMUData.IMUloop > lastIMUloop)
    {
        lastIMUloop = recIMUData.IMUloop;

        if (MiniStatus != 1)
        {
            MiniStatus = 1;
        }
    }
    else if (recIMUData.IMUloop <= lastIMUloop && MiniStatus != 0)
    {
        lastIMUloop++;
    }

    if (recIMUData.IMUloop - lastIMUloop < -20 && recIMUData.IMUloop - lastIMUloop > -800)
    {
        MiniStatus = 0;
        lastIMUloop = 0;
        recIMUData.IMUloop = 0;
    }

    if (lastIMUloop >= 999)
    {
        lastIMUloop = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sounds
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void sounds() {
  if ((recFromRemote.but2 == 0 || recFromRemote.but7 == 0) && recFromRemote.but8 == 1 && readPinState == 1 && BTstate ==1 && soundState == 0 && playSound == 0) {   
    playSound = 1;
  }///*else if (recFromRemote.but2 == 1 && recFromRemote.but7 == 1 && playSound == 0) {
    //soundState=0;
    //digitalWrite((soundPins[0]), HIGH);
    //digitalWrite((soundPins[1]), HIGH);
    //digitalWrite((soundPins[2]), HIGH);
    //digitalWrite((soundPins[3]), HIGH);
    //digitalWrite((soundPins[4]), HIGH);
  //}//
          
  if ((recFromRemote.but3 == 0) && (readPinState == 1) && (BTstate ==1)) {
    musicState = 1;
    musicStateMillis = millis();
    digitalWrite(soundpin6, LOW);
  }
  else if (recFromRemote.but3 == 1) {
    digitalWrite(soundpin6, HIGH);
  }

  if(playSound == 1) {
    randSoundPin = random(0, 5);
    digitalWrite((soundPins[randSoundPin]), LOW);
    soundMillis = millis();
    playSound = 2;
    
  }
  else if(playSound == 2 && (millis() - soundMillis > 200)) {
    digitalWrite((soundPins[randSoundPin]), HIGH);
    playSound = 0;
  }
}
*/
void handleSounds()
{
    //
    // Naigon - NEC Audio
    // This method handles sending sound to the custom Naigon's Electronic Creations gen 3 BB-8 sound player.
    //
    bool played = false;

    if (
        (button2Handler.GetState() == ButtonState::Pressed || button2Handler.GetState() == ButtonState::Held)
        && soundPlayer->SoundTypeCurrentlyPlaying() == NaigonBB8::SoundTypes::NotPlaying)
    {
        // Button will only do a sound that is deemed "happy".
        int randomType = random(0, 3) * 2;
        if (button2Handler.GetState() == ButtonState::Held)
        {
            randomType += 1;
        }
        soundPlayer->PlaySound((NaigonBB8::SoundTypes)randomType);
        played = true;
    }
    else if (button3Handler.GetState() == ButtonState::Pressed
        && soundPlayer->TrackTypeCurrentlyPlaying() == NaigonBB8::SoundTypes::NotPlaying)
    {
        soundPlayer->PlaySound(NaigonBB8::SoundTypes::PlayTrack);
        played = true;
    }
    else if (button3Handler.GetState() == ButtonState::Held
        && soundPlayer->TrackTypeCurrentlyPlaying() != NaigonBB8::SoundTypes::StopTrack)
    {
        soundPlayer->PlaySound(NaigonBB8::SoundTypes::StopTrack);
        played = true;
    }

    if (!played)
    {
        // If nothing was played, then update this loop with nothing.
        soundPlayer->PlaySound(NaigonBB8::SoundTypes::NotPlaying);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PSI Value
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void psiVal()
{
    // Naigon - Method not needed since dome communicates via XBee to audio sub-board currently.
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read voltage in
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void readVin()
{
    // Naigon
    // I've been having issues where this reads a constant value of 12.86 regardless of my battery. So I'm going to
    // watch it for a while and try to deduce what is wrong.
    //
    // DO NOT TAKE THIS CHANGE
    //
    sendToRemote.bodyBatt = analogRead(battMonitor); //((analogRead(battMonitor) * outputVoltage) / 1024.0) / (R2 / (R1+R2));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Bluetooth enable
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void BTenable()
{
    BTstate = digitalRead(BTstatePin); //read whether BT is connected

    if (recFromRemote.motorEnable == 0 && BTstate == 1)
    { //if motor enable switch is on and BT connected, turn on the motor drivers
        autoDisableMotors();
        digitalWrite(enablePinDome, HIGH);
    }
    else if (recFromRemote.motorEnable == 1 || BTstate == 0)
    { //if motor enable switch is off OR BT disconnected, turn off the motor drivers
        digitalWrite(enablePin, LOW);
        digitalWrite(enablePinDome, LOW);
        autoDisableState = 0;
        autoDisableDoubleCheck = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Drive speed selection
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setDriveSpeed()
{
    incrementBodySpeedToggle();

    if (sendToRemote.bodySpeed == SpeedToggle::Slow)
    {
        driveSpeed = DRIVE_SPEED_SLOW;
    }
    else if (sendToRemote.bodySpeed == SpeedToggle::Medium)
    {
        driveSpeed = DRIVE_SPEED_MEDIUM;
    }
    else if (sendToRemote.bodySpeed == SpeedToggle::Fast)
    {
        driveSpeed = DRIVE_SPEED_HIGH;
    }
    else if (sendToRemote.bodySpeed == SpeedToggle::Stationary)
    {
        // For safety, set the drive speed back to slow, even though the stick shouldn't use it.
        driveSpeed = DRIVE_SPEED_SLOW / 2;
    }

    // Indicate if the droid is in stationary mode only when in the stationary state.
    IsStationary = sendToRemote.bodySpeed == SpeedToggle::Stationary
        ? true
        : false;
}

void incrementBodySpeedToggle()
{
    // Only increment when the button was pressed and released.
    if (button6Handler.GetState() != ButtonState::Pressed)
    {
        return;
    }

    //
    // Naigon - Drive-side (Server-side) Refactor
    // This method was taken directly from the remote code and ported here. In general, the drive should do all state
    // changes and should be the state "master", while the remote ect should just send commands. This will allow
    // secondary controllers to operate on the drive and the drive can maintain a state.
    //
    sendToRemote.bodySpeed =
        sendToRemote.bodySpeed >= (int)LastSpeedEntry || sendToRemote.bodySpeed == SpeedToggle::UnknownSpeed
            ? 1
            : sendToRemote.bodySpeed + 1;
}

void reverseDirection()
{
    // Naigon - Safe Joystick Button Toggle.
    //
    // I've had some pretty catastropic issues where I accidently hit reverse when driving and didn't realize it. This
    // is because the reverse is pressing the drive stick.
    //
    // To prevent that, I'm only going to accept the input when the ch1 and ch2 are below a threshold.
    if (button5Handler.GetState() == ButtonState::Pressed
        && abs(recFromRemote.ch1 - 255) < JoystickMinMovement
        && abs(recFromRemote.ch2 - 255) < JoystickMinMovement)
    {
        sendToRemote.bodyDirection = sendToRemote.bodyDirection == Direction::Forward
            ? Direction::Reverse
            : Direction::Forward;

        // Naigon - Fix for Issue #5
        // Force the motor to enable here if they were disabled.
        forcedMotorEnable = true;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Body calibration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bodyCalib()
{
    if (recFromRemote.but8 == 0 && recFromRemote.but7 == 1)
    {
        timeBodyCalibration();
    }
    else if (
        (recFromRemote.but8 == 1 || recFromRemote.but7 == 0 || recFromRemote.motorEnable == 0) && bodyCalibState != 0)
    {
        bodyCalibState = 0;
    }

    if (sendToRemote.bodyStatus == BodyStatus::BodyCalibration)
    {
        waitForConfirmationToSetOffsets();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Update animations
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void updateAnimations()
{
    // Naigon - Animations
    // TODO: Finish the animation code here.
    /*
  if (recFromRemote.but7 == 0 && !animate.GetIsRunning())
  {
      animate.Start();
  }
  animationState = animate.RunIteration();
  */
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Actual movement
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void movement()
{
    debugRoutines();

    if (SaveToEEPROM != 0)
    {
        setOffsetsAndSaveToEEPROM();
    }

    if (sendToRemote.bodyStatus == BodyStatus::DomeCalibration)
    {
        waitForConfirmationToSetDomeOffsets();
    }

    if (recFromRemote.motorEnable == 0 && BTstate == 1 && MiniStatus != 0)
    {
        unsigned long currentMillis = millis();

        if (sendToRemote.bodySpeed == SpeedToggle::PushToRoll)
        {
            //
            // Naigon - Safe Mode
            //

            // Disallow all input from the remote in this case, and only use stabilization. Remote values come in as zero (0)
            // to 512, so use the middle.
            recFromRemote.ch1 = 255;
            recFromRemote.ch2 = 255;
            recFromRemote.ch3 = 255;
            recFromRemote.ch4 = 255;
            recFromRemote.ch5 = 255;

            // Only move the main drive as s2s and dome could be compromised (flywheel unneeded).
            // The active stabilization will allow pushing the ball to the desired orientation for access.
            mainDrive();
        }
        else
        {
            // Normal modes do all the things.
            sideTilt();
            mainDrive();
            domeTilt();
            flywheelSpin();
        }
    }
    else
    {
        turnOffAllTheThings();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dome calibration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void domeCalib()
{
    ButtonState state = button1Handler.GetState();

    if (
        (sendToRemote.bodyStatus == BodyStatus::Default || sendToRemote.bodyStatus == BodyStatus::Servo)
        && state == ButtonState::Held)
    {
        sendToRemote.bodyStatus = BodyStatus::DomeCalibration;
    }
    else if (
        (sendToRemote.bodyStatus == BodyStatus::Default || sendToRemote.bodyStatus == BodyStatus::Servo)
        && abs(recFromRemote.ch3 - 255) < JoystickMinMovement
        && abs(recFromRemote.ch4 - 255) < JoystickMinMovement
        && state == ButtonState::Pressed)
    {
        // Naigon - Safe Joystick Button Toggle
        // Only swap the states when the joystick is not in movement.
        servoMode = servoMode == BodyStatus::Servo
            ? BodyStatus::Default
            : BodyStatus::Servo;
        sendToRemote.bodyStatus = servoMode;
    }

    if (servoMode == BodyStatus::Default || autoDisable == 1 || recFromRemote.motorEnable == 1)
    {
        domeSpin();
    }
    else if (servoMode == BodyStatus::Servo && autoDisable == 0)
    {
        domeSpinServo();
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Motor Drivers
//
// The following group of functions are for driving the output to the motors.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ------------------------------------------------------------------------------------
// Main drive
// ------------------------------------------------------------------------------------
void mainDrive()
{
#ifdef reverseDrive
    joystickDrive = map(recFromRemote.ch1, 0, 512, driveSpeed, (driveSpeed * -1)); //Read joystick - change -55/55 to adjust for speed.
#else
    joystickDrive = map(recFromRemote.ch1, 0, 512, (driveSpeed * -1), driveSpeed);                  //Read joystick - change -55/55 to adjust for speed.
#endif

    // Naigon - Stationary/Wiggle Mode
    // When in wiggle/stationary mode, don't use the joystick to move at all.
    if (IsStationary == true)
    {
        joystickDrive = 0;
    }

    // Moves through speedArray to match joystick. speedArray is set up to create an 's curve' for increasing/decreasing speed

    if ((joystickDrive > driveAccel) && (driveAccel >= 0))
    {
        driveAccel++;
        Setpoint3 = speedArray[constrain(abs(driveAccel), 0, 110)];
    }
    else if ((joystickDrive < driveAccel) && (driveAccel >= 0))
    {
        driveAccel--;
        Setpoint3 = speedArray[constrain(abs(driveAccel), 0, 110)];
    }
    else if ((joystickDrive > driveAccel) && (driveAccel <= 0))
    {
        driveAccel++;
        Setpoint3 = (speedArray[constrain(abs(driveAccel), 0, 110)] * -1);
    }
    else if ((joystickDrive < driveAccel) && (driveAccel <= 0))
    {
        driveAccel--;
        Setpoint3 = (speedArray[constrain(abs(driveAccel), 0, 110)] * -1);
    }

    Setpoint3 = constrain(Setpoint3, -55, 55);

    Input3 = (pitch + pitchOffset); // - domeOffset;
    // domeTiltOffset used to keep the ball from rolling when dome is tilted front/back

    PID3.Compute();
    writeMotorPwm(drivePwm, Output3, 0 /*input*/, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Side to Side
// ------------------------------------------------------------------------------------

//
//s2s left joystick goes from 0(LEFT) to 512(RIGHT)
//The IMU roll should go DOWN as it tilts to the right, and UP as it tilts to the left
//The side to side pot should go UP as the ball tilts left, and LOW as it tilts right
//
void sideTilt()
{
#ifdef reverseS2S
    joystickS2S = map(constrain(recFromRemote.ch2, 0, 512), 0, 512, SideToSideMax, -SideToSideMax); //- is  left, + is  right
#else
    joystickS2S = map(constrain(recFromRemote.ch2, 0, 512), 0, 512, -SideToSideMax, SideToSideMax); //- is  left, + is  right
#endif

    // Setpoint will increase/decrease by S2SEase each time the code runs until it matches the joystick. This slows the side to side movement.

    if ((Setpoint2 > -S2SEase) && (Setpoint2 < S2SEase) && (joystickS2S == 0))
    {
        Setpoint2 = 0;
    }
    else if ((joystickS2S > Setpoint2) && (joystickS2S != Setpoint2))
    {
        Setpoint2 += S2SEase;
    }
    else if ((joystickS2S < Setpoint2) && (joystickS2S != Setpoint2))
    {
        Setpoint2 -= S2SEase;
    }

#ifdef reverseS2SPot
    S2Spot = map(analogRead(S2SpotPin), 0, 1024, 135, -135);
#else
    S2Spot = map(analogRead(S2SpotPin), 0, 1024, -135, 135);
#endif
    Input2 = roll + rollOffset;
    Setpoint2 = constrain(Setpoint2, -SideToSideMax, SideToSideMax);
    PID2.Compute(); //PID2 is used to control the 'servo' control of the side to side movement.

    Input1 = S2Spot + potOffsetS2S;
    Setpoint1 = map(
        constrain(Output2, -SideToSideMax, SideToSideMax),
        -SideToSideMax,
        SideToSideMax,
        SideToSideMax,
        -SideToSideMax);
    PID1.Compute(); //PID1 is for side to side stabilization
    writeMotorPwm(sideToSidePWM, Output1, Input1, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome tilt
// ------------------------------------------------------------------------------------
void domeTilt()
{
    //
    //The joystick will go from 0(Forward) to 512(Back).
    //The pot will get HIGH as it moves back, and LOW as it moves forward
    //

#ifdef TiltDomeForwardWhenDriving
    // speedDomeTilt offsets the dome based on the main drive to tilt it in the direction of movement.
    speedDomeTilt = Setpoint3 < 3 && Setpoint3 > -3
        ? 0
        : Output3 * DomeTiltAmount;
#else
    speedDomeTilt = 0;
#endif

#ifdef HeadTiltStabilization
    // Naigon - Head Tilt Stabilization
    // Calculate the pitch to input into the head tilt input in order to keep it level.
    // Naigon - TODO: once the ease applicator is created, use it here to increment to pitch adjust.
    int pitchAdjust = (pitch + pitchOffset) * HeadTiltPitchAndRollProportion;
#else
    int pitchAdjust = 0;
#endif

#ifdef reverseDomeTiltPot
    domeTiltPot = (map(analogRead(domeTiltPotPin), 0, 1024, HeadTiltPotMax, -HeadTiltPotMax) + domeTiltPotOffset);
#else
    domeTiltPot = (map(analogRead(domeTiltPotPin), 0, 1024, -HeadTiltPotMax, HeadTiltPotMax) + domeTiltPotOffset);
#endif

    int ch3Val = recFromRemote.ch3;
    if (animationState.hasResult && animationState.ch3 != NOT_RUNNING && abs(ch3Val) < 10)
    {
        ch3Val = animationState.ch3;
    }

    // Naigon: BUG
    // Joe's code had a bug here; you need to subtract within the constrain, otherwise driving can cause this value to go
    // outside the bounds and really bad things happen like the drive locking and losing the head.
    joystickDome = constrain(
        map(ch3Val, 0, 512, -MaxDomeTiltAngle, MaxDomeTiltAngle) - speedDomeTilt - pitchAdjust,
        MaxDomeTiltAngle revDome2,
        MaxDomeTiltAngle revDome1); // Reading the stick for angle -40 to 40

    Input4 = domeTiltPot + (pitch + pitchOffset);

    if ((Setpoint4 > -1) && (Setpoint4 < 1) && (joystickDome == 0))
    {
        Setpoint4 = 0;
    }
    else if (joystickDome > Setpoint4)
    {
        Setpoint4 += easeDomeTilt;
    }
    else if (joystickDome < Setpoint4)
    {
        Setpoint4 -= easeDomeTilt;
    }

    Setpoint4 = constrain(Setpoint4, -MaxDomeTiltAngle, MaxDomeTiltAngle);
    PID4.Compute();
    writeMotorPwm(headTiltPWM, Output4, domeTiltPot, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin
// ------------------------------------------------------------------------------------
void domeSpin()
{
#ifdef reverseDomeSpin
    domeRotation = map(recFromRemote.ch4, 0, 512, 255, -255);
#else
    domeRotation = map(recFromRemote.ch4, 0, 512, -255, 255);
#endif

    if (domeRotation < 3 && domeRotation > -3 && currentDomeSpeed > -15 && currentDomeSpeed < 15)
    {
        domeRotation = 0;
        currentDomeSpeed = 0;
    }

    if ((domeRotation > currentDomeSpeed) && (currentDomeSpeed >= 0))
    {
        currentDomeSpeed += easeDome;
    }
    else if ((domeRotation < currentDomeSpeed) && (currentDomeSpeed >= 0))
    {
        currentDomeSpeed -= easeDome;
    }
    else if ((domeRotation > currentDomeSpeed) && (currentDomeSpeed <= 0))
    {
        currentDomeSpeed += easeDome;
    }
    else if ((domeRotation < currentDomeSpeed) && (currentDomeSpeed <= 0))
    {
        currentDomeSpeed -= easeDome;
    }

    currentDomeSpeed = constrain(currentDomeSpeed, -255, 255);

    // Joe has always allowed the dome to spin regardless of whether the motors were enabled or not, which I like.
    writeMotorPwm(domeSpinPWM, currentDomeSpeed, 0 /*input*/, true /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Flywheel spin
// ------------------------------------------------------------------------------------
void flywheelSpin()
{
    if (IsStationary)
    {
        // Naigon - Stationary/Wiggle Mode
        // When in stationary mode, use the drive stick as the flywheel, as the drive is disabled.
#ifdef reverseFlywheel
        ch5PWM = constrain(map(recFromRemote.ch1, 0, 512, 255, -255), -FlywheelStationaryMax, FlywheelStationaryMax);
#else
        ch5PWM = constrain(map(recFromRemote.ch1, 0, 512, -255, 255), -FlywheelStationaryMax, FlywheelStationaryMax);
#endif
    }
    else
    {
#ifdef reverseFlywheel
        ch5PWM = constrain(map(recFromRemote.ch5, 0, 512, 255, -255), -FlywheelDriveMax, FlywheelDriveMax);
#else
        ch5PWM = constrain(map(recFromRemote.ch5, 0, 512, -255, 255), -FlywheelDriveMax, FlywheelDriveMax);
#endif
    }

    if (ch5PWM > -1 && ch5PWM < 35)
    {
        ch5PWM = 0;
    }
    else if (ch5PWM < 0 && ch5PWM > -35)
    {
        ch5PWM = 0;
    }
    else if (ch5PWM > 35)
    {
        map(ch5PWM, 35, 255, 0, 255);
    }
    else if (ch5PWM < -35)
    {
        map(ch5PWM, -35, -255, 0, -255);
    }

    constrain(ch5PWM, -255, 255);

    if (
        (ch5PWM < -240 && ((flywheelRotation > -30 && flywheelRotation < 30) || flywheelRotation > 240))
        || ((ch5PWM > 240) && ((flywheelRotation > -30 && flywheelRotation < 30) || flywheelRotation < -240)))
    {
        if (ch5PWM > 240)
        {
            flywheelRotation = 255;
        }
        else if (ch5PWM < -240)
        {
            flywheelRotation = -255;
        }
    }
    else if (flywheelRotation < 0 && ch5PWM > 240)
    {
        flywheelRotation = 255;
    }
    else if (flywheelRotation > 0 && ch5PWM < -240)
    {
        flywheelRotation = 255;
    }
    else if (ch5PWM > flywheelRotation)
    {
        flywheelRotation += flywheelEase;
    }
    else if (ch5PWM < flywheelRotation)
    {
        flywheelRotation -= flywheelEase;
    }

    constrain(flywheelRotation, -255, 255);

    writeMotorPwm(flywheelPWM, flywheelRotation, 0 /*input*/, true /*requireBT*/, true /*requireMotorEnable*/);
}

void writeMotorPwm(MotorPWM &motorPwm, int output, int input, bool requireBT, bool requireMotorEnable)
{
    if (
        (requireBT == true && BTstate != 1) || (requireMotorEnable == true && recFromRemote.motorEnable != 0))
    {
        motorPwm.WriteZeros();
    }
    else
    {
        motorPwm.WritePWM(output, input);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Disable droid
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void turnOffAllTheThings()
{
    //disables all PIDS and movement. This is to avoid any sudden jerks when re-enabling motors.
    joystickS2S = 0;
    Input2 = 0;
    Setpoint2 = 0;
    Output2 = 0;
    Input1 = 0;
    Setpoint1 = 0;
    Output1 = 0;
    joystickDrive = 0;
    driveAccel = 0;
    Input3 = 0;
    Setpoint3 = 0;
    Output3 = 0;
    joystickDome = 0;
    Input4 = 0;
    Setpoint4 = 0;
    Output4 = 0;
    flywheelRotation = 0;
    analogWrite(domeSpinPWM2, 0);
    analogWrite(domeSpinPWM1, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set dome rotation to servo mode
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void domeSpinServo()
{
#ifndef reverseDomeRotation
    ch4Servo = map(recFromRemote.ch4, 0, 512, 70, -70);
#else
    ch4Servo = map(recFromRemote.ch4, 0, 512, -70, 70);
#endif

#ifdef reverseDomeSpinPot
    if (recFromRemote.but5 == 1)
    {
        Input5 = ((map(analogRead(domeSpinPot), 0, 1023, -180, 180) + domeSpinOffset) - 180);
    }
    else
    {
        Input5 = map(analogRead(domeSpinPot), 0, 1023, -180, 180) + domeSpinOffset;
    }
#else
    if (recFromRemote.but5 == 1)
    {
        Input5 = ((map(analogRead(domeSpinPot), 0, 1023, 180, -180) + domeSpinOffset) - 180);
    }
    else
    {
        Input5 = map(analogRead(domeSpinPot), 0, 1023, 180, -180) + domeSpinOffset;
    }
#endif
    if (Input5 < -180)
    {
        Input5 += 360;
    }
    else if (Input5 > 180)
    {
        Input5 -= 360;
    }
    else
    {
        Input5 = Input5;
    }

    if ((Setpoint5 > -5) && (Setpoint5 < 5) && (ch4Servo == 0))
    {
        Setpoint5 = 0;
    }
    else if ((ch4Servo > Setpoint5) && (ch4Servo != Setpoint5))
    {
        Setpoint5 += 5;
    }
    else if ((ch4Servo < Setpoint5) && (ch4Servo != Setpoint5))
    {
        Setpoint5 -= 5;
    }

    constrain(Setpoint5, -70, 70);
    PID5.Compute();

    if (Output5 < -4)
    {
        Output5a = constrain(abs(Output5), 0, 255);
        analogWrite(domeSpinPWM1, Output5a);
        analogWrite(domeSpinPWM2, 0);
    }
    else if (Output5 > 4)
    {
        Output5a = constrain(abs(Output5), 0, 255);
        analogWrite(domeSpinPWM2, Output5a);
        analogWrite(domeSpinPWM1, 0);
    }
    else
    {
        analogWrite(domeSpinPWM2, 0);
        analogWrite(domeSpinPWM1, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Count how long right select is pressed.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void timeBodyCalibration()
{
    // TODO: Naigon - get rid of this method and use a button handler.
    unsigned long currentMillisBodyCalib = millis();

    if (recFromRemote.but8 == 0 && recFromRemote.but7 == 1 && bodyCalibState == 0)
    {
        setCalibMillis = millis();
        bodyCalibState = 1;
    }

    if (bodyCalibState == 1 && currentMillisBodyCalib - setCalibMillis >= 3000)
    {
        //setOffsetsAndSaveToEEPROM();
        sendToRemote.bodyStatus = BodyStatus::BodyCalibration;
        bodyCalibState = 0;
    }

#ifdef debugRSelectMillis
    Serial.print(" currentMillisBodyCalib: ");
    Serial.print(currentMillisBodyCalib);
#endif
}

void waitForConfirmationToSetOffsets()
{
    countdown += .15;
    if (countdown > 10 && recFromRemote.but8 == 0 && recFromRemote.motorEnable == 1)
    {
        //countdown = 0;
        SaveToEEPROM = 1;
        //sendToRemote.bodyStatus = 0;
    }
    else if (countdown >= 500)
    {
        // Naigon - Drive-side (Server-side) Refactor
        // Set the body status to whether servo mode is active or not so that can be displayed.
        sendToRemote.bodyStatus = servoMode;
        countdown = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set offsets and save to EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsAndSaveToEEPROM()
{
    if (SaveToEEPROM == 1)
    {
        pitchOffset = pitch * -1;
        EEPROM.writeFloat(0, pitchOffset);
        SaveToEEPROM = 2;
    }
    else if (SaveToEEPROM == 2)
    {
        rollOffset = roll * -1;
        EEPROM.writeFloat(4, rollOffset);
        SaveToEEPROM = 3;
    }
    else if (SaveToEEPROM == 3)
    {
        potOffsetS2S = 0 - (map(analogRead(S2SpotPin), 0, 1024, -135, 135));
        EEPROM.writeInt(8, potOffsetS2S);
        SaveToEEPROM = 4;
    }
    else if (SaveToEEPROM == 4)
    {
        domeTiltPotOffset = 0 - (map(analogRead(domeTiltPotPin), 0, 1024, -HeadTiltPotMax, HeadTiltPotMax));
        EEPROM.writeInt(12, domeTiltPotOffset);
        SaveToEEPROM = 0;
        // Naigon - Drive-side (Server-side) Refactor
        // Set the status to the correct servo mode so the remote will display properly.
        sendToRemote.bodyStatus = servoMode;
        countdown = 0;
        //playSound = 1;
    }
}

void setDomeSpinOffset()
{
    if (recFromRemote.but5 == 1)
    {
        domeSpinOffset = 180 - map(analogRead(domeSpinPot), 0, 1023, 180, -180);
    }
    else
    {
        domeSpinOffset = 0 - map(analogRead(domeSpinPot), 0, 1023, 180, -180);
    }

    EEPROM.writeInt(16, domeSpinOffset);
    // delay(200);
    // Naigon - Drive-side (Server-side) Refactor
    // Set the body status to the current servo mode so the remote will display the info.
    sendToRemote.bodyStatus = servoMode;

    //playSound = 1;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set offsets ONLY; this is used if nothing is stored in EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsONLY()
{
    pitchOffset = 0 - pitch;
    rollOffset = 0 - roll;
    potOffsetS2S = 0 - (map(analogRead(S2SpotPin), 0, 1024, -135, 135));
    domeTiltPotOffset = 0 - (map(analogRead(domeTiltPotPin), 0, 1024, -HeadTiltPotMax, HeadTiltPotMax));
    //delay(200);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Set dome offsets
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void waitForConfirmationToSetDomeOffsets()
{
    if (servoMode == BodyStatus::Servo)
    {
        // Naigon - Drive-side (Server-side) Refactor
        // Might want to remove this so that servo mode persists after dome offset update.
        servoMode = BodyStatus::Default;
    }

    countdown += .15;
    if (countdown > 5 && recFromRemote.but8 == 0 && recFromRemote.motorEnable == 1)
    {
        setDomeSpinOffset();
        sendToRemote.bodyStatus = servoMode;
    }
    else if (countdown >= 250)
    {
        sendToRemote.bodyStatus = servoMode;
        countdown = 0;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Auto disable motors
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void autoDisableMotors()
{
    double output1A = abs(Output1);
    double output3A = abs(Output3);

    if (
        (joystickDrive > -2 && joystickDrive < 2)
        && (joystickS2S > -2 && joystickS2S < 2)
        // Naigon - Head Tilt Stabilization: Add a bit more tolerance here since the stabilization adds to the stick.
        && (joystickDome > -10 && joystickDome < 10)
        && (flywheelRotation < 25 && flywheelRotation > -25)
        && (recFromRemote.ch4 < 276 && recFromRemote.ch4 > 236)
        && (autoDisableState == 0))
    {
        autoDisableMotorsMillis = millis();
        autoDisableState = 1;
    }
    else if (
        joystickDrive < -2
        || joystickDrive > 2
        || joystickS2S < -2
        || joystickS2S > 2
        // Naigon - Head Tilt Stabilization: Add a bit more tolerance here since the stabilization adds to the stick.
        || joystickDome < -10
        || joystickDome > 10
        || flywheelRotation > 30
        || flywheelRotation < -30
        || recFromRemote.ch4 > 276
        || recFromRemote.ch4 < 236
        || forcedMotorEnable == true)
    {
        autoDisableState = 0;
        digitalWrite(enablePin, HIGH);
        autoDisableDoubleCheck = 0;
        autoDisable = 0;
        forcedMotorEnable = false;
    }

    if (autoDisableState == 1 && (millis() - autoDisableMotorsMillis) >= (unsigned long)AutoDisableMS && output1A < 25 && output3A < 8)
    {
        digitalWrite(enablePin, LOW);
        autoDisable = 1;
    }
    else if (output1A > 50 || output3A > 20)
    {
        autoDisableState = 0;
        digitalWrite(enablePin, HIGH);
        autoDisableDoubleCheck = 0;
        autoDisable = 0;
    }
    else if ((output1A > 25 || output3A > 8) && autoDisableDoubleCheck == 0)
    {
        autoDisableDoubleCheckMillis = millis();
        autoDisableDoubleCheck = 1;
    }
    else if ((autoDisableDoubleCheck == 1) && (millis() - autoDisableDoubleCheckMillis >= 100))
    {
        if (output1A > 30 || output3A > 8)
        {
            autoDisableState = 0;
            digitalWrite(enablePin, HIGH);
            autoDisableDoubleCheck = 0;
            autoDisable = 0;
        }
        else
        {
            autoDisableDoubleCheck = 0;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Debug
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void debugRoutines()
{
    // Uncomment " #Define " above
#ifdef debugDrive
    Serial.print(F(" joystickDrive: "));
    Serial.print(joystickDrive);
    Serial.print(F(" accel: "));
    Serial.print(driveAccel);
    Serial.print(F(" SetDrive: "));
    Serial.print(Setpoint3);
    Serial.print(F(" InDrive: "));
    Serial.print(Input3);
    Serial.print(F(" OutDrive: "));
    Serial.print(Output3);
#endif

#ifdef debugS2S
    Serial.print(F(" joystickS2S: "));
    Serial.print(joystickS2S);
    Serial.print(F(" Roll: "));
    Serial.print(roll);
    Serial.print(F(" RollOffset: "));
    Serial.print(rollOffset);
    Serial.print(F(" S2SPot: "));
    Serial.print(S2Spot);
    Serial.print(F(" In2: "));
    Serial.print(Input2);
    Serial.print(F(" Set2: "));
    Serial.print(Setpoint2);
    Serial.print(F(" Out2/Set1: "));
    Serial.print(Output2);
    Serial.print(F(" In1: "));
    Serial.print(Input1);
    Serial.print(F(" Out1: "));
    Serial.println(Output1);
#endif

#ifdef debugDomeTilt
    Serial.print(F(" joystickDome: "));
    Serial.print(joystickDome);
    Serial.print(F(" In4 :"));
    Serial.print(Input4);
    Serial.print(F(" Set4 :"));
    Serial.print(Setpoint4);
    Serial.print(F(" Out4 :"));
    Serial.println(Output4);
#endif

#ifdef debugdomeRotation
    Serial.print(F(" domeRotation: "));
    Serial.print(domeRotation);
    Serial.print(F(" currentDomeSpeed: "));
    Serial.print(currentDomeSpeed);
    Serial.print(F(" ch4Servo: "));
    Serial.print(ch4Servo);
    Serial.print(F(" In5: "));
    Serial.print(Input5);
    Serial.print(F(" Set5: "));
    Serial.print(Setpoint5);
    Serial.print(F(" Out5: "));
    Serial.print(Output5);
    //Serial.print(F(" yawOffset: "));
    //Serial.print(yawOffset);
    Serial.print(F(" domeServo: "));
    Serial.print(domeServo);
    //Serial.print(F(" domeYaw: "));
    //Serial.print(recFromDome.domeYaw);
    //Serial.print(F(" yaw: "));
    //Serial.print(yaw);
    Serial.print(F(" pot: "));
    Serial.println(analogRead(domeSpinPot));
#endif

#ifdef debugSound
    Serial.print(F(" playing sound: "));
    Serial.println(soundPlayer->SoundTypeCurrentlyPlaying());
#endif

#ifdef debugPSI
    Serial.print(F(" readPinState: "));
    Serial.print(readPinState);
    Serial.print(F(" fadeVal: "));
    Serial.print(fadeVal);
#endif

#ifdef printbodyBatt
    Serial.print(F(" Vin: "));
    Serial.print(sendToRemote.bodyBatt);
#endif

#ifdef printYPR
    //Serial.print(F(" Yaw: "));
    //Serial.print(yaw);
    Serial.print(F(" Roll: "));
    Serial.print(roll);
    Serial.print(F("  Pitch: "));
    Serial.println(pitch);
#endif

#ifdef printDome
    //Serial.print(F(" Dome Yaw: "));
    //Serial.print(recFromDome.domeYaw);
    Serial.print(F(" Dome Batt: "));
    Serial.print(recFromDome.domeBatt);
#endif

#ifdef printRemote
    Serial.print(F("  Remote: "));
    Serial.print(recFromRemote.ch1);
    Serial.print(" , ");
    Serial.print(recFromRemote.ch2);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch3);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch4);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.ch5);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but1);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but2);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but3);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but4);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but5);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but6);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but7);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.but8);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.motorEnable);
    Serial.print('\n');
#endif

#ifdef printOffsets
    Serial.print(" pitchOffset: ");
    Serial.print(pitchOffset);
    Serial.print(" rollOffset: ");
    Serial.print(rollOffset);
    Serial.print(" potOffsetS2S: ");
    Serial.print(potOffsetS2S);
    Serial.print("domeTiltPotOffset: ");
    Serial.println(domeTiltPotOffset);
#endif

#ifdef debugRSelectMillis
    //Serial.print(" currentMillisBodyCalib: ");
    //Serial.print(currentMillisBodyCalib);
    Serial.print(" setCalibMillis: ");
    Serial.print(setCalibMillis);
    Serial.print(" motorEnable: ");
    Serial.print(recFromRemote.motorEnable);
    Serial.print(" bodyCalibState: ");
    Serial.print(bodyCalibState);
    Serial.print(" bodyStatus: ");
    Serial.print(sendToRemote.bodyStatus);
    Serial.print(" countdown: ");
    Serial.print(countdown);
#endif

#ifdef printOutputs
    Serial.print(F(" Out1: "));
    Serial.print(abs(Output1));
    Serial.print(F(" Out2: "));
    Serial.print(abs(Output2));
    Serial.print(F(" Out3: "));
    Serial.print(abs(Output3));
    Serial.print(F(" Out4: "));
    Serial.println(abs(Output4));
#endif

#ifdef printSoundPin
    Serial.print(F(" Pin1: "));
    Serial.print(digitalRead(soundpin1));
    Serial.print(F(" Pin2: "));
    Serial.print(digitalRead(soundpin2));
    Serial.print(F(" Pin3: "));
    Serial.print(digitalRead(soundpin3));
    Serial.print(F(" Pin4: "));
    Serial.print(digitalRead(soundpin4));
    Serial.print(F(" Pin5: "));
    Serial.print(digitalRead(soundpin5));
    Serial.print(F(" Pin6: "));
    Serial.print(digitalRead(soundpin6));
    Serial.print(F(" soundState: "));
    Serial.print(soundState);
    Serial.print(F(" readPinState: "));
    Serial.print(digitalRead(readpin));
    Serial.print(F(" randSoundPin: "));
    Serial.println(randSoundPin);
#endif

#ifdef debugFlywheelSpin
    Serial.print(F(" ch5: "));
    Serial.print(recFromRemote.ch5);
    Serial.print(F(" ch5PWM: "));
    Serial.print(ch5PWM);
    Serial.print(F(" flywheelRotation: "));
    Serial.println(flywheelRotation);
#endif
}
