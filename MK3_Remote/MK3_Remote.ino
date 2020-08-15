// ====================================================================================================================
//                         Joe's Drive  - Remote "MK3"  - Updated 4/15
//
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty,
//                         guarantee, or other tomfoolery. This entire project was masterminded by an average Joe,
//                         your mileage may vary. 
// ====================================================================================================================
//                         Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//
//                         You will need libraries:
//                             i2cdevlib: https://github.com/jrowberg/i2cdevlib
//                             Modified Low PowerLab: https://travis-ci.org/LowPowerLab/RFM69  
//                             EEPROM Extended: https://github.com/thijse/Arduino-EEPROMEx
//                             SSD1306Ascii Text Only: https://github.com/greiman/SSD1306Ascii
//
//
// ===================================================================================================================================================================================================== 
// =====================================================================================================================================================================================================
 
/*
 *        Find file: RFM69.h
 *        Find: #define RF69_CSMA_LIMIT_MS 1000
 *        Change to: #define RF69_CSMA_LIMIT_MS 100
 */

#define droidName     "BB-8"


#define battPin       A9
#define enablePIN     1
#define lJoySelectPIN 13
#define Joy2XPIN      A1
#define Joy2YPIN      A0
#define lBut1PIN      10
#define lBut2PIN      11
#define lBut3PIN      12
#define rJoySelectPIN 0
#define Joy1XPIN      A4
#define Joy1YPIN      A5
#define rBut1PIN      6
#define rBut2PIN      5
#define rBut3PIN      9    //   This is the only button / pin that must not change. 
#define Joy3XPIN      A3
#define Joy4XPIN      A2
#define dataDelay     0
#define sendDelay     20
#define recDelay      2

#define debug

#include <EEPROMex.h>
#include "Arduino.h"
#include <Wire.h>
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;

#include <RFM69.h>    //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE ************
//*********************************************************************************************
#define NETWORKID       100  // The same on all nodes that talk to each other
#define REMOTE_ADDRESS  1    // The unique identifier of this node
#define BODY_ADDRESS    2    // The recipient of packets
#define DOME_ADDRESS    3    // The recipient of packets
#define DRIVE_ADDRESS   4

// Naigon - Visual Studio Code
//
// This is just to make the VS Code IDE happy
#ifndef RF69_915MHZ
#define RF69_915MHZ 91
#endif

//Match frequency to the hardware version of the radio on your Feather
#define FREQUENCY     RF69_915MHZ
#define IS_RFM69HCW   true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************
#define SERIAL_BAUD   115200

// for Feather 32u4 Radio
#define RFM69_CS      8
#define RFM69_IRQ     7
#define RFM69_IRQN    4  // Pin 7 is IRQ 4!
#define RFM69_RST     4

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
} sendToBody;

byte packet[sizeof(sendToBody)];

typedef struct RECEIVE_DATA_STRUCTURE_DOME
{
    float bodyBatt;
    float domeBatt;
} recDomeData;
recDomeData recFromDome;

typedef struct RECEIVE_DATA_STRUCTURE_BODY
{
    double bodyBatt = 0.0;
    double domeBatt = 0.0;
    uint8_t bodyStatus;
    uint8_t bodyMode;
    uint8_t bodyDirection;
} recBodyData;
recBodyData recFromBody;

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

//
// Naigon
// I removed the complicated calibration and went back to the old way of only
// calibrating the joystick centers. This simplifies the business logic, adhering to my
// goal of the remote being a light-weight client.
//
const int JoyLow = 0;
const int JoyHigh = 1023;

bool SEND;
int Joy1X, Joy1Xa, Joy1Xb;
int Joy1Y, Joy1Ya, Joy1Yb;
int Joy2X, Joy2Xa, Joy2Xb;
int Joy2Y, Joy2Ya, Joy2Yb;
int Joy3X, Joy3Xa, Joy3Xb;
int Joy4X, Joy4Xa, Joy4Xb;
int Joy1XCenter, Joy1YCenter, Joy2XCenter, Joy2YCenter, Joy3XCenter, Joy4XCenter;
byte updateScreen = 1;
byte wireless, bodyWireless;

