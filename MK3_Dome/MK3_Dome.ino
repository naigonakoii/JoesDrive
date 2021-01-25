// =====================================================================================================================================================================================================
//                         Joe's Drive  - Dome "MK3"  - Updated 4/14/18
//
//             ***         You are free to use, and share this code as long as it is not sold. There is no warranty, guarantee, or other tomfoolery.
//                         This entire project was masterminded by an average Joe, your mileage may vary.
// =====================================================================================================================================================================================================
//                            Written by Joe Latiola - https://www.facebook.com/groups/JoesDrive/
//
//                            You will need libraries: AdafruitNeopixel: https://github.com/adafruit/Adafruit_NeoPixel
//                                                     Modified Low PowerLab: https://travis-ci.org/LowPowerLab/RFM69
//
//
// =====================================================================================================================================================================================================
// =====================================================================================================================================================================================================

#define psiPIN 10
#define sLogicPIN 5
#define lLogicPIN 6
#define hpPIN 3
#define eyePIN 11
#define battPin A9
#define dataDelay 0
#define recDelay 10
#define interval 40
#define interval2 2
#define sendDelay 50

#define useNeoPixelPSI   // Uncomment if you are using a neopixel for the PSI, leave commented if using a standard led.

#include <Adafruit_NeoPixel.h>

#include <RFM69.h> //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>

//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE ************
//*********************************************************************************************
#define NETWORKID 100    // The same on all nodes that talk to each other
#define REMOTE_ADDRESS 1 // The unique identifier of this node
#define BODY_ADDRESS 2   // The recipient of packets
#define DOME_ADDRESS 3   // The recipient of packets
#define DRIVE_ADDRESS 4

#define FREQUENCY RF69_915MHZ // THIS NEEDS TO MATCH YOUR FEATHER

#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************

#define RFM69_CS 8
#define RFM69_IRQ 7
#define RFM69_IRQN 4 // Pin 7 is IRQ 4!
#define RFM69_RST 4

//#define RFM69_CS      0
//#define RFM69_IRQ     1
//#define RFM69_IRQN    3  // Pin 1 is IRQ 3!
//#define RFM69_RST     23

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);

struct SEND_DATA_STRUCTURE
{
    float bodyBatt;
    float domeBatt;
} sendFromDome;

byte packet[sizeof(sendFromDome)];
float domeBatt1;

typedef struct RECEIVE_DATA_STRUCTURE
{
  int psi = 0;
  byte button4 = 1;
  float bodyBatt;
} recBodyData;
recBodyData recFromBody;

byte i, a, b, Set = 1, s;

#define LED_PIN 13 // (Arduino is 13, Teensy is 11, Teensy++ is 6)
bool blinkState = false;

