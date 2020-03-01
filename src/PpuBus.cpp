#include "PpuBus.hpp"

PpuBus::PpuBus(std::shared_ptr<ITable> nameTable,
        std::shared_ptr<ITable> patternTable,
        std::shared_ptr<ITable> paletteTable)
: _nameTable{nameTable}
, _patternTable{patternTable}
, _paletteTable{paletteTable}
{
}

bool PpuBus::read(uint16_t address, uint8_t &data)
{
    return _nameTable->read(address, data);
}

bool PpuBus::write(uint16_t address, uint8_t data)
{
    return _nameTable->write(address, data);
}

