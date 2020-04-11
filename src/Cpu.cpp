#include "Cpu.hpp"
#include "CpuBus.hpp"

constexpr uint8_t resetStackOffset = 0xFD;
constexpr uint16_t stackBaseAddress = 0x0100;
constexpr uint16_t nonMaskableInterruptAddress = 0xFFFA;
constexpr uint16_t resetInterruptAddress = 0xFFFC;
constexpr uint16_t breakInterruptAddress = 0xFFFE;

Cpu::Cpu(std::shared_ptr<IDevice> bus)
: _bus{bus}
{
    // Command Table
    _commandTable = {
        {0x00, OpCode::BRK, 1, AddressMode::IMM, "BRK", 7}, {0x01, OpCode::ORA, 2, AddressMode::IZX, "ORA", 6},
        {0x02, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x03, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x04, OpCode::NOP, 2, AddressMode::IMP, "NOP", 3}, {0x05, OpCode::ORA, 2, AddressMode::ZP0, "ORA", 3},
        {0x06, OpCode::ASL, 2, AddressMode::ZP0, "ASL", 5}, {0x07, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0x08, OpCode::PHP, 1, AddressMode::IMP, "PHP", 3}, {0x09, OpCode::ORA, 2, AddressMode::IMM, "ORA", 2},
        {0x0A, OpCode::ASL, 1, AddressMode::IMP, "ASL", 2}, {0x0B, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0x0C, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0x0D, OpCode::ORA, 3, AddressMode::ABS, "ORA", 4},
        {0x0E, OpCode::ASL, 3, AddressMode::ABS, "ASL", 6}, {0x0F, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x10, OpCode::BPL, 2, AddressMode::REL, "BPL", 2}, {0x11, OpCode::ORA, 2, AddressMode::IZY, "ORA", 5},
        {0x12, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x13, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x14, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0x15, OpCode::ORA, 2, AddressMode::ZPX, "ORA", 4},
        {0x16, OpCode::ASL, 2, AddressMode::ZPX, "ASL", 6}, {0x17, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x18, OpCode::CLC, 1, AddressMode::IMP, "CLC", 2}, {0x19, OpCode::ORA, 3, AddressMode::ABY, "ORA", 4},
        {0x1A, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0x1B, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x1C, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0x1D, OpCode::ORA, 3, AddressMode::ABX, "ORA", 4},
        {0x1E, OpCode::ASL, 3, AddressMode::ABX, "ASL", 7}, {0x1F, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x20, OpCode::JSR, 3, AddressMode::ABS, "JSR", 6}, {0x21, OpCode::AND, 2, AddressMode::IZX, "AND", 6},
        {0x22, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x23, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x24, OpCode::BIT, 2, AddressMode::ZP0, "BIT", 3}, {0x25, OpCode::AND, 2, AddressMode::ZP0, "AND", 3},
        {0x26, OpCode::ROL, 2, AddressMode::ZP0, "ROL", 5}, {0x27, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0x28, OpCode::PLP, 1, AddressMode::IMP, "PLP", 4}, {0x29, OpCode::AND, 2, AddressMode::IMM, "AND", 2},
        {0x2A, OpCode::ROL, 1, AddressMode::IMP, "ROL", 2}, {0x2B, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0x2C, OpCode::BIT, 3, AddressMode::ABS, "BIT", 4}, {0x2D, OpCode::AND, 3, AddressMode::ABS, "AND", 4},
        {0x2E, OpCode::ROL, 3, AddressMode::ABS, "ROL", 6}, {0x2F, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x30, OpCode::BMI, 2, AddressMode::REL, "BMI", 2}, {0x31, OpCode::AND, 2, AddressMode::IZY, "AND", 5},
        {0x32, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x33, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x34, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0x35, OpCode::AND, 2, AddressMode::ZPX, "AND", 4},
        {0x36, OpCode::ROL, 2, AddressMode::ZPX, "ROL", 6}, {0x37, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x38, OpCode::SEC, 1, AddressMode::IMP, "SEC", 2}, {0x39, OpCode::AND, 3, AddressMode::ABY, "AND", 4},
        {0x3A, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0x3B, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x3C, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0x3D, OpCode::AND, 3, AddressMode::ABX, "AND", 4},
        {0x3E, OpCode::ROL, 3, AddressMode::ABX, "ROL", 7}, {0x3F, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x40, OpCode::RTI, 1, AddressMode::IMP, "RTI", 6}, {0x41, OpCode::EOR, 2, AddressMode::IZX, "EOR", 6},
        {0x42, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x43, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x44, OpCode::NOP, 2, AddressMode::IMP, "NOP", 3}, {0x45, OpCode::EOR, 2, AddressMode::ZP0, "EOR", 3},
        {0x46, OpCode::LSR, 2, AddressMode::ZP0, "LSR", 5}, {0x47, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0x48, OpCode::PHA, 1, AddressMode::IMP, "PHA", 3}, {0x49, OpCode::EOR, 2, AddressMode::IMM, "EOR", 2},
        {0x4A, OpCode::LSR, 1, AddressMode::IMP, "LSR", 2}, {0x4B, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0x4C, OpCode::JMP, 3, AddressMode::ABS, "JMP", 3}, {0x4D, OpCode::EOR, 3, AddressMode::ABS, "EOR", 4},
        {0x4E, OpCode::LSR, 3, AddressMode::ABS, "LSR", 6}, {0x4F, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x50, OpCode::BVC, 2, AddressMode::REL, "BVC", 2}, {0x51, OpCode::EOR, 2, AddressMode::IZY, "EOR", 5},
        {0x52, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x53, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x54, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0x55, OpCode::EOR, 2, AddressMode::ZPX, "EOR", 4},
        {0x56, OpCode::LSR, 2, AddressMode::ZPX, "LSR", 6}, {0x57, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x58, OpCode::CLI, 1, AddressMode::IMP, "CLI", 2}, {0x59, OpCode::EOR, 3, AddressMode::ABY, "EOR", 4},
        {0x5A, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0x5B, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x5C, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0x5D, OpCode::EOR, 3, AddressMode::ABX, "EOR", 4},
        {0x5E, OpCode::LSR, 3, AddressMode::ABX, "LSR", 7}, {0x5F, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x60, OpCode::RTS, 1, AddressMode::IMP, "RTS", 6}, {0x61, OpCode::ADC, 2, AddressMode::IZX, "ADC", 6},
        {0x62, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x63, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x64, OpCode::NOP, 2, AddressMode::IMP, "NOP", 3}, {0x65, OpCode::ADC, 2, AddressMode::ZP0, "ADC", 3},
        {0x66, OpCode::ROR, 2, AddressMode::ZP0, "ROR", 5}, {0x67, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0x68, OpCode::PLA, 1, AddressMode::IMP, "PLA", 4}, {0x69, OpCode::ADC, 2, AddressMode::IMM, "ADC", 2},
        {0x6A, OpCode::ROR, 1, AddressMode::IMP, "ROR", 2}, {0x6B, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0x6C, OpCode::JMP, 3, AddressMode::IND, "JMP", 5}, {0x6D, OpCode::ADC, 3, AddressMode::ABS, "ADC", 4},
        {0x6E, OpCode::ROR, 3, AddressMode::ABS, "ROR", 6}, {0x6F, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x70, OpCode::BVS, 2, AddressMode::REL, "BVS", 2}, {0x71, OpCode::ADC, 2, AddressMode::IZY, "ADC", 5},
        {0x72, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x73, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0x74, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0x75, OpCode::ADC, 2, AddressMode::ZPX, "ADC", 4},
        {0x76, OpCode::ROR, 2, AddressMode::ZPX, "ROR", 6}, {0x77, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x78, OpCode::SEI, 1, AddressMode::IMP, "SEI", 2}, {0x79, OpCode::ADC, 3, AddressMode::ABY, "ADC", 4},
        {0x7A, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0x7B, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x7C, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0x7D, OpCode::ADC, 3, AddressMode::ABX, "ADC", 4},
        {0x7E, OpCode::ROR, 3, AddressMode::ABX, "ROR", 7}, {0x7F, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0x80, OpCode::NOP, 2, AddressMode::IMP, "NOP", 2}, {0x81, OpCode::STA, 2, AddressMode::IZX, "STA", 6},
        {0x82, OpCode::NOP, 0, AddressMode::IMP, "INV", 2}, {0x83, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x84, OpCode::STY, 2, AddressMode::ZP0, "STY", 3}, {0x85, OpCode::STA, 2, AddressMode::ZP0, "STA", 3},
        {0x86, OpCode::STX, 2, AddressMode::ZP0, "STX", 3}, {0x87, OpCode::INV, 0, AddressMode::IMP, "INV", 3},
        {0x88, OpCode::DEY, 1, AddressMode::IMP, "DEY", 2}, {0x89, OpCode::NOP, 0, AddressMode::IMP, "INV", 2},
        {0x8A, OpCode::TXA, 1, AddressMode::IMP, "TXA", 2}, {0x8B, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0x8C, OpCode::STY, 3, AddressMode::ABS, "STY", 4}, {0x8D, OpCode::STA, 3, AddressMode::ABS, "STA", 4},
        {0x8E, OpCode::STX, 3, AddressMode::ABS, "STX", 4}, {0x8F, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0x90, OpCode::BCC, 2, AddressMode::REL, "BCC", 2}, {0x91, OpCode::STA, 2, AddressMode::IZY, "STA", 6},
        {0x92, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0x93, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0x94, OpCode::STY, 2, AddressMode::ZPX, "STY", 4}, {0x95, OpCode::STA, 2, AddressMode::ZPX, "STA", 4},
        {0x96, OpCode::STX, 2, AddressMode::ZPY, "STX", 4}, {0x97, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0x98, OpCode::TYA, 1, AddressMode::IMP, "TYA", 2}, {0x99, OpCode::STA, 3, AddressMode::ABY, "STA", 5},
        {0x9A, OpCode::TXS, 1, AddressMode::IMP, "TXS", 2}, {0x9B, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0x9C, OpCode::NOP, 0, AddressMode::IMP, "INV", 5}, {0x9D, OpCode::STA, 3, AddressMode::ABX, "STA", 5},
        {0x9E, OpCode::INV, 0, AddressMode::IMP, "INV", 5}, {0x9F, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0xA0, OpCode::LDY, 2, AddressMode::IMM, "LDY", 2}, {0xA1, OpCode::LDA, 2, AddressMode::IZX, "LDA", 6},
        {0xA2, OpCode::LDX, 2, AddressMode::IMM, "LDX", 2}, {0xA3, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0xA4, OpCode::LDY, 2, AddressMode::ZP0, "LDY", 3}, {0xA5, OpCode::LDA, 2, AddressMode::ZP0, "LDA", 3},
        {0xA6, OpCode::LDX, 2, AddressMode::ZP0, "LDX", 3}, {0xA7, OpCode::INV, 0, AddressMode::IMP, "INV", 3},
        {0xA8, OpCode::TAY, 1, AddressMode::IMP, "TAY", 2}, {0xA9, OpCode::LDA, 2, AddressMode::IMM, "LDA", 2},
        {0xAA, OpCode::TAX, 1, AddressMode::IMP, "TAX", 2}, {0xAB, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0xAC, OpCode::LDY, 3, AddressMode::ABS, "LDY", 4}, {0xAD, OpCode::LDA, 3, AddressMode::ABS, "LDA", 4},
        {0xAE, OpCode::LDX, 3, AddressMode::ABS, "LDX", 4}, {0xAF, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0xB0, OpCode::BCS, 2, AddressMode::REL, "BCS", 2}, {0xB1, OpCode::LDA, 2, AddressMode::IZY, "LDA", 5},
        {0xB2, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0xB3, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0xB4, OpCode::LDY, 2, AddressMode::ZPX, "LDY", 4}, {0xB5, OpCode::LDA, 2, AddressMode::ZPX, "LDA", 4},
        {0xB6, OpCode::LDX, 2, AddressMode::ZPY, "LDX", 4}, {0xB7, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0xB8, OpCode::CLV, 1, AddressMode::IMP, "CLV", 2}, {0xB9, OpCode::LDA, 3, AddressMode::ABY, "LDA", 4},
        {0xBA, OpCode::TSX, 1, AddressMode::IMP, "TSX", 2}, {0xBB, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0xBC, OpCode::LDY, 3, AddressMode::ABX, "LDY", 4}, {0xBD, OpCode::LDA, 3, AddressMode::ABX, "LDA", 4},
        {0xBE, OpCode::LDX, 3, AddressMode::ABY, "LDX", 4}, {0xBF, OpCode::INV, 0, AddressMode::IMP, "INV", 4},
        {0xC0, OpCode::CPY, 2, AddressMode::IMM, "CPY", 2}, {0xC1, OpCode::CMP, 2, AddressMode::IZX, "CMP", 6},
        {0xC2, OpCode::NOP, 0, AddressMode::IMP, "INV", 2}, {0xC3, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0xC4, OpCode::CPY, 2, AddressMode::ZP0, "CPY", 3}, {0xC5, OpCode::CMP, 2, AddressMode::ZP0, "CMP", 3},
        {0xC6, OpCode::DEC, 2, AddressMode::ZP0, "DEC", 5}, {0xC7, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0xC8, OpCode::INY, 1, AddressMode::IMP, "INY", 2}, {0xC9, OpCode::CMP, 2, AddressMode::IMM, "CMP", 2},
        {0xCA, OpCode::DEX, 1, AddressMode::IMP, "DEX", 2}, {0xCB, OpCode::INV, 0, AddressMode::IMP, "INV", 2},
        {0xCC, OpCode::CPY, 3, AddressMode::ABS, "CPY", 4}, {0xCD, OpCode::CMP, 3, AddressMode::ABS, "CMP", 4},
        {0xCE, OpCode::DEC, 3, AddressMode::ABS, "DEC", 6}, {0xCF, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0xD0, OpCode::BNE, 2, AddressMode::REL, "BNE", 2}, {0xD1, OpCode::CMP, 2, AddressMode::IZY, "CMP", 5},
        {0xD2, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0xD3, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0xD4, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0xD5, OpCode::CMP, 2, AddressMode::ZPX, "CMP", 4},
        {0xD6, OpCode::DEC, 2, AddressMode::ZPX, "DEC", 6}, {0xD7, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0xD8, OpCode::CLD, 1, AddressMode::IMP, "CLD", 2}, {0xD9, OpCode::CMP, 3, AddressMode::ABY, "CMP", 4},
        {0xDA, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0xDB, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0xDC, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0xDD, OpCode::CMP, 3, AddressMode::ABX, "CMP", 4},
        {0xDE, OpCode::DEC, 3, AddressMode::ABX, "DEC", 7}, {0xDF, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0xE0, OpCode::CPX, 2, AddressMode::IMM, "CPX", 2}, {0xE1, OpCode::SBC, 2, AddressMode::IZX, "SBC", 6},
        {0xE2, OpCode::NOP, 0, AddressMode::IMP, "INV", 2}, {0xE3, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0xE4, OpCode::CPX, 2, AddressMode::ZP0, "CPX", 3}, {0xE5, OpCode::SBC, 2, AddressMode::ZP0, "SBC", 3},
        {0xE6, OpCode::INC, 2, AddressMode::ZP0, "INC", 5}, {0xE7, OpCode::INV, 0, AddressMode::IMP, "INV", 5},
        {0xE8, OpCode::INX, 1, AddressMode::IMP, "INX", 2}, {0xE9, OpCode::SBC, 2, AddressMode::IMM, "SBC", 2},
        {0xEA, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0xEB, OpCode::SBC, 0, AddressMode::IMP, "INV", 2},
        {0xEC, OpCode::CPX, 3, AddressMode::ABS, "CPX", 4}, {0xED, OpCode::SBC, 3, AddressMode::ABS, "SBC", 4},
        {0xEE, OpCode::INC, 3, AddressMode::ABS, "INC", 6}, {0xEF, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0xF0, OpCode::BEQ, 2, AddressMode::REL, "BEQ", 2}, {0xF1, OpCode::SBC, 2, AddressMode::IZY, "SBC", 5},
        {0xF2, OpCode::INV, 0, AddressMode::IMP, "INV", 2}, {0xF3, OpCode::INV, 0, AddressMode::IMP, "INV", 8},
        {0xF4, OpCode::NOP, 2, AddressMode::IMP, "NOP", 4}, {0xF5, OpCode::SBC, 2, AddressMode::ZPX, "SBC", 4},
        {0xF6, OpCode::INC, 2, AddressMode::ZPX, "INC", 6}, {0xF7, OpCode::INV, 0, AddressMode::IMP, "INV", 6},
        {0xF8, OpCode::SED, 1, AddressMode::IMP, "SED", 2}, {0xF9, OpCode::SBC, 3, AddressMode::ABY, "SBC", 4},
        {0xFA, OpCode::NOP, 1, AddressMode::IMP, "NOP", 2}, {0xFB, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
        {0xFC, OpCode::NOP, 3, AddressMode::IMP, "NOP", 4}, {0xFD, OpCode::SBC, 3, AddressMode::ABX, "SBC", 4},
        {0xFE, OpCode::INC, 3, AddressMode::ABX, "INC", 7}, {0xFF, OpCode::INV, 0, AddressMode::IMP, "INV", 7},
    };
}

Cpu::~Cpu() {}

// Reset
void Cpu::reset()
{
    auto data = uint8_t{0x00};

    _currentAddress = resetInterruptAddress;
    _bus->read(_currentAddress, data);
    auto lowByte = static_cast<uint16_t>(data);
    _bus->read(_currentAddress + 1, data);
    auto highByte = static_cast<uint16_t>(data);

    // Set it
    registers.programCounter = (highByte << 8) | lowByte;

    // Reset registers
    registers.accumulator = 0x00;
    registers.registerX = 0x00;
    registers.registerY = 0x00;
    registers.stackPointer = resetStackOffset;
    registers.status = 0x00;
    _setStatusFlag(StatusBit::bitUnused, true);
    _setStatusFlag(StatusBit::bitDisableInterrupt, true);

    // Reset Addresses/Data
    _relativeAddress = 0x0000;
    _currentAddress = 0x0000;
    _currentData = 0x00;

    // Reset cycles
    _cycles = 0;
}

// Interrupt Request
void Cpu::interruptRequest()
{
    // Check if interrupt was enabled
    if (!registers.statusFlag.disableInterrupt) {
        _bus->write(stackBaseAddress + registers.stackPointer,
                    static_cast<uint8_t>((registers.programCounter >> 8) & 0x00FF));
        registers.stackPointer--;
        _bus->write(stackBaseAddress + registers.stackPointer, static_cast<uint8_t>(registers.programCounter & 0x00FF));
        registers.stackPointer--;

        _setStatusFlag(StatusBit::bitBreakCommand, false);
        _setStatusFlag(StatusBit::bitUnused, true);
        _setStatusFlag(StatusBit::bitDisableInterrupt, true);
        _bus->write(stackBaseAddress + registers.stackPointer, registers.status);
        registers.stackPointer--;

        _currentAddress = breakInterruptAddress;
        auto data = uint8_t{0x00};
        _bus->read(_currentAddress, data);
        auto lowByte = static_cast<uint16_t>(data);
        _bus->read(_currentAddress + 1, data);
        auto highByte = static_cast<uint16_t>(data);
        registers.programCounter = (highByte << 8) | lowByte;

        // Interrupt Request cycles
        _cycles = 7;
    }
}

// Non-Maskable Interrupt Request
void Cpu::nonMaskableInterruptRequest()
{
    _bus->write(stackBaseAddress + registers.stackPointer,
                static_cast<uint8_t>((registers.programCounter >> 8) & 0x00FF));
    registers.stackPointer--;
    _bus->write(stackBaseAddress + registers.stackPointer, static_cast<uint8_t>(registers.programCounter & 0x00FF));
    registers.stackPointer--;

    _setStatusFlag(StatusBit::bitBreakCommand, false);
    _setStatusFlag(StatusBit::bitUnused, true);
    _setStatusFlag(StatusBit::bitDisableInterrupt, true);
    _bus->write(stackBaseAddress + registers.stackPointer, registers.status);
    registers.stackPointer--;

    _currentAddress = nonMaskableInterruptAddress;
    auto data = uint8_t{0x00};
    _bus->read(_currentAddress, data);
    auto lowByte = static_cast<uint16_t>(data);
    _bus->read(_currentAddress + 1, data);
    auto highByte = static_cast<uint16_t>(data);
    registers.programCounter = (highByte << 8) | lowByte;

    // Non-Maskable Interrupt Request cycles
    _cycles = 8;
}

void Cpu::_disassemble()
{
    auto address = registers.programCounter;
    auto data = uint8_t{0x00};
    auto opCode = uint8_t{0x00};

    _bus->read(address, opCode);
    if (_commandTable[opCode].opCodeLength > 0) {
        char hexData[4] = {0};
        std::string opData;

        fprintf(stdout, "%04X  ", registers.programCounter);
        for (int i = 0; i < _commandTable[opCode].opCodeLength; i++) {
            _bus->read(address, data);
            address++;
            fprintf(stdout, "%02X ", data);
            if (i > 0) {
                if (i == 1) {
                    opData += "$";
                }
                snprintf(hexData, sizeof(hexData), "%02X", data);
                opData += hexData;
            }
        }
        if (_commandTable[opCode].opCodeLength == 1) {
            fprintf(stdout, "\t");
        }
        fprintf(stdout, "\t%s %s", _commandTable[opCode].name.c_str(), opData.c_str());
        if (_commandTable[opCode].opCodeLength <= 2) {
            fprintf(stdout, "\t");
        }
        fprintf(stdout, "\t\t\tA:%02X X:%02X Y:%02X P:%02X SP:%02X\n", registers.accumulator, registers.registerX,
                registers.registerY, registers.status, registers.stackPointer);
    }
}

// Execute one clock cycle
void Cpu::tick()
{
    if (_cycles == 0) {
        // To enable ASM debugging only
        //_disassemble();

        // Read next OpCode
        _bus->read(registers.programCounter, _currentOpCode);
        registers.programCounter++;

        _setStatusFlag(StatusBit::bitUnused, true);

        _cycles = _commandTable[_currentOpCode].cycles;

        // Execute AddressMode and OpCode and add cycles if needed
        auto checkCycle1 = _runAddressMode(_commandTable[_currentOpCode].addressMode);
        auto checkCycle2 = _runOpCode(_commandTable[_currentOpCode].opCode);
        if (checkCycle1 && checkCycle2) {
            _cycles++;
        }

        // Always set the unused status flag bit to 1
        _setStatusFlag(StatusBit::bitUnused, true);
    }
    _cycles--;
}

bool Cpu::_runAddressMode(AddressMode addressMode)
{
    switch (addressMode) {
    case AddressMode::IMP:
        return _modeIMP();
    case AddressMode::IMM:
        return _modeIMM();
    case AddressMode::ZP0:
        return _modeZP0();
    case AddressMode::ZPX:
        return _modeZPX();
    case AddressMode::ZPY:
        return _modeZPY();
    case AddressMode::REL:
        return _modeREL();
    case AddressMode::ABS:
        return _modeABS();
    case AddressMode::ABX:
        return _modeABX();
    case AddressMode::ABY:
        return _modeABY();
    case AddressMode::IND:
        return _modeIND();
    case AddressMode::IZX:
        return _modeIZX();
    case AddressMode::IZY:
        return _modeIZY();
    default:
        break;
    }

    return _modeIMP();
}

bool Cpu::_runOpCode(OpCode opCode)
{
    switch (opCode) {
    case OpCode::ADC:
        return _codeADC();
    case OpCode::AND:
        return _codeAND();
    case OpCode::ASL:
        return _codeASL();
    case OpCode::BCC:
        return _codeBCC();
    case OpCode::BCS:
        return _codeBCS();
    case OpCode::BEQ:
        return _codeBEQ();
    case OpCode::BIT:
        return _codeBIT();
    case OpCode::BMI:
        return _codeBMI();
    case OpCode::BNE:
        return _codeBNE();
    case OpCode::BPL:
        return _codeBPL();
    case OpCode::BRK:
        return _codeBRK();
    case OpCode::BVC:
        return _codeBVC();
    case OpCode::BVS:
        return _codeBVS();
    case OpCode::CLC:
        return _codeCLC();
    case OpCode::CLD:
        return _codeCLD();
    case OpCode::CLI:
        return _codeCLI();
    case OpCode::CLV:
        return _codeCLV();
    case OpCode::CMP:
        return _codeCMP();
    case OpCode::CPX:
        return _codeCPX();
    case OpCode::CPY:
        return _codeCPY();
    case OpCode::DEC:
        return _codeDEC();
    case OpCode::DEX:
        return _codeDEX();
    case OpCode::DEY:
        return _codeDEY();
    case OpCode::EOR:
        return _codeEOR();
    case OpCode::INC:
        return _codeINC();
    case OpCode::INX:
        return _codeINX();
    case OpCode::INY:
        return _codeINY();
    case OpCode::JMP:
        return _codeJMP();
    case OpCode::JSR:
        return _codeJSR();
    case OpCode::LDA:
        return _codeLDA();
    case OpCode::LDX:
        return _codeLDX();
    case OpCode::LDY:
        return _codeLDY();
    case OpCode::LSR:
        return _codeLSR();
    case OpCode::NOP:
        return _codeNOP();
    case OpCode::ORA:
        return _codeORA();
    case OpCode::PHA:
        return _codePHA();
    case OpCode::PHP:
        return _codePHP();
    case OpCode::PLA:
        return _codePLA();
    case OpCode::PLP:
        return _codePLP();
    case OpCode::ROL:
        return _codeROL();
    case OpCode::ROR:
        return _codeROR();
    case OpCode::RTI:
        return _codeRTI();
    case OpCode::RTS:
        return _codeRTS();
    case OpCode::SBC:
        return _codeSBC();
    case OpCode::SEC:
        return _codeSEC();
    case OpCode::SED:
        return _codeSED();
    case OpCode::SEI:
        return _codeSEI();
    case OpCode::STA:
        return _codeSTA();
    case OpCode::STX:
        return _codeSTX();
    case OpCode::STY:
        return _codeSTY();
    case OpCode::TAX:
        return _codeTAX();
    case OpCode::TAY:
        return _codeTAY();
    case OpCode::TSX:
        return _codeTSX();
    case OpCode::TXA:
        return _codeTXA();
    case OpCode::TXS:
        return _codeTXS();
    case OpCode::TYA:
        return _codeTYA();
    case OpCode::INV:
        return _codeINV();
    default:
        break;
    }

    return _codeINV();
}

void Cpu::_setStatusFlag(StatusBit statusBit, bool value)
{
    switch (statusBit) {
    case StatusBit::bitCarry:
        registers.statusFlag.carry = value;
        break;
    case StatusBit::bitZero:
        registers.statusFlag.zero = value;
        break;
    case StatusBit::bitDisableInterrupt:
        registers.statusFlag.disableInterrupt = value;
        break;
    case StatusBit::bitDecimalMode:
        registers.statusFlag.decimalMode = value;
        break;
    case StatusBit::bitBreakCommand:
        registers.statusFlag.breakCommand = value;
        break;
    case StatusBit::bitUnused:
        registers.statusFlag.unused = value;
        break;
    case StatusBit::bitOverflow:
        registers.statusFlag.overflow = value;
        break;
    case StatusBit::bitNegative:
        registers.statusFlag.negative = value;
        break;
    default:
        break;
    }
}

uint8_t Cpu::_getCurrentData()
{
    if (!(_commandTable[_currentOpCode].addressMode == AddressMode::IMP)) {
        _bus->read(_currentAddress, _currentData);
    }
    return _currentData;
}

// Implied Addressing
bool Cpu::_modeIMP()
{
    _currentData = registers.accumulator;

    return false;
}

// Immediate Address
bool Cpu::_modeIMM()
{
    _currentAddress = registers.programCounter++;

    return false;
}

// Zero Page Addressing
bool Cpu::_modeZP0()
{
    auto data = uint8_t{0x00};
    _bus->read(registers.programCounter, data);
    _currentAddress = static_cast<uint16_t>(data);

    registers.programCounter++;
    _currentAddress &= 0x00FF;

    return false;
}

// Register X Zero Page Addressing
bool Cpu::_modeZPX()
{
    auto data = uint8_t{0x00};
    _bus->read(registers.programCounter, data);
    _currentAddress = static_cast<uint16_t>(data) + registers.registerX;

    registers.programCounter++;
    _currentAddress &= 0x00FF;

    return false;
}

// Register X Zero Page Addressing
bool Cpu::_modeZPY()
{
    auto data = uint8_t{0x00};
    _bus->read(registers.programCounter, data);
    _currentAddress = static_cast<uint16_t>(data) + registers.registerY;

    registers.programCounter++;
    _currentAddress &= 0x00FF;

    return false;
}

// Relative Addressing
bool Cpu::_modeREL()
{
    auto data = uint8_t{0x00};
    _bus->read(registers.programCounter, data);
    _relativeAddress = static_cast<uint16_t>(data);

    registers.programCounter++;
    if (_relativeAddress & 0x0080) {
        _relativeAddress |= 0xFF00;
    }

    return false;
}

// Absolute Addressing
bool Cpu::_modeABS()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto lowByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read(registers.programCounter, data);
    auto highByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _currentAddress = (highByte << 8) | lowByte;

    return false;
}

