#include "openi_in.h"

openi_in::openi_in() {
    _lastDigitalRead[0] = LOW;
    _lastDigitalRead[1] = LOW;
    _lastDigitalRead[2] = LOW;
    _changed[0] = false;
    _changed[1] = false;
    _changed[2] = false;
}

void openi_in::begin(uint8_t address) {
    _address = address;
    _mcp23008.begin_I2C(_address);
    _mcp23008.pinMode(0, INPUT);
    _mcp23008.pinMode(1, INPUT);
    _mcp23008.pinMode(2, INPUT);
}
/**
 * @brief Reads a digital value from a specified pin.
 * @param pin The pin number to read from.
 * @return The digital value (HIGH or LOW).
 */
uint8_t openi_in::digitalRead(uint8_t pin) {
    // 現在のピンの状態を読み取る
    uint8_t currentState = _mcp23008.digitalRead(pin);

    // 状態が変わった場合、_lastDigitalReadを更新し、_changedをtrueにする
    if (currentState != _lastDigitalRead[pin]) {
        _lastDigitalRead[pin] = currentState;
        _changed[pin] = true;
    } else {
        _changed[pin] = false;
    }

    return currentState;
}

/**
 * @brief Checks if the digital value of a specified pin has changed.
 * @param pin The pin number to check.
 * @return True if the value has changed, false otherwise.
 */
bool openi_in::isChanged(uint8_t pin) {
    return _changed[pin];
}
