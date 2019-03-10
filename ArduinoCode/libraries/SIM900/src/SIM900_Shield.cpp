#include "SIM900_Shield.h"

/**
 * Constructor
 */
SIM900::SIM900(SoftwareSerial *simSerial, byte power_pin) {
    // Set software serial pointer
    SIM = simSerial;

    // Start communication
    SIM->begin(19200);
    if (SIM->listen()) {
        Serial.println(F("Cannot listen SIM"));
    }
    SIM->listen();

    // Set power control pin on Arduino
    POWER_PIN = power_pin;

    // Reset/Setup variables
    resetVariables();
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
            // Always read for a reply
            read();
            
            // Check module is connected to the network
            if (!connectedToNetwork) {
                // Test every 10 seconds for network registration 
                if (currentMillis - lastNetworkCheck > CHECK_NETWORK_MILLIS) {
                    // Send network registration check
                    sendATCommands(TEST_NETWORK_REGISTRATION);
                    sendATCommands(CARRIAGE_RETURN);

                    Serial.println(F("Testing network"));

                    // Update last checked millis
                    lastNetworkCheck = currentMillis;
                }
            }
        }
        // Module has elapsed power on time so turn OFF
        else if (currentMillis - powerOnMillis > SIM_POWER_ON_MILLIS) {
            // Turn off module
            powerDown();
        }
    } // End poweredOn if
} // End update function

/**
 * Function to read and read messages from the module
 */
void SIM900::read() {
    // Listen to serial port for SIM communication
    SIM->listen();
    if (!SIM->isListening()) {
        Serial.println(F("Not listening to SIM!"));
    }

    while (SIM->available() && messageIndex < (MAX_SIM_MESSAGE_SIZE - 1)) {
        // Read incoming character/byte and add to raw message array
        char inByte = SIM->read();
        rawMessage[messageIndex++] = inByte;

        // Entire message has been received, \r and \n mean "end of message"
        if (inByte == '\r' || inByte == '\n' || inByte == '>') {
            // Terminate the raw message replacing both the '\r' and '\n' characters
            if (inByte != '>') {
                messageIndex--;
            }
            rawMessage[messageIndex] = '\0';

            Serial.println(rawMessage);

            // Message has been dealt with, reset message index
            messageIndex = 0;

            // Compare received message to list of expected values
            if (strcmp(rawMessage, OK) == 0) {
                Serial.println(F("Hit OK"));
                // Only set response if a command was sent
                if (commandSent) {
                    okReceived = true;
                }
            }
            else if (strcmp(rawMessage, ERROR) == 0) {
                Serial.println(F("Hit ERROR"));
                // Only set response if a command was sent
                if (commandSent) {
                    errorReceived = true;
                }
            }
            else if (strcmp(rawMessage, WAITING_FOR_TEXT) == 0) {
                Serial.println(F("Hit WAITING_FOR_TEXT"));
                // Only set response if a command was sent
                if (commandSent) {
                    readyForEnteringText = true;
                }
            }
            
            // Always check for the number of text messages sent reply
            if (strncmp(rawMessage, NUMBER_OF_MESSAGES, 6) == 0) {
                textMessageSent = true;
                Serial.println(F("TXT sent"));
            }

            // Always check for network connection status updates
            if (strcmp(rawMessage, HOME_NETWORK_REGISTERED) == 0) {
                connectedToNetwork = true;
                Serial.println(F("Connected to network home"));
            }
            else if (strcmp(rawMessage, ROAMING_NETWORK_REGISTERED) == 0) {
                connectedToNetwork = true;
                Serial.println(F("Connected to network roaming"));
            }
            else if (strcmp(rawMessage, UNSOLICITED_NETWORK_REGISTERED) == 0) {
                connectedToNetwork = true;
                Serial.println(F("Connected to network, unsolicited reply"));
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

/**
 * Function to send the Twilio phone number to the module to start a text message
 */
void SIM900::sendTwilioPhoneNumber() {
    sendATCommands(TWILIO_PHONE_NUMBER);
    sendATCommands(CARRIAGE_RETURN);
    commandSent = true;
}

/**
 * Function to end a text message and send it
 */
void SIM900::sendEndOfTextMessage() {
    sendATCommands(END_OF_MESSAGE);
    sendATCommands(CARRIAGE_RETURN);
    commandSent = true;
}

/**
 * Function to get and reset whether a text message has been sent
 */
boolean SIM900::wasTextMessageSent() {
    return textMessageSent;
}
void SIM900::resetTextMessageSent() {
    textMessageSent = false;
}

/**
 * Function to get and reset whether the module is ready to enter body text
 */
boolean SIM900::isReadyForEnteringText() {
    return readyForEnteringText;
}
void SIM900::resetReadyForEnteringText() {
    readyForEnteringText = false;
}

boolean SIM900::isConnectedToNetwork() {
    return connectedToNetwork;
}

boolean SIM900::getPowerStatus() {
    return poweredOn;
}

/**
 * Function to set all the member variables to initial values
 */
void SIM900::resetVariables() {
    // Power and timing
    poweredOn = false;       // Starts turned off as Arduino pin is low
    powerOnMillis = -1;
    lastNetworkCheck = 0;

    // Connectivity and response vars
    connectedToNetwork = false;
    okReceived = false;
    errorReceived = false;
    commandSent = false;

    // Text message vars
    textMessageSent = false;
    readyForEnteringText = false;

    // Message
    messageIndex = 0;
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

        // Reset all member variables
        resetVariables();

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



// TODO: Remove or change back
/* 
// Check commands are sent and update if response received
if (commandSent) {
    if (!textModeSet) {
        if (okReceived) {
            okReceived = false;
            commandSent = false;
            textModeSet = true;
        }
    }
    else if (!phoneNumberSet) {
        if (okReceived) {
            // No response after phone number is set apart from >
            okReceived = false;
            commandSent = false;
            phoneNumberSet = true;

            // Set flag ready for typing text message
            textMessageBodyReady = true;
        }
    }
    
    // Always check for an error
    if (errorReceived) {
        // Resend commands if error received
        // TODO: Continual fail?
        //commandSent = false;
    }
}   // End commandSent

if (!textMessageBodyReady) {
    if (!commandSent && !textModeSet) {
        // Setup text mode on the module
        sendATCommands(TEXT_MODE);
        sendATCommands(CARRIAGE_RETURN);
        Serial.println(F("Text mode sent"));

        commandSent = true;
    }
    else if (!commandSent && !phoneNumberSet && textModeSet) {
        // Set phone number to send text to
        sendATCommands(TWILIO_PHONE_NUMBER);
        sendATCommands(CARRIAGE_RETURN);
        Serial.println(F("Twilio phone number sent"));

        commandSent = true;
    }


}   // End textMessageBodyReady
*/