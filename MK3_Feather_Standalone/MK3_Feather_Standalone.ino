// ===================================================================================================================================================================================================== 
//                         Joe's Drive  - Remote "MK3"  - Updated 4/11
//
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty, guarantee, or other tomfoolery. 
//                         This entire project was masterminded by an average Joe, your mileage may vary. 
// ===================================================================================================================================================================================================== 
//                            Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//
//                            You will need libraries: Modified Low PowerLab: https://travis-ci.org/LowPowerLab/RFM69  
//                                                     EasyTransfer: https://github.com/madsci1016/Arduino-EasyTransfer
//                                                     
//
// ===================================================================================================================================================================================================== 
// =====================================================================================================================================================================================================

/*
 * 
 * 
 * 
 *        Find file: RFM69.h
 *        Find: #define RF69_CSMA_LIMIT_MS 1000
 *        Change to: #define RF69_CSMA_LIMIT_MS 100
 * 
 * 
 * 
 * 
 */ 

#define dataDelay 5
#define sendDelay 45

#include <SPI.h>
#include <RFM69.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>
#include <RH_Serial.h>
#include "Arduino.h"
#include <EasyTransfer.h>

EasyTransfer SendRemote;
EasyTransfer SendBody;

// Naigon
// Uncomment this to debug the values coming from the remote across the RMF69 channel.
//#define DebugRemote

// Naigon
// Uncomment this to debug the values received from the body
//#define DebugBody

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

RH_Serial driver(Serial1);
RHReliableDatagram manager(driver, BODY_ADDRESS);

uint8_t remfrom;
uint8_t bodyfrom;
unsigned long lastLoop, lastSent;

typedef struct RECEIVE_DATA_STRUCTURE_REMOTE
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
    // Naigon - button 2 press plays a happy/neutral sound. Holding plays a longer/sadder sound
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
} recRemoteData; 
recRemoteData recFromRemote;

byte send_buf[sizeof(recFromRemote)];

typedef struct RECEIVE_DATA_STRUCTURE_DRIVE
{
    double bodyBatt = 0.0;
    double domeBatt = 0.0;
    uint8_t bodyStatus;
    uint8_t bodyMode;
    uint8_t bodyDirection;
    uint8_t psi;
} recBodyData;
recBodyData recFromBody;

typedef struct SEND_DATA_STRUCTURE_REMOTE
{
    double bodyBatt = 0.0;
    double domeBatt = 0.0;
    uint8_t bodyStatus;
    uint8_t bodyMode;
    uint8_t bodyDirection;
} sendRemoteData;
sendRemoteData sendToRemote;

typedef struct RECEIVE_DATA_STRUCTURE
{
    int psi=0;
    byte button4 = 1;
    float bodyBatt;
} sendDomeData;
sendDomeData sendToDome;

uint8_t bodybuf[sizeof(recFromBody)];
uint8_t bodybuflen = sizeof(recFromBody);

uint8_t from;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial1.begin(57600);
    driver.serial().begin(57600);

    if (!manager.init())
    {
        Serial.println("init failed");
    }

    // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);

    // Initialize radio
    radio.initialize(FREQUENCY,BODY_ADDRESS,NETWORKID);
    if (IS_RFM69HCW)
    {
      radio.setHighPower();    // Only for RFM69HCW & HW!
    }
    radio.setPowerLevel(20); // power output ranges from 0 (5dBm) to 31 (20dBm)

    SendRemote.begin(details(recFromRemote), &Serial1);
    SendBody.begin(details(recFromBody), &Serial1);
}

void loop()
{
    if(millis() - lastLoop >= 2)
    {
        lastLoop = millis();
        recRemote();
    } 

    if(millis() - lastSent >= 100)
    {
        lastSent = millis();
        recBody();
    }
}

void PrintDebugValues()
{
    Serial.print(recFromRemote.Joy1Y);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy1X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy2Y);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy2X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy3X);
    Serial.print(F(" , "));
    Serial.print(recFromRemote.Joy4X);
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
    Serial.println();
}

void recRemote()
{
    bool received = radio.receiveDone();

    // Do nothing if no new data.
    if (!received) { return; }

    uint16_t id = radio.SENDERID;

    if (id == uint16_t(REMOTE_ADDRESS))
    {
        if (radio.DATALEN != sizeof(recFromRemote))
        {
            Serial.print("Invalid payload received, not matching Payload struct!");
        }
        else
        {
            recFromRemote = *(recRemoteData*)radio.DATA;

            #ifdef DebugRemote
            PrintDebugValues();
            #endif

            SendRemote.sendData();
            delay(1);
        }
    }

    recBody();
}

void recBody()
{
    if(Serial1.available() > 0)
    {
        SendBody.receiveData();

        // Naigon
        // I combined the remote and dome stuff into one send object, and just split out the dome data to send separately.
        //
        sendToRemote.bodyBatt = recFromBody.bodyBatt;
        sendToRemote.bodyMode = recFromBody.bodyMode;
        sendToRemote.bodyStatus = recFromBody.bodyStatus;
        sendToRemote.bodyDirection = recFromBody.bodyDirection;
        sendToRemote.domeBatt = recFromBody.domeBatt;

        #ifdef DebugBody
        Serial.print(F("bodyStatus: "));
        Serial.print(sendToRemote.bodyStatus);
        Serial.println();
        #endif

        memcpy(bodybuf, &sendToRemote, sizeof(sendToRemote));
        radio.send(REMOTE_ADDRESS, bodybuf, sizeof(sendToRemote));
        delay(10);

        // Copy out the psi value directly from the remote values.
        sendToDome.psi = recFromBody.psi;
        memcpy(bodybuf, &sendToDome, sizeof(sendToDome));
        radio.send(DOME_ADDRESS, bodybuf, sizeof(sendToDome));
        delay(1);
    }
    lastSent = millis();
}
