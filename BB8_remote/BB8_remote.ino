// ====================================================================================================================
// This work is licensed under the Creative Commons Attribution-NonCommercial 4.0 International License. To view a copy
// of this license, visit http://creativecommons.org/licenses/by-nc/4.0/ or send a letter to
// Creative Commons, PO Box 1866, Mountain View, CA 94042, USA.
// ====================================================================================================================

// ====================================================================================================================
//                         Joe's Drive  - Single Arduino Pro Mini Remote
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                         Joe's Drive powered by Naigon
//                         Last Updated 27 May 2020
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// ====================================================================================================================
//                         Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//                         You will need libraries: EepromEX: https://github.com/thijse/Arduino-EEPROMEx
//                                                  SSD1306AsciiWire: https://github.com/greiman/SSD1306Ascii
//
//
// ====================================================================================================================
// ====================================================================================================================

#define battPin A6
#define btStatePin 11
#define enablePin 10
#define lSelect 6
#define lHorizontal A2
#define lVertical A3
#define lBut1 9
#define lBut2 8
#define lBut3 7
#define rSelect 2
#define rHorizontal A1
#define rVertical A0
#define rBut1 5
#define rBut2 4
#define rBut3 3
#define Flywheel A7

#include <EEPROMex.h> // https://github.com/thijse/Arduino-EEPROMEx
#include "Arduino.h"
#include <Wire.h>
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
#include <EasyTransfer.h>

EasyTransfer RecRemote;
EasyTransfer SendRemote;

enum BodyStatus : uint8_t
{
    NormalOperation = 0,
    BodyCalibration = 1,
    DomeCalibration = 2,
};

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

enum Direction : uint8_t
{
    UnknownDirection = 0,
    Forward = 1,
    Reverse = 2,
};

struct SEND_DATA_STRUCTURE
{
    int Joy1Y;       // main drive
    int Joy1X;       // tilt / steer
    int Joy2Y;       // head tilt
    int Joy2X;       // head spin
    int Joy3X;       // spin Flywheel
    int Joy4X;       // unused for single remote
    uint8_t but1;        // Select on left joystick
    uint8_t but2;        // left 1
    uint8_t but3;        // left 2
    uint8_t but4;        // left3
    uint8_t but5;        // Select on right joystick
    uint8_t but6;        // right 1
    uint8_t but7;        // right 2
    uint8_t but8;        // right 3
    uint8_t motorEnable; //toggle on top
};

struct RECEIVE_DATA_STRUCTURE
{
    double bodyBatt = 0.0;
    double domeBatt = 0.0;
    uint8_t bodyStatus;
    uint8_t bodyMode;
    uint8_t bodyDirection;
};

SEND_DATA_STRUCTURE sendToBody;
RECEIVE_DATA_STRUCTURE recFromBody;

int ch1a; //main drive
int ch1b;
int ch2a; //tilt / steer
int ch2b;
int ch3a; //head tilt
int ch3b;
int ch4a; //head spin
int ch5a; //spin Flywheel

int printScreen;

int ch1Center;
int ch2Center;
int ch3Center;
int ch4Center;
int ch5Center;

int btConnectedState = 0;
int stateLast = 0;

long previousMillis;
long interval = 40;
long previousMillisScreen;
unsigned long bodyCalibrationMillis;

float remoteBatt = 0.0;

uint8_t lastDriveSpeed = BodyMode::UnknownSpeed;
uint8_t lastDirection = Direction::UnknownDirection;
int lastBodyStatus = -1;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];

boolean newData = false;

long joystickCalibMillis;
int joystickCalibState;

