#include "Mapper002.hpp"
#include <stdio.h>

constexpr auto prgRomChunkSize = 0x4000;

Mapper002::Mapper002(uint8_t prgRomChunks, uint8_t chrRomChunks)
: _prgRomChunks{prgRomChunks}
, _chrRomChunks{chrRomChunks}
{
}

bool Mapper002::readPrg(uint16_t address, uint32_t& prgAddress, uint8_t& /*data*/)
{
    auto chunkAddress = address & (prgRomChunkSize - 1);
    if (address >= 0x8000 && address <= 0xBFFF) {
        prgAddress = _prgRomChunkSelectLow * prgRomChunkSize + chunkAddress;
        return true;
    } else if (address >= 0xC000 && address <= 0xFFFF) {
        prgAddress = _prgRomChunkSelectHigh * prgRomChunkSize + chunkAddress;
        return true;
    }

    return false;
}

bool Mapper002::writePrg(uint16_t address, uint32_t& prgAddress, uint8_t data)
{
    if (address >= 0x8000 && address <= 0xFFFF) {
        _prgRomChunkSelectLow = data & 0x0F;
    }

    // Always return false as we don't want to update our program memory
    return false;
}

bool Mapper002::readChr(uint16_t address, uint32_t& chrAddress)
{
    if (address >= 0x0000 && address <= 0x1FFF) {
        chrAddress = address;
        return true;
    }

    return false;
}

bool Mapper002::writeChr(uint16_t address, uint32_t& chrAddress)
{
    if (address >= 0x0000 && address <= 0x1FFF) {
        if (_chrRomChunks == 0) {
            // Used for RAM
            chrAddress = address;
            return true;
        }
    }

    return false;
}

void Mapper002::reset()
{
    _prgRomChunkSelectLow = 0;
    _prgRomChunkSelectHigh = _prgRomChunks - 1;
}
