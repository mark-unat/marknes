#include "Mapper000.hpp"
#include <stdio.h>

Mapper000::Mapper000(uint8_t prgRomChunks, uint8_t chrRomChunks)
: _prgRomChunks{prgRomChunks}
, _chrRomChunks{chrRomChunks}
{
}

bool Mapper000::getPrgAddress(uint16_t address, uint32_t &prgAddress)
{
	if (address >= 0x8000 && address <= 0xFFFF) {
        if (_prgRomChunks > 1) {
            prgAddress = address & 0x7FFF;
        } else {
            prgAddress = address & 0x3FFF;
        }
		return true;
	}

	return false;
}


bool Mapper000::getChrAddress(uint16_t address, uint32_t &chrAddress)
{
	if (address >= 0x0000 && address <= 0x1FFF) {
		chrAddress = address;
		return true;
	}

	return false;
}

