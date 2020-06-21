#include "CpuBus.hpp"

CpuBus::CpuBus(std::shared_ptr<IMemory> memory,
        std::shared_ptr<Ppu> ppu,
        std::shared_ptr<Cartridge> cartridge,
        std::shared_ptr<IDevice> controller)
: _memory{memory}
, _ppu{ppu}
, _cartridge{cartridge}
, _controller{controller}
{
}

bool CpuBus::read(uint16_t address, uint8_t& data)
{
    switch (address) {
    case memoryBaseAddress ... memoryEndAddress:
        return _memory->read(address, data);
    case ppuBaseAddress ... ppuEndAddress:
        return _ppu->read(address, data);
    case controller1Address ... controller2Address:
        return _controller->read(address, data);
    case cartridgeBaseAddress ... cartridgeEndAddress:
        return _cartridge->readPRG(address, data);
    default:
        break;
    }

    return false;
}

bool CpuBus::write(uint16_t address, uint8_t data)
{
    switch (address) {
    case memoryBaseAddress ... memoryEndAddress:
        return _memory->write(address, data);
    case ppuBaseAddress ... ppuEndAddress:
        return _ppu->write(address, data);
    case controller1Address ... controller2Address:
        return _controller->write(address, data);
    case cartridgeBaseAddress ... cartridgeEndAddress:
        return _cartridge->writePRG(address, data);
    default:
        break;
    }

    return false;
}
