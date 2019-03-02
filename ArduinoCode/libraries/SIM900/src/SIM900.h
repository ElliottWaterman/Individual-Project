#ifndef SIM900_h
#define SIM900_h

#include <Arduino.h>
#include <SoftwareSerial.h>

const byte MAX_SIM_MESSAGE_SIZE = 34;

const unsigned long INITIAL_POWER_ON_MILLIS = 300000;   // Number of milliseconds to stay powered on (5 minutes)

// Text message commands
const char *TEXT_MODE = "AT+CMGF=1";
const char *PHONE_NUMBER = "AT+CMGS=\"+441233800093\"";
const char *END_MESSAGE_CHAR = char(26);
const char *NUMBER_OF_MESSAGES = "+CMGS:";

// Replies
const char *OK = "OK";
const char *ERR = "ERR";


class SIM900
{
	public:
        // Constructor and Destructor
        SIM900(SoftwareSerial *simSerial, byte power_pin);
		virtual ~SIM900();

        // Usage funcs
        void update();
        boolean read();
        void sendATCommands();
        boolean dailySMSSent();

        // Power funcs
        void powerDown();
        void powerUp();
        boolean getPowerStatus();
        
    protected:
        SoftwareSerial *SIM;   // Serial communication by simulating serial on Arduino pins (pointer to SoftSerial in SBSBS)

        // Pin vars
        byte POWER_PIN = -1;         // Using BJT/MOSFET as a switch to turn module on and off, saves battery power

        // Power and timing vars
        boolean poweredOn;                  // Hold the state of module power, on (true) or off (false)
        unsigned long powerOnMillis;        // Time in milliseconds when the module was powered on

        // Connectivity vars
        boolean connectedToNetwork;

        // Message vars
        char rawMessage[MAX_RFID_MESSAGE_SIZE];     // Character array holding incoming bytes from module
        byte messageIndex;                          // Index of the next character to place
        boolean messageReceived;                    // Flag to tell when a message was received

        // vars
        boolean dailySMSSent = false;
};

#endif