#pragma once

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

typedef struct ControlFlags {
    uint8_t nameTableAddress : 2;
    bool vramAddressIncrement : 1;
    bool spritePatternTable : 1;
    bool backgroundPatternTable : 1;
    bool spriteSize : 1;
    bool masterSalveSelect : 1;
    bool NMIEnabledOnVBlank : 1;
} ControlFlags;

typedef struct MaskFlags {
    bool greyScale : 1;
    bool showBackgroundLeft : 1;
    bool showSpritesLeft : 1;
    bool showBackground : 1;
    bool showSprites : 1;
    bool emphasizeRed : 1;
    bool emphasizeGreen : 1;
    bool emphasizeBlue : 1;
} MaskFlags;

typedef struct PpuStatusFlags {
    uint8_t unused : 5;
    bool spriteOverflow : 1;
    bool spriteZeroHit : 1;
    bool verticalBlankFlag : 1;
} PpuStatusFlags;

typedef struct PpuVramFlags {
    uint8_t coarseXScroll : 5;
    uint8_t coarseYScroll : 5;
    uint8_t nameTableAddress : 2;
    uint8_t fineYScroll : 3;
    uint8_t unused : 1;
} PpuVramFlags;

typedef struct PpuRegister {
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
} PpuRegister;

typedef struct Pixel {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} Pixel;

typedef struct Tile {
    Pixel pixel[8 * 8];
} Tile;

typedef struct PatternTableTile {
    Tile tile[16 * 16];
} PatternTableTile;

typedef struct NameTableTile {
    Tile tile[32 * 30];
} NameTableTile;

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
    PatternTableTile getPatternTableTile(uint8_t type, uint8_t paletteIndex);
    NameTableTile getNameTableTile(uint8_t index);
    bool isFrameDone();
    bool isVBlankTriggered();

    // PPU registers
    PpuRegister registers;

private:
    uint16_t _cycles = 0;
    uint16_t _scanLine = 0;
    bool _frameDone{false};
    bool _vBlank{false};

    // Bus device attached to this Ppu
    std::shared_ptr<IDevice> _bus;

    // NES Catridge
    std::shared_ptr<Cartridge> _cartridge;

    uint8_t _frameBufferRGB[PPU_FRAME_BUFFER_RGB_SIZE];
    std::vector<Pixel> _paletteTablePixel;
    std::array<PatternTableTile, 2> _patternTablePixel;
    std::array<NameTableTile, 4> _nameTablePixel;
};
