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

#define serialSound   //Uncomment this if you are using Serial3 for sounds in the body

//#define debug

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

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

#ifdef serialSound
struct SEND_DATA_STRUCTURE
{
    int Joy1Y=256; //main drive
    int Joy1X=256; //tilt / steer
    int Joy2Y=256; //head tilt
    int Joy2X=256; //head spin
    int Joy3X=256; //spin Flywheel
    int Joy4X=256;
    byte ServoMode; //Select on left joystick
    byte lBut1=0; //left 1
    byte lBut2=0; //left 2
    byte lBut3; //left3
    byte Fwd; //Select on right joystick = rJoySelect
    byte Speed;
    byte rBut2; //right 2
    byte rBut3=0; //right 3
    byte motorEnable=1; //toggle on top
    byte CalibID;
    byte wireless=1;
} sendToBody;
#else
struct SEND_DATA_STRUCTURE{
    int Joy1Y=256; //main drive
    int Joy1X=256; //tilt / steer
    int Joy2Y=256; //head tilt
    int Joy2X=256; //head spin
    int Joy3X=256; //spin Flywheel
    int Joy4X=256;
    byte ServoMode; //Select on left joystick
    byte lBut1=1; //left 1
    byte lBut2=1; //left 2
    byte lBut3; //left3
    byte Fwd; //Select on right joystick = rJoySelect
    byte Speed;
    byte rBut2; //right 2
    byte rBut3=1; //right 3
    byte motorEnable=1; //toggle on top
    byte CalibID;
    byte wireless=1;
} sendToBody;
#endif

byte packet[sizeof(sendToBody)];

typedef struct RECEIVE_DATA_STRUCTURE_DOME
{
    float bodyBatt;
    float domeBatt;
} recDomeData;
recDomeData recFromDome;

typedef struct RECEIVE_DATA_STRUCTURE_BODY
{
    int psi=0;
    byte button4 = 1;
    float bodyBatt;
} recBodyData;
recBodyData recFromBody;

bool SEND;
int Joy1X, Joy1Xa, Joy1Xb;
int Joy1Y, Joy1Ya, Joy1Yb;
int Joy2X, Joy2Xa, Joy2Xb;
int Joy2Y, Joy2Ya, Joy2Yb;
int Joy3X, Joy3Xa, Joy3Xb;
int Joy4X, Joy4Xa, Joy4Xb;
int Joy1XCenter, Joy1YCenter, Joy2XCenter, Joy2YCenter, Joy3XCenter, Joy4XCenter;
int Joy1XLow=512, Joy1YLow=512, Joy2XLow=512, Joy2YLow=512, Joy3XLow=512, Joy4XLow=512;
int Joy1XHigh=512, Joy1YHigh=512, Joy2XHigh=512, Joy2YHigh=512, Joy3XHigh=512, Joy4XHigh=512;
int joyconfigX1, joyconfigY1, joyconfigX2, joyconfigY2, joyconfigX3, joyconfigX4;
byte enable, bodyConfigStep, ServoMode, lBut1State, rBut1State;
byte rJoySelect, domeSend;
byte JoySelectState, ServoJoySelectState, dome180, DomeDirection;
byte menuConfirm, Display, cursorMove, updateScreen = 1;
byte casenum, lastcasenum = 1;
byte start, wireless, bodyWireless;
byte joyConfStep = 1;
byte But1State, But2State, But1Sound, voiceSend, But2Sound, musicSend;
int joyConfCountdown;
byte waitTime = 5;
String menuItems[] = {"Reverse Dome", "Dome Config", "Body Config", "Joystick Config"};

String speedDisplay[] = {"Slow", "Medium", "Fast"};
//int rectime[20];
//byte rectimeloc;
const int eepromSet = 890;
int rBut1 = 1;
float measuredvbat;
unsigned long buttonTimer, menuTimeout, joyConfMillis, lastScreenUpdate, domeConfigTimeout; 
unsigned long bodyConfigMillis, domeConnectivityMillis, lastrecdata = 1000; 
unsigned long lastReading, lastLoopMillis, lastrecDataMillis, lastSend;
unsigned long But1Millis, But2Millis;

byte lJoySelect, lBut1=1, lBut2=1, lBut3, Fwd; 
byte Speed, rBut2, rBut3=1, motorEnable=1, lBut1Timer, lBut2Timer, rBut3Timer;
unsigned long lBut1Millis, lBut2Millis, rBut3Millis;

