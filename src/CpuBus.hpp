#pragma once

#include <cstdint>

#include "IDevice.hpp"
#include "IMemory.hpp"

class CpuBus : public IDevice {
public:
    CpuBus(std::shared_ptr<IMemory> memory);

    /// @name Implementation IDevice
    /// @[
	bool read(uint16_t address, uint8_t &data);
	bool write(uint16_t address, uint8_t data);
    /// @]
private:
	// Memory device attached to this Cpu Bus
    std::shared_ptr<IMemory> _memory;
};