//int rectime[20];
//byte rectimeloc;
const int eepromSet = 890;
float measuredvbat;
unsigned long lastScreenUpdate; 
unsigned long lastrecdata = 1000; 
unsigned long lastReading, lastLoopMillis, lastrecDataMillis, lastSend;

byte startup = 1;

uint8_t lastBodyStatus = 1000;
uint8_t lastDirveMode = BodyMode::UnknownSpeed;
uint8_t lastDirection = Direction::UnknownDirection;

bool displayJoystickCalibration = false;

void setup()
{
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
    Wire.setClock(400000L);
    oled.setFont(Callibri15);
    oled.println(F("==========================="));
    oled.println(F("            Joe's Drive       "));
    oled.println(F("        Naigon Edition"));
    oled.println(F("==========================="));
    Serial.begin(115200);
  
    pinMode (lJoySelectPIN, INPUT_PULLUP); 
    pinMode (rJoySelectPIN, INPUT_PULLUP); 
    pinMode (lBut1PIN, INPUT_PULLUP); 
    pinMode (lBut2PIN, INPUT_PULLUP); 
    pinMode (lBut3PIN, INPUT_PULLUP); 
    pinMode (rBut1PIN, INPUT_PULLUP); 
    pinMode (rBut2PIN, INPUT_PULLUP);
    pinMode (enablePIN, INPUT_PULLUP);

    // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);

    // Initialize radio
    radio.initialize(FREQUENCY, REMOTE_ADDRESS, NETWORKID);
    if (IS_RFM69HCW) {
        radio.setHighPower();    // Only for RFM69HCW & HW!
    }
    radio.setPowerLevel(20); // power output ranges from 0 (5dBm) to 31 (20dBm)

    if(EEPROM.readInt(36) == eepromSet)
    {
        Joy1XCenter = EEPROM.readInt(0);
        Joy1YCenter = EEPROM.readInt(2);
        Joy2XCenter = EEPROM.readInt(4);
        Joy2YCenter = EEPROM.readInt(6);
        Joy3XCenter = EEPROM.readInt(8);
        Joy4XCenter = EEPROM.readInt(10);
    }
    else
    {
        Joy1XCenter = 512;
        Joy1YCenter = 512;
        Joy2XCenter = 512;
        Joy2YCenter = 512;
        Joy3XCenter = 512;
        Joy4XCenter = 512;
    }

    delay(2000);
    oled.clear();
  }

void loop()
{
    recData();

    if(millis() - lastLoopMillis >= 20)
    {
        lastLoopMillis = millis(); 
        readInputs();
        measureVoltage();
        Screen();
        timeout();
    }

    if(millis() - lastSend >= sendDelay || SEND)
    {
        SendData();
    }
}

void measureVoltage()
{
    if((millis() - lastReading >= 10000 && analogRead(battPin) > 1) || startup == 1)
    {
        lastReading = millis();
        measuredvbat = analogRead(battPin);
        measuredvbat *= 2;    // we divided by 2, so multiply back
        measuredvbat *= 3.3;  // Multiply by 3.3V, our reference voltage
        measuredvbat /= 1024; // convert to voltage
        if(startup == 1)
        {
            startup = 0;
        }
    }
}

