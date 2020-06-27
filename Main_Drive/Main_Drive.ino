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


#include "Arduino.h"

#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include <EasyTransfer.h>
#include <PID_v1.h> //PID loop from http://playground.arduino.cc/Code/PIDLibrary

#include "Animations.h"
#include "ButtonHandler.h"
#include "Constants.h"
#include "EaseApplicator.h"
#include "MotorPWM.h"
#include "SoundPlayer.h"

using NaigonBB8::MotorPWM;
using NaigonBB8::FunctionEaseApplicator;
using NaigonBB8::FunctionEaseApplicatorType;
using NaigonBB8::LinearEaseApplicator;

EasyTransfer RecRemote;
EasyTransfer SendRemote;
EasyTransfer RecIMU;

struct RECEIVE_DATA_STRUCTURE_REMOTE
{
    int ch1; //right joystick up/down
    int ch2; //right joystick left/right
    int ch3; //left joystick up/down
    int ch4; //left joystick left/right
    int ch5; //back stick left/right
    // but1 (stick 1) from Joe is selecting between dome servo and dome spin
    // Naigon - this now cycles through the combined drive mode and dome mode.
    int but1 = 1; //left select
    // but2 from Joe is audio
    // Naigon - button 2 press plays a happy/neutral sound. Holding plays a longer/sader sound
    int but2 = 1; //left button 1
    // but3 from Joe is audio
    // Naigon - button 3 press starts music, and cycles tracks. Holding stops music.
    int but3 = 1; //left button 2
    // but4 from Joe is to trigger Body/dome lighting changes
    // Naigon - Button 4 TBD
    int but4 = 1; //left button 3
    // but5 (stick 2) toggles fwd/rev
    int but5 = 0; //right select (fwd/rev)
    // but6 from Joe is for switching between drive speeds
    // Naigon - Button 6 is now TBD.
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
    uint8_t bodyStatus = 0;
    uint8_t bodyMode = 0;
    uint8_t bodyDirection = 0;
};

struct RECEIVE_DATA_STRUCTURE_IMU
{
    float IMUloop;
    float pitch;
    float roll;
};

enum BodyMode : uint8_t
{
    UnknownSpeed = 0,
    Slow = 1,
    SlowWithTilt = 2,
    Automated = 3,
    AutomatedServo = 4,
    Servo = 5,
    ServoWithTilt = 6,
    Stationary = 7,
    Medium = 8,
    Fast = 9,
    PushToRoll = 10,
};
const BodyMode LastSpeedEntry = BodyMode::PushToRoll;

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
    FullSpinMode = 0,
    ServoMode = 1,
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
};

RECEIVE_DATA_STRUCTURE_REMOTE recFromRemote;
SEND_DATA_STRUCTURE_REMOTE sendToRemote;
RECEIVE_DATA_STRUCTURE_IMU recIMUData;

//
// Feature Enabled/Disable flags
//
// Naigon - Stationary/Wiggle Mode
bool IsStationary = false;
// Naigon - Dome Automation
bool IsDomeAutomation = false;

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
ButtonHandler button7Handler(0 /* onVal */, 2000 /* heldDuration */);
ButtonHandler button8Handler(0 /* onVal */, 2000 /* heldDuration */);

