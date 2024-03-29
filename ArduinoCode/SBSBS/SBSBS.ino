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
//#include <Streaming.h>      //Used to make print statements easier to write
#include <SPI.h>            //Used for SD card communication

#include <SoftwareSerial.h> //Library to communicate with RFID reader (TODO: create class?)
#include <DS3232RTC.h>      //RTC library used to control the RTC module
#include <SimpleDHT.h>      //DHT22 library used to read from a DHT22 module
#include <Q2HX711.h>        //HX711 library used to read weight sensor (amplifier + load cell)
#include <RFID_Priority1Design.h> //RFID_P1D library used to read animal FBD-X tags
#include <Fat16.h>                //SD library used to read and write to a FAT16 SD card module
#include <SIM900_Shield.h>        //SIM900 library used to send SMS messages containing snake data


/* DEFINES */


/* CONSTANTS */
//Digital pins
const byte PIN_WAKE_UP =      2;  //Interrupt pin (or #3) to wake up the Arduino
const byte PIN_DHT22 =        3;  //Digital pin connected to the DHT22 module
const byte PIN_RFID_POWER =   4;  //Digital pin connected BJT/MOSFET switch to power module
const byte PIN_SIM900_POWER = 5;  //Digital pin connected BJT/MOSFET switch to power module
const byte PIN_SIM900_RX =    6;  //Receiving pin for the SIM900 module
const byte PIN_SIM900_TX =    7;  //Transmitting pin for the SIM900 module
const byte PIN_RFID_RX =      8;  //Receiving pin on Arduino (use tx wire on board)
const byte PIN_RFID_TX =      9;  //Transmitting pin on Arduino (use rx wire on board)
//const byte PIN_SD_SPI =     10;
//const byte PIN_SD_MOSI =    11;
//const byte PIN_SD_MISO =    12;
//const byte PIN_SD_SCK =     13;

//Analogue pins
const byte PIN_SEND_SMS =     A0; //Read button input to send SMS (DEBUG)
const byte PIN_SD_SS =        A1; //Digital pin connected to the SD module (the hardware SS pin must be kept as an output)
const byte PIN_HX711_DATA =   A2; //Weight sensor data pin
const byte PIN_HX711_CLOCK =  A3; //Weight sensor clock pin
const byte PIN_RTC_SDA =      A4; //Connect RTC data to Arduino pin A4
const byte PIN_RTC_SCL =      A5; //Connect RTC clock to Arduino pin A5

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
 * So 5 gives 1 snake and up to 4 skinks that have been eaten.
 */
const byte MAX_RFID_TAGS = 4;


/* LIBRARIES INSTANTIATED */
SoftwareSerial SIM900_Serial(PIN_SIM900_RX, PIN_SIM900_TX); //Create soft serial to pass to SIM900 class
SIM900 SIM(&SIM900_Serial, PIN_SIM900_POWER);                              //Controls the SIM900 module

SoftwareSerial RFID_Serial(PIN_RFID_RX, PIN_RFID_TX); //Create soft serial to pass to RFID_P1D class
RFID_P1D RFID(&RFID_Serial, PIN_RFID_POWER);          //Controls the RFID module

SimpleDHT22 DHT22(PIN_DHT22);                         //Controls the DHT22 module

Q2HX711 HX711(PIN_HX711_DATA, PIN_HX711_CLOCK);       //Controls the HX711 module


/* VARIABLES */
// Temperature
float DHT22Temperature;
float DHT22Humidity;

// SD Card
SdCard card;
Fat16 storageFile;

// Flag to determine if an SMS message of snake data should be sent
boolean snakeDataSavedToFile = false;
// Flag for button press to send an SMS
boolean sendSMS = false;


/* STRUCTURES */
struct snakeData {
  time_t epochTime;
  String rfidTag[MAX_RFID_TAGS];    // If less space could use char[17][MAX_RFID_TAGS] or char * [MAX_RFID_TAGS]
  byte rfidTagIndex = 0;
  float temperature;
  float humidity;
  float weight;   // highest and lowest?
};
struct snakeData SnakeData;