Adafruit_NeoPixel sLOGIC = Adafruit_NeoPixel(3, sLogicPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel lLOGIC = Adafruit_NeoPixel(6, lLogicPIN, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel HP = Adafruit_NeoPixel(1, hpPIN, NEO_RGB + NEO_KHZ800);
Adafruit_NeoPixel EYE = Adafruit_NeoPixel(1, eyePIN, NEO_GRB + NEO_KHZ800);

#ifdef useNeoPixelPSI
Adafruit_NeoPixel PSI = Adafruit_NeoPixel(1, psiPIN, NEO_GRB + NEO_KHZ800);
#endif

unsigned long randomMillis, previousMillis, previousMillis2, lastSendRecMillis;
unsigned long lastHPCycleMillis, randomMillisSingle, but4StateMillis, lastBattUpdate;
unsigned long lastBodyReceive, lastFlash;
int psiVal;

const byte numChars = 32;
int LEDState = 1;
int but4State;
int flashtime;
int holoPulseState = 2;
int bpulse = 80;

int hpCycleState, hpRed, hpGreen, hpBlue;

int rearFadeState, rearFadeRed, rearFadeBlue, rearFadeGreen;

//     ==========================================================================

void setup()
{
    Serial.begin(115200);
    //Serial1.begin(115200);

    // configure LED for output
    pinMode(LED_PIN, OUTPUT);
    sLOGIC.begin();
    lLOGIC.begin();
    HP.begin();
    EYE.begin();
#ifdef useNeoPixelPSI
    PSI.begin();
#endif

    // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);

    // Initialize radio
    radio.initialize(FREQUENCY, DOME_ADDRESS, NETWORKID);
    if (IS_RFM69HCW)
    {
        radio.setHighPower(); // Only for RFM69HCW & HW!
    }
    radio.setPowerLevel(31); // power output ranges from 0 (5dBm) to 31 (20dBm)

    eyeLED();
}

void loop()
{
    sendAndReceive();

    if (millis() - previousMillis > interval)
    {
        previousMillis = millis();

        if (LEDState == 1)
        {
            doubleLogic();
            Holo();
            rearLogic();
        }
        else if (LEDState == 2)
        {
            doubleLogicFade();
            holoPulse();
            rearLogicFade();
            hpCycleState = 0;
        }
        else if (LEDState == 3)
        {
            doubleLogicRandom();
            hpCycle();
            rearLogicRandom();
        }

        if (recFromBody.button4 == 0 && but4State < 2)
        {
            LED_State();
        }
        else if (recFromBody.button4 == 1 && but4State != 0)
        {
            but4State = 0;
        }
    }
}


//==============             
void rearLogic()
{
    for(int i = 0; i < 3; i++)
    {
        sLOGIC.setPixelColor(i, sLOGIC.Color(23, 67, 96)); //small logic is light blue
        sLOGIC.show();
    }
}

//==============             
void doubleLogic()
{
    lLOGIC.setPixelColor(0, lLOGIC.Color(47, 49, 50)); //large logic is almost white
    lLOGIC.setPixelColor(1, lLOGIC.Color(47, 49, 50)); //large logic is almost white
    lLOGIC.setPixelColor(2, lLOGIC.Color(47, 49, 50)); //large logic is almost white
    lLOGIC.setPixelColor(3, lLOGIC.Color(0, 0, 0)); 
    lLOGIC.setPixelColor(4, lLOGIC.Color(0, 0, 0)); 
    lLOGIC.setPixelColor(5, lLOGIC.Color(0, 0, 0)); 
    lLOGIC.show();
}

//==============             
void eyeLED(){ 
    
    EYE.setPixelColor(0, EYE.Color(0, 200, 0)); // Eye is red
    EYE.show();
  }
  
//==============             
  void Holo(){
    
    HP.setPixelColor(0, HP.Color(0, 0, 0));
    HP.show();
  } 

//==============             
void PSILED()
{
    if(recFromBody.psi != 0)
    {
        if(millis() - lastFlash > flashtime)
        {
            lastFlash = millis();
            flashtime = random(20,70);
            psiVal = psiVal == 255 ? 60 : 255;
        }
    }
    else
    {
        psiVal = 0;
    }

#ifdef useNeoPixelPSI
    PSI.setPixelColor(0, PSI.Color(psiVal,psiVal,psiVal)); 
    PSI.show();
#else
    analogWrite(psiPIN, psiVal); 
#endif 
}


//=================
void doubleLogicRandom()
{
    if(millis() - randomMillis > 300)
    {
        int random_i = random(0,6);
        lLOGIC.setPixelColor(random_i, lLOGIC.Color(random(0,255),random(0,255),random(0,255))); 
        lLOGIC.show();
        randomMillis = millis();
    }
}
//=================
void doubleLogicFade()
{
    if(Set == 1)
    {
        a++;
        if (b > 0)
        {
            b--;
        }
    }
    else if (Set == 2)
    {
        a--;
        b++;
    }
    constrain(a, 0, 50);
    constrain(b, 0, 50);

    if(a== 50)
    {
        Set = 2;
    }
    else if (a == 0)
    {
        Set = 1;
    }

    lLOGIC.setPixelColor(0, lLOGIC.Color(a, a, a)); 
    lLOGIC.setPixelColor(1, lLOGIC.Color(a, a, a)); 
    lLOGIC.setPixelColor(2, lLOGIC.Color(a, a, a)); 
    lLOGIC.setPixelColor(3, lLOGIC.Color(b, b, b));
    lLOGIC.setPixelColor(4, lLOGIC.Color(b, b, b)); 
    lLOGIC.setPixelColor(5, lLOGIC.Color(b, b, b));  
    lLOGIC.show();
}

void LED_State()
{
    if(but4State == 0)
    {
        but4StateMillis = millis();
        but4State = 1;
    }
    if(but4State == 1 && (millis() - but4StateMillis > 400))
    {
        if(LEDState == 3)
        {
            LEDState = 1;
        }
        else
        {
            LEDState++;
        }
        but4State = 2;
    }
}

void holoPulse()
{
    if(holoPulseState == 1 && bpulse <= 155)
    {
        bpulse++;
    }
    else if(bpulse >= 155)
    {
        holoPulseState = 2;
    }

    if(holoPulseState == 2 && bpulse >= 80)
    {
        bpulse--;
    }
    else if(bpulse <= 80)
    {
        holoPulseState = 1;
    }

    HP.setPixelColor(0, HP.Color(0, 0, bpulse));
    HP.show();
}

void hpCycle()
{
    if(hpCycleState == 0)
    {
        hpRed = 0;
        hpGreen = 0;
        hpBlue = 0;
        hpCycleState = 1;
    }
    else if(hpCycleState == 1)
    {
        if( hpRed <= 250)
        {
            hpRed += 3;
        }
        else
        {
            hpCycleState = 2;
        }
    }
    else if(hpCycleState == 2)
    {
        if(hpRed >= 10)
        {
            hpRed -= 3;
        }
        else
        {
            hpCycleState = 3;
        }
    }
    else if(hpCycleState == 3)
    {
        if(hpGreen <= 250)
        {
            hpGreen += 3;
        }
        else
        {
            hpCycleState = 4;
        }
    }
    else if(hpCycleState == 4)
    {
        if(hpGreen >=10)
        {
            hpGreen -= 3;
        }
        else
        {
            hpCycleState = 5;
        }
    }
    else if(hpCycleState == 5)
    {
        if(hpBlue <= 250)
        {
            hpBlue += 3;
        }
        else
        {
            hpCycleState = 6; 
        }
    }
    else if(hpCycleState == 6)
    {
        if(hpBlue >=10)
        {
            hpBlue -= 3;
        }
        else
        {
            hpCycleState = 7;
        }
    }
    else if(hpCycleState == 7)
    {
        if(hpRed <= 250)
        {
            hpRed+= 3;
        }
        else if(hpGreen <= 250)
        {
            hpGreen+= 3;
        }
        else if(hpBlue <= 250)
        {
            hpBlue+= 3;
        }
        else
        {
            hpCycleState=8;
        }
    }
    else if(hpCycleState == 8 )
    {
        if(hpRed >= 10)
        {
            hpRed-= 3;
        }
        else if(hpGreen >= 10)
        {
            hpGreen-= 3;
        }
        else if(hpBlue >= 10)
        {
            hpBlue-= 3;
        }
        else
        {
            hpCycleState = 1;
        }
    }

    HP.setPixelColor(0, HP.Color(hpRed, hpGreen, hpBlue));
    HP.show();
}

void rearLogicRandom()
{
    if(millis() - randomMillisSingle > 300)
    {
        int random_i = random(0,3);
        sLOGIC.setPixelColor(random_i, sLOGIC.Color(random(0,255),random(0,255),random(0,255))); 
        sLOGIC.show();
        randomMillisSingle = millis();
    }
}

void rearLogicFade()
{
    if(rearFadeState == 0)
    {
        if(rearFadeRed < 24)
        {
            rearFadeRed++;
            rearFadeGreen = map(rearFadeRed, 0, 23, 0, 67);
            rearFadeBlue = map(rearFadeRed, 0, 23, 0, 96);
        }
        else
        {
            rearFadeState = 1;
        }
    }
    else if(rearFadeState == 1)
    {
        if(rearFadeRed > 0)
        {
            rearFadeRed--;
            rearFadeGreen = map(rearFadeRed, 0, 23, 0, 67);
            rearFadeBlue = map(rearFadeRed, 0, 23, 0, 96);
        }
        else
        {
            rearFadeState = 0;
        }
    }
    sLOGIC.setPixelColor(0, sLOGIC.Color(rearFadeRed, rearFadeGreen, rearFadeBlue));
    sLOGIC.setPixelColor(1, sLOGIC.Color(rearFadeRed, rearFadeGreen, rearFadeBlue));
    sLOGIC.setPixelColor(2, sLOGIC.Color(rearFadeRed, rearFadeGreen, rearFadeBlue));
    sLOGIC.show();
}

void sendAndReceive()
{
    if(millis() - lastSendRecMillis >= recDelay)
    {
        if (radio.receiveDone() && radio.SENDERID == uint8_t(BODY_ADDRESS))
        {
            if (radio.DATALEN != sizeof(recFromBody))
            {
                Serial.print("Invalid payload received, not matching Payload struct! Size received: "); Serial.print(sizeof(radio.DATALEN));
                Serial.print(" Size should be: "); Serial.println(sizeof(recFromBody)); 
            }
            else
            {
                recFromBody = *(recBodyData*)radio.DATA;
                PSILED();
                lastBodyReceive = millis();
            }
        }
        lastSendRecMillis = millis(); 
    }

    battLevel();
}

//==============             
void battLevel()
{
    if(millis() - lastBattUpdate >= sendDelay)
    {
        if(millis() - lastBodyReceive >= 3000)
        {
            sendFromDome.bodyBatt = 99.99;
        }
        else
        {
            sendFromDome.bodyBatt = recFromBody.bodyBatt;
        }
        domeBatt1 = analogRead(battPin);
        domeBatt1 *= 2;    // we divided by 2, so multiply back
        domeBatt1 *= 3.3;  // Multiply by 3.3V, our reference voltage
        domeBatt1  /= 1024; // convert to voltage
        sendFromDome.domeBatt = domeBatt1;
        lastBattUpdate = millis();
        memcpy(packet, &sendFromDome, sizeof(sendFromDome)); //Copy data from "sendToBody" array to "send_buf" byte array 
        radio.send(REMOTE_ADDRESS, packet, sizeof(packet)); //target node Id, message as string or byte array, message length
        delay(5);
    }
}

//==============    

