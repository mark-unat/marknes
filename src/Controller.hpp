#pragma once

#include <cstdint>

#include "IDevice.hpp"

enum class ControllerButton {
    Right = 0,
    Left,
    Down,
    Up,
    Start,
    Select,
    B,
    A
};

class Controller : public IDevice {
public:
    Controller();

    void setKey(uint8_t id, ControllerButton button, bool state);

    /// @name Implementation IDevice
    /// @[
    bool write(uint16_t address, uint8_t data);
    bool read(uint16_t address, uint8_t& data);
    /// @]

private:
    uint8_t _buttons[2];
    uint8_t _buttonsCached[2];
};