void readInputs()
{
    sendToBody.but1 = digitalRead(lJoySelectPIN);
    sendToBody.but2 = digitalRead(lBut1PIN);
    sendToBody.but3 = digitalRead(lBut2PIN);
    sendToBody.but4 = digitalRead(lBut3PIN);

    sendToBody.but5 = digitalRead(rJoySelectPIN);
    sendToBody.but6 = digitalRead(rBut1PIN);
    sendToBody.but7 = digitalRead(rBut2PIN);
    sendToBody.but8 = analogRead(rBut3PIN);

    Joy1Xa = analogRead(Joy1XPIN);
    Joy1Ya = analogRead(Joy1YPIN);
    Joy2Xa = analogRead(Joy2XPIN);
    Joy2Ya = analogRead(Joy2YPIN);
    Joy3Xa = analogRead(Joy3XPIN);
    Joy4Xa = analogRead(Joy4XPIN);
    sendToBody.motorEnable = digitalRead(enablePIN);

    centerChannels();

    sendToBody.Joy1Y = Joy1Y; //main drive
    sendToBody.Joy1X = Joy1X; //tilt / steer
    sendToBody.Joy2Y = Joy2Y; //head tilt
    sendToBody.Joy2X = Joy2X; //head spin
    sendToBody.Joy3X = Joy3X; //spin Flywheel
    sendToBody.Joy4X = Joy4X;

    if (sendToBody.motorEnable == 1 && sendToBody.but8 == 0 && sendToBody.but7 == 0)
    {
        displayJoystickCalibration = true;
    }
}

void centerChannels()
{
    if (Joy1Xa == Joy1XCenter)
    {
        Joy1Xb = 256;
    }
    else if (Joy1Xa > Joy1XCenter)
    {
        Joy1Xb = map(Joy1Xa, Joy1XCenter, JoyHigh, 255, 0);
    }
    else if (Joy1Xa < Joy1XCenter)
    {
        Joy1Xb = map(Joy1Xa, JoyLow, Joy1XCenter, 512, 257);
    }

    if (Joy1Ya == Joy1YCenter)
    {
        Joy1Yb = 256;
    }
    else if (Joy1Ya > Joy1YCenter)
    {
        Joy1Yb = map(Joy1Ya, Joy1YCenter, JoyHigh, 255, 0);
    }
    else if (Joy1Ya < Joy1YCenter)
    {
        Joy1Yb = map(Joy1Ya, JoyLow, Joy1YCenter, 512, 257);
    }

    if (Joy2Xa == Joy2XCenter)
    {
        Joy2Xb = 256;
    }
    else if (Joy2Xa > Joy2XCenter)
    {
        Joy2Xb = map(Joy2Xa, Joy2XCenter, JoyHigh, 255, 0);
    }
    else if (Joy2Xa < Joy2XCenter)
    {
        Joy2Xb = map(Joy2Xa, JoyLow, Joy2XCenter, 512, 257);
    }
  
    if (Joy2Ya == Joy2YCenter)
    {
        Joy2Yb = 256;
    }
    else if (Joy2Ya > Joy2YCenter)
    {
        Joy2Yb = map(Joy2Ya, Joy2YCenter, JoyHigh, 255, 0);
    }
    else if (Joy2Ya < Joy2YCenter)
    {
        Joy2Yb = map(Joy2Ya, JoyLow, Joy2YCenter, 512, 257);
    }
  
    if (Joy3Xa == Joy3XCenter)
    {
        Joy3Xb = 256;
    }
    else if (Joy3Xa > Joy3XCenter)
    {
        Joy3Xb = map(Joy3Xa, Joy3XCenter, JoyHigh, 255, 0);
    }
    else if (Joy3Xa < Joy3XCenter)
    {
        Joy3Xb = map(Joy3Xa, JoyLow, Joy3XCenter, 512, 257);
    }
  
    if (Joy4Xa == Joy4XCenter)
    {
        Joy4Xb = 256;
    }
    else if (Joy4Xa > Joy4XCenter)
    {
        Joy4Xb = map(Joy4Xa, Joy4XCenter, JoyHigh, 255, 0);
    }
    else if (Joy4Xa < Joy4XCenter)
    {
        Joy4Xb = map(Joy4Xa, JoyLow, Joy4XCenter, 512, 257);
    }

    Joy1X = constrain(Joy1Xb, 0, 512);
    Joy1Y = constrain(Joy1Yb, 0, 512);
    Joy2X = constrain(Joy2Xb, 0, 512);
    Joy2Y = constrain(Joy2Yb, 0, 512);
    Joy3X = constrain(Joy3Xb, 0, 512);
    Joy4X = constrain(Joy4Xb, 0, 512);
  }

