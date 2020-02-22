#include "Memory64KB.hpp"

constexpr auto size64KB = 64 * 1024;

Memory64KB::Memory64KB()
{
    // Allocate new memory
    _memory = std::make_unique<uint8_t[]>(size64KB);

    // Zero-out memory contents
    for (int i = 0; i < size64KB; i++) {
        _memory[i] = 0x00;
    }
}

bool Memory64KB::read(uint16_t address, uint8_t* data)
{
    // Check if address is within this Memory range
	if (address >= 0 && address < static_cast<uint16_t>(size64KB)) {
		*data = _memory[address];
        return true;
    }

	return false;
}

bool Memory64KB::write(uint16_t address, uint8_t data)
{
    // Check if address is within this Memory range
	if (address >= 0 && address < static_cast<uint16_t>(size64KB)) {
		_memory[address] = data;
        return true;
    }

    return false;
}

