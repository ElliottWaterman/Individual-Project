#include <SoftwareSerial.h>

SoftwareSerial RFID(8, 9); // RX and TX

int i;

void setup()
{
  RFID.begin(9600);    // start serial to RFID reader
  Serial.begin(9600);  // start serial to PC

  Serial.println("Hello");
}

void loop()
{
  //Serial.println(RFID.available());
  if (RFID.available() > 0) {
     i = RFID.read();
     Serial.print(i, HEX); //Display the Serial Number in HEX
     Serial.print(" ");
     Serial.println("");
  }
  // If Serial Command Code sent from serial monitor send command to RFID module
  // See for commands http://www.priority1design.com.au/rfidrw-e-ttl.pdf
  if (Serial.available()) {
    RFID.write(Serial.read());
  }
  
  //delay(200);
}
