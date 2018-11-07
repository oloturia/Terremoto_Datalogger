// DS1302_Serial_Easy 
// Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/
//
// A quick demo of how to use my DS1302-library to 
// quickly send time and date information over a serial link
//
// I assume you know how to connect the DS1302.
// DS1302:  CE pin    -> Arduino Digital 10
//          I/O pin   -> Arduino Digital 9
//          SCLK pin  -> Arduino Digital 8
// 
// SD CARD attached to SPI as follows
// MOSI pin 11
// MISO pin 12
// CLK  pin 13
// CS   pin 4
//
// Accelerometer attached to
// X axis pin A4
// Y axis pin A3
// Z axis pin A2

#include <DS1302.h>
#include <SPI.h>
#include <SD.h>

//DS1302 init
const int SCLKRTC = 8;
const int IORTC = 9;
const int CERTC = 10;
DS1302 rtc(CERTC, IORTC, SCLKRTC);

//Accelerometer
const int accx= A4;
const int accy= A3;
const int accz= A2;

//SD Card
const int chipSelect = 4;

void setup()
{
  // Set the clock to run-mode, and disable the write protection
  rtc.halt(false);
  rtc.writeProtect(false);
  
  // Setup Serial connection
  Serial.begin(9600);

  // The following lines can be commented out to use the values already stored in the DS1302
  //rtc.setDOW(FRIDAY);        // Set Day-of-Week to FRIDAY
  //rtc.setTime(12, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(6, 8, 2010);   // Set the date to August 6th, 2010
  
  // Test if SD card is present
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed or not present");
    return;
  }
  Serial.println("OK");
}

void loop()
{
  //Compose the string as a row of a csv file
  String dataString ="";
  dataString += rtc.getDateStr();
  dataString += ",";
  dataString += rtc.getTimeStr();
  dataString += ",";
  dataString += String(analogRead(accx));
  dataString += ",";
  dataString += String(analogRead(accy));
  dataString += ",";
  dataString += String(analogRead(accz));
  //Write the string on SD Card and serial for debugging
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if(dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    Serial.println(dataString);
  } else {
    Serial.println("error opening datalog.csv");
  }
  delay (1);
}
