#include "Ppu.hpp"

constexpr uint8_t resetStackOffset = 0xFD;
constexpr uint16_t stackBaseAddress = 0x0100;
constexpr uint16_t nonMaskableInterruptAddress = 0xFFFA;
constexpr uint16_t resetInterruptAddress = 0xFFFC;
constexpr uint16_t breakInterruptAddress = 0xFFFE;

Ppu::Ppu(std::shared_ptr<IDevice> bus,
    std::shared_ptr<Cartridge> cartridge)
: _bus{bus}
, _cartridge{cartridge}
{
}

Ppu::~Ppu()
{
}

bool Ppu::read(uint16_t address, uint8_t &/*data*/)
{
    switch (static_cast<PpuRegisterAddress>(address)) {
    case PpuRegisterAddress::Control:
        break;
    case PpuRegisterAddress::Mask:
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        break;
    case PpuRegisterAddress::VRAMData:
        break;
    default:
        break;
    }

    return true;
}

bool Ppu::write(uint16_t address, uint8_t /*data*/)
{
    switch (static_cast<PpuRegisterAddress>(address)) {
    case PpuRegisterAddress::Control:
        break;
    case PpuRegisterAddress::Mask:
        break;
    case PpuRegisterAddress::Status:
        break;
    case PpuRegisterAddress::OAMAddress:
        break;
    case PpuRegisterAddress::OAMData:
        break;
    case PpuRegisterAddress::Scroll:
        break;
    case PpuRegisterAddress::VRAMAddress:
        break;
    case PpuRegisterAddress::VRAMData:
        break;
    default:
        break;
    }

    return true;
}

// Execute one clock cycle
void Ppu::tick()
{
	_cycles++;
	if (_cycles >= 341)
	{
		_cycles = 0;
		_scanLine++;
		if (_scanLine >= 261)
		{
			_scanLine = -1;
			_frameDone = true;
		}
	}
}

