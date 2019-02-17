/**
   Author: Elliott Waterman
   Date Created: 11/01/2019
   Date Last Modified: 17/02/2019
   Description: Complete program to run the arduino in the
   Smart Boa Snake Basking Station.
*/

/* INCLUDES */
#include <avr/sleep.h>      //AVR library contains methods and sleep modes to control the sleep of an Arduino
#include <Streaming.h>      //Used to make print statements easier to write

#include <SoftwareSerial.h> //Library to communicate with RFID reader (TODO: create class?)
#include <DS3232RTC.h>      //RTC library used to control the RTC module
#include <SimpleDHT.h>      //DHT22 library used to read from a DHT22 module
#include <Q2HX711.h>        //HX711 library used to read weight sensor (amplifier + load cell)
#include <RFID_Priority1Design.h> //HX711 library used to read weight sensor (amplifier + load cell)


/* DEFINES */
#define PIN_WAKE_UP       2   //Interrupt pin (or #3) to wake up the Arduino
#define PIN_DHT22         3   //Digital pin connected to the DHT22 module
#define PIN_RFID_POWER    4   //Digital pin connected BJT/MOSFET switch to power module
#define PIN_SIM900_RX     6   //Receiving pin for the SIM900 module
#define PIN_SIM900_TX     7   //Transmitting pin for the SIM900 module
#define PIN_RFID_RX       8   //Receiving pin on Arduino (use tx wire on board)
#define PIN_RFID_TX       9   //Transmitting pin on Arduino (use rx wire on board)
#define PIN_SD            10  //Digital pin connected to the SD module

#define PIN_HX711_DATA    A2  //Weight sensor data pin
#define PIN_HX711_CLOCK   A3  //Weight sensor clock pin
#define PIN_RTC_SDA       A4  //Connect RTC data to Arduino pin A4
#define PIN_RTC_SCL       A5  //Connect RTC clock to Arduino pin A5

//#define EXAMPLE           1   //Comment


/* Constants */
// RTC
const byte RTC_ALARM_TIME_INTERVAL = 5;   //Sets the wakeup intervall in minutes
const int RTC_TIME_READ_INTERVAL = 1500;

// RFID
const byte RFID_TAG_BUFFER_SIZE = 48;     // Maximum RFID tag size is 33
const byte INITIAL_RFID_POWER_ON_SECONDS = 20;

// Weight sensor
const int HX711_TIME_READ_INTERVAL = 1;       //Interval in seconds to read sensor

// Temperature
const int DHT22_TIME_READ_INTERVAL = 2000;


/* INSTANTIATE LIBRARIES */
SoftwareSerial SIM900(PIN_SIM900_RX, PIN_SIM900_TX);  //Controls the SIM900 module

RFID_P1D RFID(PIN_RFID_RX, PIN_RFID_TX, PIN_RFID_POWER);              //Controls the RFID module

SimpleDHT22 DHT22(PIN_DHT22);                         //Controls the DHT22 module

Q2HX711 HX711(PIN_HX711_DATA, PIN_HX711_CLOCK);       //Controls the HX711 module
//File storageFile;               //Controls writing to the SD card


/* VARIABLES */
// Temperature
float DHT22Temperature;
float DHT22Humidity;
long DHT22TimeRead;

// Weight sensor
time_t HX711TimeRead;
double HX711Weight = 0;
double HX711WeightPrevious = 0;

// RTC
long RTCTimeRead;

// RFID variables
unsigned long RFIDPowerOnTime = 0;
byte RFIDPowerOnSeconds = INITIAL_RFID_POWER_ON_SECONDS;  // Seconds powered on not over 255 (4 mins 15 secs)
static char RFIDTag[RFID_TAG_BUFFER_SIZE];
static byte RFIDTagIndex;
String RFIDTagString;

// DEBUG
bool DEBUGOnce = true;
unsigned long DEBUGStartTime;

