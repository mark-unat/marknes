#pragma once

#include <cstdint>
#include <memory>

#include "IDevice.hpp"

class ITable : public IDevice {
public:
    virtual ~ITable() = default;
	virtual bool read(uint16_t address, uint8_t &data) = 0;
	virtual bool write(uint16_t address, uint8_t data) = 0;

protected:
    std::unique_ptr<std::unique_ptr<uint8_t[]>[]> _table;
};
