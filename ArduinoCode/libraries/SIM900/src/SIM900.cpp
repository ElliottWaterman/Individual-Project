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

    // TODO: Could set reader active here (SRA<crn>)
    //SIM->write("SRA" + (char)13);

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
        if (currentMillis - powerOnMillis <= totalPowerOnMillis) {
            // Check module is connected to the network
            if (connectedToNetwork) {
                //T
                read();
            }
            else {
                // Send command to check network connection
                sendATCommands("AT+...");
            }
        }
        // Module has elapsed power on time so turn OFF
        else if (currentMillis - powerOnMillis > totalPowerOnMillis) {
            // Turn off module
            powerDown();

            // Reset millis when module was powered on
            powerOnMillis = -1;

            // Reset total power on run time
            //totalPowerOnMillis = INITIAL_POWER_ON_MILLIS;
            // TODO if static on time remove totalPowerOnMillis var and replace with constant
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


String SIM900::getMessage() {
    return message;
}

boolean SIM900::hasTagBeenRead() {
	return tagRead;
}

void SIM900::resetTagRead() {
	tagRead = false;
}

boolean SIM900::isFirstTagSincePowerUp() {
    return firstTagSincePowerUp;
}

void SIM900::resetFirstTagSincePowerUp() {
    firstTagSincePowerUp = false;
}

boolean SIM900::getPowerStatus() {
    return poweredOn;
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

        // Set flag for reading first tag after starting
        firstTagSincePowerUp = true;

        // DEBUG
        digitalWrite(LED_BUILTIN, HIGH);  // DEBUG LED
        Serial.println(F("SIM powering up"));
    }
}