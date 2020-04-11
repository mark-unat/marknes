#include "NameTable.hpp"

constexpr auto size4KB = 4 * 1024;
constexpr auto nameTableBaseAddress = 0x2000;

NameTable::NameTable()
{
    // Allocate new memory
    _memory = std::make_unique<uint8_t[]>(size4KB);

    // Zero-out memory contents
    for (uint16_t i = 0; i < size4KB; i++) {
        _memory[i] = 0x00;
    }
}

bool NameTable::read(uint16_t address, uint8_t& data)
{
    auto localAddress = address - nameTableBaseAddress;
    if ((localAddress >= 0) && (localAddress < size4KB)) {
        data = _memory[localAddress];
        return true;
    }

    return false;
}

bool NameTable::write(uint16_t address, uint8_t data)
{
    auto localAddress = address - nameTableBaseAddress;
    if ((localAddress >= 0) && (localAddress < size4KB)) {
        _memory[localAddress] = data;
        return true;
    }

	return false;
}

