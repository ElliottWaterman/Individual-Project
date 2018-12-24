#include <Q2HX711.h>

const byte HX711_DATA_PIN = A2;
const byte HX711_CLOCK_PIN = A3;

Q2HX711 HX711(HX711_DATA_PIN, HX711_CLOCK_PIN);

double output = 0;
long t;

void setup() {
  Serial.begin(9600);
  
  Serial.println("Starting setup");
  HX711.startSensorSetup();
}

void loop() {
  // Start weight sensor setup
  if (!HX711.isSetupComplete()) {
    HX711.read(); // fill up averaging array
  }

  if (millis() > t + 500) {
    if (HX711.readyToSend()) {
      Serial.print(HX711.read());
      Serial.println(" grams");
      t = millis();
    }
  }
}

