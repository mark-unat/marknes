#include "CpuBus.hpp"

CpuBus::CpuBus(std::shared_ptr<IMemory> memory,
    std::shared_ptr<Ppu> ppu,
    std::shared_ptr<Cartridge> cartridge)
: _memory{memory}
, _ppu{ppu}
, _cartridge{cartridge}
{
}

bool CpuBus::read(uint16_t address, uint8_t &data)
{
    if ((address >= memoryBaseAddress) && (address <= memoryEndAddress)) {
        return _memory->read(address, data);
    } else if ((address >= ppuBaseAddress) && (address <= ppuEndAddress)) {
        return _ppu->read(address, data);
    } else if ((address >= cartridgeBaseAddress) && (address <= cartridgeEndAddress)) {

        return _cartridge->readPrgRom(address, data);
    } else {
    }

    return false;
}

bool CpuBus::write(uint16_t address, uint8_t data)
{
    if ((address >= memoryBaseAddress) && (address <= memoryEndAddress)) {
        return _memory->write(address, data);
    } else if ((address >= ppuBaseAddress) && (address <= ppuEndAddress)) {
        return _ppu->write(address, data);
    } else if ((address >= cartridgeBaseAddress) && (address <= cartridgeEndAddress)) {
        return _cartridge->writePrgRom(address, data);
    } else {
    }

    return false;
}

