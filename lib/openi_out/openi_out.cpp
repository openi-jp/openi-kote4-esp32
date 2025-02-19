/**
 * @file openi_out.cpp
 * @brief Implementation file for the openi_out class.
 */

#include "openi_out.h"

/**
 * @brief Constructor that sets the device address.
 * @param address I2C address of the device.
 */
openi_out::openi_out() {
}

/**
 * @brief Initializes the Wire library.
 */
void openi_out::begin(uint8_t address) {
    _address = address;
        if (!_mcp23008.begin_I2C(_address)) {
        Serial .print(_address);
        Serial.println("Error: Failed to initialize I2C device.");
        return;
    }
    _mcp23008.pinMode(0, OUTPUT);
    _mcp23008.pinMode(1, OUTPUT);
    _mcp23008.pinMode(2, OUTPUT);
    _mcp23008.digitalWrite(0, LOW);
    _mcp23008.digitalWrite(1, LOW);
    _mcp23008.digitalWrite(2, LOW);
}

/**
 * @brief Writes a digital value to a specified pin.
 * @param pin The pin number to write to.
 * @param val The digital value to write (HIGH or LOW).
 */
void openi_out::digitalWrite(uint8_t pin, uint8_t val) {
    _mcp23008.digitalWrite(pin, val);
}
