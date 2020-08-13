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
      uint8_t rembuf[RH_RF69_MAX_MESSAGE_LEN];
      uint8_t rembuflen = sizeof(rembuf);
      uint8_t bodyfrom;
      
  unsigned long lastLoop, lastRec, lastSent;

  typedef struct RECEIVE_DATA_STRUCTURE_REMOTE{
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
        
  }recRemoteData; 
  recRemoteData recFromRemote;

  byte send_buf[sizeof(recFromRemote)];
  bool Send;   


  typedef struct RECEIVE_DATA_STRUCTURE_DRIVE{
        int PSI;
        byte ledStatus;
        float bodyBatt;
        //float domeBatt;
        
  }recBodyData;
  recBodyData recFromBody;

  uint8_t bodybuf[sizeof(recFromBody)];
  uint8_t bodybuflen = sizeof(recFromBody);

  typedef struct RECEIVE_DATA_STRUCTURE_DOME{
 
        float domeBatt;
        
  }recDomeData;
  recDomeData recFromDome;

  uint8_t from;
  uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
  uint8_t buflen = sizeof(buf);


  

  
  void setup() {
    // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(57600);
  driver.serial().begin(57600);
    if (!manager.init())
    Serial.println("init failed");

 // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);
  
    // Initialize radio
    radio.initialize(FREQUENCY,BODY_ADDRESS,NETWORKID);
    if (IS_RFM69HCW) {
      radio.setHighPower();    // Only for RFM69HCW & HW!
    }
    radio.setPowerLevel(20); // power output ranges from 0 (5dBm) to 31 (20dBm)

  SendRemote.begin(details(recFromRemote), &Serial1);
  SendBody.begin(details(recFromBody), &Serial1);
  
    
  
  }
  
  void loop() {
    if(millis() - lastLoop >= 2){
      lastLoop = millis();
      recRemote();
    } 
     
    if(millis() - lastSent >= 80){
      recBody();
    }


  
  }  
  
  
  
  
