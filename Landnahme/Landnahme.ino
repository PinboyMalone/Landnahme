 /*
 * MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY      COOQROBOT.
 * The library file MFRC522.h has a wealth of useful info. Please read it.
 * The functions are documented in MFRC522.cpp.
 *
 * Based on code Dr.Leong   ( WWW.B2CQSHOP.COM )
 * Created by Miguel Balboa (circuitito.com), Jan, 2012.
 * Rewritten by Søren Thing Andersen (access.thing.dk), fall of 2013 (Translation to      English, refactored, comments, anti collision, cascade levels.)
 * Released into the public domain.
 * Sample program showing how to read data from a PICC using a MFRC522 reader on the      Arduino SPI interface.
 *----------------------------------------------------------------------------- empty_skull 
 * Aggiunti pin per arduino Mega
 * add pin configuration for arduino mega
 * http://mac86project.altervista.org/
 ----------------------------------------------------------------------------- Nicola      Coppola
 * Pin layout should be as follows:
 * Signal     Pin              Pin               Pin
 *            Arduino Uno      Arduino Mega      MFRC522 board
 * ------------------------------------------------------------
 * Reset      9                5                 RST
 * SPI SS     10               53                SDA
 * SPI MOSI   11               52                MOSI
 * SPI MISO   12               51                MISO
 * SPI SCK    13               50                SCK
 *
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on    ebay.com. 
 */


#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>


//die modi in denen sich der Schrein befinden kann
#define real 0
#define demo 1
#define diagnostic 2


//der status in dem sich der schrein befinden kann
#define Free 0
#define activating 1
#define loading 2
#define active 3
#define blocked 4



//pin belegungen. kann je nach aufbau verändert werden.
#define rgb_red_PIN1 0
#define rgb_red_PIN2 1
#define rgb_green_PIN1 2
#define rgb_green_PIN2 3
#define rgb_blue_PIN1 4
#define rgb_blue_PIN2 5
#define servoLED1_PIN 6
#define servoLED2_PIN 7
#define servo_PIN 8
#define RST_PIN 9
#define SS_PIN 10
//9-13 muss frei bleiben für den rfid-sensor#define rgb_blue_PIN2 14



MFRC522 mfrc522(SS_PIN, RST_PIN);

Servo myServo;

#define minuten 60000

//die einzelnen Karten Identifier

#define Antike 0
#define Elben 1
#define Komet 2
#define HDC 3
#define Imperium 4
#define Krone 5
#define Licht 6
#define Norrelag 7
#define OHL 8
#define Pilger 9
#define Zusammenkunft 10
#define Stadt 11
#define Lesath 12
#define Neutral 13
#define Dunkel 14
#define Demo 15
#define Diagnostic 16
#define Unbekannt 17


//die zeiten für die phasen
const unsigned long ta = 10*minuten;      //Aktivierungszeit, Zeit in Phase 1
const unsigned long tl = 60*minuten;      //Ladezeit, Zeit in Phase 2
const unsigned long tn = 10*minuten;      //Landnahmezeit, Zeit in Phase 3
const unsigned long tb = 90*minuten;      //Blockzeit, Zeit in Phase 4
                                          //Phase 0 hat keine Zeit und kann beliebig lange gehen

                                        

short _status;
short newKeeper;
short mode;
long servoValue;
short servoDirection;

unsigned long activeTime;

int failCount;

int scan;

void setup() {
Serial.begin(9600);
  pinMode(servoLED1_PIN, OUTPUT);
  pinMode(servoLED2_PIN, OUTPUT);
  pinMode(rgb_red_PIN1,OUTPUT);
  pinMode(rgb_red_PIN2,OUTPUT);
  pinMode(rgb_blue_PIN1,OUTPUT);
  pinMode(rgb_blue_PIN2,OUTPUT);
  pinMode(rgb_green_PIN1,OUTPUT);
  pinMode(rgb_green_PIN2,OUTPUT);

  _status=Free;
  newKeeper=Neutral;
  mode = real;
  
  servoDirection=1;

  failCount=0;

  activeTime=0;

  scan = 0;

  SPI.begin();
  mfrc522.PCD_Init();

  setShrine();
}

