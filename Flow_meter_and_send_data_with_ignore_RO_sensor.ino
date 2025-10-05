
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <EasyTransfer.h>

#include "SSD1306Ascii.h"
#include "SSD1306AsciiAvrI2c.h"
SSD1306AsciiAvrI2c oled;
#define I2C_ADDRESS 0x3C  // Yes this is correct regardless of what the screen says

//LiquidCrystal_I2C lcd(0x27,20,4);

byte statusLed    = 13;
const int pingPin = 7;

byte sensorInterrupt = 0;  // 0 = digital pin 2
byte sensorPin       = 2;

float calibrationFactor = 18.1; // 5.1 = 13260 ml per gal fill, 17.85 = 3520, 14.6 was getting about 1.24 gal read per gal pour in current form
//float calibrationFactor = 4.5;

volatile byte pulseCount;  

float flowRate;
unsigned int flowMilliLitres;
unsigned long totalMilliLitres;
unsigned long totalGallons;

unsigned long oldTime;

int displayTime;
int roTime;
int roLevel;

EasyTransfer ET;

struct SEND_DATA_STRUCTURE{ // Experimental sending code
  unsigned long totalMLData;
  unsigned long flowData;
  int roData;
};

SEND_DATA_STRUCTURE mydata;

void setup(){

  Serial.begin(9600);
  ET.begin(details(mydata), &Serial);

  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(System5x7);
  oled.clear();
   
  // Set up the status LED line as an output
  pinMode(statusLed, OUTPUT);
  digitalWrite(statusLed, HIGH);  // We have an active-low LED attached
  
  pinMode(sensorPin, INPUT);
  digitalWrite(sensorPin, HIGH);

  pulseCount        = 0;
  flowRate          = 0.0;
  flowMilliLitres   = 0;
  totalMilliLitres  = 0;
  oldTime           = 0;
  totalGallons      = 0;
  roLevel = 1; //Ignoring RO sensor
  
  displayTime = 10; 
  roTime = 1;

  attachInterrupt(sensorInterrupt, pulseCounter, FALLING);

  //roPing();
  delay (1000);
}

void loop()
{
   
   if((millis() - oldTime) > 100)    // Only process counters once per tenth of second   
  { 

    detachInterrupt(sensorInterrupt);
        
    flowRate = ((1000.0 / (millis() - oldTime)) * pulseCount) / calibrationFactor;
    
    oldTime = millis();
    
    flowMilliLitres = (flowRate / 600) * 1000; // divide by 600 for 1/10 second
    
    totalMilliLitres += flowMilliLitres;
    //totalGallons = totalMilliLitres / 3785;

    unsigned int frac;

    pulseCount = 0;

    /*if(roTime >= 300){
      roPing();
      roTime = 1;
    }*/
    
    attachInterrupt(sensorInterrupt, pulseCounter, FALLING);
  }

    if(displayTime > 100){ // only update diplay every 1 seconds (10 x 1/10th seconds)

      oled.clear();
      oled.setCursor(0,3);
      oled.print("Flow Rate (L/min):");
      oled.setCursor(0,4);
      oled.print(flowRate); 

      oled.setCursor(0,6);
      oled.print("Total Output (mL):");
      oled.setCursor(0,7);
      oled.print(totalMilliLitres); 

      oled.setCursor(0,0);
      oled.print("RO Tank Level:");
      oled.setCursor(0,1);
      /*char buff[5];
      sprintf(buff, "%02d%%", roLevel);
      oled.print(buff);

      roTime ++;*/
      displayTime = 1;
    }
    
    displayTime ++;

    mydata.totalMLData = totalMilliLitres;
    mydata.flowData = flowRate;
    mydata.roData = roLevel;
    ET.sendData();   
}

void pulseCounter()
{
  pulseCount++;
}

/*void roPing(){

  long duration, inches, cm;

  pinMode(pingPin, OUTPUT);
  digitalWrite(pingPin, LOW);
  delayMicroseconds(2);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(pingPin, LOW);

  pinMode(pingPin, INPUT);
  duration = pulseIn(pingPin, HIGH);

  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);

  roLevel = map(cm, 10, 80, 100, 0);
}

long microsecondsToInches(long microseconds) {

  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds) {

  return microseconds / 29 / 2;
}*/