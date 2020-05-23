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
        {0x54, 0x54, 0x54}, {0x00, 0x1E, 0x74}, {0x08, 0x10, 0x90}, {0x30, 0x00, 0x88},
        {0x44, 0x00, 0x64}, {0x5C, 0x00, 0x30}, {0x54, 0x04, 0x00}, {0x3C, 0x18, 0x00},
        {0x20, 0x2A, 0x00}, {0x08, 0x3A, 0x00}, {0x00, 0x40, 0x00}, {0x00, 0x3C, 0x00},
        {0x00, 0x32, 0x3C}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0x98, 0x96, 0x98}, {0x08, 0x4C, 0xC4}, {0x30, 0x32, 0xEC}, {0x5C, 0x1E, 0xE4},
        {0x88, 0x14, 0xB0}, {0xA0, 0x14, 0x64}, {0x98, 0x22, 0x20}, {0x78, 0x3C, 0x00},
        {0x54, 0x5A, 0x00}, {0x28, 0x72, 0x00}, {0x08, 0x7C, 0x00}, {0x00, 0x76, 0x28},
        {0x00, 0x66, 0x78}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0xEC, 0xEE, 0xEC}, {0x4C, 0x9A, 0xEC}, {0x78, 0x7C, 0xEC}, {0xB0, 0x62, 0xEC},
        {0xE4, 0x54, 0xEC}, {0xEC, 0x58, 0xB4}, {0xEC, 0x6A, 0x64}, {0xD4, 0x88, 0x20},
        {0xA0, 0xAA, 0x00}, {0x74, 0xC4, 0x00}, {0x4C, 0xD0, 0x20}, {0x38, 0xCC, 0x6C},
        {0x38, 0xB4, 0xCC}, {0x3C, 0x3C, 0x3C}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
        {0xEC, 0xEE, 0xEC}, {0xA8, 0xCC, 0xEC}, {0xBC, 0xBC, 0xEC}, {0xD4, 0xB2, 0xEC},
        {0xEC, 0xAE, 0xEC}, {0xEC, 0xAE, 0xD4}, {0xEC, 0xB4, 0xB0}, {0xE4, 0xC4, 0x90},
        {0xCC, 0xD2, 0x78}, {0xB4, 0xDE, 0x78}, {0xA8, 0xE2, 0x90}, {0x98, 0xE2, 0xB4},
        {0xA0, 0xD6, 0xE4}, {0xA0, 0xA2, 0xA0}, {0x00, 0x00, 0x00}, {0x00, 0x00, 0x00},
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
        data = (registers.status & 0xE0) | (registers.tempVramData & 0x1F);
        registers.statusFlag.verticalBlankFlag = false;

        // VRAM Implementation:
        //     w:                  = 0
        registers.vramAddressLatch = false;
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        readOAMData(_oamAddress, data);
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        // Nothing to read from here
        break;
    case PpuRegisterAddress::VRAMData:
        _bus->read(registers.currVramAddress, registers.currVramData);
        if (registers.currVramAddress >= paletteTableBaseAddress) {
            // Only the palette address range would get the data immediately,
            // thus no read delay by one cycle.
            data = registers.currVramData;
        } else {
            // Return previous data here to delay read by one cycle, since
            // writing to VRAM address requires two cycles because it's a 16-bit
            // address.
            data = registers.tempVramData;
            registers.tempVramData = registers.currVramData;
        }

        // Auto increment VRAM address when reading from data
        // The increment step depends on the PPU control register
        if (registers.controlFlag.vramAddressIncrement) {
            registers.currVramAddress += 32;
        } else {
            registers.currVramAddress++;
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

        // VRAM Implementation:
        //     t: ...BA.. ........ = d: ......BA
        registers.tempVramFlag.nameTableXAddress = registers.controlFlag.nameTableXAddress;
        registers.tempVramFlag.nameTableYAddress = registers.controlFlag.nameTableYAddress;
        break;
    case PpuRegisterAddress::Mask:
        registers.mask = data;
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        _oamAddress = data;
        break;
    case PpuRegisterAddress::OAMData:
        writeOAMData(_oamAddress, data);
        break;
    case PpuRegisterAddress::Scroll:
        if (!registers.vramAddressLatch) {
            // Scroll X
            // VRAM Implementation:
            //     t: ....... ...HGFED = d: HGFED...
            //     x:              CBA = d: .....CBA
            //     w:                  = 1
            registers.tempVramFlag.coarseXScroll = data >> 3;
            registers.fineXScroll = data & 0x07;
        } else {
            // Scroll Y
            // VRAM Implementation:
            //     t: CBA..HG FED..... = d: HGFEDCBA
            //     w:                  = 0
            registers.tempVramFlag.coarseYScroll = data >> 3;
            registers.tempVramFlag.fineYScroll = data & 0x07;
        }
        registers.vramAddressLatch = !registers.vramAddressLatch;
        break;
    case PpuRegisterAddress::VRAMAddress: {
        auto word = static_cast<uint16_t>(data);
        if (!registers.vramAddressLatch) {
            // High byte of VRAM Address, but we need to zero out the upper
            // 2-bits of the byte
            // VRAM Implementation:
            //     t: .FEDCBA ........ = d: ..FEDCBA
            //     t: X...... ........ = 0
            //     w:                  = 1
            word = word & 0x003F;
            registers.tempVramAddress = (registers.tempVramAddress & 0x00FF) | (word << 8);
            //
        } else {
            // Low byte of VRAM Address
            // VRAM Implementation:
            //     t: ....... HGFEDCBA = d: HGFEDCBA
            //     v:                  = t
            //     w:                  = 0
            registers.tempVramAddress = (registers.tempVramAddress & 0xFF00) | (word);

            // Copy temporary VRAM to current VRAM
            registers.currVramAddress = registers.tempVramAddress;
        }
        registers.vramAddressLatch = !registers.vramAddressLatch;
    } break;
    case PpuRegisterAddress::VRAMData:
        _bus->write(registers.currVramAddress, data);

        // Auto increment VRAM address when writing to data
        // The increment step depends on the PPU control register
        if (registers.controlFlag.vramAddressIncrement) {
            registers.currVramAddress += 32;
        } else {
            registers.currVramAddress++;
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
    // PPU rendering: https://wiki.nesdev.com/w/index.php/PPU_rendering
    // There are 262 scanlines per frame: 0 to 261
    // Each scanline lasts 341 cycles (each cycle is one pixel): 0 to 340
    // Refer to NTSC PPU Frame Timing Diagram

    switch (_scanLine) {
    case 0 ... 239:
    case 261:
    {
        // Skip cycle
        if ((_scanLine == 0) && (_cycles == 0)) {
            _cycles = 1;
        }

        // Visible scanlines (0-239) and
        // Pre-render (dummy) scanline (261)
        switch (_cycles) {
        case 1 ... 256:
        case 321 ... 336:
        {
            // Move our shift registers by 1-bit in the visible scanlines
            _moveShiftRegisters();

            // The data for each tile is fetched during this phase. Each memory
            // access takes 2 PPU cycles to complete, and 4 must be performed per tile:
            //   1. Nametable byte
            //   2. Attribute table byte
            //   3. Pattern table tile low
            //   4. Pattern table tile high (+8 bytes from pattern table tile low)
            switch ((_cycles - 1) % 8) {
            case 0:
                // Load pattern and attribute to our shift registers
                _loadShiftRegisters();

                // NT byte
                nextNameTableByte = _getNextNameTableByte();
                break;
            case 2:
                // AT byte
                nextAttributeByte = _getNextAttributeByte();
                break;
            case 4:
                // Low BG tile byte
                nextLowBGTileByte = _getBackgroundTileByte(false /*LSB*/);
                break;
            case 6:
                // High BG tile byte
                nextHighBGTileByte = _getBackgroundTileByte(true /*MSB*/);
                break;
            case 7:
                // Increment horizontal current VRAM
                _incrementVramHorizontalInfo();
                break;
            default:
                break;
            }

            if (_cycles == 256) {
                // Increment vertical current VRAM
                _incrementVramVerticalInfo();
            }
            break;
        }
        case 257 ... 320:
        {
            if (_cycles == 257) {
                // Load pattern and attribute to our shift registers
                _loadShiftRegisters();

                // Copy horizontal temporary VRAM to current VRAM
                _updateVramHorizontalInfo();
            }
            break;
        }
        case 337 ... 340:
        {
            // Two Nametable bytes are fetched, but the purpose for this is unknown.
            if ((_cycles == 337) || (_cycles == 339)) {
                // NT byte
                auto tile = registers.currVramAddress & 0x0FFF;
                _bus->read(nameTableBaseAddress + tile, nextNameTableByte);
            }
            break;
        }
        default:
            break;
        }

        if (_scanLine == 261) {
            // Events for the pre-render scanline only
            if (_cycles == 1) {
                // Clear VBlank flag
                registers.statusFlag.verticalBlankFlag = false;

                // Clear Sprite Overflow flag
                registers.statusFlag.spriteOverflow = false;

                // Clear Sprite Zero Hit flag
                registers.statusFlag.spriteZeroHit = false;
            } else if ((_cycles >= 280) && (_cycles <= 304)) {
                // Copy vertical temporary VRAM to current VRAM
                _updateVramVerticalInfo();
            }
        } else {
            // Events for visible scanlines only
            // These are foreground related evaluation
            switch (_cycles) {
            case 1:
                // Clear Secondary OAM data with 0xFF values
                clearSecondaryOAMData(0xFF);
                break;
            case 65:
            {
                // Sprite Evaluation Logic
                _spriteZeroNextScanLine = false;
                uint8_t index = 0;
                for (uint8_t secondaryIndex = 0; index < PPU_MAX_SPRITES; index++) {
                    _spritesSecondary[secondaryIndex].positionY = _sprites[index].positionY;
                    uint8_t spriteHeight = _spritesSecondary[secondaryIndex].positionY + 8;
                    if (registers.controlFlag.spriteSize) {
                        // sprite's height is 16 pixel high
                        spriteHeight += 8;
                    }
                    // Check if this sprite can be seen on this scanline
                    if ((_scanLine >= _spritesSecondary[secondaryIndex].positionY) &&
                        (_scanLine < spriteHeight)) {
                        memcpy(&_spritesSecondary[secondaryIndex], &_sprites[index], sizeof(SpriteInformation));
                        secondaryIndex++;
                        if (index == 0) {
                            _spriteZeroNextScanLine = true;
                        }
                    }
                    if (secondaryIndex >= PPU_MAX_SPRITES_SECONDARY) {
                        // We've filled up our Secondary OAM buffer, stop
                        // evaluating sprites from Primary OAM buffer.
                        break;
                    }
                }

                // Sprite Overflow Detection: Emulate hardware bug
                // Check the remaining Primary OAM buffer, but in a weird way.
                // Get to the next primary OAM index after evaluating
                index++;
                uint8_t* OAMData = reinterpret_cast<uint8_t*>(_sprites);
                uint8_t infoIndex = 0;
                for (; index < PPU_MAX_SPRITES; index++) {
                    // This is the bug: infoIndex should've remained 0 while
                    // searching for a sprite overflow, but what happened is
                    // that this index got always incremented, so we're now
                    // checking for the wrong data as positionY, thus could led
                    // to false positives and negatives.
                    uint8_t positionY = OAMData[index * 4 + infoIndex];
                    uint8_t spriteHeight = positionY + 8;
                    if (registers.controlFlag.spriteSize) {
                        // sprite's height is 16 pixel high
                        spriteHeight += 8;
                    }
                    // Check if this sprite can be seen on this scanline
                    if ((_scanLine >= positionY) && (_scanLine < spriteHeight)) {
                        // Set Sprite Overflow flag
                        registers.statusFlag.spriteOverflow = true;
                        break;
                    }
                    infoIndex++;
                    if (infoIndex >= 4) {
                        infoIndex = 0;
                    }
                }
                break;
            }
            case 257 ... 320:
            {
                if (_cycles == 257) {
                    // Copy Sprite Zero Possible flag for this scanline
                    _spriteZeroOnScanLine = _spriteZeroNextScanLine;
                }
                // Sprite fetches: 8 sprites total, 8 cycles per sprite
                uint8_t spriteIndex = (_cycles - 257) / 8;
                switch ((_cycles - 257) % 8) {
                case 0:
                {
                    uint8_t positionY = _scanLine - _spritesSecondary[spriteIndex].positionY;
                    uint8_t tileIndex = _spritesSecondary[spriteIndex].tileIndex;
                    _spriteAttribute[spriteIndex] = _spritesSecondary[spriteIndex].attributes;
                    _spritePositionX[spriteIndex] = _spritesSecondary[spriteIndex].positionX;

                    if (!registers.controlFlag.spriteSize) {
                        // Sprite is 8x8 dimension

                        // Check which table we get our sprite from
                        if (registers.controlFlag.spritePatternTable) {
                            _spritePatternAddress = patternTableBackgroundAddress;
                        } else {
                            _spritePatternAddress = patternTableSpriteAddress;
                        }

                        // Check if sprite is flipped vertically from its attribute
                        if (_spritesSecondary[spriteIndex].attributeFlag.isVerticalFlip) {
                            // Invert our Y position (8 pixel high)
                            positionY = 7 - positionY;
                        }

                        // 16 bytes per tile and which position in a row
                        _spritePatternAddress += 16 * tileIndex + positionY;
                    } else {
                        // Sprite is 8x16 dimension

                        // Check which table we get our sprite from
                        if (tileIndex & 0x01) {
                            _spritePatternAddress = patternTableBackgroundAddress;
                        } else {
                            _spritePatternAddress = patternTableSpriteAddress;
                        }

                        // Check if sprite is flipped vertically from its attribute
                        if (_spritesSecondary[spriteIndex].attributeFlag.isVerticalFlip) {
                            // Invert our Y position (16 pixel high)
                            positionY = 15 - positionY;
                        }

                        // Deal with the 16-pixel height
                        if (positionY < 8) {
                            // Top half of Tile
                            // 16 bytes per tile and which position in a row
                            _spritePatternAddress += 16 * (tileIndex & 0xFE) + positionY;
                        } else {
                            // Bottom half of Tile
                            // 16 bytes per tile and add extra 16 for the next half
                            // Adjust the position to point to correct row
                            _spritePatternAddress += 16 * (tileIndex & 0xFE) + 16 + (positionY - 8);
                        }
                    }
                    break;
                }
                case 4:
                    // Fetch Low Sprite Tile
                    if (_spritesSecondary[spriteIndex].positionY >= (PPU_FRAME_HEIGHT - 1)) {
                        // Not a visible sprite, set to transparent sprite
                        shiftRegisterLowSpriteTile[spriteIndex] = 0x00;
                    } else {
                        _bus->read(_spritePatternAddress, shiftRegisterLowSpriteTile[spriteIndex]);
                        if (_spritesSecondary[spriteIndex].attributeFlag.isHorizontalFlip) {
                            _flipBits(shiftRegisterLowSpriteTile[spriteIndex]);
                        }
                    }
                    break;
                case 6:
                    // Fetch High Sprite Tile
                    if (_spritesSecondary[spriteIndex].positionY >= (PPU_FRAME_HEIGHT - 1)) {
                        // Not a visible sprite, set to transparent sprite
                        shiftRegisterHighSpriteTile[spriteIndex] = 0x00;
                    } else {
                        _bus->read(_spritePatternAddress + 8, shiftRegisterHighSpriteTile[spriteIndex]);
                        if (_spritesSecondary[spriteIndex].attributeFlag.isHorizontalFlip) {
                            _flipBits(shiftRegisterHighSpriteTile[spriteIndex]);
                        }
                    }
                    break;
                default:
                    break;
                }
                break;
            }
            default:
                break;
            }
        }

        break;
    }
    case 240:
        // Post-render scanline (240)
        // PPU just idles during this scanline.
        break;
    case 241 ... 260:
    {
        // Vertical blanking lines (241-260)
        // The VBlank flag is set at tick1 (the second tick) of scanline 241
        if ((_scanLine == 241) && (_cycles == 1)) {
            registers.statusFlag.verticalBlankFlag = true;
            if (registers.controlFlag.NMIEnabledOnVBlank) {
                _vBlank = true;
            }
        }
        break;
    }
    default:
        break;
    }

    // Copy pixel to our frame buffer
    if ((_scanLine >= 0) && (_scanLine <= 239) &&
        (_cycles >= 1) && (_cycles <= 256)) {
        auto pixelIndex = uint8_t{0x00};
        auto paletteIndex = uint8_t{0x00};
        _getIndexFromShiftRegisters(pixelIndex, paletteIndex);

        Pixel myPixel = _getPixelPaletteTable(pixelIndex, paletteIndex);
        _frameBufferRGB[_bufferPixelIndex++] = myPixel.red;
        _frameBufferRGB[_bufferPixelIndex++] = myPixel.green;
        _frameBufferRGB[_bufferPixelIndex++] = myPixel.blue;
    }

    // Increment cycles and scanlines based on PPU rendering
    _cycles++;
    if (_cycles > 340) {
        _cycles = 0;
        _scanLine++;
        if (_scanLine > 261) {
            _scanLine = 0;
            _bufferPixelIndex = 0;
            _frameDone = true;
        }
    }
}

void Ppu::reset()
{
    memset(&registers, 0, sizeof(PpuRegister));

    // PPU background rendering
    nextNameTableByte = 0x00;
    nextAttributeByte = 0x00;
    nextLowBGTileByte = 0x00;
    nextHighBGTileByte = 0x00;
    shiftRegisterLowBGTile = 0x0000;
    shiftRegisterHighBGTile = 0x0000;
    shiftRegisterLowAttribute = 0x0000;
    shiftRegisterHighAttribute = 0x0000;

    memset(&_sprites, 0, sizeof(_sprites));
    memset(&_spritesSecondary, 0, sizeof(_spritesSecondary));
    _spriteZeroNextScanLine = false;
    _spriteZeroOnScanLine = false;
    _spriteZeroUsed = false;
    _spritePatternAddress = 0x0000;
    _oamAddress = 0x00;
    memset(&shiftRegisterLowSpriteTile, 0, sizeof(shiftRegisterLowSpriteTile));
    memset(&shiftRegisterHighSpriteTile, 0, sizeof(shiftRegisterHighSpriteTile));
    memset(&_spriteAttribute, 0, sizeof(_spriteAttribute));
    memset(&_spritePositionX, 0, sizeof(_spritePositionX));
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
    auto pattern = _getPatternTableTile(0, 1);
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

    /*
    // Getting NameTable #0
    auto nameTable = _getNameTableTile(0);
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
    }*/

    return _frameBufferRGB;
}

NameTableTile Ppu::_getNameTableTile(uint8_t index)
{
    auto nameTableAddress = uint16_t{0x0000};
    switch (index) {
    case 0:
        nameTableAddress = nameTable1StartAddress;
        break;
    case 1:
        nameTableAddress = nameTable2StartAddress;
        break;
    case 2:
        nameTableAddress = nameTable3StartAddress;
        break;
    case 3:
        nameTableAddress = nameTable4StartAddress;
        break;
    default:
        break;
    }

    // Temporarily get pattern #0 using palette #0
    auto pattern = _getPatternTableTile(1, 1);

    // Let's access the name table: 32x30 tiles
    for (uint16_t tile = 0; tile < 32 * 30; tile++) {
        auto patternIndex = uint8_t{0x00};
        _bus->read(nameTableAddress + tile, patternIndex);

        // Copy this Pattern to our Name Table
        _nameTablePixel[index].tile[tile] = pattern.tile[patternIndex];
    }

    return _nameTablePixel[index];
}

PatternTableTile Ppu::_getPatternTableTile(uint8_t type, uint8_t paletteIndex)
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
                auto pixelIndex = ((highByte & 0x01) << 1) + (lowByte & 0x01);
                highByte = highByte >> 1;
                lowByte = lowByte >> 1;

                auto currentPixel = _getPixelPaletteTable(pixelIndex, paletteIndex);
                _patternTablePixel[type].tile[tile].pixel[x * 8 + (y - 1)] = currentPixel;
            }
        }
    }

    return _patternTablePixel[type];
}

