#include "SIM900.h"

/**
 * Constructor
 */
SIM900::SIM900(SoftwareSerial *simSerial, byte power_pin) {
    // Set software serial pointer
    SIM = simSerial;

    // Start communication
    SIM->begin(9600);
    SIM->listen();

    // Set power control pin on Arduino
    POWER_PIN = power_pin;

    // Setup variables
    // Power and timing
    poweredOn = false;       // Starts turned off as Arduino pin is low
    powerOnMillis = -1;
    // Connectivity vars
    connectedToNetwork = false;
    // Message
    messageIndex = 0;
    messageReceived = false;
}

/**
 * Destuctor
 */
SIM900::~SIM900() {
}

/**
 * Function to regularly update the SMS sending process
 */
void SIM900::update() {
    // Check if module is powered on
    if (poweredOn && powerOnMillis != -1) {
        // Get current milliseconds
        unsigned long currentMillis = millis();

        // Module is ON so update process
        if (currentMillis - powerOnMillis <= INITIAL_POWER_ON_MILLIS) {
            // Check module is connected to the network
            if (connectedToNetwork) {
                // 

                // Set flag true for sending an SMS
                dailySMSSent = true;
            }
            else {
                // Send command to check network connection
                sendATCommands("AT+CREG=1");
                sendATCommands("AT+COPS=0,2");
            }

            // Always read for a reply
            read();
        }
        // Module has elapsed power on time so turn OFF
        else if (currentMillis - powerOnMillis > INITIAL_POWER_ON_MILLIS) {
            // Turn off module
            powerDown();

            // Reset millis when module was powered on
            powerOnMillis = -1;
        }
    } // End poweredOn if
} // End update function

/**
 * Function to listen and read messages from the module
 */
boolean SIM900::read() {
    // Listen to serial port for SIM communication
    if (SIM->listen()) {
        Serial.println(F("Cannot listen to SIM!"));
    }

    while (SIM->available() && messageIndex < (MAX_SIM_MESSAGE_SIZE - 1)) {
        // Read incoming character/byte and add to raw message array
        char inByte = SIM->read();
        rawMessage[messageIndex++] = inByte;

        // Entire message has been received, '\r' means "end of message"
        if (inByte == '\r') {
            // Terminate the raw message replacing the '\r' character
            rawMessage[--messageIndex] = '\0';

            // Set that a message has been received
            messageReceived = true;

            // Message has been dealt with, reset message index
            messageIndex = 0;

            // OK
            if (rawMessage.equals("OK")) {

            }
            // ERR
            else if (rawMessage.equals("ERR")) {

            }

            return true;
        }
    }
    return false;
}

/**
 * Function to send AT commands to the module
 */
void SIM900::sendATCommands(char *message) {
    SIM->print(message);
}

boolean SIM900::getPowerStatus() {
    return poweredOn;
}

boolean SIM900::dailySMSSent() {
    return dailySMSSent;
}

/**
 * Function to power the module OFF, does nothing if pin not defined
 */
void SIM900::powerDown() {
    if (poweredOn && POWER_PIN != -1) {
        // Turn off module
        digitalWrite(POWER_PIN, LOW);

        // Set power state to off
        poweredOn = false;

        // DEBUG
        digitalWrite(LED_BUILTIN, LOW);  // DEBUG LED
        Serial.println(F("SIM powering down"));
    }
}

/**
 * Function to power the module ON, does nothing if pin not defined
 */
void SIM900::powerUp() {
    if (!poweredOn && POWER_PIN != -1) {
        // Turn on module
        digitalWrite(POWER_PIN, HIGH);

        // Set power state to on
        poweredOn = true;

        // Save time in millis when module was turned on
        powerOnMillis = millis();

        // DEBUG
        digitalWrite(LED_BUILTIN, HIGH);  // DEBUG LED
        Serial.println(F("SIM powering up"));
    }
}