/* SETUP */
void setup() {
  // Serial communications
  Serial.begin(9600);
  Serial.println("Setup Begin");

  // Start software serial communication with SIM and RFID modules
  SIM900.begin(9600);

  // Initialise weight sensor
  Serial.println("Initialising weight sensor");
  HX711.startSensorSetup();
  // Read from sensor until averaging array fills up
  while (!HX711.isSetupComplete()) {
    HX711.read();
  }

  // Initialise RFID module, start powered off
  Serial.println("Powering down RFID module");
  RFID.powerDown();

  // Initialise RTC alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  Serial.println("Initialising RTC alarms");
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  // Initialise Arduino time from RTC
  setSyncProvider(RTC.get); // Sync to a function that gets the time from the RTC
  //setSyncInterval(CONST); // Set the number of seconds between re-syncs (5 minutes default)
  if (timeStatus() == timeSet)
    Serial.println("RTC has set the system time");
  else if (timeStatus() == timeNeedsSync)
    Serial.println("Unable to sync with the RTC");
  else if (timeStatus() == timeNotSet)
    Serial.println("RTC has set the system time");
  else
    Serial.println("Unable to sync with the RTC");

  // DEBUG
  pinMode(LED_BUILTIN, OUTPUT);       //The built-in LED on pin 13 indicates when the Arduino is asleep
  pinMode(PIN_WAKE_UP, INPUT_PULLUP); //Set pin as an input which uses the built-in pullup resistor
  //digitalWrite(LED_BUILTIN, HIGH);    //Turn built-in LED on

  DEBUGStartTime = millis();

  Serial.println("Setup Complete!");
}

/* LOOP */
void loop() {
  // Get time for this loop
  //unsigned long currentMillis = millis();
  //time_t currentEpoch = RTC.get();

  // Check HX711 weight sensor for change in weight
  HX711.update();

  // If a weight has been detected turn on RFID module
  if (HX711.getWeightDetected()) {
    // Turn on RFID module, only executes if module is OFF
    RFID.powerUp();

    // Reset weight detection
    HX711.resetWeightDetected();
  }

  // Read tags or turn off module if elapsed time on, only runs if module is ON
  RFID.update();

  // Check if tag has been read
  if (RFID.hasTagBeenRead()) {

    // Collect temperature data
    dht22ReadFromSensor();

    // Reset RFID tag read
    RFID.resetTagRead();
  }

  // Read current temperature
  // dht22ReadFromSensor();
  // Serial.print(DHT22Temperature);
  // Serial.print(" C  and  ");
  // Serial.print(DHT22Humidity);
  // Serial.println(" %RH");

  // Get any incoming SIM900 data
  //readSIM900();

  //Going_To_Sleep();

  // PUESDO-code
  // Check RTC alarm to sleep module for night time
    // Check all power states / if snake is still present
      // Set alarm for 1 hour later
    // Else 
      // Set alarm interrupt for morning
      // Turn off other modules if necessary
      // Sleep Arduino
}

struct Message {
  time_t epochTime;
  String rfidTag;
  float temperature;
  float weight; // highest and lowest?
};


/* FUNCTIONS */
template <class T> void DEBUGSecondDisplay(const T toDisplay) {
  if (millis() - DEBUGStartTime > 1000) {
    Serial.println(toDisplay);
    DEBUGStartTime = millis();
  }
}

/**
 * Function to listen and read in an RFID tag from software serial
 */
boolean readRFIDTag() {
  // Listen to serial port for RFID communication
  RFID.listen();
  while (RFID.available() && RFIDTagIndex < (RFID_TAG_BUFFER_SIZE - 1)) {
    // Read incoming character/byte
    char inByte = RFID.read();
    RFIDTag[RFIDTagIndex++] = inByte;

    // Entire message has been received
    if (inByte == '\r') {               // '\r' means "end of message"
      RFIDTag[--RFIDTagIndex] = '\0';   // Terminate the RFIDTag replacing '\r'
      RFIDTagString = String(RFIDTag);  // Set string tag to received tag

      // Message has been dealt with, reset RFIDTag position
      RFIDTagIndex = 0;

      return true;
    }
  }
  return false;
}

/**
 * Function TODO
 */
