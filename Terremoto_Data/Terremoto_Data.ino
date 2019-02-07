/* 
 Terremoto Datalogger - 3 feb 2019

 DS1302_Serial_Easy 
 Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
 web: http://www.RinkyDinkElectronics.com/

 A quick demo of how to use my DS1302-library to 
 quickly send time and date information over a serial link

 I assume you know how to connect the DS1302.
 DS1302:  CE pin    -> Arduino Digital 10
          I/O pin   -> Arduino Digital 9
          SCLK pin  -> Arduino Digital 8
 
 SD CARD attached to SPI as follows
 MOSI pin 11
 MISO pin 12
 CLK  pin 13
 CS   pin 7

 Accelerometer attached to
 X axis pin A1
 Y axis pin A3
 Z axis pin A2
*/

/******************************************************************************
TMP102_example.ino
Example for the TMP102 I2C Temperature Sensor
Alex Wende @ SparkFun Electronics
April 29th 2016
~

This sketch configures the TMP102 temperature sensor and prints the
temperature and alert state (both from the physical pin, as well as by
reading from the configuration register.

Resources:
Wire.h (included with Arduino IDE)
SparkFunTMP102.h

Development environment specifics:
Arduino 1.0+
Hardware Version 13

This code is beerware; if you see me (or any other SparkFun employee) at
the local, and you've found our code helpful, please buy us a round!

Distributed as-is; no warranty is given.

Connections
VCC = 3.3V
GND = GND
SDA = A4
SCL = A5
******************************************************************************/

#include <DS1302.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "SparkFunTMP102.h"

//DS1302 init
const int SCLKRTC = 8;
const int IORTC = 9;
const int CERTC = 10;
DS1302 rtc(CERTC, IORTC, SCLKRTC);

//Accelerometer
const int accx= A1;
const int accy= A3;
const int accz= A2;
const int alertLimit = 30;
const int shakesLimit = 100; //maximum shakes in an interval
int shakesNum = 0;
int readX = 0;
int readY = 0;
int readZ = 0;
int calibX;
int calibY;
int calibZ;


//SD Card
const int chipSelect = 7;
File dataFile;


//Photoresistor
const int photo = A0;
const int photo_limit = 150;
const int photo_pin = 6;

//TMP102 thermo
TMP102 sensorTMP(0x48);
//const int ALERT_PIN = 9;

unsigned long previousMillis = 0;
const unsigned long delayMillis = 10000;

int calibrateAccel(int pin){
  long average = 0;
  for(int x = 0; x<100; x++){
    average += analogRead(pin);
    delay(5);
  }
  average = average/100;
  Serial.println(average);
  return average;
}

String queryData(){
    //RTC
    String dataString ="";
    dataString += rtc.getDateStr();
    dataString += ",";
    dataString += rtc.getTimeStr();
    dataString += ",";
    return dataString;
}

void setup()
{
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  
  // Setup Serial connection
  Serial.begin(9600);
    // Test if SD card is present
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed or not present");
    while(true){}
  }
  //ask for a new date on startup
  Serial.print("Current date:");
  Serial.println(queryData());
  Serial.println("Enter new date (format DD MM YYYY hh mm)");
  bool set = true;
  while(!Serial.available() && set){
    if (millis()>10000){
      set = false;
      Serial.println("Timeout");
    }
  }
  if(set) {
    int rtcDay = Serial.parseInt();
    int rtcMonth = Serial.parseInt();
    int rtcYear = Serial.parseInt();
    int rtcHour = Serial.parseInt();
    int rtcMinute = Serial.parseInt();
    rtc.setTime(rtcHour, rtcMinute, 0);
    rtc.setDate(rtcDay, rtcMonth, rtcYear);
  }
  Serial.print("Date is now:");
  Serial.println(queryData());
  
  //setup accelerometer
  Serial.println("Calibrating accelerometer...");
  pinMode(accx,INPUT);
  pinMode(accy,INPUT);
  pinMode(accz,INPUT);
  calibX = calibrateAccel(accx);
  delay(100);
  calibY = calibrateAccel(accy);
  delay(100);
  calibZ = calibrateAccel(accz);
  delay(100);
  Serial.println("Accelerometer calibrated.");
  
  //start TMP120
  sensorTMP.begin();
  //pinMode(ALERT_PIN,INPUT);
  sensorTMP.setFault(0);
  sensorTMP.setAlertPolarity(1);
  sensorTMP.setAlertMode(0);
  sensorTMP.setConversionRate(2);
  sensorTMP.setExtendedMode(0);
  sensorTMP.setHighTempC(50);
  sensorTMP.setLowTempC(-50);

  //set photoresistor pin and output pin

  pinMode(photo,INPUT);
  pinMode(photo_pin,OUTPUT);
  

  Serial.println("OK");
}





void loop()
{

  //accelerometer runs only if the acceleration is over the limit

  //Accelerometer
  readX = analogRead(accx);
  readY = analogRead(accy);
  readZ = analogRead(accz);
  if (abs(readX-calibX) > alertLimit || abs(readY-calibY) > alertLimit || abs(readZ-calibZ) > alertLimit){
    shakesNum +=1;
    if(shakesNum > shakesLimit){
      Serial.println("Too many shakes, recalibrating...");
      calibX = calibrateAccel(accx);
      delay(100);
      calibY = calibrateAccel(accy);
      delay(100);
      calibZ = calibrateAccel(accz);
      delay(100);
      Serial.println("Accelerometer calibrated.");
    }
    dataFile = SD.open("accel.csv", FILE_WRITE); 
    if (dataFile){
      String accelString ="";
      accelString += String(analogRead(readX-calibX));
      accelString += ",";
      accelString += String(analogRead(readY-calibY));
      accelString += ",";
      accelString += String(analogRead(readZ-calibZ));
      accelString = queryData() + accelString;
      dataFile.println(accelString);
      delay(5);
      Serial.println("Shake detected!");
      dataFile.close();
    } else {
      Serial.println("error opening accel.csv");
      dataFile.close();
    }
    
  }

  //other sensors are queried on fixed interval
  if (millis()-previousMillis > delayMillis) {
    previousMillis += delayMillis;
    shakesNum = 0;
    //TMP102
    float temperature;
    //bool alertPinState, alertRegisterState;
    sensorTMP.wakeup();
    temperature = sensorTMP.readTempC();
    sensorTMP.sleep();
    dataFile = SD.open("temp.csv", FILE_WRITE);
    if(dataFile) {
      String tempString = queryData() + String(temperature);
      dataFile.println(tempString);
      dataFile.close();    
    } else {
      Serial.println("error opening temp.csv");
    }
  
    //Photoresistor
    int photo_read = analogRead(photo);
    if(photo_read > photo_limit) {
      digitalWrite(photo_pin, LOW);
    } else {
      digitalWrite(photo_pin, HIGH);
    }
  }
  
}
