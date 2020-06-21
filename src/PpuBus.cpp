#include "PpuBus.hpp"

PpuBus::PpuBus(std::shared_ptr<IMemory> nameTable,
               std::shared_ptr<IMemory> paletteTable,
               std::shared_ptr<Cartridge> cartridge)
: _nameTable{nameTable}
, _paletteTable{paletteTable}
, _cartridge{cartridge}
{
}

bool PpuBus::read(uint16_t address, uint8_t& data)
{
    switch (address) {
    case patternTableBaseAddress ... patternTableEndAddress:
        return _cartridge->readCHR(address, data);
    case nameTableBaseAddress ... nameTableEndAddress:
    {
        auto newAddress{address};
        if (_cartridge->getMirroringMode() == MirroringMode::Vertical) {
            // Vertical Mirroring (used for horizontal scrolling)
            // In this mode, we only use table1 (0x2000) and table2 (0x2400),
            // any read to table3 (0x2800) and table4 (0x2C00) will be
            // mirrored from table1 and table2, respectively.
            if ((address >= nameTable3StartAddress) && (address <= nameTable4EndAddress)) {
                newAddress = address - (nameTable3StartAddress - nameTable1StartAddress);
            }
        } else if (_cartridge->getMirroringMode() == MirroringMode::Horizontal) {
            // Horizontal Mirroring (used for vertical scrolling)
            // In this mode, we only use table1 (0x2000) and table3 (0x2800),
            // any read to table2 (0x2400) and table4 (0x2C00) will be
            // mirrored from table1 and table3, respectively.
            if (((address >= nameTable2StartAddress) && (address <= nameTable2EndAddress)) ||
                ((address >= nameTable4StartAddress) && (address <= nameTable4EndAddress))) {
                newAddress = address - (nameTable2StartAddress - nameTable1StartAddress);
            }
        }
        return _nameTable->read(newAddress, data);
    }
    case paletteTableBaseAddress ... paletteTableEndAddress:
    {
        auto newAddress{address};
        // The following addresses are just mirrors
        if ((address == paletteTableBG1Address) || (address == paletteTableBG2Address) ||
            (address == paletteTableBG3Address) || (address == paletteTableBG4Address)) {
            newAddress = address - (paletteTableBG1Address - paletteTableBaseAddress);
        }
        return _paletteTable->read(newAddress, data);
    }
    default:
        break;
    }

    return false;
}

bool PpuBus::write(uint16_t address, uint8_t data)
{
    switch (address) {
    case patternTableBaseAddress ... patternTableEndAddress:
        return _cartridge->writeCHR(address, data);
    case nameTableBaseAddress ... nameTableEndAddress:
    {
        auto newAddress{address};
        if (_cartridge->getMirroringMode() == MirroringMode::Vertical) {
            // Vertical Mirroring (used for horizontal scrolling)
            // In this mode, we only use table1 (0x2000) and table2 (0x2400),
            // any writes to table3 (0x2800) and table4 (0x2C00) will be
            // mirrored to table1 and table2, respectively.
            if ((address >= nameTable3StartAddress) && (address <= nameTable4EndAddress)) {
                newAddress = address - (nameTable3StartAddress - nameTable1StartAddress);
            }
        } else if (_cartridge->getMirroringMode() == MirroringMode::Horizontal) {
            // Horizontal Mirroring (used for vertical scrolling)
            // In this mode, we only use table1 (0x2000) and table3 (0x2800),
            // any writes to table2 (0x2400) and table4 (0x2C00) will be
            // mirrored to table1 and table3, respectively.
            if (((address >= nameTable2StartAddress) && (address <= nameTable2EndAddress)) ||
                ((address >= nameTable4StartAddress) && (address <= nameTable4EndAddress))) {
                newAddress = address - (nameTable2StartAddress - nameTable1StartAddress);
            }
        }
        return _nameTable->write(newAddress, data);
    }
    case paletteTableBaseAddress ... paletteTableEndAddress:
    {
        auto newAddress{address};
        // The following addresses are just mirrors
        if ((address == paletteTableBG1Address) || (address == paletteTableBG2Address) ||
            (address == paletteTableBG3Address) || (address == paletteTableBG4Address)) {
            newAddress = address - (paletteTableBG1Address - paletteTableBaseAddress);
        }
        return _paletteTable->write(newAddress, data);
    }
    default:
        break;
    }

    return false;
}
