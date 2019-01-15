/**
   Author: Elliott Waterman
   Date: 11/01/2019
   Description: Complete program to run the arduino in the
   Smart Boa Snake Basking Station.
*/

/* INCLUDES */
#include <avr/sleep.h>  //AVR library contains methods and sleep modes to control the sleep of an Arduino

#include <SoftwareSerial.h> //Library to communicate with RFID reader (TODO: create class?)
#include <DS3232RTC.h>      //RTC library used to control the RTC module
#include <SimpleDHT.h>      //DHT22 library used to read from a DHT22 module
#include <Q2HX711.h>        //HX711 library used to read weight sensor (amplifier + load cell)

/* DEFINES */
#define PIN_WAKE_UP     2   //Interrupt pin (or #3) we are going to use to wake up the Arduino
#define PIN_DHT22       3   //Digital pin connected to the DHT22 module
#define PIN_SIM900_RX   6   //Receiving pin for the SIM900 module
#define PIN_SIM900_TX   7   //Transmitting pin for the SIM900 module
#define PIN_RFID_RX     8   //Receiving pin for the RFID module
#define PIN_RFID_TX     9   //Transmitting pin for the RFID module
#define PIN_SD          10  //Digital pin connected to the SD module
#define TIME_INTERVAL   5   //Sets the wakeup intervall in minutes
//#define EXAMPLE       1   //Comment

/* INSTANTIATE LIBRARIES */
SoftwareSerial RFID(PIN_RFID_RX, PIN_RFID_TX);        //Controls the RFID module
SoftwareSerial SIM900(PIN_SIM900_RX, PIN_SIM900_TX);  //Controls the SIM900 module
SimpleDHT22 DHT22(PIN_DHT22);   //Controls the DHT22 module
//File storageFile;               //Controls writing to the SD card

/* VARIABLES */
float dht22_temperature;
float dht22_humidity;
int dht22_error = SimpleDHTErrSuccess;

/* SETUP */
void setup() {
  Serial.begin(115200);               //Start serial communication
  RFID.begin(9600);
  SIM900.begin(9600);
  
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

  /**Initializes the SD breakout board. If it is not ready or not connected correct it writes
     an error message to the serial monitor
   **/
//  Serial.print("Initializing SD card...");
//  if (!SD.begin(chipSelect)) {
//    Serial.println("initialization failed!");
//    return;
//  }
//  Serial.println("initialization done.");
}

/* LOOP */
void loop() {
  RFID.listen();
  char rfidTag[18];
  int index = 0;
  while(RFID.available() > 0) {
    char inByte = RFID.read();
    if (inByte != '\r' && index < 17) {
      rfidTag[index++] = inByte;
    }
    else {
      // Terminate buffer
      rfidTag[index++] = '\0';
    }
  }
  
  SIM900.listen();
  char response[18];
  int index = 0;
  while(SIM900.available() > 0) {
    char inByte = SIM900.read();
    if (inByte != '\r' && index < 17) {
      response[index++] = inByte;
    }
    else {
      // Terminate buffer
      response[index++] = '\0';
      if (response == "OK") {
        // do something
      }
      else if (response == "ERR") {
        // do something else
      }
    }
  }
  
  delay(5000);//wait 5 seconds before going to sleep. In real senairio keep this as small as posible
  Going_To_Sleep();

  //dht22ReadFromSensor();
}

/* FUNCTIONS */
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
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + TIME_INTERVAL, 0, 0);

  // clear the alarm flag
  RTC.alarm(ALARM_1);
}

void wakeUp() {
  Serial.println("Interrrupt Fired");//Print message to serial monitor
  sleep_disable();//Disable sleep mode
  detachInterrupt(0); //Removes the interrupt from pin 2;
}

/**
   Function to read the temperature and humidity of the DHT22 sensor.
*/
void dht22ReadFromSensor() {
  //Read from the DHT22 sensor and assign values into variables, NULL is meant for raw data array.
  int function_success = DHT22.read2(&dht22_temperature, &dht22_humidity, NULL);
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
