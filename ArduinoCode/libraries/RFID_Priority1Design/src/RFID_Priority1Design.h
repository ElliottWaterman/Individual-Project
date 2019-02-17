#ifndef RFID_P1D_h
#define RFID_P1D_h

#include <Arduino.h>

#define MAX_RFID_MESSAGE_SIZE       48

const unsigned long INITIAL_POWER_ON_MILLIS = 30000;

class RFID_P1D
{
	public:
        // Constructor and Destructor
        RFID_P1D(byte rx_pin, byte tx_pin, byte power_pin);
		virtual ~Q2HX711();

        // Usage funcs
        void update();
        boolean readMessage();

        // Tag read
        boolean hasTagBeenRead();
        void resetTagRead();

        // Power funcs
        void powerDown();
        void powerUp();
        
    protected:
        SoftwareSerial RFID;    // Serial communication by simulating serial on Arduino pins 

        // Pin vars
        byte RX_PIN;            // Receiving pin on Arduino (Use transmitting pin on RFID module)
        byte TX_PIN;            // Transmitting pin on Arduino (Use receiving pin on RFID module)
        byte POWER_PIN;         // Using BJT/MOSFET as a switch to turn module on and off, saves battery power

        // Power and timing vars
        boolean poweredOn;      // Hold the state of module power, on (true) or off (false)
        unsigned long powerOnMillis;        // Time in milliseconds when module was powered on
        unsigned long totalPowerOnMillis;   // Total number of milliseconds to run, to be powered on

        // Message vars
        char rawMessage[MAX_RFID_MESSAGE_SIZE];
        byte messageIndex;
        String message;
        boolean tagRead;
};

#endif