/* SETUP */
void setup() {
  // Serial communications
  Serial.begin(19200);
  Serial.println(F("S B."));

  // Initialise weight sensor
  HX711.startSensorSetup();
  // Read from sensor until averaging array fills up
  while (!HX711.isSetupComplete()) {
    HX711.read();
  }

  // Initialise RFID module, start powered off
  RFID.powerDown();

  // Initialise RTC alarms to known values, clear the alarm flags, clear the alarm interrupt flags
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
  if (timeStatus() != timeSet) {
    Serial.println(F("RTC failed!"));
    while(1);
  }

  // Setup morning wakeup alarm
  setMorningWakeupAlarm();

  // Serial.println(F("Connecting to SD Card Reader"));
  // Initialize the SD card
  if (!card.begin(PIN_SD_SS)) {
    Serial.println(F("Failed card.begin"));
    while(1);
  }
  
  // Initialize a FAT16 volume
  if (!Fat16::init(&card)) {
    Serial.println(F("Failed Fat16::init"));
    while(1);
  }

  // DEBUG
  pinMode(PIN_SEND_SMS, INPUT); // Set pin as an input which uses the built-in pullup resistor

  pinMode(LED_BUILTIN, OUTPUT);       // The built-in LED on pin 13 indicates when the Arduino is asleep
  pinMode(PIN_WAKE_UP, INPUT_PULLUP); // Set pin as an input which uses the built-in pullup resistor
  //digitalWrite(LED_BUILTIN, HIGH);    // Turn built-in LED on

  Serial.println(F("S C."));

  //snakeDataSavedToFile = true;  // DEBUG
}


/* LOOP */
void loop() {
  // DEBUG
  if (!sendSMS) {
    // Check for button press
    sendSMS = digitalRead(PIN_SEND_SMS);
    if (sendSMS) {
      Serial.println(F("SMS button pressed"));
      snakeDataSavedToFile = true;
    }
  }

  // Check time is after 8:00pm
  if ((hour(RTC.get()) >= 22 && minute(RTC.get()) >= 55) || sendSMS) {
    // Power down other modules if needed
    //RFID.powerDown();

    // Check snake data has been saved
    if (snakeDataSavedToFile) {
      // Power on SIM900 module, only executes power on once
      SIM.powerUp();

      // Check SIM is powered on
      if (SIM.getPowerStatus()) {
        // Update SIM900 module
        SIM.update();

        // Once connected to the network
        if (SIM.isConnectedToNetwork()) {
          // Send a text message for each line in the file
          sendSnakeDataSMS();

          // Turn off SIM900 module, also resets member variables
          SIM.powerDown();

          // Reset snake data saved boolean
          snakeDataSavedToFile = false;   // On the next loop after this, the arduino will sleep
        } // End if SIM.isConnectedToNetwork()
      }   // End if SIM.getPowerStatus()
    }     // End if snakeDataSavedToFile
    // No snake data was saved so sleep for the night
    else {
      // Check all modules are powered down
      

      // Reset that an SMS was sent
      if (sendSMS) {
        sendSMS = false;
      }
      // Do normal end of day process of sleeping and setting alarm
      else {
        // Set morning awake alarm
        setMorningWakeupAlarm();

        // Sleep Arduino
        sleepArduino();
      }
    }
  }
  // Do normal processes
  else {
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

      // Reset RFID tag read
      RFID.resetTagRead();
    }

    // RFID module turned off in update function and weight detect flag is still true
    if (!RFID.getPowerStatus() && HX711.getWeightDetected()) {
      // If a snake RFID has been read the index will be at least 1
      if (SnakeData.rfidTagIndex != 0) {
        // Throughout sensing time get the highest weight detected
        SnakeData.weight = HX711.getHighestDetectedWeight();

        // DEBUG print snake data
        printRecordedData();

        // Save snake recording to an SD card file
        saveSnakeDataToSDCard();

        // Reset SnakeData struct
        resetSnakeData();
      }

      // Reset weight detection
      HX711.resetWeightDetected();

      // Reset highest detected weight
      HX711.resetHighestDetectedWeight();
    }
  }
} // End loop


