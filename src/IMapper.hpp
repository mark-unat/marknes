#pragma once

class IMapper {
public:
    virtual ~IMapper() = default;

    /// Read program address
    /// @pararm address - which address to translate
    /// @param  prgAddress - program offset address when translated
    /// @param  data - data to read when data is needed at this layer
    /// @return bool - if read was successful or not
    virtual bool readPrg(uint16_t address, uint32_t& prgAddress, uint8_t& data) = 0;

    /// Write program address
    /// @pararm address - which address to translate
    /// @param  prgAddress - program offset address when translated
    /// @param  data - data to write when data is needed at this layer
    /// @return bool - if write was successful or not
    virtual bool writePrg(uint16_t address, uint32_t& prgAddress, uint8_t data) = 0;

    /// Read character address
    /// @pararm address - which address to translate
    /// @param chrAddress - character offset address when translated
    /// @return bool - if read was successful or not
    virtual bool readChr(uint16_t address, uint32_t& chrAddress) = 0;

    /// Write character address
    /// @pararm address - which address to translate
    /// @param chrAddress - character offset address when translated
    /// @return bool - if write was successful or not
    virtual bool writeChr(uint16_t address, uint32_t& chrAddress) = 0;

    /// Reset mapper
    virtual void reset() = 0;

protected:
    uint8_t _prgRomChunks{0};
    uint8_t _chrRomChunks{0};
};
