#include "Controller.hpp"
#include <stdio.h>

constexpr auto controller1Address = 0x4016;
constexpr auto controller2Address = 0x4017;

Controller::Controller()
{
}

void Controller::setKey(uint8_t id, ControllerButton button, bool state)
{
    // Only toggle the bit mapped to its corresponding button
    if (state) {
        // Button pressed
        _buttons[id] |= (1 << static_cast<uint8_t>(button));
    } else {
        // Button released
        _buttons[id] &= ~(1 << static_cast<uint8_t>(button));
    }
}

bool Controller::read(uint16_t address, uint8_t& data)
{
    // Only return the highest significant bit as we shift the bits once to the
    // left everytime we're done reading
    if (address == controller1Address) {
        data = (_buttonsCached[0] & 0x80) ? 1 : 0;
        _buttonsCached[0] = _buttonsCached[0] << 1;
    } else if (address == controller2Address) {
        data = (_buttonsCached[1] & 0x80) ? 1 : 0;
        _buttonsCached[1] = _buttonsCached[1] << 1;
    }

    return true;
}

bool Controller::write(uint16_t address, uint8_t data)
{
    // Take a snapshot of the current buttons w/c will be used for reading
    if (address == controller1Address) {
        _buttonsCached[0] = _buttons[0];
    } else if (address == controller2Address) {
        _buttonsCached[1] = _buttons[1];
    }

    return true;
}