/* FUNCTIONS */
/**
 * Function to read the contents of today's SD file and generate a text message 
 * body by sending AT commands to the SIM900 module.
 */
void sendSnakeDataSMS() {
  char filename[13];
  createFileName(filename);

  // Opening the file for reading and writing content to SIM900 module
  if (storageFile.open(filename, O_READ)) {
    // Reset that text mode is ready
    SIM.resetTextModeReady();
    
    // Send text mode to SIM900 module
    SIM.sendTextMode();
    Serial.println(F("Sent text mode."));

    while (!SIM.isTextModeReady()) {
      SIM.read();
    }
    Serial.println(F("Text mode ready."));


    // Reset that the body of the text message is ready for writing
    SIM.resetReadyForEnteringText();

    // Send command for setting the phone number
    SIM.sendTwilioPhoneNumber();
    Serial.println(F("Sent phone number."));

    while (!SIM.isReadyForEnteringText()) {
      SIM.read();
    }
    Serial.println(F("Phone ready for text."));

    // How many characters wil be sent
    byte messageCharacters = 0;
    // Read from the file until there's nothing else in it
    int16_t c;
    while ((c = storageFile.read()) > 0) {
      // An end of line was found
      if (c == '\r' || c == '\n') {
        // If a line without any data
        if (messageCharacters >= 37) {
          Serial.print(F("End of line reached, sending text with chars: "));
          Serial.println(messageCharacters);

          // Reset all parts of sending a text
          SIM.resetTextMessageSent();
          SIM.resetReadyForEnteringText();

          // Finish text message and send
          SIM.sendEndOfTextMessage();

          while (!SIM.wasTextMessageSent()) {
            SIM.read();
          }

          // Send command for setting the phone number
          SIM.sendTwilioPhoneNumber();

          while (!SIM.isReadyForEnteringText()) {
            SIM.read();
          }
          Serial.println(F("Phone ready for text."));
        }
        // Reset the number of characters sent
        messageCharacters = 0;
      }
      else {
        // Print each character to text message body
        SIM.sendATCommands((char)c);
        messageCharacters++;

        // DEBUG print to console the "text message"
        Serial.print((char)c);
      }
    }

    Serial.println(F("Finished texts"));
    
    // Close the file
    if (!storageFile.close()) {
      Serial.println(F("SD close file error"));
    }
  }
  else {
    // If the file didn't open, print an error
    Serial.print(F("Err reading "));
    Serial.println(filename);
  }
}

/**
 * Function to reset SnakeData struct variables to 0.
 */
void resetSnakeData() {
  Serial.println(F("Reseting Snake Data!"));
  SnakeData.epochTime = 0;
  for (int i = 0; i < MAX_RFID_TAGS; i++) {
    SnakeData.rfidTag[i] = "";
  }
  SnakeData.rfidTagIndex = 0;
  SnakeData.temperature = 0;
  SnakeData.humidity = 0;
  SnakeData.weight = 0;
}

/**
 * DEBUG Function to print the contents of the snake data struct.
 */
void printRecordedData() {
  Serial.print(SnakeData.epochTime);
  Serial.print(F(", Snake: "));
  Serial.print(SnakeData.rfidTag[0]);
  Serial.print(F(". Skinks: "));
  for (int i = 1; i < SnakeData.rfidTagIndex; i++) {
    Serial.print(SnakeData.rfidTag[i]);
    Serial.print(F(", "));
  }
  Serial.print(SnakeData.temperature);
  Serial.print(F("C, "));
  Serial.print(SnakeData.humidity);
  Serial.print(F("%, "));
  Serial.println(SnakeData.weight);
}

/**
 * Function to record all sensor values to the snake data struct.
 */
