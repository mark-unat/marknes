#pragma once

#include <functional>
#include <vector>
#include <string>

#include "IDevice.hpp"
#include "ITable.hpp"
#include "Cartridge.hpp"

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
    uint8_t vramAddress;
    uint8_t vramData;
} PpuRegister;

class Ppu {
public:
	Ppu(std::shared_ptr<IDevice> bus,
        std::shared_ptr<Cartridge> cartridge);
	~Ppu();

	bool read(uint16_t address, uint8_t &data);
	bool write(uint16_t address, uint8_t data);

    // Execute one clock cycle
	void tick();

    // PPU registers
    PpuRegister registers;

private:
	uint16_t _cycles = 0;
	uint16_t _scanLine = 0;
	bool _frameDone = 0;

    // Tables
    std::shared_ptr<ITable> _nameTable;
    std::shared_ptr<ITable> _patternTable;
    std::shared_ptr<ITable> _paletteTable;

	// Bus device attached to this Ppu
    std::shared_ptr<IDevice> _bus;

    // NES Catridge
    std::shared_ptr<Cartridge> _cartridge;
};
