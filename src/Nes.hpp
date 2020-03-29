#pragma once

#include <memory>

#include "Memory2KB.hpp"
#include "CpuBus.hpp"
#include "NameTable.hpp"
#include "PatternTable.hpp"
#include "PaletteTable.hpp"
#include "PpuBus.hpp"
#include "Cartridge.hpp"
#include "Ppu.hpp"
#include "Cpu.hpp"

#define NES_FRAME_WIDTH PPU_FRAME_WIDTH
#define NES_FRAME_HEIGHT PPU_FRAME_HEIGHT
#define NES_FRAME_BUFFER_SIZE PPU_FRAME_BUFFER_RGB_SIZE

class Nes {
public:
	Nes(std::string fileName);
	~Nes();

    void reset();
    void getFrameBuffer(uint8_t *frameBuffer);

private:
    std::string _fileName;

    std::shared_ptr<IMemory> _cpuRam;
    std::shared_ptr<IDevice> _cpuBus;

    std::shared_ptr<ITable> _nameTable;
    std::shared_ptr<ITable> _patternTable;
    std::shared_ptr<ITable> _paletteTable;
    std::shared_ptr<IDevice> _ppuBus;

    std::shared_ptr<Cartridge> _cartridge;
    std::shared_ptr<Ppu> _ppu;
    std::shared_ptr<Cpu> _cpu;
};

