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
    // Text message vars
    textMessageBodyReady = false;
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
        if (currentMillis - powerOnMillis <= SIM_POWER_ON_MILLIS) {
            // Check module is connected to the network
            if (connectedToNetwork) {
                if (!textMessageBodyReady) {
                    // Setup text mode on the module
                    sendATCommands(TEXT_MODE);
                    sendATCommands(CARRIAGE_RETURN);

                    // Set phone number to send text to
                    sendATCommands(PHONE_NUMBER);
                    sendATCommands(CARRIAGE_RETURN);

                    // Set flag ready for typing text message
                    textMessageBodyReady = true;
                }
            }
            // Send commands to test network registration
            else {
                // Test every 10 seconds for network registration 
                if (currentMillis - lastNetworkCheck <= CHECK_NETWORK_MILLIS) {
                    // Send network registration check
                    sendATCommands(TEST_NETWORK_REGISTRATION);

                    // Update last checked millis
                    lastNetworkCheck = currentMillis;
                }
            }

            // Always read for a reply
            read();
        }
        // Module has elapsed power on time so turn OFF
        else if (currentMillis - powerOnMillis > SIM_POWER_ON_MILLIS) {
            // Turn off module
            powerDown();

            // Reset millis when module was powered on
            powerOnMillis = -1;
        }
    } // End poweredOn if
} // End update function

/**
 * Function to read and read messages from the module
 */
void SIM900::read() {
    // Listen to serial port for SIM communication
    if (SIM->listen()) {
        Serial.println(F("Cannot listen to SIM!"));
    }

    while (SIM->available() && messageIndex < (MAX_SIM_MESSAGE_SIZE - 1)) {
        // Read incoming character/byte and add to raw message array
        char inByte = SIM->read();
        rawMessage[messageIndex++] = inByte;

        // Entire message has been received, \r and \n mean "end of message"
        if (inByte == '\r' || inByte == '\n') {
            // Terminate the raw message replacing both the '\r' and '\n' characters
            messageIndex--;
            rawMessage[messageIndex] = '\0';

            Serial.println(rawMessage);
            
            // Set that a message has been received
            messageReceived = true;

            // Message has been dealt with, reset message index
            messageIndex = 0;

            // Compare received message to list of expected values
            if (strcmp(rawMessage, OK) == 0) {
                Serial.println(F("Hit OK"));
            }
            else if (strcmp(rawMessage, ERROR) == 0) {
                Serial.println(F("Hit ERROR"));
            }
            // Ignore reply of the number of text messages sent
            else if (strncmp(rawMessage, NUMBER_OF_MESSAGES, 6) == 0) {

            }

            // Always check for network connection status updates
            if (strcmp(rawMessage, HOME_NETWORK_REGISTERED) == 0) {
                connectedToNetwork = true;
                Serial.println(F("Connected to network"));
            }
            else if (strcmp(rawMessage, ROAMING_NETWORK_REGISTERED) == 0) {
                connectedToNetwork = true;
                Serial.println(F("Connected to network roaming"));
            }
        }
    }
}

/**
 * Function to send AT commands to the module
 */
void SIM900::sendATCommands(const char *&message) {
    SIM->print(message);
}
void SIM900::sendATCommands(char *message) {
    SIM->print(message);
}
void SIM900::sendATCommands(char character) {
    SIM->print(character);
}

boolean SIM900::getPowerStatus() {
    return poweredOn;
}

boolean SIM900::isTextMessageBodyReady() {
    return textMessageBodyReady;
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