// Register X Absolute Addressing
bool Cpu::_modeABX()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto lowByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read(registers.programCounter, data);
    auto highByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _currentAddress = (highByte << 8) | lowByte;
    _currentAddress += registers.registerX;

    if ((_currentAddress & 0xFF00) != (highByte << 8)) {
        return true;
    }

    return false;
}

// Register Y Absolute Addressing
bool Cpu::_modeABY()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto lowByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read(registers.programCounter, data);
    auto highByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _currentAddress = (highByte << 8) | lowByte;
    _currentAddress += registers.registerY;

    if ((_currentAddress & 0xFF00) != (highByte << 8)) {
        return true;
    }

    return false;
}

// Absolute Indirect
bool Cpu::_modeIND()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto lowByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read(registers.programCounter, data);
    auto highByte = static_cast<uint16_t>(data);
    registers.programCounter++;

    auto address = (highByte << 8) | lowByte;
    if (lowByte == 0x00FF) {
        /*
         * Emulate Hardware Bug:
         * It cannot correctly read addresses if it was of the form 0x--FF. When
         * reading two bytes from the specified address, it would not carry the
         * FF->00 overflow into the --. For example when the address is 0x23FF,
         * it would read 0x2300 and 0x23FF, instead of 0x2400 and 0x23FF.
         */
        _bus->read((address & 0xFF00), data);
        _currentAddress = static_cast<uint16_t>(data) << 8;
        _bus->read(address, data);
        _currentAddress |= data;
    } else {
        // Normal behaviour
        _bus->read((address + 1), data);
        _currentAddress = static_cast<uint16_t>(data) << 8;
        _bus->read(address, data);
        _currentAddress |= data;
    }

    return false;
}

