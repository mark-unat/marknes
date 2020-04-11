#pragma once

#include <cstdint>

#include "IDevice.hpp"
#include "IMemory.hpp"
#include "Cartridge.hpp"

constexpr auto patternTableBaseAddress = 0x0000;
constexpr auto patternTableSpriteAddress = 0x0000;
constexpr auto patternTableBackgroundAddress = 0x1000;
constexpr auto patternTableEndAddress = 0x1FFF;

constexpr auto nameTableBaseAddress = 0x2000;
constexpr auto nameTable1StartAddress = 0x2000;
constexpr auto nameTable1EndAddress = 0x23FF;
constexpr auto nameTable2StartAddress = 0x2400;
constexpr auto nameTable2EndAddress = 0x27FF;
constexpr auto nameTable3StartAddress = 0x2800;
constexpr auto nameTable3EndAddress = 0x2BFF;
constexpr auto nameTable4StartAddress = 0x2C00;
constexpr auto nameTable4EndAddress = 0x2FFF;
constexpr auto nameTableEndAddress = 0x3EFF;

constexpr auto paletteTableBaseAddress = 0x3F00;
constexpr auto paletteTableBG1Address = 0x3F10;
constexpr auto paletteTableBG2Address = 0x3F14;
constexpr auto paletteTableBG3Address = 0x3F18;
constexpr auto paletteTableBG4Address = 0x3F1C;
constexpr auto paletteTableEndAddress = 0x3FFF;

class PpuBus : public IDevice {
public:
    PpuBus(std::shared_ptr<IMemory> nameTable,
           std::shared_ptr<IMemory> paletteTable,
           std::shared_ptr<Cartridge> cartridge);

    /// @name Implementation IDevice
    /// @[
    bool read(uint16_t address, uint8_t& data);
    bool write(uint16_t address, uint8_t data);
    /// @]
private:
    // Memories attached to this Ppu Bus
    std::shared_ptr<IMemory> _nameTable;
    std::shared_ptr<IMemory> _paletteTable;

    std::shared_ptr<Cartridge> _cartridge;
};
