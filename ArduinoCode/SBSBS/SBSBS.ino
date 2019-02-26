/**
   Author: Elliott Waterman
   Date Created: 11/01/2019
   Date Last Modified: 22/02/2019
   Description: Complete program to run the arduino in the
   Smart Boa Snake Basking Station.
*/


/* INCLUDES */
#include <Arduino.h>        //Standard library
#include <avr/sleep.h>      //AVR library contains methods and sleep modes to control the sleep of an Arduino
#include <Streaming.h>      //Used to make print statements easier to write
#include <SPI.h>            //Used for SD card communication

#include <SoftwareSerial.h> //Library to communicate with RFID reader (TODO: create class?)
#include <DS3232RTC.h>      //RTC library used to control the RTC module
#include <SimpleDHT.h>      //DHT22 library used to read from a DHT22 module
#include <Q2HX711.h>        //HX711 library used to read weight sensor (amplifier + load cell)
#include <RFID_Priority1Design.h> //RFID_P1D library used to read animal FBD-X tags
#include <SD.h>             //SD library used to read and write to a SD card module


/* DEFINES */
// These are integers but should be const byte X = 4;!
#define PIN_WAKE_UP       2   //Interrupt pin (or #3) to wake up the Arduino
#define PIN_DHT22         3   //Digital pin connected to the DHT22 module
#define PIN_RFID_POWER    4   //Digital pin connected BJT/MOSFET switch to power module
#define PIN_SIM900_RX     6   //Receiving pin for the SIM900 module
#define PIN_SIM900_TX     7   //Transmitting pin for the SIM900 module
#define PIN_RFID_RX       8   //Receiving pin on Arduino (use tx wire on board)
#define PIN_RFID_TX       9   //Transmitting pin on Arduino (use rx wire on board)

#define PIN_SD            A0  //Digital pin connected to the SD module (the hardware SS pin must be kept as an output)
//#define PIN_SPI         11  // 12, 13 pins work
#define PIN_HX711_DATA    A2  //Weight sensor data pin
#define PIN_HX711_CLOCK   A3  //Weight sensor clock pin
#define PIN_RTC_SDA       A4  //Connect RTC data to Arduino pin A4
#define PIN_RTC_SCL       A5  //Connect RTC clock to Arduino pin A5
//#define EXAMPLE           1   //Comment


/* CONSTANTS */
// RTC
const byte RTC_ALARM_TIME_INTERVAL = 5;   //Sets the wakeup intervall in minutes

// Snake structure recordings
/*
 * 12 hours in a day to bask in sun, 8am to 8 pm.
 * 30 minutes basking time means 24 snakes in one day.
 * Double just in case means ~50 snakes, or 2 days worth of recordings.
 */
//const byte MAX_SNAKE_RECORDINGS = 2;
/*
 * The number of RFID tags read in one weight detection.
 * Includes the SNAKE tag and any SKINK tags.
 * So 1 snake and up to 4 skinks that have been eaten.
 */
const byte MAX_RFID_TAGS = 5;


/* LIBRARIES INSTANTIATED */
SoftwareSerial SIM900(PIN_SIM900_RX, PIN_SIM900_TX);  //Controls the SIM900 module

SoftwareSerial RFID_Serial(PIN_RFID_RX, PIN_RFID_TX); //Create soft serial to pass to RFID_P1D class
RFID_P1D RFID(&RFID_Serial, PIN_RFID_POWER);          //Controls the RFID module

SimpleDHT22 DHT22(PIN_DHT22);                         //Controls the DHT22 module

Q2HX711 HX711(PIN_HX711_DATA, PIN_HX711_CLOCK);       //Controls the HX711 module


/* VARIABLES */
// Temperature
float DHT22Temperature;
float DHT22Humidity;


/* STRUCTURES */
struct snakeData {
  time_t epochTime;
  String rfidTag[MAX_RFID_TAGS];    // If less space could use char[34][MAX_RFID_TAGS]
  byte rfidTagIndex = 0;
  float temperature;
  float humidity;
  float weight; // highest and lowest?
};
struct snakeData SnakeData;



