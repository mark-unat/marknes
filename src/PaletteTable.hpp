#pragma once

#include <cstdint>

#include "IMemory.hpp"

class PaletteTable : public IMemory {
public:
    PaletteTable();

    /// @name Implementation IMemory
    /// @[
	bool write(uint16_t address, uint8_t data);
	bool read(uint16_t address, uint8_t &data);
    /// @]
};
