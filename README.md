# Terremoto_Datalogger
Datalogger for seismograph (3 axis accelerometer).

This code records the output of a sensor (a 3 axis accelerometer, plugged into the analogic pins) and writes it on a SD Card. An adapter is needed but it's possible to plug an SD card directly on the Arduino using voltage splitters, from 5v to 3.3v, for the Arduino to SD Card wires.

A DS1302 Real Time Clock is needed in order to log the data with a timestamp.

For the DS1302 RTC the library from Rinky-Dink Electronic is needed:
http://www.rinkydinkelectronics.com/library.php?id=5

For the SD Card, the standard Arduino library provided with the IDE is needed.