Pixel Ppu::_getPixelPaletteTable(uint8_t pixelIndex, uint8_t paletteIndex)
{
    auto paletteByte = uint8_t{0x00};
    _bus->read(paletteTableBaseAddress + paletteIndex * 4 + pixelIndex, paletteByte);
    return _paletteTablePixel[paletteByte];
}

void Ppu::_incrementVramHorizontalInfo()
{
    if (registers.maskFlag.showBackground || registers.maskFlag.showSprites) {
        if (registers.currVramFlag.coarseXScroll < (32 - 1)) {
            registers.currVramFlag.coarseXScroll++;
        } else {
            // We're at the end of our horizontal tile, wrap around
            registers.currVramFlag.coarseXScroll = 0;

            // Toggle Horizontal NameTable
            registers.currVramFlag.nameTableXAddress = !registers.currVramFlag.nameTableXAddress;
        }
    }
}

void Ppu::_incrementVramVerticalInfo()
{
    if (registers.maskFlag.showBackground || registers.maskFlag.showSprites) {
        if (registers.currVramFlag.fineYScroll < (8 - 1)) {
            registers.currVramFlag.fineYScroll++;
        } else {
            // We're at the end of our vertical pixel, wrap around
            registers.currVramFlag.fineYScroll = 0;
            if (registers.currVramFlag.coarseYScroll < (30 - 1)) {
                registers.currVramFlag.coarseYScroll++;
            } else {
                // We're at the end of our vertical tile, wrap around
                registers.currVramFlag.coarseYScroll = 0;

                // Toggle Vertical NameTable
                registers.currVramFlag.nameTableYAddress = !registers.currVramFlag.nameTableYAddress;
            }
        }
    }
}

