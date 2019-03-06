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
}


void loop() {
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
    if (c == '\r' || c == '\n') {       // \r means "end of message"
        buffer_pos--;
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
    }
  }
  
  // Send messages from Serial Montior to SIM900 module
  if (Serial.available()) {
    SIM900.write(Serial.read());
  }
}

void quickCommands() {
    SIM900.print("AT+CMGS=\"+441233800093\"");  // Twilio phone number
    SIM900.print('\r');
    SIM900.print("Quick message test");
    SIM900.print('\r');
    SIM900.print(char(26));
    SIM900.print('\r');
}