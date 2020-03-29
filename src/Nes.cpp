#include "Nes.hpp"

Nes::Nes(std::string fileName)
: _fileName{std::move(fileName)}
{
    _cpuRam = std::make_shared<Memory2KB>();
    //_apu = std::make_shared<Apu>();
    //_control = std::make_shared<Control>();
    _cpuBus = std::make_shared<CpuBus>(_cpuRam/*, _apu, _control*/);

    _nameTable = std::make_shared<NameTable>();
    _patternTable = std::make_shared<PatternTable>();
    _paletteTable = std::make_shared<PaletteTable>();
    _ppuBus = std::make_shared<PpuBus>(_nameTable, _patternTable, _paletteTable);

    _cartridge = std::make_shared<Cartridge>(_fileName);

    _ppu = std::make_shared<Ppu>(_ppuBus, _cartridge);
    _cpu = std::make_shared<Cpu>(_cpuBus, _ppu, _cartridge);

    // Reset CPU
    //nes->reset();
    //
    /*
    auto counter{0u};
    while (true) {
        // One cycle PPU tick
        // PPU runs 3 times faster than CPU
        ppu.tick();
        if (counter % 3 == 0) {
            // One cycle CPU tick
            cpu.tick();
        }
        counter++;
    }
    */
}

Nes::~Nes()
{
}

void Nes::reset()
{
    //_cpu->reset();
    //_ppu->reset();
}

void Nes::getFrameBuffer(uint8_t *frameBuffer)
{
    _ppu->getFrameBuffer(frameBuffer);
}