void readSIM900() {
  SIM900.listen();
  char response[18];
  int simIndex = 0;
  while (SIM900.available() > 0) {
    char inByte = SIM900.read();
    if (inByte != '\r' && simIndex < 17) {
      response[simIndex++] = inByte;
    }
    else {
      // Terminate buffer
      response[simIndex++] = '\0';

      Serial.println(response);

      if (response == "OK") {
        // do something
      }
      else if (response == "ERR") {
        // do something else
      }
    }
  }
}

/**
   Function to read the temperature and humidity of the DHT22 sensor.
*/
void dht22ReadFromSensor() {
  //Read from the DHT22 sensor and assign values into variables, NULL is meant for raw data array.
  int function_success = DHT22.read2(&DHT22Temperature, &DHT22Humidity, NULL);
  //Check return value is NOT equal to constant for success
  if (function_success != SimpleDHTErrSuccess) {
    Serial.print("Read DHT22 failed, err=");
    Serial.println(function_success);
    //delay(2000);
    return;
  }
  //TODO: Could return success value?
}

/**
 * Function TODO
 */
void printDateTime(time_t t)
{
  Serial << ((day(t) < 10) ? "0" : "") << _DEC(day(t));
  Serial << monthShortStr(month(t));
  Serial << _DEC(year(t)) << ' ';
  Serial << ((hour(t) < 10) ? "0" : "") << _DEC(hour(t)) << ':';
  Serial << ((minute(t) < 10) ? "0" : "") << _DEC(minute(t)) << ':';
  Serial << ((second(t) < 10) ? "0" : "") << _DEC(second(t)) << endl;
}

/**
 * Function TODO
 */
void setRTCMinutesAlarm() {
    // Set alarm on RTC for RTC_ALARM_TIME_INTERVAL + t
  time_t t;
  t = RTC.get();    //Gets the current time of the RTC
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + RTC_ALARM_TIME_INTERVAL, 0, 0);  // Setting alarm 1 to go off 5 minutes from now
  //Clear the alarm flag
  RTC.alarm(ALARM_1);
  //Configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.squareWave(SQWAVE_NONE);
  //Enable interrupt output for Alarm 1
  RTC.alarmInterrupt(ALARM_1, true);
}

/**
 * Function TODO
 */
void Going_To_Sleep() {
  sleep_enable();                       //Enabling sleep mode
  //Pin to detect change, method to call, the change to detect
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
  //temp_Humi();                          //function that reads the temp and the humidity
  //Set New Alarm
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + RTC_ALARM_TIME_INTERVAL, 0, 0);

  // clear the alarm flag
  RTC.alarm(ALARM_1);
}

/**
 * Function TODO
 */
void wakeUp() {
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
}

/**
   the writeData function gets the humidity (h), temperature in celsius (t) and farenheit (f) as input
   parameters. It uses the RTC to create a filename using the current date, and writes the temperature
   and humidity with a date stamp to this file
*/
//void writeData(float h, float t, float f) {
//  time_t p; //create time object for time and date stamp
//  p = RTC.get(); //gets the time from RTC
//  String file_Name = String(day(p)) + monthShortStr(month(p)) + String(year(p)) + ".txt"; //creates the file name we are writing to.
//  storageFile = SD.open(file_Name, FILE_WRITE);// creates the file object for writing
//
//  // if the file opened okay, write to it:
//  if (storageFile) {
//    Serial.print("Writing to " + file_Name);
//    //appends a line to the file with time stamp and humidity and temperature data
//    storageFile.println(String(hour(p)) + ":" + String(minute(p)) + " Hum: " + String(h) + "% C: " + String(t) + " F: " + String(f));
//    // close the file:
//    storageFile.close();
//    Serial.println("done.");
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening " + file_Name);
//  }
//  //opening file for reading and writing content to serial monitor
//  storageFile = SD.open(file_Name);
//  if (storageFile) {
//    Serial.println(file_Name);
//
//    // read from the file until there's nothing else in it:
//    while (storageFile.available()) {
//      Serial.write(storageFile.read());
//    }
//    // close the file:
//    storageFile.close();
//  } else {
//    // if the file didn't open, print an error:
//    Serial.println("error opening" + file_Name);
//  }
//}
