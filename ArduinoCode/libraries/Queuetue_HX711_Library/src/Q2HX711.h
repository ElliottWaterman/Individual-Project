#ifndef Q2HX711_h
#define Q2HX711_h

#include "Arduino.h"
#include <Time.h>

#define NUM_OF_READINGS 16
#define SCALE_FACTOR 712.0
#define ADJUST_AVERAGE 534
// const byte NUM_OF_READINGS = 16;
// const uint16_t SCALE_FACTOR = 712;
// const uint16_t ADJUST_AVERAGE = 500;
const byte HX711_WEIGHT_BOUNDARY_TRIGGER = 25; // difference in weight to "detect" snake

class Q2HX711
{
	protected:
		byte CLOCK_PIN;
		byte OUT_PIN;
		byte GAIN;
		//void setGain(byte gain = 128);

		// Elliott
		// Unnormalised weight array vars (ie 72415)
		long averageWeights[NUM_OF_READINGS];	// Readings from the analog input
		uint8_t readIndex = 0;              	// Index of the current reading in averageWeights array
		uint32_t total = 0;                 	// Running total of the averageWeights array
		long average = 0;                		// Average of the averageWeights array
		long initialZeroPosition = 0;			// Average weight at start up
		long currentZeroPosition = 0;			// Current zero position (opposite of tare weight; with added weight)
		
		// Normalised weight vars (ie 101.71 grams)
		double currentWeight;
		double previousWeight;
		boolean weightDetected;

		// Setup vars
		bool setupSensor = false;				// flag for sensor to be setup
		bool isSensorSetup = false;				// flag for startup average completion

		// Timing variables
		unsigned long previousMillis;
		unsigned long millisInterval;

		// Functions
		void updateAverage(long incomingData);	// internal func to calc average
		//long smoothedAverage();					// remove top and bottom value

	public:
		Q2HX711(byte output_pin, byte clock_pin);	// constructor
		virtual ~Q2HX711();							// destructor
		bool readyToSend();
		double read();

		// Elliott
		// Usage funcs
		void update();
		boolean getWeightDetected();
		void resetWeightDetected();

		// Setup funcs
		void startSensorSetup();
		bool isSetupComplete();

		// Power funcs
		void powerDown();			// not sure if work
		void powerUp();				// not sure if work (will read anyway)

		// TODO: are functions needed
		long getAverage();
		long getInitialZeroPosition();
		long getCurrentZeroPosition();
		void updateCurrentZeroPosition();
};

#endif /* Q2HX711_h */