void setup()
{
    Serial.begin(115200);
    pinMode(rSelect, INPUT_PULLUP);
    pinMode(rBut1, INPUT_PULLUP);
    pinMode(rBut2, INPUT_PULLUP);
    pinMode(rBut3, INPUT_PULLUP);
    pinMode(lSelect, INPUT_PULLUP);
    pinMode(lBut1, INPUT_PULLUP);
    pinMode(lBut2, INPUT_PULLUP);
    pinMode(lBut3, INPUT_PULLUP);
    pinMode(enablePin, INPUT_PULLUP);
    pinMode(btStatePin, INPUT); // check for Bluetooth enable

    Wire.begin();
    // Naigon - Since upgrading to Windows 10 with the new Arduino, oled.Set400k stopped working.
    Wire.setClock(400000L);
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
    oled.setFont(Callibri15);
    //oled.clear();
    oled.println(F("==========================="));
    oled.println(F("         Joe's Drive       "));
    oled.println(F("  Naigon MK3 Flywheel  "));
    oled.println(F("==========================="));
    delay(2000);
    oled.clear();

    ch1Center = EEPROM.readInt(0);
    ch2Center = EEPROM.readInt(4);
    ch3Center = EEPROM.readInt(8);
    ch4Center = EEPROM.readInt(12);
    ch5Center = EEPROM.readInt(16);

    if (ch1Center < 100)
    {
        ch1Center = 512;
    }
    if (ch2Center < 100)
    {
        ch2Center = 512;
    }
    if (ch3Center < 100)
    {
        ch3Center = 512;
    }
    if (ch4Center < 100)
    {
        ch4Center = 512;
    }
    if (ch5Center < 100)
    {
        ch5Center = 512;
    }

    RecRemote.begin(details(recFromBody), &Serial);
    SendRemote.begin(details(sendToBody), &Serial);
}

void loop()
{
    //unsigned long currentMillis = millis();
    checkForScreenUpdate();
    checkBodyStatus();

    // Naigon - Drive-side (Server-side) Refactor
    // Update the variables to use for next iterations comparison.
    lastBodyStatus = recFromBody.bodyStatus;
    lastDriveSpeed = recFromBody.bodyMode;
    lastDirection = recFromBody.bodyDirection;

    if (millis() - previousMillis > interval)
    {
        previousMillis = millis();

        readInputs();
        sendAndReceive();
    }
}

//==================================  Times how long far right button is pressed  ====================================

void timeJoystickCalibration()
{

    unsigned long currentMillisCenter = millis();

    if (sendToBody.but8 == 0 && sendToBody.but7 == 0 && sendToBody.motorEnable == 1 && joystickCalibState == 0)
    {
        joystickCalibMillis = millis();
        joystickCalibState = 1;
    }

    if (joystickCalibState == 1 && currentMillisCenter - joystickCalibMillis >= 3000)
    {
        oled.clear();
        oled.setFont(Callibri15);
        oled.println(F("========================="));
        oled.println(F("   Joystick Calibration"));
        oled.println(F("========================="));
        delay(4000);
        oled.clear();
        oled.println(F("1. Release joysticks      "));
        oled.println(F("2. Release all buttons "));
        oled.print(F("                    "));

        while (
            digitalRead(lBut1) == 0
            || digitalRead(lBut2) == 0
            || digitalRead(lBut3) == 0
            || digitalRead(rBut1) == 0
            || digitalRead(rBut2) == 0
            || digitalRead(rBut3) == 0)
        {
        }

        oled.clear();
        oled.println(F("1. Release joysticks          "));
        oled.println(F("2. Press any button         "));
        oled.print(F("                    "));

        while (
            digitalRead(lBut1) == 1
            && digitalRead(lBut2) == 1
            && digitalRead(lBut3) == 1
            && digitalRead(rBut1) == 1
            && digitalRead(rBut2) == 1
            && digitalRead(rBut3) == 1
            && digitalRead(enablePin) == 1)
        {
        }

        if (digitalRead(enablePin == 1))
        {
            setJoystickCenter();
            joystickCalibState = 0;
        }

        oled.clear();
        printScreen = 1;
    }
}

//==================================  Set joystick centers  ====================================

void setJoystickCenter()
{
    ch1Center = ch1a;
    ch2Center = ch2a;
    ch3Center = ch3a;
    ch4Center = ch4a;
    ch5Center = ch5a;

    EEPROM.writeInt(0, ch1Center);
    EEPROM.writeInt(4, ch2Center);
    EEPROM.writeInt(8, ch3Center);
    EEPROM.writeInt(12, ch4Center);
    EEPROM.writeInt(16, ch5Center);
    delay(1000);
}

//==================================  Calibration  ====================================

