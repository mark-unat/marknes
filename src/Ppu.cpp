#include <string.h>
#include <algorithm>

#include "Ppu.hpp"
#include "PpuBus.hpp"

constexpr uint8_t resetStackOffset = 0xFD;
constexpr uint16_t stackBaseAddress = 0x0100;
constexpr uint16_t nonMaskableInterruptAddress = 0xFFFA;
constexpr uint16_t resetInterruptAddress = 0xFFFC;
constexpr uint16_t breakInterruptAddress = 0xFFFE;
constexpr auto ppuBaseAddress = 0x2000;

Ppu::Ppu(std::shared_ptr<IDevice> bus, std::shared_ptr<Cartridge> cartridge)
: _bus{bus}
, _cartridge{cartridge}
{
    // NTSC Palette Table: wiki.nesdev.com/w/index.php/PPU_palettes
    _paletteTablePixel = {
        {0x54, 0x54, 0x54}, {0x00, 0x1E, 0x74}, {0x08, 0x10, 0x90}, {0x30, 0x00, 0x88}, {0x44, 0x00, 0x64},
        {0x5C, 0x00, 0x30}, {0x54, 0x04, 0x00}, {0x3C, 0x18, 0x00}, {0x20, 0x2A, 0x00}, {0x08, 0x3A, 0x00},
        {0x00, 0x40, 0x00}, {0x00, 0x3C, 0x00}, {0x00, 0x32, 0x3C}, {0x00, 0x00, 0x00}, {0x98, 0x96, 0x98},
        {0x08, 0x4C, 0xC4}, {0x30, 0x32, 0xEC}, {0x5C, 0x1E, 0xE4}, {0x88, 0x14, 0xB0}, {0xA0, 0x14, 0x64},
        {0x98, 0x22, 0x20}, {0x78, 0x3C, 0x00}, {0x54, 0x5A, 0x00}, {0x28, 0x72, 0x00}, {0x08, 0x7C, 0x00},
        {0x00, 0x76, 0x28}, {0x00, 0x66, 0x78}, {0x00, 0x00, 0x00}, {0xEC, 0xEE, 0xEC}, {0x4C, 0x9A, 0xEC},
        {0x78, 0x7C, 0xEC}, {0xB0, 0x62, 0xEC}, {0xE4, 0x54, 0xEC}, {0xEC, 0x58, 0xB4}, {0xEC, 0x6A, 0x64},
        {0xD4, 0x88, 0x20}, {0xA0, 0xAA, 0x00}, {0x74, 0xC4, 0x00}, {0x4C, 0xD0, 0x20}, {0x38, 0xCC, 0x6C},
        {0x38, 0xB4, 0xCC}, {0x3C, 0x3C, 0x3C}, {0xEC, 0xEE, 0xEC}, {0xA8, 0xCC, 0xEC}, {0xBC, 0xBC, 0xEC},
        {0xD4, 0xB2, 0xEC}, {0xEC, 0xAE, 0xEC}, {0xEC, 0xAE, 0xD4}, {0xEC, 0xB4, 0xB0}, {0xE4, 0xC4, 0x90},
        {0xCC, 0xD2, 0x78}, {0xB4, 0xDE, 0x78}, {0xA8, 0xE2, 0x90}, {0x98, 0xE2, 0xB4}, {0xA0, 0xD6, 0xE4},
        {0xA0, 0xA2, 0xA0},
    };
}

Ppu::~Ppu() {}

bool Ppu::read(uint16_t address, uint8_t& data)
{
    auto localAddress = address - ppuBaseAddress;
    // PPU Address mirrored every 8 bytes
    localAddress = localAddress & 0x7;

    switch (static_cast<PpuRegisterAddress>(localAddress)) {
    case PpuRegisterAddress::Control:
        break;
    case PpuRegisterAddress::Mask:
        break;
    case PpuRegisterAddress::Status:
        data = (registers.status & 0xE0) | (vramCachedData & 0x1F);
        registers.statusFlag.verticalBlankFlag = false;
        vramAddressLatch = false;
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        // Nothing to read from here
        break;
    case PpuRegisterAddress::VRAMData:
        _bus->read(registers.vramAddress, registers.vramData);
        if (registers.vramAddress >= paletteTableBaseAddress) {
            // Only the palette address range would get the data immediately,
            // thus no read delay by one cycle.
            data = registers.vramData;
        } else {
            // Return previous data here to delay read by one cycle, since
            // writing to VRAM address requires two cycles because it's a 16-bit
            // address.
            data = vramCachedData;
            vramCachedData = registers.vramData;
        }

        // Auto increment VRAM address when reading from data
        // The increment step depends on the PPU control register
        if (registers.controlFlag.vramAddressIncrement) {
            registers.vramAddress += 32;
        } else {
            registers.vramAddress++;
        }
        break;
    default:
        break;
    }

    return true;
}

bool Ppu::write(uint16_t address, uint8_t data)
{
    auto localAddress = address - ppuBaseAddress;
    // PPU Address mirrored every 8 bytes
    localAddress = localAddress & 0x7;

    switch (static_cast<PpuRegisterAddress>(localAddress)) {
    case PpuRegisterAddress::Control:
        registers.control = data;
        break;
    case PpuRegisterAddress::Mask:
        registers.mask = data;
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress: {
        auto word = static_cast<uint16_t>(data);
        if (!vramAddressLatch) {
            // High byte of VRAM Address
            registers.vramAddress = (registers.vramAddress & 0x00FF) | (word << 8);
        } else {
            // Low byte of VRAM Address
            registers.vramAddress = (registers.vramAddress & 0xFF00) | (word);
        }
        vramAddressLatch = !vramAddressLatch;
    } break;
    case PpuRegisterAddress::VRAMData:
        _bus->write(registers.vramAddress, data);

        // Auto increment VRAM address when writing to data
        // The increment step depends on the PPU control register
        if (registers.controlFlag.vramAddressIncrement) {
            registers.vramAddress += 32;
        } else {
            registers.vramAddress++;
        }
        break;
    default:
        break;
    }

    return true;
}

