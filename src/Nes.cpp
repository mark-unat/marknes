#include "Nes.hpp"

Nes::Nes() {}

Nes::~Nes() {}

void Nes::load(std::string fileName)
{
    _fileName = std::move(fileName);

    _cpuRam = std::make_shared<Memory2KB>();
    //_apu = std::make_shared<Apu>();
    //_control = std::make_shared<Control>();

    _nameTable = std::make_shared<NameTable>();
    _paletteTable = std::make_shared<PaletteTable>();
    _cartridge = std::make_shared<Cartridge>(_fileName);

    _ppuBus = std::make_shared<PpuBus>(_nameTable, _paletteTable, _cartridge);
    _ppu = std::make_shared<Ppu>(_ppuBus, _cartridge);

    _cpuBus = std::make_shared<CpuBus>(_cpuRam, _ppu, _cartridge /*, _apu, _control*/);
    _cpu = std::make_shared<Cpu>(_cpuBus);
}

void Nes::renderFrame()
{
    while (!_ppu->isFrameDone()) {
        // One PPU cycle
        _ppu->tick();

        // PPU runs 3 times faster than CPU
        if (_counter % 3 == 0) {
            // One CPU cycle
            _cpu->tick();
            _counter = 0;
        }
        _counter++;

        // Check if PPU need to send NMI to CPU
        if (_ppu->isVBlankTriggered()) {
            _cpu->nonMaskableInterruptRequest();
        }
    }
}

void Nes::reset()
{
    _cpu->reset();
    _ppu->reset();
    _counter = 0;
}

uint8_t* Nes::getFrameBuffer()
{
    return _ppu->getFrameBuffer();
}