void SendData()
{
    if(millis() > 2000 && millis() - lastSend >= sendDelay)
    {
        memcpy(packet, &sendToBody, sizeof(sendToBody)); //Copy data from "sendToBody" array to "send_buf" byte array 
        radio.send(BODY_ADDRESS, packet, sizeof(packet)); //target node Id, message as string or byte array, message length
        delay(5); 
        lastSend = millis();
        #ifdef debug
          debugRoutine(); 
        #endif
    }
}

void recData()
{
    if(millis() - lastrecDataMillis >= recDelay && radio.receiveDone())
    {
        if(radio.SENDERID == uint8_t(DOME_ADDRESS)) //********** This needs to be fixed for new lib
        {
            if(radio.DATALEN != sizeof(recFromDome))
            {
                Serial.print("Invalid DOME payload received, not matching Payload struct! Should Be:");
                Serial.print(sizeof(recFromDome)); Serial.print(" received:"); Serial.println(radio.DATALEN);
            }
            else
            {
                recFromDome = *(recDomeData*)radio.DATA;
                //Serial.println(millis() - lastrecdata);
                lastrecdata = millis();
                lastrecDataMillis = millis();
                if(bodyWireless == 0)
                {
                    bodyWireless = 1;
                }
            }
            SEND = true;
        }
        else if(radio.SENDERID == uint8_t(BODY_ADDRESS)) //********** This needs to be fixed for new lib
        {
            if(radio.DATALEN != sizeof(recFromBody))
            {
                Serial.print("Invalid BODY payload received, not matching Payload struct! Should Be:");
                Serial.print(sizeof(recFromBody)); Serial.print(" received:"); Serial.println(radio.DATALEN);
            }
            else
            {
                recFromBody = *(recBodyData*)radio.DATA; 
                recFromDome.bodyBatt = recFromBody.bodyBatt;
                Serial.println("rec body");
                lastrecdata = millis();
                lastrecDataMillis = millis();
                if(recFromDome.domeBatt != 99.99)
                {
                    recFromDome.domeBatt = 99.99;
                }
                if(bodyWireless == 1)
                {
                    bodyWireless = 0;
                }
            }
            SEND = true;
        }
    }
}

bool needsScreenUpdate()
{
    if (recFromBody.bodyStatus != BodyStatus::NormalOperation)
    {
        return false;
    }

    // Naigon - Drive-side (Server-side) Refactor
    // Toggle displaying new values based on changes coming from the drive.
    return ((millis() - lastScreenUpdate) > 15000
        || lastBodyStatus != recFromBody.bodyStatus
        || lastDirveMode != recFromBody.bodyMode
        || lastDirection != recFromBody.bodyDirection
        || updateScreen == 1
        || displayJoystickCalibration);
}

void Screen()
{
    if(!needsScreenUpdate())
    {
        return;
    }

    if (displayJoystickCalibration)
    {
        timeJoystickCalibration();
        displayJoystickCalibration = false;
    }
    else if(recFromBody.bodyStatus == BodyStatus::NormalOperation)
    {
        infoScreen();
    }
    else if(recFromBody.bodyStatus == BodyStatus::BodyCalibration)
    {
        bodyCalibration();
    }
    else if(recFromBody.bodyStatus == BodyStatus::DomeCalibration)
    {
        domeCenterCalibration();
    }

    lastDirveMode = recFromBody.bodyMode;
    lastDirection = recFromBody.bodyDirection;
    lastBodyStatus = recFromBody.bodyStatus;
}

