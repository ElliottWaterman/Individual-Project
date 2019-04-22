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
  Serial.begin(9600);               //Start serial communication

  // Initialise the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  /* METHOD 1*/
  // Serial.println("Method 1: ");
  // // Set date and time of the RTC module
  // tmElements_t tm;
  // tm.Hour = 12;
  // tm.Minute = 00;
  // tm.Second = 00;
  // tm.Day = 14;
  // tm.Month = 1;
  // tm.Year = 2019 - 1970;  //tmElements_t.Year is offset from 1970

  // RTC.write(tm);          //Set the RTC from the tm structure


  /* METHOD 2 */
  Serial.println("Method 2: ");
  time_t t = processSyncMessage();
  if (t != 0) {
    // Set the RTC and the system time to the received value
    RTC.set(t);
    setTime(t);
  }

  setSyncProvider(RTC.get);   // the function to get the time from the RTC
  if (timeStatus() != timeSet) 
     Serial.println("Unable to sync with the RTC");
  else
     Serial.println("RTC has set the system time");
}

void loop() {
  Serial.println("Loop: ");

  // Using time_t structure
  time_t checkTimeT;
  checkTimeT = RTC.get();         //Get the current time of the RTC
  printDateTime(checkTimeT);

  // Using tmElements_t structure
  tmElements_t checkTimeTmElement;
  RTC.read(checkTimeTmElement);   //Read the current time of the RTC
  printDateTimeTmElement(checkTimeTmElement);

  delay(1000);
}

/*  code to process time sync messages from the serial port   */
#define TIME_HEADER  "T"   // Header tag for serial time sync message

unsigned long processSyncMessage() {
  unsigned long pctime = 0L;
  const unsigned long DEFAULT_TIME = 1549666800; // Fri, 08 Feb 2019 23:00:00 GMT

  if(Serial.find(TIME_HEADER)) {
    pctime = Serial.parseInt();
    return pctime;

    if (pctime < DEFAULT_TIME) { // check the value is a valid time (greater than Jan 1 2013)
      pctime = 0L; // return 0 to indicate that the time is not valid
    }
  }
  return pctime;
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