byte startup = 1;

void setup()
{
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
    Wire.setClock(400000L);
    oled.setFont(Callibri15);
    oled.println(F("==========================="));
    oled.println(F("            Joe's Drive       "));
    oled.println(F("Naigon Edition"));
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
    radio.initialize(FREQUENCY,REMOTE_ADDRESS,NETWORKID);
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
        Joy1XLow = EEPROM.readInt(12);
        Joy1XHigh = EEPROM.readInt(14);
        Joy1YLow = EEPROM.readInt(16);
        Joy1YHigh = EEPROM.readInt(18);
        Joy2XLow = EEPROM.readInt(20);
        Joy2XHigh = EEPROM.readInt(22);
        Joy2YLow = EEPROM.readInt(24);
        Joy2YHigh = EEPROM.readInt(26);
        Joy3XLow = EEPROM.readInt(28);
        Joy3XHigh = EEPROM.readInt(30);
        Joy4XLow = EEPROM.readInt(32);
        Joy4XHigh = EEPROM.readInt(34);
    }
    else
    {
        Joy1XCenter = 512;
        Joy1YCenter = 512;
        Joy2XCenter = 512;
        Joy2YCenter = 512;
        Joy3XCenter = 512;
        Joy4XCenter = 512;
        Joy1XLow = 0;
        Joy1XHigh = 1023;
        Joy1YLow = 0;
        Joy1YHigh = 1023;
        Joy2XLow = 0;
        Joy2XHigh = 1023;
        Joy2YLow = 0;
        Joy2YHigh = 1023;
        Joy3XLow = 0;
        Joy3XHigh = 1023;
        Joy4XLow = 0;
        Joy4XHigh = 1023;
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
    #ifdef serialSound
    lBut1 = digitalRead(lBut1PIN);
    lBut2 = digitalRead(lBut2PIN);
    rBut3 = digitalRead(lBut3PIN);

    if(analogRead(rBut3PIN) > 1)
    {
        rBut3 = 1;
    }
    else
    {
        rBut3 = 0;
    }

    if((lBut1 == 0 || rBut3 == 0) && But1State == 0)
    {
        But1State = 1;
        But1Millis = millis();
    }
    else if(lBut1 == 1 && rBut3 == 1 && But1State == 1)
    {
        But1State = 0;
        But1Sound ++;
        
        if(But1Sound > 6)
        {
            But1Sound -= 6;
        }
    }
    if(But1Sound > 0 && millis() - But1Millis >= 800)
    {
        voiceSend = 0;
        But1Sound = 0;
        sendToBody.lBut1 = But1Sound;
    }
    else if(But1Sound > 0 && millis() - But1Millis >= 400)
    {
        sendToBody.lBut1 = But1Sound;
    }

    if(lBut2 == 0 && But2State == 0)
    {
        But2State = 1;
        But2Millis = millis();
    }
    else if(lBut2 == 1 && But2State == 1)
    {
        But2State = 0;
        But2Sound ++;

        if(But2Sound > 6)
        {
            But2Sound -= 6;
        }
    }

    if(But2Sound > 0 && millis() - But2Millis >= 800)
    {
        musicSend = 0;
        But2Sound = 0;
        sendToBody.lBut2 = But2Sound;
        
    }
    else if(But2Sound > 0 && millis() - But2Millis >= 400)
    {
        sendToBody.lBut2 = But2Sound;
    }

    if(sendToBody.rBut3 != 0)
    {
        sendToBody.rBut3 = 0;
    }

    #else
    lBut1Timer = digitalRead(lBut1PIN);
    lBut2Timer = digitalRead(lBut2PIN);

    if(analogRead(rBut3PIN) > 1)
    {
        rBut3Timer = 1;
    }
    else
    {
        rBut3Timer = 0;
    }

    if(lBut1Timer == 0 && lBut1 == 1)
    {
        lBut1 = 0;
        lBut1Millis = millis();
    }
    else if(lBut1Timer == 1 && lBut1 == 0)
    {
        if(millis() - lBut1Millis >= 300)
        {
            lBut1 = 1;
        }
    }

    if(lBut2Timer == 0 && lBut2 == 1)
    {
        lBut2 = 0;
        lBut2Millis = millis();
    }
    else if(lBut2Timer == 1 && lBut2 == 0)
    {
        if(millis() - lBut2Millis >= 300)
        {
            lBut2 = 1;
        }
    }

    if(rBut3Timer == 0 && rBut3 == 1)
    {
        rBut3 = 0;
        rBut3Millis = millis();
    }
    else if(rBut3Timer == 1 && rBut3 == 0)
    {
        if(millis() - rBut3Millis >= 300)
        {
            rBut3 = 1;
        }
    }
    #endif

    lBut3 = digitalRead(lBut3PIN);
    rBut1 = digitalRead(rBut1PIN);
    rBut2 = digitalRead(rBut2PIN);
    
    Joy1Xa = analogRead(Joy1XPIN);
    Joy1Ya = analogRead(Joy1YPIN);
    Joy2Xa = analogRead(Joy2XPIN);
    Joy2Ya = analogRead(Joy2YPIN);
    Joy3Xa = analogRead(Joy3XPIN);
    Joy4Xa = analogRead(Joy4XPIN);
    lJoySelect = digitalRead(lJoySelectPIN);
    rJoySelect = digitalRead(rJoySelectPIN);
    motorEnable = digitalRead(enablePIN);
    centerChannels();
    setServoMode();
}

