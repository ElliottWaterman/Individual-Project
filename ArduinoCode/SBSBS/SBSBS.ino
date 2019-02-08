/**
   Author: Elliott Waterman
   Date: 11/01/2019
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


/* DEFINES */
#define PIN_WAKE_UP       2   //Interrupt pin (or #3) to wake up the Arduino
#define PIN_DHT22         3   //Digital pin connected to the DHT22 module
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
const byte RTC_ALARM_TIME_INTERVAL = 5;   //Sets the wakeup intervall in minutes

const byte RFID_TAG_BUFFER_SIZE = 128;

const int HX711_TIME_READ_INTERVAL = 500;
const int DHT22_TIME_READ_INTERVAL = 2000;
const int RTC_TIME_READ_INTERVAL = 1500;


/* INSTANTIATE LIBRARIES */
SoftwareSerial RFID(PIN_RFID_RX, PIN_RFID_TX);        //Controls the RFID module: Uses 9600 baud and carridge return
SoftwareSerial SIM900(PIN_SIM900_RX, PIN_SIM900_TX);  //Controls the SIM900 module
SimpleDHT22 DHT22(PIN_DHT22);                         //Controls the DHT22 module
Q2HX711 HX711(PIN_HX711_DATA, PIN_HX711_CLOCK);       //Controls the HX711 module
//File storageFile;               //Controls writing to the SD card


/* VARIABLES */
float DHT22Temperature;
float DHT22Humidity;

// Instead of using delay() read time and check above x amount
long HX711TimeRead;
long DHT22TimeRead;
long RTCTimeRead;

// RFID variables
static char RFIDTag[RFID_TAG_BUFFER_SIZE];
static byte RFIDTagIndex;
String RFIDTagString;

// HX711 weight sensor variables
double HX711Weight;

/* SETUP */
void setup() {
  // Serial communications
  Serial.begin(9600);
  Serial.println("Setup Begin");

  // Start software serial communication with SIM and RFID modules
  SIM900.begin(9600);
  RFID.begin(9600);
  RFID.listen();

  // Initialise weight sensor
  Serial.println("Initialising weight sensor");
  HX711.startSensorSetup();
  // Read from sensor until averaging array fills up
  while (!HX711.isSetupComplete()) {
    HX711.read();
  }

  // Initialise RTC alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  Serial.println("Initialising RTC alarms");
  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  // Initializes the SD breakout board. If it is not ready or not connected correct it writes an error 
  // message to the serial monitor
/*  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");*/


  // DEBUG
  pinMode(LED_BUILTIN, OUTPUT);       //The built-in LED on pin 13 indicates when the Arduino is asleep
  pinMode(PIN_WAKE_UP, INPUT_PULLUP); //Set pin as an input which uses the built-in pullup resistor
  digitalWrite(LED_BUILTIN, HIGH);    //Turn built-in LED on

  Serial.println("Setup Complete");
}

/* LOOP */
void loop() {

  // Get any incoming RFID data
  // Collect data from other sensors
  if (readRFIDTag()) {
    Serial.println("RFID Tag: " + RFIDTagString);

    // Get temperature from sensor
    if (millis() > DHT22TimeRead + DHT22_TIME_READ_INTERVAL) {
      dht22ReadFromSensor();
      Serial.print(DHT22Temperature);
      Serial.print(" C  and  ");
      Serial.print(DHT22Humidity);
      Serial.println(" %RH");
      DHT22TimeRead = millis();
    }

    // Check weight for snake entering basking station
    readWeight();
    Serial.print(HX711Weight);
    Serial.println(" grams");

    // Get current time from RTC module
    if (millis() > RTCTimeRead + RTC_TIME_READ_INTERVAL) {
      // Using time_t structure
      time_t checkTimeT;
      checkTimeT = RTC.get(); //Get the current time of the RTC
      printDateTime(checkTimeT);
      RTCTimeRead = millis();
    }
  }

  // Get any incoming SIM900 data
  //readSIM900();

  //Going_To_Sleep();
}

/* FUNCTIONS */
/**
 * Function to listen and read in an RFID tag from software serial
 */
boolean readRFIDTag() {
  // Set listening serial to RFID module
  RFID.listen();
  while (RFID.available() && RFIDTagIndex < (RFID_TAG_BUFFER_SIZE - 1)) {
    // Read incoming character/byte
    char inByte = RFID.read();
    RFIDTag[RFIDTagIndex++] = inByte;

    // Entire message has been received
    if (inByte == '\r') {               // '\r' means "end of message"
      RFIDTag[RFIDTagIndex] = '\0';     // Terminate the RFIDTag
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
 * Function TODO
 */
void readWeight() {
  if (!HX711.isSetupComplete()) {
    HX711.read(); // fill up averaging array
  }
  else if (millis() > HX711TimeRead + HX711_TIME_READ_INTERVAL) {
    if (HX711.readyToSend()) {
      HX711Weight = HX711.read();
      HX711TimeRead = millis();
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