void bodyCalibration()
{
    oled.setCursor(0, 0);
    oled.setFont(Callibri15);
    //oled.println(F("========================="));
    oled.println(F("** Body Calibration **            "));
    oled.println(F("1. Center BB8.                 "));
    oled.println(F("2. Disable motors.      "));
    oled.println(F("3. Press right button.                      "));
}

void domeCenterCalibration()
{
    oled.setCursor(0, 0);
    oled.setFont(Callibri15);
    //oled.println(F("========================="));
    oled.println(F("** Dome Calibration **            "));
    oled.println(F("1. Face BB-8 'Forward'.                 "));
    oled.println(F("2. Disable motors.      "));
    oled.println(F("3. Press right button.                      "));
}

//==================================  Update screen  ====================================

void printVoltage()
{
    // Body voltage
    //
    oled.setCursor(0, 0);
    oled.setFont(Callibri15);
    oled.print(F("Body: "));
    if (btConnectedState == 1)
    {
        oled.print(recFromBody.bodyBatt);
    }
    else
    {
        oled.print(F("0.00"));
    }
    oled.print(F("V          "));

    // Naigon - Drive-side (Server-side) Refactor
    // Read the speed from the value received from the body as opposed to calculating it directly.
    // Movement speed
    //
    BodyMode bodyM = (BodyMode)recFromBody.bodyMode;
    oled.setCursor(95, 0);
    if (bodyM == BodyMode::Slow || bodyM == BodyMode::Servo)
    {
        oled.println(F("Slw     "));
    }
    else if (bodyM == BodyMode::SlowWithTilt || bodyM == BodyMode::ServoWithTilt)
    {
        oled.println(F("SlwT   "));
    }
    else if (bodyM == BodyMode::Medium)
    {
        oled.println(F("Med    "));
    }
    else if (bodyM == BodyMode::Fast)
    {
        oled.println(F("Fst     "));
    }
    else if (bodyM == BodyMode::Stationary)
    {
        oled.println(F("Wgl     "));
    }
    else if (bodyM == BodyMode::PushToRoll)
    {
        oled.println(F("Safe    "));
    }
    else if (bodyM == BodyMode::Automated
        || bodyM == BodyMode::AutomatedServo)
    {
        oled.println(F("Auto    "));
    }
    else if (bodyM == BodyMode::AutomatedTilt)
    {
        oled.println(F("aTlt    "));
    }
    else
    {
        oled.println(F("OFF      "));
    }

    // Remote voltage
    //
    oled.print(F("Remote: "));
    oled.print(remoteBatt);
    oled.print(F("V                             "));

    // Movement direction
    // Naigon - Drive-side (Server-side) Refactor
    // Get the body direction (Fwd/Reverse) directly from the body as opposed to maintaining it here.
    oled.setCursor(95, 15);
    if (recFromBody.bodyDirection == Direction::Reverse)
    {
        oled.println(F("Rev          "));
    }
    else
    {
        oled.println(F("Fwd            "));
    }

    oled.print(F("Dome Rotation: "));
    if (bodyM == BodyMode::AutomatedServo || bodyM == BodyMode::Servo || bodyM == BodyMode::ServoWithTilt)
    {
        oled.println("Servo");
    }
    else
    {
        oled.println("Spin   ");
    }

    // Bluetooth connected state
    //
    oled.print(F("BT: "));
    if (btConnectedState == 0)
    {
        oled.println(F("Not Connected                     "));
    }
    else
    {
        oled.println(F("Connected                      "));
    }

    // Update current values
    stateLast = btConnectedState;
}

void sendAndReceive()
{
    RecRemote.receiveData();
    SendRemote.sendData();
}

void checkForScreenUpdate()
{
    if (recFromBody.bodyStatus != BodyStatus::NormalOperation)
    {
        return;
    }

    // Naigon - Drive-side (Server-side) Refactor
    // Update if statement to see if body sent new values for state variables.
    if ((millis() - previousMillisScreen > 15000)
        || (stateLast != btConnectedState)
        || (lastBodyStatus != recFromBody.bodyStatus)
        || (lastDriveSpeed != recFromBody.bodyMode)
        || (lastDirection != recFromBody.bodyDirection)
        || (printScreen == 1))
    {
        previousMillisScreen = millis();
        printVoltage();
        printScreen = 0;
    }
}