void recordSnakeData() {
  Serial.println(F("Recording SNAKE tag"));

  // Assign epoch time
  SnakeData.epochTime = RTC.get();
  // Assign RFID tag and increase index
  SnakeData.rfidTag[SnakeData.rfidTagIndex++] = RFID.getMessage();

  // Collect and assign temperature data
  readDHT22();
  SnakeData.temperature = DHT22Temperature;
  SnakeData.humidity = DHT22Humidity;

  // Get early max weight, will also assign later when detection ends
  SnakeData.weight = HX711.getHighestDetectedWeight();
}

/**
 * Function to record further RFID tags read to the snake data struct.
 */
void recordSkinkTags() {
  boolean sameTag = false;

  Serial.println(F("Recording SKINK tag"));

  // Check that existing tags are not saved more than once
  for (int index = 0; index < SnakeData.rfidTagIndex; index++) {
    // If a recorded tag equals the tag to save
    if (SnakeData.rfidTag[index].equals(RFID.getMessage())) {
      // Exit function without saving
      Serial.print(F("Same tag read: "));
      Serial.print(RFID.getMessage());
      sameTag = true;
      return;
    }
  }

  if (SnakeData.rfidTagIndex < MAX_RFID_TAGS && !sameTag) {
    // Add tag to list of tags in the snake
    SnakeData.rfidTag[SnakeData.rfidTagIndex++] = RFID.getMessage();
  }
  else {
    // Too many skinks been eaten, over MAX_RFID_TAGS (4)
    Serial.println(F("Too many skink tags read!"));
  }
}

/**
 * Function to read the temperature and humidity of the DHT22 sensor.
 * If read more than once per 2 seconds it will print an error.
 * Temperature and Humidity values will be same as last read if error.
 */
void readDHT22() {
  //Read from the DHT22 sensor and assign values into variables, NULL is meant for raw data array.
  byte functionSuccess = DHT22.read2(&DHT22Temperature, &DHT22Humidity, NULL);

  //Check return value is NOT equal to constant for success
  if (functionSuccess != SimpleDHTErrSuccess) {
    Serial.print(F("Read DHT22 failed, err: "));
    Serial.println(functionSuccess, HEX);
  }
}

/**
 * Function to create the file name for today.
 * Using DOS 8.3 naming format, result: (YYYYMMDD.csv).
 */
void createFileName(char* filename) {
  // Get current date time
  time_t dateTime = RTC.get();

  // Generate C string (char array) using date time and formaters
  snprintf(filename, 13, "%d%s%d%s%d.txt", year(dateTime), ((month(dateTime) < 10) ? "0" : ""), month(dateTime), ((day(dateTime) < 10) ? "0" : ""), day(dateTime));
}

/**
 * Function to open the SD card file for today (YYYYMMDD.csv) and write 
 * the contents of the snake data struct to the file.
 * Order saved: epoch, temp, hum, weight, rfid tags
 */
void saveSnakeDataToSDCard() {
  // Get file name for today
  char filename[13];
  createFileName(filename);

  // Clear write error
  storageFile.writeError = false;

  // O_CREAT - create the file if it does not exist
  // O_APPEND - seek to the end of the file prior to each write
  // O_WRITE - open for write
  if (storageFile.open(filename, O_CREAT | O_APPEND | O_WRITE)) {
    Serial.print(F("Writing to "));
    Serial.println(filename);

    char COMMA = ',';

    // Print time. temperature, humidity, weight
    storageFile.print(SnakeData.epochTime);
    storageFile.print(COMMA);
    storageFile.print(SnakeData.temperature);
    storageFile.print(COMMA);
    storageFile.print(SnakeData.humidity);
    storageFile.print(COMMA);
    storageFile.print(SnakeData.weight);
    storageFile.print(COMMA);

    // Print each RFID tag read to file
    for (int index = 0; index < SnakeData.rfidTagIndex; index++) {
      storageFile.print(SnakeData.rfidTag[index]);

      // Print comma until last tag printed
      if (index < (SnakeData.rfidTagIndex - 1)) {
        storageFile.print(COMMA);
      }
    }

    // Start new line
    storageFile.println();

    // Set snake data was saved to file attribute
    snakeDataSavedToFile = true;

    // Check for file writing errors
    if (storageFile.writeError) {
      Serial.print(F("SD write error"));
    }

    // Close the file
    if (!storageFile.close()) {
      Serial.print(F("SD close file error"));
    }

    Serial.println(F("Saved snake data to file."));
  }
  else {
    // If the file didn't open, print an error
    Serial.println(F("Error SD file open "));
    Serial.println(filename);
  }

  // DEBUG TODO remove later
  // Opening the file for reading and writing content to serial monitor
  if (storageFile.open(filename, O_READ)) {
    // Read from the file until there's nothing else in it
    int16_t c;
    while ((c = storageFile.read()) > 0) Serial.write((char)c);

    Serial.println();
    
    // Close the file
    if (!storageFile.close()) {
      Serial.print(F("SD close file error"));
    }
  }
  else {
    // If the file didn't open, print an error
    Serial.print(F("Error opening/reading "));
    Serial.println(filename);
  }
}

