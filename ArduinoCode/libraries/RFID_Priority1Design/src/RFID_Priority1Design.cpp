#include "RFID_Priority1Design.h"
#include <Arduino.h>

/**
 * Constructor
 */
RFID_P1D::RFID_P1D(byte rx_pin, byte tx_pin, byte power_pin) {
    RX_PIN = rx_pin;
    TX_PIN = tx_pin;
    POWER_PIN = power_pin;
    RFID = new SoftwareSerial(RX_PIN, TX_PIN);
    RFID.begin(9600);

    // TODO: Could set reader active here (SRA<crn>)

    // Setup variables
    // Power and timing
    poweredOn = true;       // Starts turned on
    powerOnMillis = -1;
    totalPowerOnMillis = INITIAL_POWER_ON_MILLIS;
    // Message
    messageIndex = 0;
    tagRead = false;
}

/**
 * Destuctor
 */
RFID_P1D::~RFID_P1D() {
}

/**
 * Function to regularly read message from RFID module
 */
void RFID_P1D::update() {
    // Check if module is powered on
    if (poweredOn && powerOnMillis != -1) {
        // Get current milliseconds
        unsigned long currentMillis = millis();

        // Module is ON so read tags
        if (currentMillis - powerOnMillis <= totalPowerOnMillis) {
            // Message has been read so keep module on for longer
            if (readMessage()) {
                // Only extend once the power on time
                if (totalPowerOnMillis == INITIAL_POWER_ON_MILLIS) {
                    // Increase power on millis by elapsed time
                    totalPowerOnMillis += (currentMillis - powerOnMillis);
                }
            }
        }
        // Module has elapsed power on time so turn OFF
        else if (currentMillis - powerOnMillis > totalPowerOnMillis) {
            // Turn off module
            powerDown();

            // Reset millis when module was powered on
            powerOnMillis = -1;

            // Reset total power on run time
            totalPowerOnMillis = INITIAL_POWER_ON_MILLIS;
        }
    } // End poweredOn if
} // End update function

/**
 * Function to listen and read in an RFID tag from software serial
 */
boolean RFID_P1D::readMessage() {
    // Listen to serial port for RFID communication
    RFID.listen();
    while (RFID.available() && messageIndex < (MAX_RFID_MESSAGE_SIZE - 1)) {
        // Read incoming character/byte and add to raw message array
        char inByte = RFID.read();
        rawMessage[messageIndex++] = inByte;

        // Entire message has been received, '\r' means "end of message"
        if (inByte == '\r') {
            // Terminate the raw message replacing the '\r' character
            rawMessage[--messageIndex] = '\0';

            // Assign string object fom character array
            message = String(rawMessage);

            // Set that a tag has been read
            tagRead = true;

            // Message has been dealt with, reset message index
            messageIndex = 0;

            return true;
        }
    }
    return false;
}

boolean RFID_P1D::hasTagBeenRead() {
	return tagRead;
}

void RFID_P1D::resetTagRead() {
	tagRead = false;
}

/**
 * Function to power the module OFF, does nothing if pin not defined
 */
void RFID_P1D::powerDown() {
    if (poweredOn && POWER_PIN != null) {
        // Turn off module
        digitalWrite(POWER_PIN, LOW);
        // Set power state to off
        poweredOn = false;

        digitalWrite(LED_BUILTIN, LOW);  // DEBUG LED
    }
}

/**
 * Function to power the module ON, does nothing if pin not defined
 */
void RFID_P1D::powerUp() {
    if (!poweredOn && POWER_PIN != null) {
        // Turn on module
        digitalWrite(POWER_PIN, HIGH);
        // Set power state to on
        poweredOn = true;
        // Save time in millis when module was turned on
        powerOnMillis = millis();

        digitalWrite(LED_BUILTIN, HIGH);  // DEBUG LED
    }
}