void Ppu::_updateVramHorizontalInfo()
{
    if (registers.maskFlag.showBackground || registers.maskFlag.showSprites) {
        registers.currVramFlag.coarseXScroll = registers.tempVramFlag.coarseXScroll;
        registers.currVramFlag.nameTableXAddress = registers.tempVramFlag.nameTableXAddress;
    }
}

void Ppu::_updateVramVerticalInfo()
{
    if (registers.maskFlag.showBackground || registers.maskFlag.showSprites) {
        registers.currVramFlag.coarseYScroll = registers.tempVramFlag.coarseYScroll;
        registers.currVramFlag.fineYScroll = registers.tempVramFlag.fineYScroll;
        registers.currVramFlag.nameTableYAddress = registers.tempVramFlag.nameTableYAddress;
    }
}

uint8_t Ppu::_getNextNameTableByte()
{
    // The lower 12-bits of the PPU current VRAM register is the
    // address of the tile to fetch: Bit10/11 has the info which
    // NameTable and Bit0-9 has the info which byte from that NT.
    auto data = uint8_t{0x00};
    auto tile = registers.currVramAddress & 0x0FFF;
    _bus->read(nameTableBaseAddress + tile, data);

    return data;
}

uint8_t Ppu::_getNextAttributeByte()
{
    // The location of the attribute  byte is found inside the
    // nametable. Bit10/11 has still the info which NameTable to
    // fetch:
    auto data = uint8_t{0x00};
    auto table = registers.currVramAddress & 0x0C00;

    // For each 4x4 tiles in the NT represents one byte in the
    // attribute table, thus we simple divide each scroll by 4 to
    // get which X,Y byte in the attribute table:
    auto attributeX = registers.currVramFlag.coarseXScroll / 4;
    auto attributeY = registers.currVramFlag.coarseYScroll / 4;
    // Since it has now become an 8x8 AT from a 32x32 NT:
    auto attribute = attributeX + attributeY * 8;
    _bus->read(attributeBaseAddress + table + attribute, data);

    // Now, deduce which specific tile we are on, so we know the
    // exact 2-bit palette to use from the 8-bit attribute byte:
    //   bit 0/1 - top left 2x2 tile
    //   bit 2/3 - top right 2x2 tile
    //   bit 4/5 - bottom left 2x2 tile
    //   bit 6/7 - bottom right 2x2 tile
    auto isTopPosition = ((registers.currVramFlag.coarseYScroll % 4 == 0) ||
            (registers.currVramFlag.coarseYScroll % 4 == 1));
    auto isLeftPosition = ((registers.currVramFlag.coarseXScroll % 4 == 0) ||
            (registers.currVramFlag.coarseXScroll % 4 == 1));

    // Only retain the 2-bit palette
    if (isTopPosition && isLeftPosition) {
        // Top left 2x2 tile
        data = (data) & 0x03;
    } else if (isTopPosition && !isLeftPosition) {
        // Top right 2x2 tile
        data = (data >> 2) & 0x03;
    } else if (!isTopPosition && isLeftPosition) {
        // Bottom left 2x2 tile
        data = (data >> 4) & 0x03;
    } else if (!isTopPosition && !isLeftPosition) {
        // Bottom right 2x2 tile
        data = (data >> 6) & 0x03;
    }

    return data;
}

