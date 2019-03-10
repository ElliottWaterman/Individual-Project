#include <SoftwareSerial.h>
#include <SIM900_Shield.h>

// RX on Arduino (use tx wire on board)
// TX on Arduino (use rx wire on board)
// Use Carriage return and 9600 baud
SoftwareSerial SIM900(6, 7);

// SoftwareSerial SIM900_Serial(6, 7); //Create soft serial to pass to SIM900 class
// SIM900 SIM(&SIM900_Serial, PIN_SIM900_POWER);                              //Controls the SIM900 module

// Defines and Constants
#define BUFFER_SIZE (256)
static char buffer[BUFFER_SIZE];
static size_t buffer_pos = 0;        // position of next write


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Setup.");

  SIM900.begin(19200);
  if (SIM900.listen()) {
    Serial.println("Cannot listen!");
  }

  //quickCommands();
  SIM900.print(TEXT_MODE);
  SIM900.print('\r');
}

#define TEXT_MODE                       "AT+CMGF=1"

void loop() {
  readSIM();
  
  // Send messages from Serial Montior to SIM900 module
  if (Serial.available()) {
    SIM900.write(Serial.read());
  }
}

boolean rdy = false;
boolean textMessageSent = false;

void readSIM() {
  if (SIM900.available() && buffer_pos < (BUFFER_SIZE - 1)) {
    // Read incoming character/byte
    char c = SIM900.read();
    buffer[buffer_pos++] = c;

    // if (c == '\r') {
    //     Serial.print("CR");
    // }
    // if (c == '\n') {
    //     Serial.print("LF");
    // }

    // Entire message has been received
    if (c == '\r' || c == '\n' || c == '>') {
      if (c != '>') {
        buffer_pos--;
      }

      buffer[buffer_pos] = '\0';      // terminate the buffer

      Serial.print(buffer);

      String message = String(buffer);
      
      // Message has been dealt with, reset buffer position
      buffer_pos = 0;

      if (strcmp(buffer, "OK") == 0) {
          Serial.println("Hit OK");
      }
      else if (strcmp(buffer, "ERROR") == 0) {
          Serial.println("Hit ERROR");
      }
      else if (strcmp(buffer, ">") == 0) {
        rdy = true;
        Serial.println(F("Hit WAITING_FOR_TEXT"));
      }

      // Always check for the number of text messages sent reply
      if (strncmp(buffer, NUMBER_OF_MESSAGES, 6) == 0) {
        textMessageSent = true;
        Serial.println(F("TXT sent"));
      }
    }
  }
}

#define TEXT_MODE                       "AT+CMGF=1"
#define TWILIO_PHONE_NUMBER             "AT+CMGS=\"+441233800093\""
#define BECCA                           "AT+CMGS=\"+447510507599\""
#define NUMBER_OF_MESSAGES              "+CMGS:"

void quickCommands() {
  SIM900.print(TWILIO_PHONE_NUMBER);  // Twilio phone number
  SIM900.print('\r');

  while (!rdy) {
    readSIM();
  }

  rdy = false;

  SIM900.print("1552139378,22.40,45.10,390.34,900_067000096387,941_000014361389");
  SIM900.print(char(26));
  SIM900.print('\r');

  while (!textMessageSent) {
    readSIM();
  }

  textMessageSent = false;
  Serial.println();

  SIM900.print(TWILIO_PHONE_NUMBER);  // Twilio phone number
  SIM900.print('\r');

  while (!rdy) {
    readSIM();
  }

  rdy = false;

  SIM900.print("1552139435,22.80,45.30,11507.23,941_000014361389,900_067000096387");
  SIM900.print((char)26);
  SIM900.print('\r');

  while (!textMessageSent) {
    readSIM();
  }

  textMessageSent = false;
  Serial.println();

  SIM900.print(TWILIO_PHONE_NUMBER);  // Twilio phone number
  SIM900.print('\r');

  while (!rdy) {
    readSIM();
  }

  rdy = false;

  SIM900.print("1552140019,22.90,44.90,11507.23,900_067000096387,941_000014361389");
  SIM900.print(char(26));
  SIM900.print('\r');

  while (!textMessageSent) {
    readSIM();
  }

  textMessageSent = false;
  Serial.println();
}