// Execute one clock cycle
void Ppu::tick()
{
    if (_scanLine == 241 && _cycles == 1) {
        registers.statusFlag.verticalBlankFlag = true;
        if (registers.controlFlag.NMIEnabledOnVBlank) {
            _vBlank = true;
        }
    }

    _cycles++;
    if (_cycles >= 341) {
        _cycles = 0;
        _scanLine++;
        if (_scanLine >= 261) {
            _scanLine = -1;
            _frameDone = true;
        }
    }
}

void Ppu::reset()
{
    memset(&registers, 0, sizeof(PpuRegister));
}

bool Ppu::isVBlankTriggered()
{
    auto vBlank{_vBlank};

    // Auto-reset once we detect that it's triggered
    if (_vBlank) {
        _vBlank = false;
    }
    return vBlank;
}

bool Ppu::isFrameDone()
{
    auto frameDone{_frameDone};

    // Auto-reset once we detect that it's done
    if (_frameDone) {
        _frameDone = false;
    }
    return frameDone;
}

uint8_t* Ppu::getFrameBuffer()
{
    // Temporary static noise
    /*for (uint32_t pixel = 0; pixel < PPU_FRAME_BUFFER_RGB_SIZE;) {
        uint8_t value = rand() % 255;
        _frameBufferRGB[pixel++] = value;
        _frameBufferRGB[pixel++] = value;
        _frameBufferRGB[pixel++] = value;
    }*/

    /*
    // Getting PatternTable #0
    auto pattern = getPatternTableTile(0, 1);
    uint16_t pixel = 0;
    for (int a = 0; a < 16; a++) {
        for (int j = 0; j < 8; j++) {
            for (int i = a * 16; i < (a+1)*16; i++) {
                for (int k = j * 8; k < (j+1)*8; k++) {
                    Pixel myPixel = pattern.tile[i].pixel[k];
                    _frameBufferRGB[pixel++] = myPixel.red;
                    _frameBufferRGB[pixel++] = myPixel.green;
                    _frameBufferRGB[pixel++] = myPixel.blue;
                }
            }
        }
    }*/

    // Getting NameTable #0
    auto nameTable = getNameTableTile(0);
    uint32_t pixel = 0;
    for (int a = 0; a < 30; a++) {
        for (int j = 0; j < 8; j++) {
            for (int i = a * 32; i < (a + 1) * 32; i++) {
                for (int k = j * 8; k < (j + 1) * 8; k++) {
                    Pixel myPixel = nameTable.tile[i].pixel[k];
                    _frameBufferRGB[pixel++] = myPixel.red;
                    _frameBufferRGB[pixel++] = myPixel.green;
                    _frameBufferRGB[pixel++] = myPixel.blue;
                }
            }
        }
    }

    return _frameBufferRGB;
}

NameTableTile Ppu::getNameTableTile(uint8_t index)
{
    auto nameTableAddress = uint16_t{0x0000};
    if (index == 0) {
        nameTableAddress = nameTable1StartAddress;
    } else if (index == 1) {
        nameTableAddress = nameTable2StartAddress;
    } else if (index == 2) {
        nameTableAddress = nameTable3StartAddress;
    } else if (index == 3) {
        nameTableAddress = nameTable4StartAddress;
    }

    // Temporarily get pattern #0 using palette #0
    auto pattern = getPatternTableTile(1, 1);

    // Let's access the name table: 32x30 tiles
    for (uint16_t tile = 0; tile < 32 * 30; tile++) {
        auto patternIndex = uint8_t{0x00};
        _bus->read(nameTableAddress + tile, patternIndex);

        // Copy this Pattern to our Name Table
        _nameTablePixel[index].tile[tile] = pattern.tile[patternIndex];
    }

    return _nameTablePixel[index];
}

PatternTableTile Ppu::getPatternTableTile(uint8_t type, uint8_t paletteIndex)
{
    auto patternAddress = uint16_t{0x0000};
    if (type == 0) {
        patternAddress = patternTableSpriteAddress;
    } else {
        patternAddress = patternTableBackgroundAddress;
    }

    // Let's access this pattern: 16x16 tiles
    for (uint16_t tile = 0; tile < 16 * 16; tile++) {
        // Let's access this tile: 8x8 pixels
        for (uint8_t x = 0; x < 8; x++) {
            auto lowByte = uint8_t{0x00};
            auto highByte = uint8_t{0x00};
            _bus->read(patternAddress + tile * 16 + x + 0, lowByte);
            _bus->read(patternAddress + tile * 16 + x + 8, highByte);

            // The least significant bit goes to the last pixel index, that's
            // why we count from 7 to 0 index
            for (uint8_t y = 8; y > 0; y--) {
                auto pixelIndex = (highByte & 0x01) + (lowByte & 0x01);
                auto paletteByte = uint8_t{0x00};
                highByte = highByte >> 1;
                lowByte = lowByte >> 1;

                _bus->read(paletteTableBaseAddress + paletteIndex * 4 + pixelIndex, paletteByte);

                _patternTablePixel[type].tile[tile].pixel[x * 8 + (y - 1)] = _paletteTablePixel[paletteByte];
            }
        }
    }

    return _patternTablePixel[type];
}
