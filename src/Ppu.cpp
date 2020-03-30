#include <algorithm>

#include "Ppu.hpp"

constexpr uint8_t resetStackOffset = 0xFD;
constexpr uint16_t stackBaseAddress = 0x0100;
constexpr uint16_t nonMaskableInterruptAddress = 0xFFFA;
constexpr uint16_t resetInterruptAddress = 0xFFFC;
constexpr uint16_t breakInterruptAddress = 0xFFFE;

Ppu::Ppu(std::shared_ptr<IDevice> bus,
    std::shared_ptr<Cartridge> cartridge)
: _bus{bus}
, _cartridge{cartridge}
{
    // NTSC Palette Table: wiki.nesdev.com/w/index.php/PPU_palettes
    _palettePixelTable = {
        { 0x54, 0x54, 0x54 }, { 0x00, 0x1E, 0x74 }, { 0x08, 0x10, 0x90 }, { 0x30, 0x00, 0x88 },
        { 0x44, 0x00, 0x64 }, { 0x5C, 0x00, 0x30 }, { 0x54, 0x04, 0x00 }, { 0x3C, 0x18, 0x00 },
        { 0x20, 0x2A, 0x00 }, { 0x08, 0x3A, 0x00 }, { 0x00, 0x40, 0x00 }, { 0x00, 0x3C, 0x00 },
        { 0x00, 0x32, 0x3C }, { 0x00, 0x00, 0x00 }, { 0x98, 0x96, 0x98 }, { 0x08, 0x4C, 0xC4 },
        { 0x30, 0x32, 0xEC }, { 0x5C, 0x1E, 0xE4 }, { 0x88, 0x14, 0xB0 }, { 0xA0, 0x14, 0x64 },
        { 0x98, 0x22, 0x20 }, { 0x78, 0x3C, 0x00 }, { 0x54, 0x5A, 0x00 }, { 0x28, 0x72, 0x00 },
        { 0x08, 0x7C, 0x00 }, { 0x00, 0x76, 0x28 }, { 0x00, 0x66, 0x78 }, { 0x00, 0x00, 0x00 },
        { 0xEC, 0xEE, 0xEC }, { 0x4C, 0x9A, 0xEC }, { 0x78, 0x7C, 0xEC }, { 0xB0, 0x62, 0xEC },
        { 0xE4, 0x54, 0xEC }, { 0xEC, 0x58, 0xB4 }, { 0xEC, 0x6A, 0x64 }, { 0xD4, 0x88, 0x20 },
        { 0xA0, 0xAA, 0x00 }, { 0x74, 0xC4, 0x00 }, { 0x4C, 0xD0, 0x20 }, { 0x38, 0xCC, 0x6C },
        { 0x38, 0xB4, 0xCC }, { 0x3C, 0x3C, 0x3C }, { 0xEC, 0xEE, 0xEC }, { 0xA8, 0xCC, 0xEC },
        { 0xBC, 0xBC, 0xEC }, { 0xD4, 0xB2, 0xEC }, { 0xEC, 0xAE, 0xEC }, { 0xEC, 0xAE, 0xD4 },
        { 0xEC, 0xB4, 0xB0 }, { 0xE4, 0xC4, 0x90 }, { 0xCC, 0xD2, 0x78 }, { 0xB4, 0xDE, 0x78 },
        { 0xA8, 0xE2, 0x90 }, { 0x98, 0xE2, 0xB4 }, { 0xA0, 0xD6, 0xE4 }, { 0xA0, 0xA2, 0xA0 },
    };
}

Ppu::~Ppu()
{
}

bool Ppu::read(uint16_t address, uint8_t &/*data*/)
{
    switch (static_cast<PpuRegisterAddress>(address)) {
    case PpuRegisterAddress::Control:
        break;
    case PpuRegisterAddress::Mask:
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        break;
    case PpuRegisterAddress::VRAMData:
        break;
    default:
        break;
    }

    return true;
}

bool Ppu::write(uint16_t address, uint8_t /*data*/)
{
    switch (static_cast<PpuRegisterAddress>(address)) {
    case PpuRegisterAddress::Control:
        break;
    case PpuRegisterAddress::Mask:
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        break;
    case PpuRegisterAddress::VRAMData:
        break;
    default:
        break;
    }

    return true;
}

// Execute one clock cycle
void Ppu::tick()
{
	_cycles++;
	if (_cycles >= 341)
	{
		_cycles = 0;
		_scanLine++;
		if (_scanLine >= 261)
		{
			_scanLine = -1;
			_frameDone = true;
		}
	}
}

uint8_t* Ppu::getFrameBuffer()
{
    // Temporary static noise
    for (uint32_t pixel = 0; pixel < PPU_FRAME_WIDTH * PPU_FRAME_HEIGHT * 3;) {
        uint8_t value = rand() % 255;
        _frameBufferRGB[pixel++] = value;
        _frameBufferRGB[pixel++] = value;
        _frameBufferRGB[pixel++] = value;
    }

    return _frameBufferRGB;
}
