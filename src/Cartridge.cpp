#include "Cartridge.hpp"
#include "Mapper000.hpp"
#include "Mapper002.hpp"

constexpr auto size8KB = 8 * 1024;
constexpr auto size16KB = 16 * 1024;

Cartridge::Cartridge(std::string fileName)
: _fileName{std::move(fileName)}
{
    auto file = std::ifstream{_fileName, std::ifstream::binary};
    if (file) {
        file.read((char*)&_nesHeader, sizeof(NesHeader));

        // Check if trainer is available
        if (_nesHeader.bitFlags6.trainerAvailable) {
            // Read past 512-byte
            file.seekg(512, std::ios_base::cur);
        }

        _mapperID = (_nesHeader.bitFlags7.mapperHighNibble << 4) | _nesHeader.bitFlags6.mapperLowNibble;
        _mirroringMode = static_cast<MirroringMode>(_nesHeader.bitFlags6.mirroringMode);

        // Assume fileFormatType=1 for now
        _prgRomSize = _nesHeader.prgRomChunks * size16KB;
        _prgRom.resize(_prgRomSize);
        file.read(reinterpret_cast<char*>(_prgRom.data()), _prgRomSize);

        if (_nesHeader.chrRomChunks == 0) {
            // Used for RAM
            _chrRomSize = size8KB;
        } else {
            // Used for ROM
            _chrRomSize = _nesHeader.chrRomChunks * size8KB;
        }
        _chrRom.resize(_chrRomSize);
        file.read(reinterpret_cast<char*>(_chrRom.data()), _chrRomSize);

        // Switch to correct Mapper
        switch (_mapperID) {
        case 0:
            _mapper = std::make_shared<Mapper000>(_nesHeader.prgRomChunks, _nesHeader.chrRomChunks);
            break;
        case 2:
            _mapper = std::make_shared<Mapper002>(_nesHeader.prgRomChunks, _nesHeader.chrRomChunks);
            break;
        default:
            fprintf(stderr, "NES ROM Mapper [%d] not yet supported\n", _mapperID);
            break;
        }

        _isValid = true;
    }
}

Cartridge::~Cartridge() {}

bool Cartridge::readPRG(uint16_t address, uint8_t& data)
{
    auto prgAddress = uint32_t{0};
    if (_mapper->readPrg(address, prgAddress, data)) {
        data = _prgRom[prgAddress];
        return true;
    }

    return false;
}

bool Cartridge::writePRG(uint16_t address, uint8_t data)
{
    auto prgAddress = uint32_t{0};
    if (_mapper->writePrg(address, prgAddress, data)) {
        _prgRom[prgAddress] = data;
        return true;
    }

    return false;
}

bool Cartridge::readCHR(uint16_t address, uint8_t& data)
{
    auto chrAddress = uint32_t{0};
    if (_mapper->readChr(address, chrAddress)) {
        data = _chrRom[chrAddress];
        return true;
    }

    return false;
}

bool Cartridge::writeCHR(uint16_t address, uint8_t data)
{
    auto chrAddress = uint32_t{0};
    if (_mapper->writeChr(address, chrAddress)) {
        _chrRom[chrAddress] = data;
        return true;
    }

    return false;
}

void Cartridge::reset()
{
    if (_mapper != nullptr) {
        _mapper->reset();
    }
}
