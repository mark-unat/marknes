#pragma once

#include <stdio.h>
#include <functional>
#include <vector>
#include <string>
#include <array>

#include "IDevice.hpp"
#include "IMemory.hpp"
#include "Cartridge.hpp"

#define PPU_FRAME_WIDTH 256
#define PPU_FRAME_HEIGHT 240
#define PPU_FRAME_BUFFER_SIZE (PPU_FRAME_WIDTH * PPU_FRAME_HEIGHT)
#define PPU_FRAME_BUFFER_RGB_SIZE (PPU_FRAME_BUFFER_SIZE * 3)

#define PPU_MAX_SPRITES 64
#define PPU_MAX_SPRITES_SECONDARY 8

enum class PpuRegisterAddress {
    Control,
    Mask,
    Status,
    OAMAddress,
    OAMData,
    Scroll,
    VRAMAddress,
    VRAMData,
};

struct ControlFlags {
    bool nameTableXAddress : 1;
    bool nameTableYAddress : 1;
    bool vramAddressIncrement : 1;
    bool spritePatternTable : 1;
    bool backgroundPatternTable : 1;
    bool spriteSize : 1;
    bool masterSalveSelect : 1;
    bool NMIEnabledOnVBlank : 1;
};

struct MaskFlags {
    bool greyScale : 1;
    bool showBackgroundLeft : 1;
    bool showSpritesLeft : 1;
    bool showBackground : 1;
    bool showSprites : 1;
    bool emphasizeRed : 1;
    bool emphasizeGreen : 1;
    bool emphasizeBlue : 1;
};

struct PpuStatusFlags {
    uint8_t unused : 5;
    bool spriteOverflow : 1;
    bool spriteZeroHit : 1;
    bool verticalBlankFlag : 1;
};

struct PpuVramFlags {
    uint16_t coarseXScroll : 5;
    uint16_t coarseYScroll : 5;
    uint16_t nameTableXAddress : 1;
    uint16_t nameTableYAddress : 1;
    uint16_t fineYScroll : 3;
    uint16_t unused : 1;
};

struct PpuRegister {
    union {
        uint8_t control;
        ControlFlags controlFlag;
    };
    union {
        uint8_t mask;
        MaskFlags maskFlag;
    };
    union {
        uint8_t status;
        PpuStatusFlags statusFlag;
    };
    uint8_t oamAddress;
    uint8_t oamData;
    uint8_t scroll;
    union {
        uint16_t currVramAddress;
        PpuVramFlags currVramFlag;
    };
    union {
        uint16_t tempVramAddress;
        PpuVramFlags tempVramFlag;
    };
    uint8_t fineXScroll;
    bool vramAddressLatch;
    uint8_t currVramData;
    uint8_t tempVramData;
};

struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct Tile {
    Pixel pixel[8 * 8];
};

struct PatternTableTile {
    Tile tile[16 * 16];
};

struct NameTableTile {
    Tile tile[32 * 30];
};

// The OAM (Object Attribute Memory) is internal memory inside the PPU that
// contains a display list of up to 64 sprites, where each sprite's information
// occupies 4 bytes.
// Reference: https://wiki.nesdev.com/w/index.php/PPU_OAM
struct SpriteAttributeFlags {
    uint8_t spritePaletteIndex : 2;
    uint8_t unused : 3;
    bool isBehindBackground : 1;
    bool isHorizontalFlip : 1;
    bool isVerticalFlip : 1;
};

struct SpriteInformation {
    uint8_t positionY;
    uint8_t tileIndex;
    union {
        uint8_t attributes;
        SpriteAttributeFlags attributeFlag;
    };
    uint8_t positionX;
};

class Ppu {
public:
    Ppu(std::shared_ptr<IDevice> bus, std::shared_ptr<Cartridge> cartridge);
    ~Ppu();

    bool read(uint16_t address, uint8_t& data);
    bool write(uint16_t address, uint8_t data);

    // Execute one clock cycle
    void tick();
    void reset();

    // Get Frame Buffer
    uint8_t* getFrameBuffer();
    bool isFrameDone();
    bool isVBlankTriggered();

    // OAM Interface
    void writeOAMData(uint8_t address, uint8_t data);
    void readOAMData(uint8_t address, uint8_t& data);
    void clearSecondaryOAMData(uint8_t data);

    // PPU registers
    PpuRegister registers;

private:
    PatternTableTile _getPatternTableTile(uint8_t type, uint8_t paletteIndex);
    Pixel _getPixelPaletteTable(uint8_t pixelIndex, uint8_t paletteIndex);
    NameTableTile _getNameTableTile(uint8_t index);

    void _incrementVramHorizontalInfo();
    void _incrementVramVerticalInfo();
    void _updateVramHorizontalInfo();
    void _updateVramVerticalInfo();
    uint8_t _getNextNameTableByte();
    uint8_t _getNextAttributeByte();
    uint8_t _getBackgroundTileByte(bool isMSB);
    void _loadShiftRegisters();
    void _moveShiftRegisters();
    void _getIndexFromShiftRegisters(uint8_t& pixelIndex, uint8_t& paletteIndex);
    void _flipBits(uint8_t& byte);

    uint16_t _cycles = 0;
    uint16_t _scanLine = 0;
    uint32_t _bufferPixelIndex = 0;
    bool _frameDone{false};
    bool _vBlank{false};

    // PPU background rendering
    uint8_t nextNameTableByte;
    uint8_t nextAttributeByte;
    uint8_t nextLowBGTileByte;
    uint8_t nextHighBGTileByte;

    // Shift registers
    uint16_t shiftRegisterLowBGTile;
    uint16_t shiftRegisterHighBGTile;
    uint16_t shiftRegisterLowAttribute;
    uint16_t shiftRegisterHighAttribute;

    // Bus device attached to this Ppu
    std::shared_ptr<IDevice> _bus;

    // NES Catridge
    std::shared_ptr<Cartridge> _cartridge;

    uint8_t _frameBufferRGB[PPU_FRAME_BUFFER_RGB_SIZE];
    std::vector<Pixel> _paletteTablePixel;
    std::array<PatternTableTile, 2> _patternTablePixel;
    std::array<NameTableTile, 4> _nameTablePixel;

    // Sprites
    SpriteInformation _sprites[PPU_MAX_SPRITES];
    SpriteInformation _spritesSecondary[PPU_MAX_SPRITES_SECONDARY];
    uint8_t _oamAddress{0x00};
    bool _spriteZeroNextScanLine{false};
    bool _spriteZeroOnScanLine{false};
    bool _spriteZeroUsed{false};
    uint16_t _spritePatternAddress{0x0000};
    uint8_t shiftRegisterLowSpriteTile[PPU_MAX_SPRITES_SECONDARY];
    uint8_t shiftRegisterHighSpriteTile[PPU_MAX_SPRITES_SECONDARY];
    uint8_t _spriteAttribute[PPU_MAX_SPRITES_SECONDARY];
    uint8_t _spritePositionX[PPU_MAX_SPRITES_SECONDARY];
};