uint8_t Ppu::_getBackgroundTileByte(bool isMSB)
{
    auto data = uint8_t{0x00};
    auto patternAddress = uint16_t{0x0000};
    if (registers.controlFlag.backgroundPatternTable) {
        patternAddress = patternTableBackgroundAddress;
    } else {
        patternAddress = patternTableSpriteAddress;
    }

    // One tile is worth 16 bytes in the Pattern Table:
    // 8-byte for LSB and another 8-byte for MSB
    auto fineYScroll = registers.currVramFlag.fineYScroll;
    if (isMSB) {
        fineYScroll += 8;
    }
    _bus->read(patternAddress + (nextNameTableByte * 16) + (fineYScroll), data);

    return data;
}

void Ppu::_loadShiftRegisters()
{
    shiftRegisterLowBGTile = (shiftRegisterLowBGTile & 0xFF00) | nextLowBGTileByte;
    shiftRegisterHighBGTile = (shiftRegisterHighBGTile & 0xFF00) | nextHighBGTileByte;

    // Attribute value is only 4-bits, let's split it in Low (bit0) and High
    // (bit1). Since this value apply for the whole 8 cycles (or 8 pixels),
    // let's just write 0xFF if bit is 1 and 0x00 if bit is 0.
    auto lowAttributeByte = (nextAttributeByte & 0x01) ? 0xFF : 0x00;
    auto highAttributeByte = (nextAttributeByte & 0x02) ? 0xFF : 0x00;
    shiftRegisterLowAttribute = (shiftRegisterLowAttribute & 0xFF00) | lowAttributeByte;
    shiftRegisterHighAttribute = (shiftRegisterHighAttribute & 0xFF00) | highAttributeByte;
}

