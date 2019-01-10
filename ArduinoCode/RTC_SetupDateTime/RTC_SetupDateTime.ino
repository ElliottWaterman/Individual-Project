/**
   Author: Elliott Waterman
   Date: 09/01/2019
   Description: Program to setup the date and time
   of an RTC module.
   Hardware:
   Arduino Uno, DS3231 RTC.
   Connect RTC SDA to Arduino pin A4.
   Connect RTC SCL to Arduino pin A5.
*/

/* INCLUDES */
#include <DS3232RTC.h>    //RTC library used to control the RTC module
#include <Streaming.h>    //Used to make print statements easier to write

/* DEFINES */

/* INSTANTIATE LIBRARIES */

/* VARIABLES */

void setup() {
  Serial.begin(115200);               //Start serial communication

  // Initialise the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  // Set date and time of the RTC module
  tmElements_t tm;
  tm.Hour = 12;
  tm.Minute = 00;
  tm.Second = 00;
  tm.Day = 14;
  tm.Month = 1;
  tm.Year = 2019 - 1970;  //tmElements_t.Year is offset from 1970
  RTC.write(tm);          //Set the RTC from the tm structure

  Serial.println("Setup:");

  // Using time_t structure
  time_t checkTimeT;
  checkTimeT = RTC.get(); //Get the current time of the RTC
  printDateTime(checkTimeT);

  // Using tmElements_t structure
  tmElements_t checkTimeTmElement;
  RTC.read(checkTimeTmElement);
  printDateTimeTmElement(checkTimeTmElement);
}

void loop() {
  Serial.println("Loop:");

  // Using time_t structure
  time_t checkTimeT;
  checkTimeT = RTC.get();         //Get the current time of the RTC
  printDateTime(checkTimeT);

  // Using tmElements_t structure
  tmElements_t checkTimeTmElement;
  RTC.read(checkTimeTmElement);   //Read the current time of the RTC
  printDateTimeTmElement(checkTimeTmElement);
}

void printDateTime(time_t t)
{
  Serial << ((day(t) < 10) ? "0" : "") << _DEC(day(t));
  Serial << monthShortStr(month(t)) << _DEC(year(t)) << ' ';
  Serial << ((hour(t) < 10) ? "0" : "") << _DEC(hour(t)) << ':';
  Serial << ((minute(t) < 10) ? "0" : "") << _DEC(minute(t)) << ':';
  Serial << ((second(t) < 10) ? "0" : "") << _DEC(second(t));
}

void printDateTimeTmElement(tmElements_t t)
{
  Serial.print(t.Day, DEC);
  Serial.print(' ');
  Serial.print(t.Month, DEC);
  Serial.print(' ');
  Serial.print((t.Year + 1970), DEC);
  Serial.print(t.Hour, DEC);
  Serial.print(':');
  Serial.print(t.Minute, DEC);
  Serial.print(':');
  Serial.println(t.Second, DEC);
}