void centerChannels()
{
    if (Joy1Xa == Joy1XCenter)
    {
        Joy1Xb = 256;
    }
    else if (Joy1Xa > Joy1XCenter)
    {
        Joy1Xb = map(Joy1Xa, Joy1XCenter, Joy1XHigh, 255, 0);
    }
    else if (Joy1Xa < Joy1XCenter)
    {
        Joy1Xb = map(Joy1Xa, Joy1XLow, Joy1XCenter, 512, 257);
    }

    if (Joy1Ya == Joy1YCenter)
    {
        Joy1Yb = 256;
    }
    else if (Joy1Ya > Joy1YCenter)
    {
        Joy1Yb = map(Joy1Ya, Joy1YCenter, Joy1YHigh, 255, 0);
    }
    else if (Joy1Ya < Joy1YCenter)
    {
        Joy1Yb = map(Joy1Ya, Joy1YLow, Joy1YCenter, 512, 257);
    }

    if (Joy2Xa == Joy2XCenter)
    {
        Joy2Xb = 256;
    }
    else if (Joy2Xa > Joy2XCenter)
    {
        Joy2Xb = map(Joy2Xa, Joy2XCenter, Joy2XHigh, 255, 0);
    }
    else if (Joy2Xa < Joy2XCenter)
    {
        Joy2Xb = map(Joy2Xa, Joy2XLow, Joy2XCenter, 512, 257);
    }
  
    if (Joy2Ya == Joy2YCenter)
    {
        Joy2Yb = 256;
    }
    else if (Joy2Ya > Joy2YCenter)
    {
        Joy2Yb = map(Joy2Ya, Joy2YCenter, Joy2YHigh, 255, 0);
    }
    else if (Joy2Ya < Joy2YCenter)
    {
        Joy2Yb = map(Joy2Ya, Joy2YLow, Joy2YCenter, 512, 257);
    }
  
    if (Joy3Xa == Joy3XCenter)
    {
        Joy3Xb = 256;
    }
    else if (Joy3Xa > Joy3XCenter)
    {
        Joy3Xb = map(Joy3Xa, Joy3XCenter, Joy3XHigh, 255, 0);
    }
    else if (Joy3Xa < Joy3XCenter)
    {
        Joy3Xb = map(Joy3Xa, Joy3XLow, Joy3XCenter, 512, 257);
    }
  
    if (Joy4Xa == Joy4XCenter)
    {
        Joy4Xb = 256;
    }
    else if (Joy4Xa > Joy4XCenter)
    {
        Joy4Xb = map(Joy4Xa, Joy4XCenter, Joy4XHigh, 255, 0);
    }
    else if (Joy4Xa < Joy4XCenter)
    {
        Joy4Xb = map(Joy4Xa, Joy4XLow, Joy4XCenter, 512, 257);
    }

    Joy1X = constrain(Joy1Xb,0,512);
    Joy1Y = constrain(Joy1Yb,0,512);
    Joy2X = constrain(Joy2Xb,0,512);
    Joy2Y = constrain(Joy2Yb,0,512);
    Joy3X = constrain(Joy3Xb,0,512);
    Joy4X = constrain(Joy4Xb,0,512);

    if(Display < 2)
    {
        //reverseControls();
    }
  }