void Ppu::_moveShiftRegisters()
{
    if (registers.maskFlag.showBackground) {
        shiftRegisterLowBGTile <<= 1;
        shiftRegisterHighBGTile <<= 1;
        shiftRegisterLowAttribute <<= 1;
        shiftRegisterHighAttribute <<= 1;
    }
}

void Ppu::_getIndexFromShiftRegisters(uint8_t& pixelIndex, uint8_t& paletteIndex)
{
    uint8_t backgroundPixelIndex = 0x00;
    uint8_t backgroundPaletteIndex = 0x00;
    uint8_t spritePixelIndex = 0x00;
    uint8_t spritePaletteIndex = 0x00;
    bool spritePriority = false;

    if (registers.maskFlag.showBackground) {
        // Selects which bit in the MSB of our shift registers to figure out
        // which pattern and attribute to render pixel at this point
        uint16_t selectBit = 0x8000 >> registers.fineXScroll;

        auto lowBit = uint8_t{0x00};
        auto highBit = uint8_t{0x00};
        if (shiftRegisterLowBGTile & selectBit) {
            lowBit = 0x01;
        }
        if (shiftRegisterHighBGTile & selectBit) {
            highBit = 0x01;
        }
        backgroundPixelIndex = (highBit << 1) + lowBit;

        lowBit = 0x00;
        highBit = 0x00;
        if (shiftRegisterLowAttribute & selectBit) {
            lowBit = 0x01;
        }
        if (shiftRegisterHighAttribute & selectBit) {
            highBit = 0x01;
        }
        backgroundPaletteIndex = (highBit << 1) + lowBit;
    }

    if (registers.maskFlag.showSprites) {
        _spriteZeroUsed = false;
        for (uint8_t i = 0; i < PPU_MAX_SPRITES_SECONDARY; i++) {
            if (_spritePositionX[i] > 0) {
                _spritePositionX[i]--;
            } else {
                shiftRegisterLowSpriteTile[i] <<= 1;
                shiftRegisterHighSpriteTile[i] <<= 1;
            }
        }
        for (uint8_t i = 0; i < PPU_MAX_SPRITES_SECONDARY; i++) {
            if (_spritePositionX[i] == 0) {
                auto lowBit = uint8_t{0x00};
                auto highBit = uint8_t{0x00};
                if (shiftRegisterLowSpriteTile[i] & 0x80) {
                    lowBit = 0x01;
                }
                if (shiftRegisterHighSpriteTile[i] & 0x80) {
                    highBit = 0x01;
                }
                spritePixelIndex = (highBit << 1) + lowBit;

                SpriteAttributeFlags spriteAttribute = *((SpriteAttributeFlags*)&_spriteAttribute[i]);
                spritePriority = !spriteAttribute.isBehindBackground;

                // Add 0x04 since this is on sprite palette table
                spritePaletteIndex = spriteAttribute.spritePaletteIndex + 0x04;

                if (spritePixelIndex != 0) {
                    // Check if SpriteZero is being used
                    if (i == 0 && _spriteZeroOnScanLine) {
                        _spriteZeroUsed = true;
                    }
                    break;
                }
            }
        }
    }

    if (backgroundPixelIndex == 0) {
        if (spritePixelIndex == 0) {
            // Nothing to draw
            pixelIndex = 0x00;
            paletteIndex = 0x00;
        } else {
            pixelIndex = spritePixelIndex;
            paletteIndex = spritePaletteIndex;
        }
    } else {
        if (spritePixelIndex == 0) {
            pixelIndex = backgroundPixelIndex;
            paletteIndex = backgroundPaletteIndex;
        } else {
            // Let's check if sprite has priority over background
            if (spritePriority) {
                pixelIndex = spritePixelIndex;
                paletteIndex = spritePaletteIndex;
            } else {
                pixelIndex = backgroundPixelIndex;
                paletteIndex = backgroundPaletteIndex;
            }

            if (_spriteZeroUsed) {
                // Set Sprite Zero Hit flag
                if ((!registers.maskFlag.showBackgroundLeft || !registers.maskFlag.showSpritesLeft) &&
                    ((_cycles >= 1) && (_cycles <= 8))) {
                    // Sprite Zero Hit does not happen at first 8 cycles if the
                    // left-side clipping window is enabled (Bit2 or Bit1 of PPUMASK is 0)
                    // https://wiki.nesdev.com/w/index.php?title=PPU_OAM&redirect=no#Sprite_zero_hits
                    // Do not set sprite zero hit here
                } else {
                    registers.statusFlag.spriteZeroHit = true;
                }
            }
        }
    }
}

