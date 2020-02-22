#pragma once

#include <cstdint>
#include <array>

#include "IDevice.hpp"

class Memory64KB : public IDevice {
public:
    Memory64KB();

    /// @name Implementation IDevice
    /// @[
	bool write(uint16_t address, uint8_t data);
	bool read(uint16_t address, uint8_t *data);
    /// @]
private:
    std::unique_ptr<uint8_t[]> _memory;
};
