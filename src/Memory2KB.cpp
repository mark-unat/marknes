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

bool Memory2KB::read(uint16_t address, uint8_t& data)
{
    // Address mirrored every 2KB
    auto localAddress = address & (size2KB - 1);
    if ((localAddress >= 0) && (localAddress < size2KB)) {
        data = _memory[localAddress];
        return true;
    }

    return false;
}

bool Memory2KB::write(uint16_t address, uint8_t data)
{
    // Address mirrored every 2KB
    auto localAddress = address & (size2KB - 1);
    if ((localAddress >= 0) && (localAddress < size2KB)) {
        _memory[localAddress] = data;
        return true;
    }

    return false;
}
