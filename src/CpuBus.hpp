#pragma once

#include <cstdint>

#include "IDevice.hpp"
#include "IMemory.hpp"
#include "Ppu.hpp"
#include "Cartridge.hpp"

constexpr auto memoryBaseAddress = 0x0000;
constexpr auto memoryEndAddress  = 0x1FFF;

constexpr auto ppuBaseAddress = 0x2000;
constexpr auto ppuEndAddress  = 0x3FFF;

constexpr auto cartridgeBaseAddress = 0x8000;
constexpr auto cartridgeEndAddress  = 0xFFFF;

class CpuBus : public IDevice {
public:
    CpuBus(std::shared_ptr<IMemory> memory,
        std::shared_ptr<Ppu> ppu,
        std::shared_ptr<Cartridge> cartridge);

    /// @name Implementation IDevice
    /// @[
	bool read(uint16_t address, uint8_t &data);
	bool write(uint16_t address, uint8_t data);
    /// @]
private:
	// Memory device attached to this Cpu Bus
    std::shared_ptr<IMemory> _memory;

    // PPU Interface
    std::shared_ptr<Ppu> _ppu;

    // NES Catridge
    std::shared_ptr<Cartridge> _cartridge;
};
