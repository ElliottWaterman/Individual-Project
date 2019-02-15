#include <Arduino.h>
#include "Q2HX711.h"

Q2HX711::Q2HX711(byte output_pin, byte clock_pin) {
  CLOCK_PIN  = clock_pin;
  OUT_PIN  = output_pin;
  GAIN = 1;
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
}

Q2HX711::~Q2HX711() {
}

void Q2HX711::powerDown() 
{
	digitalWrite(CLOCK_PIN, LOW);
	digitalWrite(CLOCK_PIN, HIGH);
}

void Q2HX711::powerUp() 
{
	digitalWrite(CLOCK_PIN, LOW);
}

bool Q2HX711::readyToSend() {
  return digitalRead(OUT_PIN) == LOW;
}

double Q2HX711::read() {
  // TODO: do not want to get stuck in infinite loop
  while (!readyToSend());

  // HX711 returns 24 bits of data
  byte data[3]; // 3 * 8 = 24 bits

  // For each byte shift in data by clock with MSB first
  for (byte j = 3; j--;) {
      data[j] = shiftIn(OUT_PIN, CLOCK_PIN, MSBFIRST);
  }

  // Set the channel and the gain factor for the next reading using the clock pin
  for (int i = 0; i < GAIN; i++) {
    digitalWrite(CLOCK_PIN, HIGH);
    digitalWrite(CLOCK_PIN, LOW);
  }

  // XOR with 1000 0000 (MSB). Set to 1 if data is 0, set to 0 if data is 1.
  data[2] ^= 0x80;
  
  long value = ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
  
  // Add data to average buffer
  updateAverage(value);
  
  // Return weight in grams
  if (initialZeroPosition == 0) {
	  return abs((value - average) / SCALE_FACTOR);
  }
  else {
	 return abs((value - initialZeroPosition) / SCALE_FACTOR);
  }
}

void Q2HX711::updateAverage(long incomingValue) {
	// Remove last reading from the total
	total = total - averageWeights[readIndex];
	// Update with current reading
	averageWeights[readIndex] = incomingValue;
	// Add current reading to the total
	total = total + incomingValue;

	// calculate the average:
	average = total / NUM_OF_READINGS;

	readIndex++;

	// more accurate at first use (could remove - as setup instead)
	if ((initialZeroPosition == 0) && (readIndex < NUM_OF_READINGS)) {
		average = total / readIndex;
	}

	// Reached end of array wrap to beginning index
	if (readIndex >= NUM_OF_READINGS) {
		readIndex = 0;
		// If startup average not set then set now
		if (initialZeroPosition == 0) {
			initialZeroPosition = average - ADJUST_AVERAGE; //smoothedAverage() - ADJUST_AVERAGE;
			// setup is complete
			if (setupSensor) {
				isSensorSetup = true;
			}
		}
	}
}

long Q2HX711::getAverage() {
	return average;
}

long Q2HX711::getInitialZeroPosition() {
	return initialZeroPosition;
}

long Q2HX711::getCurrentZeroPosition() {
	return currentZeroPosition;
}

void Q2HX711::updateCurrentZeroPosition() {
	
}

void Q2HX711::startSensorSetup() {
	setupSensor = true;
	isSensorSetup = false;
	// for after beginning setup
	initialZeroPosition = 0;
}

bool Q2HX711::isSetupComplete() {
	return isSensorSetup;
}

/* long Q2HX711::smoothedAverage() {
	long high = 0x000000;
	long low = 0xFFFFFF;
	for (int index = 0; index < NUM_OF_READINGS; index++) {
		if (low > averageWeights[index]) low = averageWeights[index];	//find lowest value
		if (high < averageWeights[index]) high = averageWeights[index];	//find highest value
	}
	return ((total - (low + high))/(NUM_OF_READINGS-2));	//remove lowest and highest value
} */

/* Does NOT get called as far as I know*/
/* void Q2HX711::setGain(byte gain) {
  switch (gain) {
    case 128:
      GAIN = 1;
      break;
    case 64:
      GAIN = 3;
      break;
    case 32:
      GAIN = 2;
      break;
  }

  digitalWrite(CLOCK_PIN, LOW);
  read();
} */
