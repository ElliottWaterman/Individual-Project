#include <DS3232RTC.h>        // https://github.com/JChristensen/DS3232RTC
#include <Streaming.h>        // http://arduiniana.org/libraries/streaming/

// pin definitions
const uint8_t alarmInput(2);

void setup()
{
    Serial.begin(115200);
    pinMode(2, INPUT_PULLUP);

    // initialize the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
    RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
    RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
    RTC.alarm(ALARM_1);
    RTC.alarm(ALARM_2);
    RTC.alarmInterrupt(ALARM_1, false);
    RTC.alarmInterrupt(ALARM_2, false);
    RTC.squareWave(SQWAVE_NONE);

    setSyncProvider(RTC.get);
    
    // set Alarm 2 for every minute
    RTC.setAlarm(ALM2_EVERY_MINUTE, 0, 0, 0, 0);
    // clear the alarm flags
    RTC.alarm(ALARM_1);
    RTC.alarm(ALARM_2);
    // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
    RTC.squareWave(SQWAVE_NONE);
    // enable interrupt output for Alarm 2 only
    RTC.alarmInterrupt(ALARM_1, false);
    RTC.alarmInterrupt(ALARM_2, true);
}

void loop()
{
    // check to see if the INT/SQW pin is low, i.e. an alarm has occurred
    if ( !digitalRead(alarmInput) )
    {
        RTC.alarm(ALARM_2);    // reset the alarm flag
        Serial << "ALARM_2\n";
    }

    // print the time when it changes
    static time_t tLast;
    time_t t = RTC.get();
    if (t != tLast)
    {
      unsigned long currentMillis = millis();
      Serial << currentMillis << endl;
      digitalClockDisplay();
      
      Serial << t << endl;
      printDateTime(t);
      Serial << endl << endl;

      tLast = t;
    }
}

void printDateTime(time_t t)
{
    Serial << ((day(t)<10) ? "0" : "") << _DEC(day(t));
    Serial << monthShortStr(month(t)) << _DEC(year(t)) << ' ';
    Serial << ((hour(t)<10) ? "0" : "") << _DEC(hour(t)) << ':';
    Serial << ((minute(t)<10) ? "0" : "") << _DEC(minute(t)) << ':';
    Serial << ((second(t)<10) ? "0" : "") << _DEC(second(t));
}

void digitalClockDisplay()
{
    // digital clock display of the time
    Serial.print(day());
    Serial.print(' ');
    Serial.print(month());
    Serial.print(' ');
    Serial.print(year());
    Serial.print(' ');
    Serial.print(hour());
    printDigits(minute());
    printDigits(second());
    Serial.println();
}

void printDigits(int digits)
{
    // utility function for digital clock display: prints preceding colon and leading 0
    Serial.print(':');
    if(digits < 10)
        Serial.print('0');
    Serial.print(digits);
}
