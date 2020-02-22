#include "Memory64KB.hpp"
#include "Bus.hpp"
#include "Cpu.hpp"

int main(int, char**)
{
    fprintf(stdout, "Mark NES Emulator\n");

    auto memory = std::make_shared<Memory64KB>();
    auto bus = std::make_shared<Bus>(memory);
    auto cpu = std::make_shared<Cpu>(bus);

    // One cycle CPU tick
    cpu->tick();

    return 0;
}