// Regiter X Indirect Addressing
bool Cpu::_modeIZX()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto address = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read((address + registers.registerX) & 0x00FF, data);
    auto lowByte = static_cast<uint16_t>(data);
    _bus->read((address + registers.registerX + 1) & 0x00FF, data);
    auto highByte = static_cast<uint16_t>(data);

    _currentAddress = (highByte << 8) | lowByte;

    return false;
}

// Regiter Y Indirect Addressing
bool Cpu::_modeIZY()
{
    auto data = uint8_t{0x00};

    _bus->read(registers.programCounter, data);
    auto address = static_cast<uint16_t>(data);
    registers.programCounter++;

    _bus->read(address & 0x00FF, data);
    auto lowByte = static_cast<uint16_t>(data);
    _bus->read((address + 1) & 0x00FF, data);
    auto highByte = static_cast<uint16_t>(data);

    _currentAddress = (highByte << 8) | lowByte;
    _currentAddress += registers.registerY;
    if ((_currentAddress & 0xFF00) != (highByte << 8)) {
        return true;
    }

    return false;
}

/*
 *====================
 * Status OpCodes
 *====================
 */

// Clear Carry Flag
bool Cpu::_codeCLC()
{
    _setStatusFlag(StatusBit::bitCarry, false);

    return false;
}

