#pragma once

#include <memory>
#include <cstdint>
#include <string>
#include <fstream>
#include <vector>

#include "IMapper.hpp"

/*
 * The .NES file format is the de facto standard for distribution of NES binary
 * programs. It is often called the iNES file or header format. The details can
 * be found: https://wiki.nesdev.com/w/index.php/INES
 */
typedef struct NesHeader {
    uint8_t magic[4];
    uint8_t prgRomChunks;
    uint8_t chrRomChunks;
    union {
        uint8_t flagByte6;
        struct {
            bool mirroringMode : 1;
            bool hasPersistentMemory : 1;
            bool trainerAvailable : 1;
            bool vRamExpansion : 1;
            uint8_t mapperLowNibble : 4;
        } bitFlags6;
    };
    union {
        uint8_t flagByte7;
        struct {
            bool vsUnisystem : 1;
            bool playChoice : 1;
            uint8_t nes2Extension : 2;
            uint8_t mapperHighNibble : 4;
        } bitFlags7;
    };
    uint8_t prgRamSize;
    uint8_t tvSystem1;
    uint8_t tvSystem2;
    uint8_t unused[5];
} NesHeader;

enum class MirroringMode {
    Horizontal,
    Vertical,
    // OneScreenLow,
    // OneScreenHigh,
};

class Cartridge {
public:
    Cartridge(std::string fileName);
    ~Cartridge();

    bool isValid() const { return _isValid; }
    MirroringMode getMirroringMode() const { return _mirroringMode; }
    bool readPRG(uint16_t address, uint8_t& data);
    bool writePRG(uint16_t address, uint8_t data);
    bool readCHR(uint16_t address, uint8_t& data);
    bool writeCHR(uint16_t address, uint8_t data);
    void reset();

private:
    std::string _fileName;
    NesHeader _nesHeader;
    uint8_t _mapperID{0};
    uint32_t _prgRomSize{0};
    uint32_t _chrRomSize{0};
    bool _isValid{false};
    MirroringMode _mirroringMode{MirroringMode::Horizontal};
    std::vector<uint8_t> _prgRom;
    std::vector<uint8_t> _chrRom;
    std::shared_ptr<IMapper> _mapper{nullptr};
};
