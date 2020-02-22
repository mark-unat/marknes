#include "Memory64KB.hpp"
#include "Bus.hpp"

int main(int, char**)
{
    fprintf(stdout, "Mark NES Emulator\n");

    auto memory = std::make_shared<Memory64KB>();
    auto myBus = std::make_shared<Bus>(memory);

    // Sample write
    myBus->write(0, 0);
    return 0;
}
