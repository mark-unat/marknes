#include "PaletteTable.hpp"

constexpr auto size32 = 32;
constexpr auto paletteTableBaseAddress = 0x3F00;

PaletteTable::PaletteTable()
{
    // Allocate new memory
    _memory = std::make_unique<uint8_t[]>(size32);

    // Zero-out memory contents
    for (uint16_t i = 0; i < size32; i++) {
        _memory[i] = 0x00;
    }
}

bool PaletteTable::read(uint16_t address, uint8_t& data)
{
    auto localAddress = address - paletteTableBaseAddress;
    localAddress = localAddress % size32;
    if ((localAddress >= 0) && (localAddress < size32)) {
        data = _memory[localAddress];
        return true;
    }

    return false;
}

bool PaletteTable::write(uint16_t address, uint8_t data)
{
    auto localAddress = address - paletteTableBaseAddress;
    localAddress = localAddress % size32;
    if ((localAddress >= 0) && (localAddress < size32)) {
        _memory[localAddress] = data;
        return true;
    }

    return false;
}
