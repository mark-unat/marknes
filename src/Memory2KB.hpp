#pragma once

#include <cstdint>

#include "IMemory.hpp"

class Memory2KB : public IMemory {
public:
    Memory2KB();

    /// @name Implementation IDevice
    /// @[
	bool write(uint16_t address, uint8_t data);
	bool read(uint16_t address, uint8_t &data);
    /// @]
};
