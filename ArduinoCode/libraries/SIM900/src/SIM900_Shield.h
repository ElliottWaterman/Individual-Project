#ifndef SIM900_SHIELD_h
#define SIM900_SHIELD_h

#include <Arduino.h>
#include <SoftwareSerial.h>


/* CONSTANTS */
const byte MAX_SIM_MESSAGE_SIZE = 30;               // Size of receiving messages buffer
const unsigned long SIM_POWER_ON_MILLIS = 300000;   // Number of milliseconds to stay powered on (5 minutes)
const unsigned long CHECK_NETWORK_MILLIS = 10000;   // Number of milliseconds between checking network registration status


/* DEFINES */
#define CARRIAGE_RETURN             '\r'

// Connectivity commands
#define TEST_NETWORK_REGISTRATION   "AT+CREG?"

// Text message commands
#define TEXT_MODE                   "AT+CMGF=1"
#define TWILIO_PHONE_NUMBER         "AT+CMGS=\"+441233800093\""

// Replies
#define OK                          "OK"
#define ERROR                       "ERROR"
#define NUMBER_OF_MESSAGES          "+CMGS:"
#define HOME_NETWORK_REGISTERED     "+CREG: 1"
#define ROAMING_NETWORK_REGISTERED  "+CREG: 5"


class SIM900
{
	public:
        // Constructor and Destructor
        SIM900(SoftwareSerial *simSerial, byte power_pin);
        virtual ~SIM900();

        // Usage funcs
        void update();
        void sendATCommands(const char *&message);
        void sendATCommands(char *message);
        void sendATCommands(char character);
        boolean isTextMessageBodyReady();
        void resetTextMessageBodyReady();
        boolean wasTextMessageSent();
        void resetTextMessageSent();

        // Power funcs
        void powerDown();
        void powerUp();
        boolean getPowerStatus();
        
    protected:
        // Functions
        void read();

        SoftwareSerial *SIM;    // Serial communication by simulating serial on Arduino pins (pointer to SoftSerial in SBSBS)

        // Pin vars
        byte POWER_PIN = -1;    // Using BJT/MOSFET as a switch to turn module on and off, saves battery power

        // Power and timing vars
        boolean poweredOn;              // Hold the state of module power, on (true) or off (false)
        unsigned long powerOnMillis;    // Time in milliseconds when the module was powered on
        unsigned long lastNetworkCheck; // Last time in milliseconds when the network registration was checked

        // Connectivity vars
        boolean connectedToNetwork;

        // Text message vars
        boolean textMessageBodyReady;
        boolean textMessageSent;

        // Message vars
        char rawMessage[MAX_SIM_MESSAGE_SIZE];  // Character array holding incoming bytes from module
        byte messageIndex;                      // Index of the next character to place
        boolean messageReceived;                // Flag to tell when a message was received
};

#endif