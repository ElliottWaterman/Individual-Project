#include "Q2HX711.h"

/**
 * Constructor
 */
Q2HX711::Q2HX711(byte output_pin, byte clock_pin) {
  CLOCK_PIN  = clock_pin;
  OUT_PIN  = output_pin;
  GAIN = 1;
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(OUT_PIN, INPUT);
}

/**
 * Destuctor
 */
Q2HX711::~Q2HX711() {
}

/**
 * Function to regularly read sensor and detect changes in weight
 */
void Q2HX711::update() {
	// Get current milliseconds
	unsigned long currentMillis = millis();

	if (currentMillis - previousMillis >= millisInterval) {
		if (readyToSend()) {
      // Set old weight to compare with new weight
      previousWeight = currentWeight;

      // Check current weight for snake entering basking station
      read();

      // Update the time the sensor was checked
			previousMillis = currentMillis;

      // Current weight is greater than previous weight by x amount
      double weightDifference = normaliseLongToWeight(currentWeight) - normaliseLongToWeight(previousWeight);
      if (weightDifference > HX711_WEIGHT_BOUNDARY_TRIGGER) {
				weightDetected = true;
      }
      else if (weightDifference < -HX711_WEIGHT_BOUNDARY_TRIGGER) {
        // Weight is decreasing.. TODO?
      }
    } // End readyToSend function
	} // End regular time update
} // End update function

/**
 * Function to read the current value from the weight sensor
 */
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
  
  currentWeight = ((uint32_t) data[2] << 16) | ((uint32_t) data[1] << 8) | (uint32_t) data[0];
  
  // Add data to average buffer
  updateAverage(currentWeight);
  
  return normaliseLongToWeight(currentWeight);
}

/**
 * Function to update the averaging array with a new reading 
 * and calculate the total and average of the array
 */
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

	// More accurate at first use (could remove - as setup instead)
	if ((initialZeroPosition == 0) && (readIndex < NUM_OF_READINGS)) {
		average = total / readIndex;
	}

	// Reached end of array, wrap index back to beginning
	if (readIndex >= NUM_OF_READINGS) {
		readIndex = 0;
		// If startup average not set then set now
		if (initialZeroPosition == 0) {
			initialZeroPosition = average - ADJUST_AVERAGE; //smoothedAverage() - ADJUST_AVERAGE;
			// setup is complete
			if (setupSensor) {
				isSensorSetup = true;
			}
		} // End set initialZeroPosition if
	} // End wrap index if
} // End updateAverage function

/**
 * Function to convert a Long into normalised weight in grams
 */
double Q2HX711::normaliseLongToWeight(long reading) {
  // Check initial zero has been set
  if (initialZeroPosition == 0) {
	  return abs((reading - average) / SCALE_FACTOR);
  }
  else {
	 return abs((reading - initialZeroPosition) / SCALE_FACTOR);
  }
}

double Q2HX711::getCurrentWeight() {
  return normaliseLongToWeight(currentWeight);
}

boolean Q2HX711::getWeightDetected() {
	return weightDetected;
}

void Q2HX711::resetWeightDetected() {
	weightDetected = false;
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