// Clear Decimal Mode
bool Cpu::_codeCLD()
{
    _setStatusFlag(StatusBit::bitDecimalMode, false);

    return false;
}

// Clear Interrupt Disable Bit
bool Cpu::_codeCLI()
{
    _setStatusFlag(StatusBit::bitDisableInterrupt, false);

    return false;
}

// Clear Overflow Flag
bool Cpu::_codeCLV()
{
    _setStatusFlag(StatusBit::bitOverflow, false);

    return false;
}

// Set Carry Flag
bool Cpu::_codeSEC()
{
    _setStatusFlag(StatusBit::bitCarry, true);

    return false;
}

// Set Decimal Mode
bool Cpu::_codeSED()
{
    _setStatusFlag(StatusBit::bitDecimalMode, true);

    return false;
}

// Set Interrupt Disable Status
bool Cpu::_codeSEI()
{
    _setStatusFlag(StatusBit::bitDisableInterrupt, true);

    return false;
}

// No Operation
bool Cpu::_codeNOP()
{
    // Some NOP's consume more bytes
    if (_commandTable[_currentOpCode].opCodeLength > 1) {
        registers.programCounter += _commandTable[_currentOpCode].opCodeLength - 1;
    }

    // Some OpCode's requires additional cycle
    switch (_currentOpCode) {
    case 0x1C:
    case 0x3C:
    case 0x5C:
    case 0x7C:
    case 0xDC:
    case 0xFC:
        return true;
    default:
        break;
    }

    return false;
}

