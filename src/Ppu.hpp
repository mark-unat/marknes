#pragma once

#include <functional>
#include <vector>
#include <string>

#include "IDevice.hpp"
#include "ITable.hpp"
#include "Cartridge.hpp"

enum class PpuRegister {
	Control,
    Mask,
	Status,
    OAMAddress,
	OAMData,
    Scroll,
	Address,
    Data,
};

class Ppu {
public:
	Ppu(std::shared_ptr<IDevice> bus,
        std::shared_ptr<Cartridge> cartridge);
	~Ppu();

	bool read(uint16_t address, uint8_t &data);
	bool write(uint16_t address, uint8_t data);

    // Execute one clock cycle
	void tick();

private:
	uint16_t _cycles = 0;
	uint16_t _scanLine = 0;
	bool _frameDone = 0;

    // Tables
    std::shared_ptr<ITable> _nameTable;
    std::shared_ptr<ITable> _patternTable;
    std::shared_ptr<ITable> _paletteTable;

	// Bus device attached to this Ppu
    std::shared_ptr<IDevice> _bus;

    // NES Catridge
    std::shared_ptr<Cartridge> _cartridge;
};
