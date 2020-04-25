#pragma once

#include <memory>

#include "Memory2KB.hpp"
#include "Controller.hpp"
#include "CpuBus.hpp"
#include "NameTable.hpp"
#include "PaletteTable.hpp"
#include "PpuBus.hpp"
#include "Cartridge.hpp"
#include "Ppu.hpp"
#include "Cpu.hpp"

enum class NesButton {
    Right = 0,
    Left,
    Down,
    Up,
    Start,
    Select,
    B,
    A
};

class Nes {
public:
    Nes();
    ~Nes();

    void load(std::string fileName);
    void reset();
    void renderFrame();
    uint8_t* getFrameBuffer();
    void setControllerKey(uint8_t id, NesButton button, bool state);
    uint32_t getWidth() const { return PPU_FRAME_WIDTH; };
    uint32_t getHeight() const { return PPU_FRAME_HEIGHT; };
    const char* getName() const { return _fileName.c_str(); };

private:
    std::string _fileName;

    std::shared_ptr<Controller> _controller;
    std::shared_ptr<IMemory> _cpuRam;
    std::shared_ptr<IDevice> _cpuBus;
    std::shared_ptr<IMemory> _nameTable;
    std::shared_ptr<IMemory> _paletteTable;
    std::shared_ptr<IDevice> _ppuBus;

    std::shared_ptr<Cartridge> _cartridge;
    std::shared_ptr<Ppu> _ppu;
    std::shared_ptr<Cpu> _cpu;

    uint8_t _counter{0x00};
};