void Ppu::writeOAMData(uint8_t address, uint8_t data)
{
    uint8_t* OAMData = reinterpret_cast<uint8_t*>(_sprites);
    OAMData[address] = data;
}

void Ppu::readOAMData(uint8_t address, uint8_t& data)
{
    uint8_t* OAMData = reinterpret_cast<uint8_t*>(_sprites);
    data = OAMData[address];
}

void Ppu::clearSecondaryOAMData(uint8_t data)
{
    uint8_t* secondaryOAMData = reinterpret_cast<uint8_t*>(_spritesSecondary);
    for (uint8_t i = 0; i < PPU_MAX_SPRITES_SECONDARY * sizeof(SpriteInformation); i++) {
        secondaryOAMData[i] = data;
    }
}

void Ppu::_flipBits(uint8_t& byte)
{
    auto copyByte = byte;
    byte = 0x00;
    byte |= (copyByte & 0x01) << 7;
    byte |= (copyByte & 0x02) << 5;
    byte |= (copyByte & 0x04) << 3;
    byte |= (copyByte & 0x08) << 1;
    byte |= (copyByte & 0x10) >> 1;
    byte |= (copyByte & 0x20) >> 3;
    byte |= (copyByte & 0x40) >> 5;
    byte |= (copyByte & 0x80) >> 7;
}
