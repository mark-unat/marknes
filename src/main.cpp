#include "Memory2KB.hpp"
#include "CpuBus.hpp"
#include "NameTable.hpp"
#include "PatternTable.hpp"
#include "PaletteTable.hpp"
#include "PpuBus.hpp"
#include "Cartridge.hpp"
#include "Ppu.hpp"
#include "Cpu.hpp"

int main(int, char**)
{
    fprintf(stdout, "Mark NES Emulator\n");

    auto cpuRam = std::make_shared<Memory2KB>();
    //auto apu = std::make_shared<Apu>();
    //auto control = std::make_shared<Control>();
    auto cpuBus = std::make_shared<CpuBus>(cpuRam/*, apu, control*/);

    auto nameTable = std::make_shared<NameTable>();
    auto patternTable = std::make_shared<PatternTable>();
    auto paletteTable = std::make_shared<PaletteTable>();
    auto ppuBus = std::make_shared<PpuBus>(nameTable, patternTable, paletteTable);

    auto cartridge = std::make_shared<Cartridge>("supermario.nes");

    auto ppu = std::make_shared<Ppu>(ppuBus, cartridge);
    auto cpu = std::make_shared<Cpu>(cpuBus, ppu, cartridge);

    //auto nes = std::make_shared<Nes>(cpu, ppu);
    //nes->insertCartridge(cartridge);

    // Reset CPU
    //nes->reset();

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

    return 0;
}
