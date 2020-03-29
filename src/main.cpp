#include "Nes.hpp"

int main(int, char**)
{
    fprintf(stdout, "Mark NES Emulator\n");

    auto nes = std::make_shared<Nes>("supermario.nes");

    return 0;
}