/*
 *====================
 * Arithmetic OpCodes
 *====================
 */

// Add Address to Accumulator with Carry
bool Cpu::_codeADC()
{
    auto data = _getCurrentData();

    // OLC method
    auto result = static_cast<uint16_t>(registers.accumulator) + static_cast<uint16_t>(data) +
                  static_cast<uint16_t>(registers.statusFlag.carry);

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);
    _setStatusFlag(StatusBit::bitOverflow,
                   (~(static_cast<uint16_t>(registers.accumulator) ^ static_cast<uint16_t>(data)) &
                    (static_cast<uint16_t>(registers.accumulator) ^ static_cast<uint16_t>(result)) & 0x0080));

    registers.accumulator = static_cast<uint8_t>(result & 0x00FF);

    // SimpleNES method
    /*
        auto result = static_cast<uint16_t>(registers.accumulator) + static_cast<uint16_t>(data) +
        static_cast<uint16_t>(registers.statusFlag.carry);

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);
    _setStatusFlag(StatusBit::bitOverflow, (!((registers.accumulator & 0x80) ^ (data & 0x80)) &&
        ((data & 0x80) ^ (result & 0x80))));

        registers.accumulator = static_cast<uint8_t>(result & 0x00FF);
    */

    return true;
}

// Bitwise AND with Accumulator
bool Cpu::_codeAND()
{
    auto data = _getCurrentData();
    registers.accumulator = registers.accumulator & data;
    _setStatusFlag(StatusBit::bitZero, !(registers.accumulator));
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return true;
}