/**
 * Function to set an interrupt alarm for waking Arduino in the morning.
 */
void setMorningWakeupAlarm() {
  // Set alarm for 9:00am in the morning
  RTC.setAlarm(ALM1_MATCH_HOURS, 0, 0, 9, 0);     //ALM2_MATCH_HOURS

  // clear the alarm flag
  RTC.alarm(ALARM_1);
  // configure the INT/SQW pin for "interrupt" operation (disable square wave output)
  RTC.squareWave(SQWAVE_NONE);
  // enable interrupt output for Alarm 1
  RTC.alarmInterrupt(ALARM_1, true);
}

/**
 * Function to set up and execute sleep mode for the Arduino.
 */
void sleepArduino() {
  // Enable sleep mode
  sleep_enable();

  // Pin to detect change, method to call, the change to detect
  // Attach an interrupt to pin D2
  attachInterrupt(0, wakeUp, LOW);

  // Setting the sleep mode, in our case full sleep
  /* 
   * Types of sleep:
   *  SLEEP_MODE_IDLE      - The least power savings
   *  SLEEP_MODE_ADC
   *  SLEEP_MODE_PWR_SAVE
   *  SLEEP_MODE_STANDBY
   *  SLEEP_MODE_PWR_DOWN  - The most power savings
   */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);

  // Turn built in LED off
  digitalWrite(LED_BUILTIN, LOW);

  time_t t;
  t = RTC.get();
  Serial.print(F("Sleep  Time: "));
  Serial.print(hour(t));
  Serial.print(F(":"));
  Serial.print(minute(t));
  Serial.print(F(":"));
  Serial.println(second(t));

  // Wait a second to allow the led to be turned off before going to sleep
  delay(1000);

  // Activate sleep mode
  sleep_cpu();

  // Next line of code executed after the interrupt
  Serial.println(F("Just woke up!"));
  
  // Turn LED on
  digitalWrite(LED_BUILTIN, HIGH);

  t = RTC.get();
  Serial.print(F("WakeUp Time: "));
  Serial.print(hour(t));
  Serial.print(F(":"));
  Serial.print(minute(t));
  Serial.print(F(":"));
  Serial.println(second(t));

  // DEBUG Set New Alarm for 5 minutes
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, minute(t) + RTC_ALARM_TIME_INTERVAL, 0, 0);

  // Clear the alarm flag
  RTC.alarm(ALARM_1);
}

/**
 * Function that is run immediately after Arduino is woken from sleep mode.
 */
void wakeUp() {
  Serial.println(F("Interrrupt Fired"));

  // Disable sleep mode
  sleep_disable();

  // Removes the interrupt from pin 2
  detachInterrupt(0);
}

// template <class T> void DEBUGSecondDisplay(const T toDisplay) {
//   if (millis() - DEBUGStartTime > 1000) {
//     Serial.println(toDisplay);
//     DEBUGStartTime = millis();
//   }
// }