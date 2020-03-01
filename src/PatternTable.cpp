#include "PatternTable.hpp"

constexpr auto size1KB = 1024;
constexpr auto rowSize = 2;
constexpr auto columnSize = 4 * size1KB;

PatternTable::PatternTable()
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

bool PatternTable::read(uint16_t /*address*/, uint8_t& /*data*/)
{
    // Check if address is within this Table range
	return false;
}

bool PatternTable::write(uint16_t /*address*/, uint8_t /*data*/)
{
    // Check if address is within this Table range
    return false;
}