/* SETUP */
void setup() {
  // Serial communications
  Serial.begin(9600);
  Serial.println(F("Setup Begin"));

  // Start software serial communication with SIM and RFID modules
  //SIM900.begin(9600);

  // Initialise weight sensor
  Serial.println(F("Initialising weight sensor"));
  HX711.startSensorSetup();
  // Read from sensor until averaging array fills up
  while (!HX711.isSetupComplete()) {
    HX711.read();
  }
  Serial << F("Initial Zero Long: ") << HX711.getInitialZero() << endl;
  Serial << F("Initial Zero Weight: ") << HX711.getInitialZero() / 712.0 << endl;

  // Initialise RFID module, start powered off
  Serial.println("Powering down RFID module");
  RFID.powerDown();

  // Initialise RTC alarms to known values, clear the alarm flags, clear the alarm interrupt flags
  Serial.println(F("Initialising RTC alarms"));
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
    Serial.println(F("RTC has set the system time"));
  else if (timeStatus() == timeNeedsSync)
    Serial.println(F("Unable to sync with the RTC"));
  else if (timeStatus() == timeNotSet)
    Serial.println(F("RTC has set the system time"));
  else
    Serial.println(F("Unable to sync with the RTC"));

  Serial.println(F("Connecting to SD Card Reader"));
  if (!SD.begin(PIN_SD)) {
    Serial.println("Initialization failed!");
    while (1);
  }

  // DEBUG
  pinMode(LED_BUILTIN, OUTPUT);       //The built-in LED on pin 13 indicates when the Arduino is asleep
  pinMode(PIN_WAKE_UP, INPUT_PULLUP); //Set pin as an input which uses the built-in pullup resistor
  //digitalWrite(LED_BUILTIN, HIGH);    //Turn built-in LED on

  Serial.println(F("Setup Complete!"));
  Serial.println(F("To start: Trigger weight sensor, then scan RFID tags"));
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
  }

  // Read tags or turn off module if elapsed time on, only runs if module is ON
  RFID.update();

  // Check if tag has been read
  if (RFID.hasTagBeenRead()) {
    // First tag that has been read since power up, must be a SNAKE tag
    if (RFID.isFirstTagSincePowerUp()) {
      // Collect and assign data to snake recordings
      recordSnakeData();

      // Reset reading the first RFID tag since power up
      RFID.resetFirstTagSincePowerUp();
    }
    // Further tags have been read, probably a SKINKS tag
    else {
      // Save RFID tag to snake data array of tags
      recordSkinkTags();
    }

    // TODO: Store on SD card, or make SnakeData an array with an index

    // Reset RFID tag read
    RFID.resetTagRead();
  }

  // RFID module turned off in update function and weight detect flag is still true
  if (!RFID.getPowerStatus() && HX711.getWeightDetected()) {
    // If a snake RFID has been read the index will be at least 1
    if (SnakeData.rfidTagIndex != 0) {
      // Throughout sensing time get the highest weight detected
      SnakeData.weight = HX711.getHighestDetectedWeight();

      printRecordedData();

      // Save snake recording to an SD card file
      saveSnakeDataToSDCard();
    }

    // Reset weight detection
    HX711.resetWeightDetected();
  }

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


/* FUNCTIONS */
void printRecordedData() {
  Serial << SnakeData.epochTime << F(" or ");
  printDateTime(SnakeData.epochTime);
  Serial << ", Snake: " << SnakeData.rfidTag[0] << F(". ");
  Serial << "Skinks: ";
  for (int i = 1; i < SnakeData.rfidTagIndex; i++) {
    Serial << SnakeData.rfidTag[i] << F(", ");
  }
  Serial << SnakeData.temperature << F("C, ") << SnakeData.humidity << F("%, ");
  Serial << SnakeData.weight << F(" vs ") << HX711.getCurrentWeight() << endl;
}

/**
 * Function to record all sensor values to the snake data struct.
 */
void recordSnakeData() {
  // Assign epoch time
  SnakeData.epochTime = RTC.get();
  // Assign RFID tag and increase index
  SnakeData.rfidTag[SnakeData.rfidTagIndex++] = RFID.getMessage();

  // Collect and assign temperature data
  readDHT22();
  SnakeData.temperature = DHT22Temperature;
  SnakeData.humidity = DHT22Humidity;

  // Get and assign weight data
  SnakeData.weight = HX711.getHighestDetectedWeight();
  // Reset highest detected weight
  HX711.resetHighestDetectedWeight();
}

/**
 * Function to record further RFID tags read to the snake data struct.
 */
