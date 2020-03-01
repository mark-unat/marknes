#include "CpuBus.hpp"

CpuBus::CpuBus(std::shared_ptr<IMemory> memory)
: _memory{memory}
{
}

bool CpuBus::read(uint16_t address, uint8_t &data)
{
    return _memory->read(address, data);
}

bool CpuBus::write(uint16_t address, uint8_t data)
{
    return _memory->write(address, data);
}

