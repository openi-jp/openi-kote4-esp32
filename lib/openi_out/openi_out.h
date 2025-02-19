#ifndef OPENI_OUT_H
#define OPENI_OUT_H

#include <Arduino.h>
#include <Adafruit_MCP23X08.h>

/**
 * @brief openi_out class
 */
class openi_out {
public:
    explicit openi_out();
    void begin(uint8_t address);
    void digitalWrite(uint8_t pin, uint8_t val);

private:
    uint8_t _address;
    Adafruit_MCP23X08 _mcp23008;
};

#endif