void recordSkinkTags() {
  String rfidTagToRecord = RFID.getMessage();

  Serial << F("Recording skink tag") << endl;

  // Check that existing tags are not saved more than once
  for (int index = 0; index < SnakeData.rfidTagIndex; index++) {
    // If a recorded tag equals the tag to save
    if (SnakeData.rfidTag[index].equals(rfidTagToRecord)) {
      // Exit function without saving
      Serial << F("Same tag read: ") << RFID.getMessage() << endl;
      return;
    }
  }

  Serial << F("Index: ") << SnakeData.rfidTagIndex << endl;

  if (SnakeData.rfidTagIndex < MAX_RFID_TAGS) {
    // Add tag to list of tags in the snake
    SnakeData.rfidTag[SnakeData.rfidTagIndex++] = RFID.getMessage();
  }
  else {
    // Too many skinks been eaten, over MAX_RFID_TAGS (5)
    Serial.println(F("Too many snake/skink tags read!"));
  }
}

/**
 * Function to print the day month year hour:minute:second of time_t input.
 */
void printDateTime(time_t t) {
  Serial << ((day(t) < 10) ? "0" : "") << _DEC(day(t));
  Serial << monthShortStr(month(t));
  Serial << _DEC(year(t)) << ' ';
  Serial << ((hour(t) < 10) ? "0" : "") << _DEC(hour(t)) << ':';
  Serial << ((minute(t) < 10) ? "0" : "") << _DEC(minute(t)) << ':';
  Serial << ((second(t) < 10) ? "0" : "") << _DEC(second(t));
}

/**
 * Function to read messages from the SIM900 module
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
 * Function to read the temperature and humidity of the DHT22 sensor.
 * If read more than once per 2 seconds it will print an error.
 * Temperature and Humidity values will be same as last read if error.
 */
void readDHT22() {
  //Read from the DHT22 sensor and assign values into variables, NULL is meant for raw data array.
  int functionSuccess = DHT22.read2(&DHT22Temperature, &DHT22Humidity, NULL);

  //Check return value is NOT equal to constant for success
  if (functionSuccess != SimpleDHTErrSuccess) {
    Serial.print(F("Read DHT22 failed, err: "));
    Serial.println(functionSuccess, HEX);
  }
}

/**
 * Function to create the file name for today, in format (YYYYMMDD.csv).
 * 
 */
void createFileName(char* filename) {
  // Get current date time
  time_t dateTime = RTC.get();

  // 8.3 format; 8 characters, 1 dot, 3 extension characters, null terminator
  //char *filename = malloc(13);
  //char filename[13];

  // Generate C string (char array) using date time and formaters
  snprintf(filename, 13, "%d%s%d%s%d.txt", year(dateTime), ((month(dateTime) < 10) ? "0" : ""), month(dateTime), ((day(dateTime) < 10) ? "0" : ""), day(dateTime));
  Serial.print("Func: ");
  Serial.println(filename);

  // return filename;

  // strcpy(filename, year(dateTime));             // Year
  // strcat(filename, ((month(dateTime) < 10) ? "0" : ""));  // Add leading zero if below 10
  // strcat(filename, month(dateTime));            // Month
  // strcat(filename, ((day(dateTime) < 10) ? "0" : ""));    // Add leading zero if below 10
  // strcat(filename, day(dateTime));              // Day
  // strcat(filename, ".csv");                               // Add extension
}

/**
 * Function to create a new SD card file in the format (YYYYMMDD.csv) and 
 * used when saving snake data which was recorded today.
 */
void createSDFileForToday() {
  char filename[13];
  createFileName(filename);

  Serial.println(filename);

  // Create file on SD card
  File storageFile = SD.open(filename, FILE_WRITE);
  storageFile.close();
}

/**
 * Function to open the SD card file for today (YYYYMMDD.csv) and write the
 * contents of the snake data struct to the file.
 * Order saved: epoch, temp, hum, weight, rfid tags
 */
