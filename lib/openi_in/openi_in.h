#ifndef OPENI_IN_H
#define OPENI_IN_H

#include <Arduino.h>
#include <Adafruit_MCP23X08.h>

/**
 * @brief The openi_in class
 */
class openi_in {
public:
    explicit openi_in(); // Constructor
    void begin(uint8_t address); // Initialize the device
    uint8_t digitalRead(uint8_t pin); // Read digital value from pin
    bool isChanged(uint8_t pin); // Check if the digital value has changed

private:
    uint8_t _address;
    Adafruit_MCP23X08 _mcp23008;
    uint8_t _lastDigitalRead[3];
    bool _changed[3];
};

#endif
