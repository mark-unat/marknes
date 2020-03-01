#pragma once

#include <cstdint>

#include "IDevice.hpp"
#include "ITable.hpp"

class PpuBus : public IDevice {
public:
    PpuBus(std::shared_ptr<ITable> nameTable,
        std::shared_ptr<ITable> patternTable,
        std::shared_ptr<ITable> paletteTable);

    /// @name Implementation IDevice
    /// @[
	bool read(uint16_t address, uint8_t &data);
	bool write(uint16_t address, uint8_t data);
    /// @]
private:
	// Tables attached to this Ppu Bus
    std::shared_ptr<ITable> _nameTable;
    std::shared_ptr<ITable> _patternTable;
    std::shared_ptr<ITable> _paletteTable;
};
