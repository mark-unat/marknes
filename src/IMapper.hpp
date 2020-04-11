#pragma once

class IMapper {
public:
    virtual ~IMapper() = default;

    /// Get PRG offset
    /// @pararm address - which address to translate
    /// @param prgAddress - PRG offset
    /// @return bool - if get was successful or not
    virtual bool getPrgAddress(uint16_t address, uint32_t& prgAddress) = 0;

    /// Get CHR offset
    /// @pararm address - which address to translate
    /// @param chrAddress - CHR offset
    /// @return bool - if get was successful or not
    virtual bool getChrAddress(uint16_t address, uint32_t& chrAddress) = 0;

protected:
    uint8_t _prgRomChunks{0};
    uint8_t _chrRomChunks{0};
};
