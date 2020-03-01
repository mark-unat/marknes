#include "Cartridge.hpp"
#include "Mapper000.hpp"

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

        _chrRomSize = _nesHeader.chrRomChunks * size8KB;
        _chrRom.resize(_chrRomSize);
        file.read(reinterpret_cast<char*>(_chrRom.data()), _chrRomSize);

		// Switch to correct Mapper
		switch (_mapperID) {
		case 0:
            _mapper = std::make_shared<Mapper000>(_nesHeader.prgRomChunks, _nesHeader.chrRomChunks);
            break;
        default:
            break;
		}

		_isValid = true;
	}
}

Cartridge::~Cartridge()
{
}

bool Cartridge::readPrgRom(uint16_t address, uint8_t &data)
{
	auto prgAddress = uint32_t{0};
	if (_mapper->getPrgAddress(address, prgAddress)) {
		data = _prgRom[prgAddress];
		return true;
	}

    return false;
}

bool Cartridge::writePrgRom(uint16_t address, uint8_t data)
{
	auto prgAddress = uint32_t{0};
	if (_mapper->getPrgAddress(address, prgAddress)) {
		_prgRom[prgAddress] = data;
		return true;
	}

    return false;
}

bool Cartridge::readChrRom(uint16_t address, uint8_t &data)
{
	auto chrAddress = uint32_t{0};
	if (_mapper->getChrAddress(address, chrAddress)) {
		data = _chrRom[chrAddress];
		return true;
	}

    return false;
}

bool Cartridge::writeChrRom(uint16_t address, uint8_t data)
{
	auto chrAddress = uint32_t{0};
	if (_mapper->getChrAddress(address, chrAddress)) {
		_chrRom[chrAddress] = data;
		return true;
	}

    return false;
}