// Shift Left One Bit (From Address or Accumulator)
bool Cpu::_codeASL()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(data) << 1;

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    if (_commandTable[_currentOpCode].addressMode == AddressMode::IMP) {
        registers.accumulator = static_cast<uint8_t>(result);
    } else {
        _bus->write(_currentAddress, static_cast<uint8_t>(result));
    }

    return false;
}

// Compare Address and Accumulator
bool Cpu::_codeCMP()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(registers.accumulator) - static_cast<uint16_t>(data);

    _setStatusFlag(StatusBit::bitCarry, registers.accumulator >= data);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    return true;
}

// Compare Address and Register X
bool Cpu::_codeCPX()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(registers.registerX) - static_cast<uint16_t>(data);

    _setStatusFlag(StatusBit::bitCarry, registers.registerX >= data);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    return false;
}

// Compare Address and Register Y
bool Cpu::_codeCPY()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(registers.registerY) - static_cast<uint16_t>(data);

    _setStatusFlag(StatusBit::bitCarry, registers.registerY >= data);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    return false;
}

// Decrement Address By One
bool Cpu::_codeDEC()
{
    auto data = _getCurrentData();
    auto result = data - 1;

    _bus->write(_currentAddress, result);

    _setStatusFlag(StatusBit::bitZero, !result);
    _setStatusFlag(StatusBit::bitNegative, result & 0x80);

    return false;
}

// Decrement Register X By One
bool Cpu::_codeDEX()
{
    registers.registerX--;
    _setStatusFlag(StatusBit::bitZero, !registers.registerX);
    _setStatusFlag(StatusBit::bitNegative, registers.registerX & 0x80);

    return false;
}

