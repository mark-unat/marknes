#pragma once

#include <cstdint>

#include "IMapper.hpp"

class Mapper002 : public IMapper {
public:
    Mapper002(uint8_t prgRomChunks, uint8_t chrRomChunks);

    /// @name Implementation IMapper
    /// @[
    bool readPrg(uint16_t address, uint32_t& prgAddress, uint8_t& data);
    bool writePrg(uint16_t address, uint32_t& prgAddress, uint8_t data);
    bool readChr(uint16_t address, uint32_t& chrAddress);
    bool writeChr(uint16_t address, uint32_t& chrAddress);
    void reset();
    /// @]

private:
    uint8_t _prgRomChunks;
    uint8_t _chrRomChunks;
    uint8_t _prgRomChunkSelectLow{0x00};
    uint8_t _prgRomChunkSelectHigh{0x00};
};