void saveSnakeDataToSDCard() {
  // Get file name for today
  char filename[13];
  createFileName(filename);

  Serial.print(F("File name: "));
  Serial.println(filename);

  // Creates the file object for writing
  File storageFile = SD.open(filename, FILE_WRITE);

  // if the file opened okay, write to it:
  if (storageFile) {
    Serial.println(F("Writing to"));
    Serial.println(filename);

    char COMMA = ',';

    // Print epoch time and temperature to file
    storageFile.print(String(SnakeData.epochTime) + COMMA + String(SnakeData.temperature) + COMMA);

    // Print humidity and weight to file
    storageFile.print(String(SnakeData.humidity) + COMMA + String(SnakeData.weight) + COMMA);

    // Print each RFID tag read to file
    for (int index = 0; index < SnakeData.rfidTagIndex; index++) {
      storageFile.print(SnakeData.rfidTag[index]);

      // Print comma until last tag printed
      if (index < (SnakeData.rfidTagIndex - 1)) {
        storageFile.print(COMMA);
      }
    }

    // OR
    // storageFile.print(String(SnakeData.epochTime));
    // storageFile.print(COMMA);
    // storageFile.print(String(SnakeData.temperature));
    // storageFile.print(COMMA);
    // storageFile.print(String(SnakeData.humidity));
    // storageFile.print(COMMA);
    // storageFile.print(String(SnakeData.weight));
    // storageFile.print(COMMA);

    // Close the file
    storageFile.close();

    Serial.println(F("Saved snake data to file."));
  }
  else {
    // If the file didn't open, print an error
    Serial.println(F("Error opening"));
    Serial.println(filename);
  }

  // DEBUG TODO remove later
  // Opening the file for reading and writing content to serial monitor
  storageFile = SD.open(filename);
  if (storageFile) {
    Serial.println(filename);

    // Read from the file until there's nothing else in it
    while (storageFile.available()) {
      Serial.write(storageFile.read());
    }
    // Close the file
    storageFile.close();
  }
  else {
    // If the file didn't open, print an error
    Serial.println(F("Error opening"));
    Serial.println(filename);
  }
}

/**
 * Function TODO
 */
// void setRTCMinutesAlarm() {
//     // Set alarm on RTC for RTC_ALARM_TIME_INTERVAL + t
//   time_t t;
//   t = RTC.get();    //Gets the current time of the RTC
//   RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + RTC_ALARM_TIME_INTERVAL, 0, 0);  // Setting alarm 1 to go off 5 minutes from now
//   //Clear the alarm flag
//   RTC.alarm(ALARM_1);
//   //Configure the INT/SQW pin for "interrupt" operation (disable square wave output)
//   RTC.squareWave(SQWAVE_NONE);
//   //Enable interrupt output for Alarm 1
//   RTC.alarmInterrupt(ALARM_1, true);
// }

/**
 * Function TODO
 */
// void Going_To_Sleep() {
//   sleep_enable();                       //Enabling sleep mode
//   //Pin to detect change, method to call, the change to detect
//   attachInterrupt(0, wakeUp, LOW);      //attaching an interrupt to pin d2
//   set_sleep_mode(SLEEP_MODE_PWR_DOWN);  //Setting the sleep mode, in our case full sleep
//   digitalWrite(LED_BUILTIN, LOW);       //turning LED off

//   time_t t;                             // creates temp time variable
//   t = RTC.get();                        //gets current time from rtc
//   Serial.println("Sleep  Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t)));
//   delay(1000);                          //wait a second to allow the led to be turned off before going to sleep
//   sleep_cpu();                          //activating sleep mode

//   Serial.println("just woke up!");      //next line of code executed after the interrupt
//   digitalWrite(LED_BUILTIN, HIGH);      //turning LED on
//   t = RTC.get();
//   Serial.println("WakeUp Time: " + String(hour(t)) + ":" + String(minute(t)) + ":" + String(second(t)));
//   //temp_Humi();                          //function that reads the temp and the humidity
//   //Set New Alarm
//   RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + RTC_ALARM_TIME_INTERVAL, 0, 0);

//   // clear the alarm flag
//   RTC.alarm(ALARM_1);
// }

/**
 * Function TODO
 */
// void wakeUp() {
//   Serial.println("Interrrupt Fired");//Print message to serial monitor
//   sleep_disable();//Disable sleep mode
//   detachInterrupt(0); //Removes the interrupt from pin 2;
// }

// template <class T> void DEBUGSecondDisplay(const T toDisplay) {
//   if (millis() - DEBUGStartTime > 1000) {
//     Serial.println(toDisplay);
//     DEBUGStartTime = millis();
//   }
// }