// Decrement Register Y By One
bool Cpu::_codeDEY()
{
    registers.registerY--;
    _setStatusFlag(StatusBit::bitZero, !registers.registerY);
    _setStatusFlag(StatusBit::bitNegative, registers.registerY & 0x80);

    return false;
}

// Exclusive-OR Address with Accumulator
bool Cpu::_codeEOR()
{
    auto data = _getCurrentData();
    registers.accumulator = registers.accumulator ^ data;
    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return true;
}

// Increment Address By One
bool Cpu::_codeINC()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(data) + 1;
    _bus->write(_currentAddress, static_cast<uint8_t>(result & 0x00FF));

    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    return false;
}

// Increment Register X By One
bool Cpu::_codeINX()
{
    registers.registerX++;
    _setStatusFlag(StatusBit::bitZero, !(registers.registerX));
    _setStatusFlag(StatusBit::bitNegative, registers.registerX & 0x80);

    return false;
}

// Increment Register Y By One
bool Cpu::_codeINY()
{
    registers.registerY++;
    _setStatusFlag(StatusBit::bitZero, !(registers.registerY));
    _setStatusFlag(StatusBit::bitNegative, registers.registerY & 0x80);

    return false;
}

// Shift Right One Bit (From Address or Accumulator)
bool Cpu::_codeLSR()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(data) >> 1;

    _setStatusFlag(StatusBit::bitCarry, data & 0x0001);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    if (_commandTable[_currentOpCode].addressMode == AddressMode::IMP) {
        registers.accumulator = static_cast<uint8_t>(result);
    } else {
        _bus->write(_currentAddress, static_cast<uint8_t>(result));
    }

    return false;
}

// OR Address with Accumulator
bool Cpu::_codeORA()
{
    auto data = _getCurrentData();
    registers.accumulator = registers.accumulator | data;
    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return true;
}

// Rotate One Bit Left (From Address or Accumulator)
bool Cpu::_codeROL()
{
    auto data = _getCurrentData();
    auto result = static_cast<uint16_t>(registers.statusFlag.carry) | static_cast<uint16_t>(data << 1);

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    if (_commandTable[_currentOpCode].addressMode == AddressMode::IMP) {
        registers.accumulator = static_cast<uint8_t>(result);
    } else {
        _bus->write(_currentAddress, static_cast<uint8_t>(result));
    }

    return false;
}

// Rotate One Bit Right (From Address or Accumulator)
bool Cpu::_codeROR()
{
    auto data = _getCurrentData();
    auto result = (static_cast<uint16_t>(registers.statusFlag.carry) << 7) | static_cast<uint16_t>(data >> 1);

    _setStatusFlag(StatusBit::bitCarry, data & 0x0001);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);

    if (_commandTable[_currentOpCode].addressMode == AddressMode::IMP) {
        registers.accumulator = static_cast<uint8_t>(result);
    } else {
        _bus->write(_currentAddress, static_cast<uint8_t>(result));
    }

    return false;
}

// Subtract Address from Accumulator with Borrow
bool Cpu::_codeSBC()
{
    auto data = _getCurrentData();

    // OLC method
    auto inverted = static_cast<uint16_t>(data) ^ 0x00FF;
    auto result =
        static_cast<uint16_t>(registers.accumulator) + inverted + static_cast<uint16_t>(registers.statusFlag.carry);

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);
    _setStatusFlag(StatusBit::bitOverflow,
                   ((result ^ static_cast<uint16_t>(registers.accumulator)) & (result ^ inverted) & 0x0080));

    registers.accumulator = static_cast<uint8_t>(result & 0x00FF);

    // SimpleNES method
    /*
    auto result = registers.accumulator - data - (1 - registers.statusFlag.carry);
    auto carryTest = static_cast<int16_t>(registers.accumulator) - static_cast<int16_t>(data) -
        static_cast<int16_t>(1 - registers.statusFlag.carry);

    _setStatusFlag(StatusBit::bitCarry, (carryTest >= 0));
    _setStatusFlag(StatusBit::bitZero, !result);
    _setStatusFlag(StatusBit::bitNegative, result & 0x80);
    _setStatusFlag(StatusBit::bitOverflow, (((registers.accumulator & 0x80) ^ (result & 0x80)) &&
        ((registers.accumulator & 0x80) ^ (data & 0x80))));

        registers.accumulator = result & 0xFF;
    */

    return true;
}

/*
 *====================
 * Branch OpCodes
 *====================
 */