void infoScreen()
{
    updateScreen = 0;
    oled.setCursor(0,0);
    oled.setFont(Callibri15);
        
    BodyMode bodyM = (BodyMode)recFromBody.bodyMode;
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

    oled.setCursor(100,0);
    if(recFromBody.bodyDirection == Direction::Forward)
    {
        oled.println(F("Fwd     "));
    }
    else
    {
        oled.println(F("Rev     "));
    }
    oled.print(F("Body: ")); 
    if(wireless == 1 && recFromDome.bodyBatt != 99.99)
    {
        oled.print(recFromDome.bodyBatt); oled.println(F("v                 "));
    }
    else
    {
        oled.println(F("Disconnected              "));
    }
    oled.print(F("Dome: ")); 
    if(wireless == 1 && recFromDome.domeBatt != 99.99)
    {
        oled.print(recFromDome.domeBatt); oled.println("v                 ");
    }
    else
    {
        oled.println(F("Disconnected              "));
    }
    oled.print("Remote: "); oled.print(measuredvbat); oled.println(F("v               "));
    lastScreenUpdate = millis();
}

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

void timeJoystickCalibration()
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
        digitalRead(lBut1PIN) == 0
        || digitalRead(lBut2PIN) == 0
        || digitalRead(lBut3PIN) == 0
        || digitalRead(rBut1PIN) == 0
        || digitalRead(rBut2PIN) == 0
        || digitalRead(rBut3PIN) == 0)
    {
    }

    oled.clear();
    oled.println(F("1. Release joysticks          "));
    oled.println(F("2. Press any button         "));
    oled.print(F("                    "));

    while (
        digitalRead(lBut1PIN) == 1
        && digitalRead(lBut2PIN) == 1
        && digitalRead(lBut3PIN) == 1
        && digitalRead(rBut1PIN) == 1
        && digitalRead(rBut2PIN) == 1
        && digitalRead(rBut3PIN) == 1
        && digitalRead(enablePIN) == 1)
    {
    }

    if (digitalRead(enablePIN == 1))
    {
        setJoystickCenter();
    }

    oled.clear();
    updateScreen = 1;
}

void resetMenu()
{
    oled.clear();
}

void setJoystickCenter()
{
    EEPROM.writeInt(0, analogRead(Joy1XPIN));
    EEPROM.writeInt(2, analogRead(Joy1YPIN));
    EEPROM.writeInt(4, analogRead(Joy2XPIN));
    EEPROM.writeInt(6, analogRead(Joy2YPIN));
    EEPROM.writeInt(8, analogRead(Joy3XPIN));
    EEPROM.writeInt(10, analogRead(Joy4XPIN));
    EEPROM.writeInt(36, eepromSet);
    delay(1000);
}

void timeout()
{
    if(millis() - lastrecdata >= 2500)
    {
        if(wireless == 1)
        {
            wireless = 0;
            updateScreen = 1;
        }
    }
    else if(millis() - lastrecdata <= 1000 && wireless == 0)
    {
        wireless = 1;
        updateScreen = 1;
    }
}

void debugRoutine()
{
    Serial.print(sendToBody.Joy1Y); Serial.print(", ");
    Serial.print(sendToBody.Joy1X); Serial.print(", ");
    Serial.print(sendToBody.Joy2Y); Serial.print(", ");
    Serial.print(sendToBody.Joy2X); Serial.print(", ");
    Serial.print(sendToBody.Joy3X); Serial.print(", ");
    Serial.print(sendToBody.Joy4X); Serial.print(", ");
    Serial.print(sendToBody.but1); Serial.print(", ");
    Serial.print(sendToBody.but2); Serial.print(", ");
    Serial.print(sendToBody.but3); Serial.print(", ");
    Serial.print(sendToBody.but4); Serial.print(", ");
    Serial.print(sendToBody.but5); Serial.print(", ");
    Serial.print(sendToBody.but6); Serial.print(", ");
    Serial.print(sendToBody.but7); Serial.print(", ");
    Serial.print(sendToBody.but8); Serial.print(", ");
    Serial.print(sendToBody.motorEnable);
    Serial.println();
}
