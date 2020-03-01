#include "PaletteTable.hpp"

constexpr auto rowSize = 32;
constexpr auto columnSize = 1;

PaletteTable::PaletteTable()
{
    // Allocate new table
    _table = std::make_unique<std::unique_ptr<uint8_t[]>[]>(rowSize);
    for (int i = 0; i < rowSize; i++) {
        _table[i] = std::make_unique<uint8_t[]>(columnSize);
    }

    // Zero-out table contents
    for (int i = 0; i < rowSize; i++) {
        for (int j = 0; j < columnSize; j++) {
            _table[i][j] = 0x00;
        }
    }
}

bool PaletteTable::read(uint16_t /*address*/, uint8_t& /*data*/)
{
    // Check if address is within this Table range
	return false;
}

bool PaletteTable::write(uint16_t /*address*/, uint8_t /*data*/)
{
    // Check if address is within this Table range
    return false;
}

