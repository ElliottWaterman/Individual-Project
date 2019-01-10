/**
   Author: Elliott Waterman
   Date: 08/01/2019
   Description: Program to use an alarm on the RTC module
   to wake a sleeping Arduino via an interrupt.
*/

/* INCLUDES */
#include <avr/sleep.h>  //AVR library contains methods and sleep modes to control the sleep of an Arduino
#include <SPI.h>        //SPI library used for the SD module. Part of the Arduino IDE
#include <SD.h>         //SD library used for the SD module. Part of the Arduino IDE

#include <DS3232RTC.h>  //RTC library used to control the RTC module
#include <SimpleDHT.h>  //DHT22 library used to read from a DHT22 module

/* DEFINES */
#define PIN_WAKE_UP   2   //Interrupt pin (or #3) we are going to use to wake up the Arduino
#define PIN_DHT22     3   //Digital pin connected to the DHT22 module
#define PIN_SD        10  //Digital pin connected to the SD module
#define TIME_INTERVAL 5   //Sets the wakeup intervall in minutes
//#define EXAMPLE     1   //Comment

/* INSTANTIATE LIBRARIES */
SimpleDHT22 DHT22(PIN_DHT22);   //Controls the DHT22 module
File storageFile;               //Controls writing to the SD card

/* VARIABLES */

void setup() {
  Serial.begin(115200);               //Start serial communication
  pinMode(LED_BUILTIN, OUTPUT);       //The built-in LED on pin 13 indicates when the Arduino is asleep
  pinMode(PIN_WAKE_UP, INPUT_PULLUP); //Set pin as an input which uses the built-in pullup resistor
  digitalWrite(LED_BUILTIN, HIGH);    //Turn built-in LED on

  // Initialise the alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  time_t t;         //create a temporary time variable so we can set the time and read the time from the RTC
  t = RTC.get();    //Gets the current time of the RTC
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + TIME_INTERVAL, 0, 0);  // Setting alarm 1 to go off 5 minutes from now
  // clear the alarm flag
  RTC.alarm(ALARM_1);
  // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.squareWave(SQWAVE_NONE);
  // enable interrupt output for Alarm 1
  RTC.alarmInterrupt(ALARM_1, true);

  dht.begin();//Start the DHT sensor

  /**Initializes the SD breakout board. If it is not ready or not connected correct it writes
     an error message to the serial monitor
   **/
  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void loop() {
  delay(5000);//wait 5 seconds before going to sleep. In real senairio keep this as small as posible
  Going_To_Sleep();
}

void Going_To_Sleep() {
  sleep_enable();                       //Enabling sleep mode
  attachInterrupt(0, wakeUp, LOW);      //attaching an interrupt to pin d2
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);  //Setting the sleep mode, in our case full sleep
  digitalWrite(LED_BUILTIN, LOW);       //turning LED off

  time_t t;                             // creates temp time variable
  t = RTC.get();                        //gets current time from rtc
  Serial.println("Sleep  Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t)));
  delay(1000);                          //wait a second to allow the led to be turned off before going to sleep
  sleep_cpu();                          //activating sleep mode

  Serial.println("just woke up!");      //next line of code executed after the interrupt
  digitalWrite(LED_BUILTIN, HIGH);      //turning LED on
  t = RTC.get();
  Serial.println("WakeUp Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t)));
  temp_Humi();                          //function that reads the temp and the humidity
  //Set New Alarm
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + TIME_INTERVAL, 0, 0);

  // clear the alarm flag
  RTC.alarm(ALARM_1);
}

void wakeUp() {
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
}

//This function reads the temperature and humidity from the DHT sensor
void temp_Humi() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();//reads humidity
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  writeData(h, t, f); //sends the data to the writeData function
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
}

/**
   the writeData function gets the humidity (h), temperature in celsius (t) and farenheit (f) as input
   parameters. It uses the RTC to create a filename using the current date, and writes the temperature
   and humidity with a date stamp to this file
*/
void writeData(float h, float t, float f) {
  time_t p; //create time object for time and date stamp
  p = RTC.get(); //gets the time from RTC
  String file_Name = String(day(p)) + monthShortStr(month(p)) + String(year(p)) + ".txt"; //creates the file name we are writing to.
  storageFile = SD.open(file_Name, FILE_WRITE);// creates the file object for writing

  // if the file opened okay, write to it:
  if (storageFile) {
    Serial.print("Writing to " + file_Name);
    //appends a line to the file with time stamp and humidity and temperature data
    storageFile.println(String(hour(p)) + ":" + String(minute(p)) + " Hum: " + String(h) + "% C: " + String(t) + " F: " + String(f));
    // close the file:
    storageFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening " + file_Name);
  }
  //opening file for reading and writing content to serial monitor
  storageFile = SD.open(file_Name);
  if (storageFile) {
    Serial.println(file_Name);

    // read from the file until there's nothing else in it:
    while (storageFile.available()) {
      Serial.write(storageFile.read());
    }
    // close the file:
    storageFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening" + file_Name);
  }
}
