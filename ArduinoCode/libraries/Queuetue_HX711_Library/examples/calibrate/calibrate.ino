#include <Q2HX711.h>

const byte hx711_data_pin = A2;
const byte hx711_clock_pin = A3;

Q2HX711 HX711(hx711_data_pin, hx711_clock_pin);

void setup() {
  Serial.begin(9600);

  // Initialise weight sensor
  Serial.println("Initialising weight sensor");
  HX711.startSensorSetup();
  // Read from sensor until averaging array fills up
  while (!HX711.isSetupComplete()) {
    HX711.read();
  }
  Serial.print("Initial Zero Long: ");
  Serial.println(HX711.getInitialZero());
  Serial.print("Initial Zero Weight: ");
  Serial.println(HX711.getInitialZero() / 712.0);
}

double scale = 712.0;
char incoming[10];
short index = 0;

void loop() {
  HX711.update();

  Serial.print("Current weight: ");
  Serial.println(HX711.getCurrentWeight());

  Serial.print("Test weight: ");
  Serial.println(HX711.read() * 712.0 / scale);

  // Set new scale from serial monitor input
  while (Serial.available() > 0) {
    byte inByte = Serial.read();

    if (inByte != '\r') {
      incoming[index++] = inByte;

      if (index >= 10) {
        index = 0;
      }
    }
    else {
      incoming[index++] = '\0';
      index = 0;
      Serial.println("");
      Serial.print("Highest Weight: ");
      Serial.println(HX711.getHighestDetectedWeight());
      HX711.resetHighestDetectedWeight();
      
      Serial.println(incoming);
      scale = atof(incoming);
      Serial.println(scale);
    }
  }
  

  delay(800);
}
