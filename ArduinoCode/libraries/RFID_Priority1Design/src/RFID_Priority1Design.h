#ifndef RFID_P1D_h
#define RFID_P1D_h

#include <Arduino.h>
#include <SoftwareSerial.h>

const byte MAX_RFID_MESSAGE_SIZE = 34;  // Maximum RFID tag size is 33, normal is 17

const unsigned long INITIAL_POWER_ON_MILLIS = 30000;        // Number of milliseconds to stay powered on

class RFID_P1D
{
	public:
        // Constructor and Destructor
        RFID_P1D(SoftwareSerial *rfidSerial, byte power_pin);
		virtual ~RFID_P1D();

        // Usage funcs
        void update();
        boolean readMessage();
        String getMessage();

        // Tag read
        boolean hasTagBeenRead();
        void resetTagRead();
        boolean isFirstTagSincePowerUp();
        void resetFirstTagSincePowerUp();

        // Power funcs
        void powerDown();
        void powerUp();
        boolean getPowerStatus();
        
    protected:
        SoftwareSerial *RFID;   // Serial communication by simulating serial on Arduino pins (pointer to SoftSerial in SBSBS)

        // Pin vars
        byte POWER_PIN = -1;         // Using BJT/MOSFET as a switch to turn module on and off, saves battery power

        // Power and timing vars
        boolean poweredOn;                  // Hold the state of module power, on (true) or off (false)
        unsigned long powerOnMillis;        // Time in milliseconds when module was powered on
        unsigned long totalPowerOnMillis;   // Total number of milliseconds to run, to be powered on

        // Message vars
        char rawMessage[MAX_RFID_MESSAGE_SIZE];     // Character array holding incoming bytes from module
        byte messageIndex;                          // Index of the next character to place
        String message;                             // A string representation of the raw message array
        boolean tagRead;                            // Flag to tell when a tag was read

        // Utility vars
        bool firstTagSincePowerUp = false;
};

#endif