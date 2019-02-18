#include <SoftwareSerial.h>
#include <RFID_Priority1Design.h>

// println HEX:   3934315F303030303134333631333839D
// println:       5752499548484848495251544951565713
// write:         941_000014361389

#define PIN_RFID_POWER    4   //Digital pin connected BJT/MOSFET switch to power module
#define PIN_RFID_RX       8   //Receiving pin on Arduino (use tx wire on board)
#define PIN_RFID_TX       9   //Transmitting pin on Arduino (use rx wire on board)

SoftwareSerial RFID_Serial(PIN_RFID_RX, PIN_RFID_TX); //Create soft serial to pass to RFID_P1D class
RFID_P1D RFID(&RFID_Serial, PIN_RFID_POWER);          //Controls the RFID module

void setup() {
  // Turn on to read straight away
  RFID.powerUp();

  // Open serial communications to serial monitor
  Serial.begin(9600);
  Serial.println("Setup complete.");
}

void loop() {
  // Check if on, turn on if off
  if (RFID.getPowerStatus()) {
    // Turns off after 30 seconds
    RFID.update();

    // Print tag if read and reset flag
    if (RFID.hasTagBeenRead()) {
      Serial.println(RFID.getMessage());

      RFID.resetTagRead();
    }
  }
  else {
    Serial.println("Powering up");
    RFID.powerUp();
  }

  // Uncomment to read all the time 
  // if (RFID.readMessage()) {
  //   Serial.println(RFID.getMessage());
  // }


  // Send messages from Serial Montior to RFID module
  // if (Serial.available()) {
  //   mySerial.write(Serial.read());
  // }
}