void loop() {
  switch(mode)
  {
    case(real):
    switch(_status)
    {
      case(activating):
      if((micros()-activeTime)<ta)
      {
        if(!mfrc522.PICC_IsNewCardPresent())
        {
          failCount++;
          if(failCount>25)
          {
            _status--;
          }
        }
        else
        {
          failCount=0;
        }
      }
      else
      {
        _status++;
        failCount=0;
        activeTime=micros();
        setShrine();
        myServo.attach(8);
      }
      break;
 
      case(loading):
      delay(100);
      pulse();
        if((micros()-activeTime)<tl)
        {
          scan=scanForCard();
          if(scan==Lesath||scan==Neutral||scan==Dunkel)
          {
            myServo.detach();
            delay(3000);
            newKeeper = scan;
            _status=0;
            setShrine();  
          }
        }
        else
        {
            _status++;
            activeTime=micros();
            setShrine();        
        }

      break;

      case(active):
      pulse();
      if((micros()-activeTime)<tn)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          myServo.detach();
          delay(3000);
          newKeeper = scan;
          _status=0;
          setShrine();  
        }
        else if((scan!=Unbekannt)&&(scan!=Demo)&&(scan!=Diagnostic))
        {
          myServo.detach();
          delay(3000);
          newKeeper=scan;
          _status++;
          setShrine();
          activeTime=micros();          
        }
        
      }
      else
      {
        myServo.detach();
        delay(3000);
        _status=0;
        setShrine();
        activeTime=micros();
      }
      
      break;

      case(blocked):
      if((micros()-activeTime)<tb)
      {
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          _status=0;
          activeTime=micros();
          setShrine();  
        }
      }
      else
      {
        _status=0;
        activeTime=micros();
        setShrine();
      }
      break;

      default:
        scan=scanForCard();
        if(scan==Lesath||scan==Neutral||scan==Dunkel)
        {
          newKeeper = scan;
          setShrine();
        }
        else if(scan==Demo)
        {
          mode=demo;
        }
        else if(scan==Diagnostic)
        {
          mode=diagnostic;
        }
        else if(scan!=Unbekannt)
        {
          _status++;
          failCount=0;
          activeTime=micros();
          setShrine();
        }
      
      return;

    }
    break;

    //demomodus: alle zeiten geteilt durch 10
    case(demo):

    break;

    case(diagnostic):
    pulse();
    if(scanForCard()==diagnostic)
    {
      _status=(_status+1)%5;
      newKeeper=random()%15;
      setShrine();
      delay(3000);      
    }

    break;
  }
}
  

int scanForCard()
{
   if (!mfrc522.PICC_IsNewCardPresent())
    return Unbekannt;                                                            //Unbekannt bedeutet das keine karte gefunden wurde = 16
   else
   {
    String rfidUid = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      rfidUid += String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
      rfidUid += String(mfrc522.uid.uidByte[i], HEX);
    }

    if(rfidUid == "000704343d")
      return Lesath;
    if(rfidUid == "0002033c00")
      return Antike;
    if(rfidUid == "000704361a")
      return Elben;
    if(rfidUid == "0007050246")
      return Komet;
    if(rfidUid == "00070b1929")
      return HDC;
    if(rfidUid == "000661214d")
      return Imperium;
    if(rfidUid == "0007050455")
      return Krone;
    if(rfidUid == "000704343d")
      return Neutral;
    if(rfidUid == "0007205d05")
      return Licht;
    if(rfidUid == "0006613b3d")
      return Norrelag;
    if(rfidUid == "0007092c5a")
      return OHL;
    if(rfidUid == "0006540c50")
      return Pilger;
    if(rfidUid == "0002034400")
      return Zusammenkunft;
    if(rfidUid == "0006531113")
      return Stadt;
    if(rfidUid == "0007050456")
      return Dunkel;
    if(rfidUid == "0006542540")
      return Demo;
    if(rfidUid == "0006533649")
      return Diagnostic;
      
  }
}

