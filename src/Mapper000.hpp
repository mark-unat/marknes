#pragma once

#include <cstdint>

#include "IMapper.hpp"

class Mapper000 : public IMapper {
public:
    Mapper000(uint8_t prgRomChunks, uint8_t chrRomChunks);

    /// @name Implementation IMapper
    /// @[
	bool getPrgAddress(uint16_t address, uint32_t &prgAddress);
	bool getChrAddress(uint16_t address, uint32_t &chrAddress);
    /// @]

private:
    uint8_t _prgRomChunks;
    uint8_t _chrRomChunks;
};
