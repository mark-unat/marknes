#pragma once

class IDevice {
public:
    virtual ~IDevice() = default;

    /// Read data to the Device
    /// @pararm address - which address to read from
    /// @param data - data that was read
    /// @return bool - if read was successful or not
	virtual bool read(uint16_t address, uint8_t *data) = 0;

    /// Write data to the Device
    /// @param address - which address to write to
    /// @param data - data to write to
    /// @return bool - if write was successful or not
	virtual bool write(uint16_t address, uint8_t data) = 0;

};
