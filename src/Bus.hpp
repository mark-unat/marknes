#pragma once

#include <cstdint>
#include <array>

#include "IDevice.hpp"

class Bus : public IDevice {
public:
    Bus(std::shared_ptr<IDevice> memory);

    /// @name Implementation IDevice
    /// @[
	bool read(uint16_t address, uint8_t *data);
	bool write(uint16_t address, uint8_t data);
    /// @]
private:
	// Memory device attached to this Bus
    std::shared_ptr<IDevice> _memory;
};
