#include "Nes.hpp"

Nes::Nes() {}

Nes::~Nes() {}

void Nes::load(std::string fileName)
{
    _fileName = std::move(fileName);

    _cpuRam = std::make_shared<Memory2KB>();
    _apu = std::make_shared<Apu>();
    _controller = std::make_shared<Controller>();

    _nameTable = std::make_shared<NameTable>();
    _paletteTable = std::make_shared<PaletteTable>();
    _cartridge = std::make_shared<Cartridge>(_fileName);

    _ppuBus = std::make_shared<PpuBus>(_nameTable, _paletteTable, _cartridge);
    _ppu = std::make_shared<Ppu>(_ppuBus, _cartridge);

    _cpuBus = std::make_shared<CpuBus>(_cpuRam, _apu, _ppu, _cartridge, _controller);
    _cpu = std::make_shared<Cpu>(_cpuBus, _ppu);

    _audioHw = std::make_shared<AudioHw>(44100, 8, 512);
    _audioHw->setReadSampleCallback([this](float time) { return _apu->getMixedOutput(time); });
}

void Nes::renderFrame()
{
    while (!_ppu->isFrameDone()) {
        // One PPU cycle
        _ppu->tick();

        // PPU runs 3 times faster than CPU
        if (_counter % 3 == 0) {
            auto isOddCycle = (_counter % 2 == 1);

            // One CPU cycle
            _cpu->tick(isOddCycle);
        }

        // PPU runs 6 times faster than APU
        if (_counter % 6 == 0) {
            // One APU cycle
            _apu->tick();
        }

        _counter++;
        if (_counter >= 0xFF) {
            _counter = 0;
        }

        // Check if PPU need to send NMI to CPU
        if (_ppu->isVBlankTriggered()) {
            _cpu->nonMaskableInterruptRequest();
        }
    }
}

void Nes::reset()
{
    _cartridge->reset();
    _cpu->reset();
    _ppu->reset();
    _counter = 0;
}

uint8_t* Nes::getFrameBuffer()
{
    return _ppu->getFrameBuffer();
}

void Nes::setControllerKey(uint8_t id, NesButton button, bool state)
{
    _controller->setKey(id, static_cast<ControllerButton>(button), state);
}