// Branch on Carry Clear
bool Cpu::_codeBCC()
{
    if (!registers.statusFlag.carry) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Carry Set
bool Cpu::_codeBCS()
{
    if (registers.statusFlag.carry) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Result Zero
bool Cpu::_codeBEQ()
{
    if (registers.statusFlag.zero) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Result Minus
bool Cpu::_codeBMI()
{
    if (registers.statusFlag.negative) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Result Not Zero
bool Cpu::_codeBNE()
{
    if (!registers.statusFlag.zero) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Result Plus
bool Cpu::_codeBPL()
{
    if (!registers.statusFlag.negative) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Overflow Clear
bool Cpu::_codeBVC()
{
    if (!registers.statusFlag.overflow) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

// Branch on Overflow Set
bool Cpu::_codeBVS()
{
    if (registers.statusFlag.overflow) {
        _cycles++;
        _currentAddress = registers.programCounter + _relativeAddress;
        if ((_currentAddress & 0xFF00) != (registers.programCounter & 0xFF00)) {
            _cycles++;
        }
        registers.programCounter = _currentAddress;
    }

    return false;
}

/*
 *====================
 * Control OpCodes
 *====================
 */

// Test bits from Address with Accumulator
bool Cpu::_codeBIT()
{
    auto data = _getCurrentData();

    _setStatusFlag(StatusBit::bitZero, !(registers.accumulator & data));
    _setStatusFlag(StatusBit::bitNegative, data & 0x80);
    _setStatusFlag(StatusBit::bitOverflow, data & 0x40);

    return false;
}

// Force Break
bool Cpu::_codeBRK()
{
    registers.programCounter++;

    _setStatusFlag(StatusBit::bitDisableInterrupt, true);
    _bus->write(stackBaseAddress + registers.stackPointer,
                static_cast<uint8_t>((registers.programCounter >> 8) & 0x00FF));
    registers.stackPointer--;
    _bus->write(stackBaseAddress + registers.stackPointer, static_cast<uint8_t>(registers.programCounter & 0x00FF));
    registers.stackPointer--;

    _setStatusFlag(StatusBit::bitBreakCommand, true);
    _bus->write(stackBaseAddress + registers.stackPointer, registers.status);
    registers.stackPointer--;
    _setStatusFlag(StatusBit::bitBreakCommand, false);

    auto data = uint8_t{0x00};
    _bus->read(breakInterruptAddress, data);
    registers.programCounter = static_cast<uint16_t>(data);
    _bus->read(breakInterruptAddress + 1, data);
    registers.programCounter |= static_cast<uint16_t>(data) << 8;

    return false;
}

// Jump to New Location
bool Cpu::_codeJMP()
{
    registers.programCounter = _currentAddress;

    return false;
}

// Jump To New Location Saving Return Address
bool Cpu::_codeJSR()
{
    registers.programCounter--;

    _bus->write(stackBaseAddress + registers.stackPointer,
                static_cast<uint8_t>((registers.programCounter >> 8) & 0x00FF));
    registers.stackPointer--;
    _bus->write(stackBaseAddress + registers.stackPointer, static_cast<uint8_t>(registers.programCounter & 0x00FF));
    registers.stackPointer--;

    registers.programCounter = _currentAddress;

    return false;
}

// Push Accumulator to Stack Pointer
bool Cpu::_codePHA()
{
    _bus->write(stackBaseAddress + registers.stackPointer, registers.accumulator);
    registers.stackPointer--;

    return false;
}

// Push Status Register to Stack Pointer
bool Cpu::_codePHP()
{
    _setStatusFlag(StatusBit::bitBreakCommand, true);
    _setStatusFlag(StatusBit::bitUnused, true);
    _bus->write(stackBaseAddress + registers.stackPointer, registers.status);

    _setStatusFlag(StatusBit::bitBreakCommand, false);
    _setStatusFlag(StatusBit::bitUnused, false);
    registers.stackPointer--;

    return false;
}

// Pop Accumulator from Stack Pointer
bool Cpu::_codePLA()
{
    auto data = uint8_t{0x00};

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.accumulator = data;

    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return false;
}

// Pop Status Register from Stack Pointer
bool Cpu::_codePLP()
{
    auto data = uint8_t{0x00};

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.status = data & 0xEF;
    _setStatusFlag(StatusBit::bitUnused, true);

    return false;
}

// Return from Interrupt
bool Cpu::_codeRTI()
{
    auto data = uint8_t{0x00};

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.status = data;

    _setStatusFlag(StatusBit::bitBreakCommand, false);
    _setStatusFlag(StatusBit::bitUnused, false);
    // status &= ~B;
    // status &= ~U; --> why use unused bit???

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.programCounter = static_cast<uint16_t>(data);

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.programCounter |= static_cast<uint16_t>(data) << 8;

    return false;
}

// Return from Sub-Routine
bool Cpu::_codeRTS()
{
    auto data = uint8_t{0x00};

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.programCounter = static_cast<uint16_t>(data);

    registers.stackPointer++;
    _bus->read(stackBaseAddress + registers.stackPointer, data);
    registers.programCounter |= static_cast<uint16_t>(data) << 8;

    registers.programCounter++;

    return false;
}

/*
 *====================
 * Transfer OpCodes
 *====================
 */

// Load Accumulator from Address
bool Cpu::_codeLDA()
{
    registers.accumulator = _getCurrentData();
    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return true;
}

// Load Register X from Address
bool Cpu::_codeLDX()
{
    registers.registerX = _getCurrentData();
    _setStatusFlag(StatusBit::bitZero, !registers.registerX);
    _setStatusFlag(StatusBit::bitNegative, registers.registerX & 0x80);

    return true;
}

// Load Register Y from Address
bool Cpu::_codeLDY()
{
    registers.registerY = _getCurrentData();
    _setStatusFlag(StatusBit::bitZero, !registers.registerY);
    _setStatusFlag(StatusBit::bitNegative, registers.registerY & 0x80);

    return true;
}

// Store Accumulator to Address
bool Cpu::_codeSTA()
{
    _bus->write(_currentAddress, registers.accumulator);

    return false;
}

// Store Register X to Address
bool Cpu::_codeSTX()
{
    _bus->write(_currentAddress, registers.registerX);

    return false;
}

// Store Register Y to Address
bool Cpu::_codeSTY()
{
    _bus->write(_currentAddress, registers.registerY);

    return false;
}

// Transfer Accumulator to Register X
bool Cpu::_codeTAX()
{
    registers.registerX = registers.accumulator;
    _setStatusFlag(StatusBit::bitZero, !registers.registerX);
    _setStatusFlag(StatusBit::bitNegative, registers.registerX & 0x80);

    return false;
}

// Transfer Accumulator to Register Y
bool Cpu::_codeTAY()
{
    registers.registerY = registers.accumulator;
    _setStatusFlag(StatusBit::bitZero, !registers.registerY);
    _setStatusFlag(StatusBit::bitNegative, registers.registerY & 0x80);

    return false;
}

// Transfer Stack Pointer to Register X
bool Cpu::_codeTSX()
{
    registers.registerX = registers.stackPointer;
    _setStatusFlag(StatusBit::bitZero, !registers.registerX);
    _setStatusFlag(StatusBit::bitNegative, registers.registerX & 0x80);

    return false;
}

// Transfer Register X to Accumulator
bool Cpu::_codeTXA()
{
    registers.accumulator = registers.registerX;
    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return false;
}

// Transfer Register X to Stack Pointer
bool Cpu::_codeTXS()
{
    registers.stackPointer = registers.registerX;

    return false;
}

// Transfer Register Y to Accumulator
bool Cpu::_codeTYA()
{
    registers.accumulator = registers.registerY;
    _setStatusFlag(StatusBit::bitZero, !registers.accumulator);
    _setStatusFlag(StatusBit::bitNegative, registers.accumulator & 0x80);

    return false;
}

// Invalid Instruction Code
bool Cpu::_codeINV()
{
    return false;
}
