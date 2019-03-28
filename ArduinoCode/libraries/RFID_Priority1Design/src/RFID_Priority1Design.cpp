#include "RFID_Priority1Design.h"

/**
 * Constructor
 */
RFID_P1D::RFID_P1D(SoftwareSerial *rfidSerial, byte power_pin) {
    // Set software serial pointer
    RFID = rfidSerial;

    // Start communication
    RFID->begin(9600);
    RFID->listen();

    // Set power control pin on Arduino
    POWER_PIN = power_pin;

    // Set power pin as an output so digital write works
    pinMode(POWER_PIN, OUTPUT);

    // TODO: Could set reader active here (SRA<crn>)
    //RFID->write("SRA" + (char)13);

    // Setup variables
    // Power and timing
    poweredOn = false;       // Starts turned off as Arduino pin is low
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
 * Function to regularly read messages from the module
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
                Serial.println(F("RFID tag read!"));

                // Only extend once the power on time
                if (totalPowerOnMillis == INITIAL_POWER_ON_MILLIS) {
                    // Increase power on millis by elapsed time
                    totalPowerOnMillis += (currentMillis - powerOnMillis);

                    // Serial.print(F("Extended power on time: "));
                    // Serial.print(totalPowerOnMillis / 1000);
                    // Serial.println(F(" seconds."));
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
    //RFID->listen();
    if (RFID->listen()) {
        Serial.println(F("Cannot listen to RFID!"));
    }

    while (RFID->available() && messageIndex < (MAX_RFID_MESSAGE_SIZE - 1)) {
        // Read incoming character/byte and add to raw message array
        char inByte = RFID->read();
        rawMessage[messageIndex++] = inByte;

        // Entire message has been received, '\r' means "end of message"
        if (inByte == '\r') {
            // Terminate the raw message replacing the '\r' character
            rawMessage[--messageIndex] = '\0';

            // If long enough a tag was read
            if (messageIndex >= 16) {
                // Set that a tag has been read
                tagRead = true;

                // Assign string object from character array
                message = String(rawMessage);
            }

            // Always print the incoming message
            Serial.println(rawMessage);

            // Message has been dealt with, reset message index
            messageIndex = 0;

            return true;
        }
    }
    return false;
}

String RFID_P1D::getMessage() {
    return message;
}

boolean RFID_P1D::hasTagBeenRead() {
	return tagRead;
}

void RFID_P1D::resetTagRead() {
	tagRead = false;
}

boolean RFID_P1D::isFirstTagSincePowerUp() {
    return firstTagSincePowerUp;
}

void RFID_P1D::resetFirstTagSincePowerUp() {
    firstTagSincePowerUp = false;
}

boolean RFID_P1D::getPowerStatus() {
    return poweredOn;
}

/**
 * Function to power the module OFF, does nothing if pin not defined
 */
void RFID_P1D::powerDown() {
    if (poweredOn && POWER_PIN != -1) {
        // Turn off module
        digitalWrite(POWER_PIN, LOW);
        // Set power state to off
        poweredOn = false;

        // DEBUG
        digitalWrite(LED_BUILTIN, LOW);  // DEBUG LED
        Serial.println(F("RFID powering down"));
    }
}

/**
 * Function to power the module ON, does nothing if pin not defined
 */
void RFID_P1D::powerUp() {
    if (!poweredOn && POWER_PIN != -1) {
        // Turn on module
        digitalWrite(POWER_PIN, HIGH);

        // Set power state to on
        poweredOn = true;

        // Save time in millis when module was turned on
        powerOnMillis = millis();

        // Set flag for reading first tag after starting
        firstTagSincePowerUp = true;

        // DEBUG
        digitalWrite(LED_BUILTIN, HIGH);  // DEBUG LED
        Serial.println(F("RFID powering up"));
    }
}