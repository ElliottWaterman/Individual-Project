#ifndef Q2HX711_h
#define Q2HX711_h
#include "Arduino.h"

// const byte NUM_OF_READINGS = 16;
// const uint16_t SCALE_FACTOR = 712;
// const uint16_t ADJUST_AVERAGE = 500;
#define NUM_OF_READINGS 16
#define SCALE_FACTOR 712.0
#define ADJUST_AVERAGE 534

class Q2HX711
{
	protected:
		byte CLOCK_PIN;
		byte OUT_PIN;
		byte GAIN;
		//void setGain(byte gain = 128);

		// Elliott
		long averageWeights[NUM_OF_READINGS];	// the readings from the analog input
		uint8_t readIndex = 0;              	// the index of the current reading
		uint32_t total = 0;                 	// the running total
		long average = 0;                		// the average
		long initialZeroPosition = 0;			// the average at start up
		long currentZeroPosition = 0;			// the current zero position (opposite of tare weight; with added weight)
		
		bool setupSensor = false;				// flag for sensor to be setup
		bool isSensorSetup = false;				// flag for startup average completion
		void updateAverage(long incomingData);	// internal func to calc average
		//long smoothedAverage();					// remove top and bottom value

	public:
		Q2HX711(byte output_pin, byte clock_pin);	// constructor
		virtual ~Q2HX711();							// destructor
		bool readyToSend();
		double read();
		
		// Elliott
		void powerDown();			// not sure if work
		void powerUp();				// not sure if work (will read anyway)
		long getAverage();
		long getInitialZeroPosition();
		long getCurrentZeroPosition();
		void updateCurrentZeroPosition();
		
		void startSensorSetup();
		bool isSetupComplete();
};

#endif /* Q2HX711_h */
