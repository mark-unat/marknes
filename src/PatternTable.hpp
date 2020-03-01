#pragma once

#include <cstdint>

#include "ITable.hpp"

class PatternTable : public ITable {
public:
    PatternTable();

    /// @name Implementation IDevice
    /// @[
	bool write(uint16_t address, uint8_t data);
	bool read(uint16_t address, uint8_t &data);
    /// @]
};
