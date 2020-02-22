#include "Bus.hpp"

Bus::Bus(std::shared_ptr<IDevice> memory)
: _memory{memory}
{
}

bool Bus::read(uint16_t address, uint8_t* data)
{
    return _memory->read(address, data);
}

bool Bus::write(uint16_t address, uint8_t data)
{
    return _memory->write(address, data);
}