void setServo()
{
  myServo.write(7.5+15*newKeeper);
  delay(3000);
}

void setShrine()
{

  switch(_status)
  {
    case(activating):
    //RGB auf Blau
    digitalWrite(rgb_red_PIN1, LOW);
    digitalWrite(rgb_red_PIN2, LOW);
    digitalWrite(rgb_green_PIN1, LOW);
    digitalWrite(rgb_green_PIN2, LOW);
    digitalWrite(rgb_blue_PIN1, HIGH);
    digitalWrite(rgb_blue_PIN2, HIGH);
    break;
    
    case(loading):
    //RGB auf Lila
    digitalWrite(rgb_red_PIN1, HIGH);
    digitalWrite(rgb_red_PIN2, HIGH);
    digitalWrite(rgb_green_PIN1, LOW);
    digitalWrite(rgb_green_PIN2, LOW);
    digitalWrite(rgb_blue_PIN1, HIGH);
    digitalWrite(rgb_blue_PIN2, HIGH);
    
    //servo leds an
    digitalWrite(servoLED1_PIN, HIGH);    
    digitalWrite(servoLED2_PIN, LOW);    
    break;

    case(active):
    //RGB auf weiss
    digitalWrite(rgb_red_PIN1, HIGH);
    digitalWrite(rgb_red_PIN2, HIGH);
    digitalWrite(rgb_green_PIN1, HIGH);
    digitalWrite(rgb_green_PIN2, HIGH);
    digitalWrite(rgb_blue_PIN1, HIGH);
    digitalWrite(rgb_blue_PIN2, HIGH);
    break;

    case(blocked):
    //servo auf Neues Lager setzen
    myServo.attach(servo_PIN);
    setServo();
    myServo.detach();
        
    //RGB auf Rot setzen
    digitalWrite(rgb_red_PIN1, HIGH);
    digitalWrite(rgb_red_PIN2, HIGH);
    digitalWrite(rgb_green_PIN1, LOW);
    digitalWrite(rgb_green_PIN2, LOW);
    digitalWrite(rgb_blue_PIN1, LOW);
    digitalWrite(rgb_blue_PIN2, LOW);

    //Servo-LEDs an, SL-LEDs aus
    digitalWrite(servoLED1_PIN, HIGH);
    digitalWrite(servoLED2_PIN, LOW);  
    break;

    default:
  
    //RGB auf Grün setzen
    digitalWrite(rgb_green_PIN1, HIGH);
    digitalWrite(rgb_green_PIN2, HIGH);
    digitalWrite(rgb_red_PIN1, LOW);
    digitalWrite(rgb_red_PIN2, LOW);
    digitalWrite(rgb_blue_PIN1, LOW);
    digitalWrite(rgb_blue_PIN2, LOW);
    
    if(newKeeper==Lesath||newKeeper==Neutral||newKeeper==Dunkel)
    {
      digitalWrite(servoLED1_PIN, LOW);
      digitalWrite(servoLED2_PIN, HIGH);
      //Servo auf SL Lager setzen
      myServo.attach(servo_PIN);
      switch(newKeeper)
      {
        case(Lesath):
        myServo.write(30);
        break;

        case(Neutral):
        myServo.write(90);
        break;

        case(Dunkel):
        myServo.write(150);
        break;        
      }
      delay(3000);
      myServo.detach();
    }
   return;     
  }
  return;
}

void pulse()
{

  switch(_status)
  {
    case(loading):
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;

    break;

    case(active):
      myServo.write(servoValue);
      delay(5);
      servoValue+=servoDirection;
      if(servoValue%180==0)
        servoDirection*=-1;
  return;
}
}

