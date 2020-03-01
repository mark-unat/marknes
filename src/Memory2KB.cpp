#include "Memory2KB.hpp"

constexpr auto size2KB = 2 * 1024;

Memory2KB::Memory2KB()
{
    // Allocate new memory
    _memory = std::make_unique<uint8_t[]>(size2KB);

    // Zero-out memory contents
    for (int i = 0; i < size2KB; i++) {
        _memory[i] = 0x00;
    }
}

bool Memory2KB::read(uint16_t address, uint8_t &data)
{
    // Check if address is within this Memory range
	if (address >= 0 && address < static_cast<uint16_t>(size2KB)) {
		data = _memory[address];
        return true;
    }

	return false;
}

bool Memory2KB::write(uint16_t address, uint8_t data)
{
    // Check if address is within this Memory range
	if (address >= 0 && address < static_cast<uint16_t>(size2KB)) {
		_memory[address] = data;
        return true;
    }

    return false;
}

