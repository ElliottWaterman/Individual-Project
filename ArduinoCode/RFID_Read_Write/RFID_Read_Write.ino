#include <SoftwareSerial.h>
// println HEX:   3934315F303030303134333631333839D
// println:       5752499548484848495251544951565713
// write:         941_000014361389

// RX on Arduino (use tx wire on board)
// TX on Arduino (use rx wire on board)
// Use Carriage return and 57600 baud
SoftwareSerial mySerial(8, 9);

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(57600);
  Serial.println("Goodnight moon!");

  mySerial.begin(9600);
}

#define BUFFER_SIZE (256)
static char buffer[BUFFER_SIZE];
static size_t buffer_pos;        // position of next write

#define TAG_SIZE (40)
static char tagPresent[TAG_SIZE];
static int tagPresentPos;
int recievedTagInfo = 0;

void loop() {
  if (mySerial.available() && buffer_pos < (BUFFER_SIZE - 1)) {
    // Read incoming character/byte
    char c = mySerial.read();
    buffer[buffer_pos++] = c;

    // Entire message has been received
    if (c == '\r') {                // \r means "end of message"
        buffer[buffer_pos] = '\0';  // terminate the buffer
        String ack = String(buffer);
        //Serial.println(buffer);     // send echo
        //Serial.println(buffer_pos); // send size of buffer

        if (buffer_pos == 17) {
          recievedTagInfo = 0;
          Serial.println("Test for tag to leave:");
          mySerial.write("LTG\r");
        }
        //recievedTagInfo = (buffer_pos == 17) ? 1 : recievedTagInfo;
        
        if (ack.equals("OK\r")) {
          tagPresent[tagPresentPos++] = 1;
          mySerial.write("LTG\r");  // Ask for tag again
        }
        else if (ack.equals("?1\r")) {
          tagPresent[tagPresentPos++] = 0;
          recievedTagInfo = checkTagPresent();
          if (!recievedTagInfo) {
            Serial.print("TagInfo: ");
            Serial.println(recievedTagInfo);
          }
        }
        else {
          Serial.println("Recieved: " + ack);
          Serial.print("TagInfo: ");
          Serial.println(recievedTagInfo);
        }
        //recievedTagInfo = (buffer == "OK" ? 1 : 0) && (recievedTagInfo == 1);
        
        // Message has been dealt with, reset buffer position
        buffer_pos = 0;
        // Circle buffer position when max reached
        tagPresentPos = (tagPresentPos >= TAG_SIZE) ? 0 : tagPresentPos;
    }
  }
//
//  if (recievedTagInfo) {
//    mySerial.write("LTG\r");
//    char locate[] = {0x4C, 0x54, 0x47, 0x0D};
//    mySerial.write(locate); //4C 54 47 0D
//    Serial.println("running");
//    
//    //Serial.println("Sent command");
//  }
  
  // Send messages from Serial Montior to RFID module
  if (Serial.available()) {
    mySerial.write(Serial.read());
  }
}

boolean checkTagPresent() {
  for (int i = 0; i < TAG_SIZE; i++) {
    if (tagPresent[i]) {
      return true;
    }
  }
  Serial.println("LOST!:");
  Serial.println(tagPresent);
  return false;
}