// Naigon - Ease Applicator
// Refactor code to use the new PWM driver.
MotorPWM drivePwm(drivePWM1, drivePWM2, 0, 2);
MotorPWM sideToSidePWM(s2sPWM1, s2sPWM2, SideToSideMax, 1);
MotorPWM headTiltPWM(domeTiltPWM1, domeTiltPWM2, HeadTiltPotThresh, 0);
MotorPWM domeSpinPWM(domeSpinPWM1, domeSpinPWM2, 0, 20);
MotorPWM domeServoPWM(domeSpinPWM1, domeSpinPWM2, 0, 4);
MotorPWM flywheelPWM(flywheelSpinPWM1, flywheelSpinPWM2, 0, 35);
// Naigon - Ease Applicator
// Refactor the code to use the new IEaseApplicator instances.
//
FunctionEaseApplicator driveApplicatorSlow(0.0, 20.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorMed(0.0, 28.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorHigh(0.0, 38.0, 0.2, FunctionEaseApplicatorType::Quadratic);
FunctionEaseApplicator driveApplicatorWiggle(0.0, 5.0, 0.1, FunctionEaseApplicatorType::Quadratic);
// Naigon - Ease Applicator: for the drive this pointer will always be the one in use.
FunctionEaseApplicator *driveApplicator = &driveApplicatorSlow;
FunctionEaseApplicator sideToSideEaseApplicator(0.0, SideToSideMax, S2SEase, FunctionEaseApplicatorType::SCurve);
FunctionEaseApplicator domeTiltEaseApplicator(0.0, MaxDomeTiltAngle, easeDomeTilt, FunctionEaseApplicatorType::SCurve);
FunctionEaseApplicator domeServoEaseApplicator(0.0, DomeSpinServoMax, 5, FunctionEaseApplicatorType::SCurve);
// Naigon - Ease Applicator: motors/modes where it free spins use the normal linear applicator.
LinearEaseApplicator domeSpinEaseApplicator(0.0, easeDome);
LinearEaseApplicator flywheelEaseApplicator(0.0, flywheelEase);


// Naigon - Head Tilt Stabilization
// To keep the head tilt from being jerky, do some filtering on the pitch and roll over time.
float pitch;
float pitchPrev[PitchAndRollFilterCount];
float roll;
float rollPrev[PitchAndRollFilterCount];
bool isFirstPitchAndRoll = true;

// Joe's EEPROM vars
float pitchOffset;
float rollOffset;
int potOffsetS2S;
int domeTiltPotOffset;

int fadeVal = 0;
int readPinState = 1;

// Joe's Audio Player
// TODO: I should bring back Joe's code and these vars in an #ifdef for those that are not using my custom driver.
int soundPins[] = {soundpin1, soundpin2, soundpin3, soundpin4};
int randSoundPin;
int soundState;
int musicState;
int autoDisableState;
unsigned long musicStateMillis = 0;
// Auto Disable
unsigned long autoDisableMotorsMillis = 0;
int autoDisableDoubleCheck;
unsigned long autoDisableDoubleCheckMillis = 0;
int autoDisable;
bool forcedMotorEnable = false;
// Main loop
unsigned long lastLoopMillis;
float lastIMUloop;
int MiniStatus;
// Save
int SaveToEEPROM;

float R1 = resistor1;
float R2 = resistor2;

// Drive motor
int driveSpeed;
int joystickDrive;
// Side to Side motor
int joystickS2S;
int S2Spot;
// Dome tilt
int joystickDome;
double domeTiltOffset;
int domeTiltPot;
// Dome spin
int domeSpinOffset;
int domeRotation;
int currentDomeSpeed;
int servoMode;
int domeServo = 0;
int ch4Servo; //left joystick left/right when using servo mode
// Flywheel Motor Drive
int ch5PWM;
int flywheelRotation;

int BTstate = 0;

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
double Setpoint5, Input5, Output5;

PID PID5(&Input5, &Output5, &Setpoint5, Kp5, Ki5, Kd5, DIRECT);


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
    sendToRemote.bodyMode = BodyMode::Slow;
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
        button7Handler.UpdateState(recFromRemote.but7);
        button8Handler.UpdateState(recFromRemote.but8);

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
        pitch = recIMUData.pitch * revPitch;
        roll = recIMUData.roll * revRoll;
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
    if ((recIMUData.IMUloop == 1 && lastIMUloop >= 980)
        || (recIMUData.IMUloop < 1 && lastIMUloop > 3))
    {
        lastIMUloop = 0;
    }

    if (recIMUData.IMUloop > lastIMUloop)
    {
        lastIMUloop = recIMUData.IMUloop;
        MiniStatus = 1;
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

    if (sendToRemote.bodyMode == BodyMode::Medium)
    {
        driveApplicator = &driveApplicatorMed;
    }
    else if (sendToRemote.bodyMode == BodyMode::Fast)
    {
        driveApplicator = &driveApplicatorHigh;
    }
    else if (sendToRemote.bodyMode == BodyMode::Stationary)
    {
        // For safety, set the drive speed back to slow, even though the stick shouldn't use it.
        driveApplicator = &driveApplicatorWiggle;
    }
    else
    {
        // For all other modes, just default to the slow speed as it honestly works the best.
        driveApplicator = &driveApplicatorSlow;
    }

    driveSpeed = driveApplicator->GetMaxValue();

    // Indicate if the droid is in stationary mode only when in the stationary state.
    IsStationary = sendToRemote.bodyMode == BodyMode::Stationary;
    
    IsDomeAutomation = sendToRemote.bodyMode == BodyMode::Automated
        || sendToRemote.bodyMode == BodyMode::AutomatedServo;

    // Naigon - Dome Modes
    // The dome servo or normal state is now parsed from the bodyMode.
    servoMode = sendToRemote.bodyMode == BodyMode::Servo
        || sendToRemote.bodyMode == BodyMode::ServoWithTilt
        || sendToRemote.bodyMode == BodyMode::AutomatedServo
            ? DomeMode::ServoMode
            : DomeMode::FullSpinMode;
}

void incrementBodySpeedToggle()
{
    if (button1Handler.GetState() != ButtonState::Pressed
        || sendToRemote.bodyStatus != BodyStatus::NormalOperation
        // Naigon - Safe Joystick Button Toggle
        || abs(recFromRemote.ch3 - 255) >= JoystickMinMovement
        || abs(recFromRemote.ch4 - 255) >= JoystickMinMovement)
    {
        // Only increment when the button was pressed and released, and the joystick was (mostly) centered.
        return;
    }

    //
    // Naigon - Drive-side (Server-side) Refactor
    // This method was taken directly from the remote code and ported here. In general, the drive should do all state
    // changes and should be the state "master", while the remote ect should just send commands. This will allow
    // secondary controllers to operate on the drive and the drive can maintain a state.
    //
    sendToRemote.bodyMode =
        sendToRemote.bodyMode >= (int)LastSpeedEntry || sendToRemote.bodyMode == BodyMode::UnknownSpeed
            ? 1
            : sendToRemote.bodyMode + 1;
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
// Calibration methods
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void bodyCalib()
{
    if (sendToRemote.bodyStatus == BodyStatus::NormalOperation
        && button8Handler.GetState() == ButtonState::Held
        && button7Handler.GetState() == ButtonState::NotPressed)
    {
        sendToRemote.bodyStatus = BodyStatus::BodyCalibration;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::BodyCalibration
        && button8Handler.GetState() == ButtonState::Pressed)
    {
        SaveToEEPROM = 1;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::BodyCalibration
        && button7Handler.GetState() == ButtonState::Pressed)
    {
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Dome calibration
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void domeCalib()
{
    if (sendToRemote.bodyStatus == BodyStatus::NormalOperation
        && button1Handler.GetState() == ButtonState::Held)
    {
        sendToRemote.bodyStatus = BodyStatus::DomeCalibration;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::DomeCalibration
        && button8Handler.GetState() == ButtonState::Pressed)
    {
        setDomeSpinOffset();
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
    }
    else if (sendToRemote.bodyStatus == BodyStatus::DomeCalibration
        && button7Handler.GetState() == ButtonState::Pressed)
    {
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
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

    if (recFromRemote.motorEnable == 0 && BTstate == 1 && MiniStatus != 0)
    {
        unsigned long currentMillis = millis();

        if (sendToRemote.bodyMode == BodyMode::PushToRoll)
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

    if (servoMode == DomeMode::FullSpinMode || autoDisable == 1 || recFromRemote.motorEnable == 1)
    {
        domeSpin();
    }
    else if (servoMode == DomeMode::ServoMode && autoDisable == 0)
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
    // Naigon - Stationary/Wiggle Mode
    // When in wiggle/stationary mode, don't use the joystick to move at all.
    joystickDrive = IsStationary == true
        ? 0
        : map(recFromRemote.ch1, 0, 512, driveSpeed revDrive1, driveSpeed revDrive2);

    Setpoint3 = constrain(
        driveApplicator->ComputeValueForCurrentIteration(joystickDrive),
        -55,
        55);

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
    // Naigon - Dome Automation
    // Read the left stick left/right when in an automated mode.
    int remoteInput = IsDomeAutomation ? recFromRemote.ch4 : recFromRemote.ch2;

    joystickS2S = map(constrain(remoteInput, 0, 512), 0, 512, SideToSideMax revS2S1, SideToSideMax revS2S2);

    // Setpoint will increase/decrease by S2SEase each time the code runs until it matches the joystick. This slows the side to side movement.
    Setpoint2 = constrain(
        sideToSideEaseApplicator.ComputeValueForCurrentIteration(joystickS2S),
        -SideToSideMax,
        SideToSideMax);

    S2Spot = map(analogRead(S2SpotPin), 0, 1024, S2SPotMax revS2SPot1, S2SPotMax revS2SPot2);
    Input2 = roll + rollOffset;
    
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

    BodyMode bodyM = (BodyMode)sendToRemote.bodyMode;
    // speedDomeTilt offsets the dome based on the main drive to tilt it in the direction of movement.
    // Naigon - Dome Modes: Add the tilt based on the main stick if the mode uses tilt.
    double speedDomeTilt =
        (Setpoint3 >= 3 || Setpoint3 <= -3)
        && (bodyM == BodyMode::SlowWithTilt || bodyM == BodyMode::ServoWithTilt)
            ? joystickDrive * (double)DomeTiltAmount / (double)driveSpeed
            : 0;

#ifdef HeadTiltStabilization
    // Naigon - Head Tilt Stabilization
    // Calculate the pitch to input into the head tilt input in order to keep it level.
    // Naigon - TODO: once the ease applicator is created, use it here to increment to pitch adjust.
    int pitchAdjust = sendToRemote.bodyMode != BodyMode::PushToRoll
        ? (pitch + pitchOffset) * HeadTiltPitchAndRollProportion
        : 0;
#else
    int pitchAdjust = 0;
#endif

    domeTiltPot = map(
        analogRead(domeTiltPotPin),
        0,
        1024,
        HeadTiltPotMax revDomeTiltPot1,
        HeadTiltPotMax revDomeTiltPot2) + domeTiltPotOffset;

    // Naigon - Dome Automation
    // Dome tilt is completely controlled by automation.
    int ch3Val = IsDomeAutomation ? 255 : recFromRemote.ch3;

    if (animationState.hasResult && animationState.ch3 != NOT_RUNNING && abs(ch3Val) < 10)
    {
        ch3Val = animationState.ch3;
    }

    // Naigon: BUG
    // Joe's code had a bug here; you need to subtract within the constrain, otherwise driving can cause this value to go
    // outside the bounds and really bad things happen like the drive locking and losing the head.
    joystickDome = constrain(
        map(ch3Val, 0, 512, -MaxDomeTiltAngle, MaxDomeTiltAngle) - (int)speedDomeTilt - pitchAdjust,
        MaxDomeTiltAngle revDome2,
        MaxDomeTiltAngle revDome1); // Reading the stick for angle -40 to 40

    Input4 = domeTiltPot + (pitch + pitchOffset);
    Setpoint4 = domeTiltEaseApplicator.ComputeValueForCurrentIteration(joystickDome);
    Setpoint4 = constrain(Setpoint4, -MaxDomeTiltAngle, MaxDomeTiltAngle);
    PID4.Compute();

    writeMotorPwm(headTiltPWM, Output4, domeTiltPot, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin - Manual
// ------------------------------------------------------------------------------------
void domeSpin()
{
    // Naigon - Dome Automation
    // Dome spin is completely controlled by the automation if the feature is enabled.
    int ch4Val = IsDomeAutomation ? 255 : recFromRemote.ch4;

    domeRotation = map(ch4Val, 0, 512, 255 revDomeSpin1, 255 revDomeSpin2);

    currentDomeSpeed = constrain(
        domeSpinEaseApplicator.ComputeValueForCurrentIteration(domeRotation),
        -255,
        255);

    // Joe has always allowed the dome to spin regardless of whether the motors were enabled or not, which I like.
    writeMotorPwm(domeSpinPWM, currentDomeSpeed, 0 /*input*/, true /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Dome spin - Servo
// ------------------------------------------------------------------------------------
void domeSpinServo()
{
    // Naigon - Dome Automation
    // Dome spin is completely controlled by the automation.
    int ch4Val = IsDomeAutomation ? 255 : recFromRemote.ch4;

    ch4Servo = map(ch4Val, 0, 512, DomeSpinServoMax revDomeR1, DomeSpinServoMax revDomeR2);

#ifdef reverseDomeSpinPot
    int minSpin = -180;
    int maxSpin = 180;
#else
    int minSpin = 180;
    int maxSpin = -180;
#endif

    Input5 = sendToRemote.bodyDirection == Direction::Forward
        ? map(analogRead(domeSpinPot), 0, 1023, minSpin, maxSpin) + domeSpinOffset - 180
        : map(analogRead(domeSpinPot), 0, 1023, minSpin, maxSpin) + domeSpinOffset;

    if (Input5 < -180)
    {
        Input5 += 360;
    }
    else if (Input5 > 180)
    {
        Input5 -= 360;
    }

    Setpoint5 = constrain(
        domeServoEaseApplicator.ComputeValueForCurrentIteration(ch4Servo),
        -DomeSpinServoMax,
        DomeSpinServoMax);
    PID5.Compute();

    writeMotorPwm(domeServoPWM, Output5, 0, false /*requireBT*/, false /*requireMotorEnable*/);
}

// ------------------------------------------------------------------------------------
// Flywheel spin
// ------------------------------------------------------------------------------------
void flywheelSpin()
{
    // Naigon - Stationary/Wiggle Mode
    // When in stationary mode, use the drive stick as the flywheel, as the drive is disabled.
    ch5PWM = IsStationary == true
        ? constrain(map(recFromRemote.ch1, 0, 512, 255 revFly1, 255 revFly2), -FlywheelStationaryMax, FlywheelStationaryMax)
        : constrain(map(recFromRemote.ch5, 0, 512, 255 revFly1, 255 revFly2), -FlywheelDriveMax, FlywheelDriveMax);

    flywheelRotation = constrain(
        flywheelEaseApplicator.ComputeValueForCurrentIteration(ch5PWM),
        -255,
        255);

    writeMotorPwm(flywheelPWM, flywheelRotation, 0 /*input*/, true /*requireBT*/, true /*requireMotorEnable*/);
}

void writeMotorPwm(MotorPWM &motorPwm, int output, int input, bool requireBT, bool requireMotorEnable)
{
    if ((requireBT == true && BTstate != 1)
        || (requireMotorEnable == true && recFromRemote.motorEnable != 0))
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
// Set offsets and save to EEPROM
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setOffsetsAndSaveToEEPROM()
{
    // Naigon - TODOD
    // Joe has this only save one thing per iteration. I'm not sure if that is due to needing to keep getting update
    // remote values, but it would be a lot better to just save everything in one go.
    //

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
        sendToRemote.bodyStatus = BodyStatus::NormalOperation;
        //playSound = 1;
    }
}

void setDomeSpinOffset()
{
    domeSpinOffset = sendToRemote.bodyDirection == Direction::Reverse
        ? 180 - map(analogRead(domeSpinPot), 0, 1023, 180, -180)
        : 0 - map(analogRead(domeSpinPot), 0, 1023, 180, -180);

    EEPROM.writeInt(16, domeSpinOffset);
    // delay(200);
    sendToRemote.bodyStatus = BodyStatus::NormalOperation;
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