void checkBodyStatus()
{
    if (recFromBody.bodyStatus == BodyStatus::BodyCalibration)
    {
        bodyCalibration();
    }
    else if (recFromBody.bodyStatus == BodyStatus::DomeCalibration)
    {
        domeCenterCalibration();
    }
}

void readInputs()
{
    btConnectedState = digitalRead(btStatePin); // check to see when BT is paired

    ch1a = analogRead(rVertical);
    ch2a = analogRead(rHorizontal);
    ch3a = analogRead(lVertical);
    ch4a = analogRead(lHorizontal);
    ch5a = analogRead(Flywheel);
    // Naigon - Drive-side (Server-side) Refactor
    // Update to directly send the values to the drive, so that it can determine button pressed state.
    // This is the crux of the change and all the rest is fallout from making the change here.
    sendToBody.but1 = digitalRead(lSelect);
    sendToBody.but2 = digitalRead(lBut1);
    sendToBody.but3 = digitalRead(lBut2);
    sendToBody.but4 = digitalRead(lBut3);
    sendToBody.but5 = digitalRead(rSelect);
    sendToBody.but6 = digitalRead(rBut1);
    sendToBody.but7 = digitalRead(rBut2);
    sendToBody.but8 = digitalRead(rBut3);
    sendToBody.motorEnable = digitalRead(enablePin);

    remoteBatt = analogRead(battPin);
    remoteBatt /= 1023;
    remoteBatt *= 5;

    if (ch1a == ch1Center)
    {
        ch1b = 256;
    }
    else if (ch1a > ch1Center)
    {
        ch1b = map(ch1a, ch1Center, 1023, 255, 0);
    }
    else if (ch1a < ch1Center)
    {
        ch1b = map(ch1a, 0, ch1Center, 512, 257);
    }

    if (ch2a == ch2Center)
    {
        ch2b = 256;
    }
    else if (ch2a > ch2Center)
    {
        ch2b = map(ch2a, ch2Center, 1023, 255, 0);
    }
    else if (ch2a < ch2Center)
    {
        ch2b = map(ch2a, 0, ch2Center, 512, 257);
    }

    if (ch3a == ch3Center)
    {
        ch3b = 256;
    }
    else if (ch3a > ch3Center)
    {
        ch3b = map(ch3a, ch3Center, 1023, 255, 0);
    }
    else if (ch3a < ch3Center)
    {
        ch3b = map(ch3a, 0, ch3Center, 512, 257);
    }

    if (recFromBody.bodyDirection == Direction::Reverse)
    {
        sendToBody.Joy1Y = map(ch1b, 0, 512, 512, 0);
        sendToBody.Joy1X = map(ch2b, 0, 512, 512, 0);
        sendToBody.Joy2Y = map(ch3b, 0, 512, 512, 0);
    }
    else
    {
        sendToBody.Joy1Y = ch1b;
        sendToBody.Joy1X = ch2b;
        sendToBody.Joy2Y = ch3b;
    }

    if (ch4a == ch4Center)
    {
        sendToBody.Joy2X = 256;
    }
    else if (ch4a > ch4Center)
    {
        sendToBody.Joy2X = map(ch4a, ch4Center, 1023, 255, 0);
    }
    else if (ch4a < ch4Center)
    {
        sendToBody.Joy2X = map(ch4a, 0, ch4Center, 512, 257);
    }

    if (ch5a == ch5Center)
    {
        sendToBody.Joy3X = 256;
    }
    else if (ch5a > ch5Center)
    {
        sendToBody.Joy3X = constrain(map(ch5a, ch5Center, 780, 255, 0), 0, 512);
    }
    else if (ch5a < ch5Center)
    {
        sendToBody.Joy3X = constrain(map(ch5a, 140, ch5Center, 512, 257), 0, 512);
    }

    if (sendToBody.but8 == 0 && sendToBody.but7 == 0)
    {
        timeJoystickCalibration();
    }
    else if (sendToBody.but8 == 1 || sendToBody.but7 == 1 || sendToBody.motorEnable == 0)
    {
        joystickCalibState = 0;
    }
}