void SendData()
{
    if(millis() > 2000 && millis() - lastSend >= sendDelay)
    {
        #ifndef serialSound
        sendToBody.lBut1 = lBut1; //left 1
        sendToBody.lBut2 = lBut2; //left 2
        sendToBody.rBut3 = rBut3; //right 3
        #endif

        sendToBody.Joy1Y = Joy1Y; //main drive
        sendToBody.Joy1X = Joy1X; //tilt / steer
        sendToBody.Joy2Y = Joy2Y; //head tilt
        sendToBody.Joy2X = Joy2X; //head spin
        sendToBody.Joy3X = Joy3X; //spin Flywheel
        sendToBody.Joy4X = Joy4X;
        sendToBody.ServoMode = ServoMode; //Select on left joystick
        sendToBody.lBut3 = lBut3; //left3
        sendToBody.Fwd = Fwd; //Select on right joystick = rJoySelect
        sendToBody.Speed = Speed;
        sendToBody.rBut2 = rBut2; //right 2
        sendToBody.motorEnable = motorEnable; //toggle on top
        sendToBody.wireless = bodyWireless;
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
    if(millis() - lastrecDataMillis >= recDelay)
    {
        if (radio.receiveDone())
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
}

void Screen()
{
    if(Display < 2)
    {
        infoScreen();
        menuTimer();
    }
    else if(Display == 2)
    {
        if(ServoMode != 0)
        {
            ServoMode = 0;
        }
        menuScreen();
        MenuTimeout();
        enable = 1;
        Joy1Xa = Joy1XCenter;
        Joy1Ya = Joy1YCenter;
        Joy2Xa = Joy2XCenter;
        Joy2Ya = Joy1YCenter;
        Joy3Xa = Joy3XCenter;
        Joy4Xa = Joy4XCenter;
        lJoySelect = 1;
        rJoySelect = 1;
    }
    else if(Display == 3)
    {
        domeReverseScreen();
    }
    else if(Display == 4)
    {
        domeConfig();
    }
    else if(Display == 5)
    {
        bodyConfig();
    }
    else if(Display == 6)
    {
        joystickConfig();
        enable = 1;
        Joy1Xa = Joy1XCenter;
        Joy1Ya = Joy1YCenter;
        Joy2Xa = Joy2XCenter;
        Joy2Ya = Joy1YCenter;
        Joy3Xa = Joy3XCenter;
        Joy4Xa = Joy4XCenter;
        lJoySelect = 1;
        rJoySelect = 1;
    }
}

void menuTimer()
{
    if(rBut2 == 0 && rBut3 == 0 && Display == 0)
    {
        buttonTimer = millis();
        Display = 1;
    }
    else if(rBut2 == 0 && rBut3 == 0 && Display == 1 && millis() - buttonTimer >= 1500)
    {
        oled.clear();
        Display = 2;
        menuTimeout = millis();
    }
    else if(rBut2 == 1 || rBut3 == 1 && Display == 1)
    {
        Display = 0; 
    }
}

void MenuTimeout()
{
    if(millis() - menuTimeout >= 15000)
    {
        resetMenu();
    }
}

void menuScreen()
{
    menuCursor();
    if(casenum != lastcasenum)
    {
        lastcasenum = casenum;
        switch (casenum)
        {
        case 0:
            oled.setFont(Callibri15);
            oled.setCursor(0,0);
            oled.print(">");
            oled.setCursor(15,0);
            oled.println(menuItems[0]);
            oled.print("   ");
            oled.setCursor(15,20);
            oled.println(menuItems[1]);
            oled.print("   ");
            oled.setCursor(15,40);
            oled.println(menuItems[2]);
            oled.print("   ");
            oled.setCursor(15,60);
            oled.println(menuItems[3]);
            break;
        case 1:
            oled.setFont(Callibri15);
            oled.setCursor(0,0);
            oled.print("   ");
            oled.setCursor(15,0);
            oled.println(menuItems[0]);
            oled.print(">");
            oled.setCursor(15,20);
            oled.println(menuItems[1]);
            oled.print("   ");
            oled.setCursor(15,40);
            oled.println(menuItems[2]);
            oled.print("   ");
            oled.setCursor(15,60);
            oled.println(menuItems[3]);
            break;
        case 2:
            oled.setFont(Callibri15);
            oled.setCursor(0,0);
            oled.print("   ");
            oled.setCursor(15,0);
            oled.println(menuItems[0]);
            oled.print("   ");
            oled.setCursor(15,20);
            oled.println(menuItems[1]);
            oled.print(">");
            oled.setCursor(15,40);
            oled.println(menuItems[2]);
            oled.print("   ");
            oled.setCursor(15,60);
            oled.println(menuItems[3]);
            break;
        case 3:
            oled.setFont(Callibri15);
            oled.setCursor(0,0);
            oled.print("   ");
            oled.setCursor(15,0);
            oled.println(menuItems[0]);
            oled.print("   ");
            oled.setCursor(15,20);
            oled.println(menuItems[1]);
            oled.print("   ");
            oled.setCursor(15,40);
            oled.println(menuItems[2]);
            oled.print(">");
            oled.setCursor(15,60);
            oled.println(menuItems[3]);
            break;
        }
    }

    if(digitalRead(lJoySelectPIN) == 0 ||digitalRead(rJoySelectPIN) == 0)
    {
        switch(casenum)
        {
        case 0:
            Display = 3;
            subMenuDome180();
            //Do This to reverse the dome
            break;
        case 1:
            JoySelectState = 1;
            Display = 4;
            oled.clear();
            oled.println("1. Face dome forward");
            oled.println("2. Press 'select'");
            domeConfigTimeout = millis();
            //Do This for Dome Config
            break;
        case 2:
            Display = 5;
            oled.clear();
            //Do This for Body Config
            break;
        case 3:
            Display = 6;
            break;
        }
    }
}

void menuCursor()
{
    if((analogRead(Joy1YPIN) < 200 && cursorMove == 0) || (analogRead(Joy2YPIN) < 200 && cursorMove == 0))
    {
        menuTimeout = millis();
        casenum++;
        cursorMove = 1;
        if(casenum > 3)
        {
            casenum = 0;
        }
    }
    else if((analogRead(Joy1YPIN) > 823 || analogRead(Joy2YPIN) > 823) && cursorMove == 0)
    {
        menuTimeout = millis();
        casenum--;
        cursorMove = 1;
        if(casenum < 0)
        {
            casenum = 3;
        }
    }
    else if(analogRead(Joy1YPIN) < 612 && analogRead(Joy1YPIN) > 412 && analogRead(Joy2YPIN) < 612 && analogRead(Joy2YPIN) > 412)
    {
        cursorMove = 0;
    }
}

void infoScreen()
{
    if(millis() - lastScreenUpdate >= 10000 || updateScreen == 1)
    {
        updateScreen = 0;
        oled.setCursor(0,0);
        oled.setFont(Callibri15);
        oled.print(speedDisplay[Speed]);oled.print(F("        "));
        oled.setCursor(100,0);
        if(Fwd == 0 || Fwd == 2)
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
}

void subMenuDome180()
{
    if(dome180 == 0)
    {
        dome180 = 2;
    }
    else
    {
        dome180 = 0;
    }
    menuTimeout = millis();
    oled.clear();
    oled.print(F("Dome Reversed!"));
}

void domeConfig(){

    if(JoySelectState == 1 && sendToBody.CalibID == 0)
    {
        if(lJoySelect == 1 && rJoySelect == 1)
        {
            JoySelectState = 0;
        }
    }
    if((lJoySelect == 0 || rJoySelect == 0) && JoySelectState != 1)
    {
        JoySelectState = 1;
        sendToBody.CalibID = 1;
        oled.clear();
        oled.print(F(" Dome Configured! "));
        domeConfigTimeout = millis() - 8500 ;
    }

    if(millis() - domeConfigTimeout >= 10000)
    {
        JoySelectState = 0;
        resetMenu();
    }
}

void bodyConfig()
{
    if(bodyConfigStep == 0)
    {
        oled.print(F(" Body Configuration "));
        bodyConfigStep = 1;
        bodyConfigMillis = millis();
        sendToBody.CalibID = 2;
    }
    else if(bodyConfigStep == 1 && millis() - bodyConfigMillis >= 1500)
    {
        oled.setCursor(0,0);
        oled.println(F(" Adjust dome tilt        "));
        oled.println(F(" until the dome is        "));
        oled.println(F(" centered front to back.        "));
        oled.println(F(" Then press 'Select'        "));
        bodyConfigStep = 2;
    }
    else if(bodyConfigStep == 2)
    {
        if((digitalRead(lJoySelectPIN) == 0 || digitalRead(rJoySelectPIN) == 0) && JoySelectState == 0)
        {
            JoySelectState = 1;
        }
        else if((digitalRead(lJoySelectPIN) == 1 && digitalRead(rJoySelectPIN) == 1) && JoySelectState == 1)
        {
            JoySelectState = 0;
            sendToBody.CalibID = 3;
            bodyConfigStep = 3;
            bodyConfigMillis = millis();
            oled.clear();
            oled.println(F(" Dome tilt offset "));
            oled.println(F(" and pitch offset "));
            oled.println(F(" saved to eeprom "));
        }
    }
    else if(bodyConfigStep == 3 && millis() - bodyConfigMillis >= 2500)
    {
        oled.clear();
        oled.println(F(" Adjust Side to Side "));
        oled.print(F(" until ")); oled.print(F(droidName)); oled.println(F(" is straight."));
        oled.println(F(" Then press 'Select'        "));
        bodyConfigStep = 4;
    }
    else if(bodyConfigStep == 4)
    {
        if((digitalRead(lJoySelectPIN) == 0 || digitalRead(rJoySelectPIN) == 0) && JoySelectState == 0)
        {
            JoySelectState = 1;
        }
        else if((digitalRead(lJoySelectPIN) == 1 && digitalRead(rJoySelectPIN) == 1) && JoySelectState == 1)
        {
            JoySelectState = 0;
            sendToBody.CalibID = 4;
            bodyConfigStep = 5;
            bodyConfigMillis = millis();
            oled.clear();
            oled.println(F(" Body Calibration ")); oled.println(F(" Complete! "));
        }
    }
    else if(bodyConfigStep == 5 && millis() - bodyConfigMillis >= 2500)
    {
        resetMenu();
    }
}

void joystickConfig()
{
    if(joyConfStep == 1)
    {
        joyConfMillis = millis();
        joyConfStep = 2;
        oled.clear();
    }
    else if(joyConfStep == 2 && millis() - joyConfMillis >= joyConfCountdown && waitTime > 0 )
    {
        oled.setCursor(0,0);
        oled.println(F("Release Joysticks"));
        oled.print(F("Wait: ")); oled.print(waitTime);
        joyConfCountdown += 1000;
        waitTime--;
    }
    else if(joyConfStep == 2 && waitTime == 0 && millis() - joyConfMillis >= joyConfCountdown)
    {
        joyConfStep = 3;
        joyConfCountdown += 2000;
        oled.setCursor(0,0);
        oled.println(F("Storing Center values"));
        oled.print(F("                    "));
        setJoystickCenter();
        waitTime = 15;
    }
    else if(joyConfStep == 3 && millis() - joyConfMillis >= joyConfCountdown && waitTime > 0)
    {
        oled.setCursor(0,0);
        oled.println(F("Continuously rotate      ")); oled.println(F("all joysticks"));
        oled.print(waitTime);oled.print("                  ");
        joyConfCountdown += 1000;
        waitTime--;
    }
    else if(joyConfStep == 3 && waitTime != 0)
    {
        readJoystickHighAndLow();
    }
    else if(joyConfStep == 3 && waitTime == 0)
    {
        joyConfStep = 4;
        joyConfCountdown += 2500;
        oled.setCursor(0,0);
        oled.println(F("Calibration Complete!              "));
        oled.println(F("                                   "));
        oled.println(F("                                   "));
        setJoystickHighAndLow();
    }
    else if(joyConfStep == 4 && millis() - joyConfMillis >= joyConfCountdown)
    {
        resetMenu();
    }
}

void resetMenu()
{
    oled.clear();
    joyConfStep = 1;
    casenum = 0;
    lastcasenum = 1;
    menuConfirm = 0;
    joyConfCountdown = 0;
    waitTime = 5;
    Display = 0;
    sendToBody.CalibID = 0;
    bodyConfigStep = 0;
}

void readJoystickHighAndLow()
{
    if(start == 0)
    {
        start = 1;
        Joy1XLow = 512;
        Joy1XHigh = 512;
        Joy1YLow = 512;
        Joy1YHigh = 512;
        Joy2XLow = 512;
        Joy2XHigh = 512;
        Joy2YLow = 512;
        Joy2YHigh = 512;
        Joy3XLow = 512;
        Joy3XHigh = 512;
        Joy4XLow = 512;
        Joy4XHigh = 512;
    }
    else if(start == 1)
    {
        constrain(joyconfigX1 = analogRead(Joy1XPIN),0,1023);
        constrain(joyconfigY1 = analogRead(Joy1YPIN),0,1023);
        constrain(joyconfigX2 = analogRead(Joy2XPIN),0,1023);
        constrain(joyconfigY2 = analogRead(Joy2YPIN),0,1023);
        constrain(joyconfigX3 = analogRead(Joy3XPIN),0,1023);
        constrain(joyconfigX4 = analogRead(Joy4XPIN),0,1023);

        if (joyconfigX1 < Joy1XLow)
        {
            Joy1XLow = joyconfigX1;
        }
        if (joyconfigY1 < Joy1YLow)
        {
            Joy1YLow = joyconfigY1;
        }
        if (joyconfigX2 < Joy2XLow)
        {
            Joy2XLow = joyconfigX2;
        }
        if (joyconfigY2 < Joy2YLow)
        {
            Joy2YLow = joyconfigY2;
        }
        if (joyconfigX3 < Joy3XLow)
        {
            Joy3XLow = joyconfigX3;
        }
        if (joyconfigX4 < Joy4XLow)
        {
            Joy4XLow = joyconfigX4;
        }
        if (joyconfigX1 > Joy1XHigh)
        {
            Joy1XHigh = joyconfigX1;
        }
        if (joyconfigY1 > Joy1YHigh)
        {
            Joy1YHigh = joyconfigY1;
        }
        if (joyconfigX2 > Joy2XHigh)
        {
            Joy2XHigh = joyconfigX2;
        }
        if (joyconfigY2 > Joy2YHigh)
        {
            Joy2YHigh = joyconfigY2;
        }
        if (joyconfigX3 > Joy3XHigh)
        {
            Joy3XHigh = joyconfigX3;
        }
        if (joyconfigX4 > Joy4XHigh)
        {
            Joy4XHigh = joyconfigX4;
        }
    }
}

void setJoystickCenter()
{
    start = 0;
    EEPROM.writeInt(0,analogRead(Joy1XPIN));
    EEPROM.writeInt(2,analogRead(Joy1YPIN));
    EEPROM.writeInt(4,analogRead(Joy2XPIN));
    EEPROM.writeInt(6,analogRead(Joy2YPIN));
    EEPROM.writeInt(8,analogRead(Joy3XPIN));
    EEPROM.writeInt(10,analogRead(Joy4XPIN));
    delay(1000);
}

void setJoystickHighAndLow()
{
    EEPROM.writeInt(12,Joy1XLow);
    EEPROM.writeInt(14,Joy1XHigh);
    EEPROM.writeInt(16,Joy1YLow);
    EEPROM.writeInt(18,Joy1YHigh);
    EEPROM.writeInt(20,Joy2XLow);
    EEPROM.writeInt(22,Joy2XHigh);
    EEPROM.writeInt(24,Joy2YLow);
    EEPROM.writeInt(26,Joy2YHigh);
    EEPROM.writeInt(28,Joy3XLow);
    EEPROM.writeInt(30,Joy3XHigh);
    EEPROM.writeInt(32,Joy4XLow);
    EEPROM.writeInt(34,Joy4XHigh);
    EEPROM.writeInt(36,eepromSet);
    delay(1000);
}

void domeReverseScreen()
{
    if(millis() - menuTimeout >= 1500)
    {
        resetMenu();
    }
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

void setServoMode()
{
    if(Display == 0)
    {
        if(lJoySelect == 0 && ServoJoySelectState == 0)
        {
            ServoJoySelectState = 1;
        }
        else if(lJoySelect == 1 && ServoJoySelectState == 1)
        {
            ServoJoySelectState = 0;
            ServoMode = ServoMode == 0 ? 1 : 0;
        }
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
    Serial.print(sendToBody.ServoMode); Serial.print(", ");
    Serial.print(sendToBody.lBut1); Serial.print(", ");
    Serial.print(sendToBody.lBut2); Serial.print(", ");
    Serial.print(sendToBody.lBut3); Serial.print(", ");
    Serial.print(sendToBody.Fwd); Serial.print(", ");
    Serial.print(sendToBody.Speed); Serial.print(", ");
    Serial.print(sendToBody.rBut2); Serial.print(", ");
    Serial.print(sendToBody.rBut3); Serial.print(", ");
    Serial.print(sendToBody.motorEnable); Serial.print(", ");
    Serial.print(sendToBody.CalibID); Serial.print(", ");
    Serial.println(sendToBody.wireless);
}
