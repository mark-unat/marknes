#pragma once

#include <cstdint>

#include "ITable.hpp"

class NameTable : public ITable {
public:
    NameTable();

    /// @name Implementation IDevice
    /// @[
	bool write(uint16_t address, uint8_t data);
	bool read(uint16_t address, uint8_t &data);
    /// @]
};
