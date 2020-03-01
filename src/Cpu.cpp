#include "Cpu.hpp"
#include "CpuBus.hpp"

constexpr uint8_t resetStackOffset = 0xFD;
constexpr uint16_t stackBaseAddress = 0x0100;
constexpr uint16_t nonMaskableInterruptAddress = 0xFFFA;
constexpr uint16_t resetInterruptAddress = 0xFFFC;
constexpr uint16_t breakInterruptAddress = 0xFFFE;

Cpu::Cpu(std::shared_ptr<IDevice> bus,
    std::shared_ptr<Ppu> ppu,
    std::shared_ptr<Cartridge> cartridge)
: _bus{bus}
, _ppu{ppu}
, _cartridge{cartridge}
{
    // Command Table
	_commandTable = {
		{ OpCode::BRK, AddressMode::IMM, "BRK", 7 },
        { OpCode::ORA, AddressMode::IZX, "ORA", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 3 },
        { OpCode::ORA, AddressMode::ZP0, "ORA", 3 },
        { OpCode::ASL, AddressMode::ZP0, "ASL", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::PHP, AddressMode::IMP, "PHP", 3 },
        { OpCode::ORA, AddressMode::IMM, "ORA", 2 },
        { OpCode::ASL, AddressMode::IMP, "ASL", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::ORA, AddressMode::ABS, "ORA", 4 },
        { OpCode::ASL, AddressMode::ABS, "ASL", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BPL, AddressMode::REL, "BPL", 2 },
        { OpCode::ORA, AddressMode::IZY, "ORA", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::ORA, AddressMode::ZPX, "ORA", 4 },
        { OpCode::ASL, AddressMode::ZPX, "ASL", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::CLC, AddressMode::IMP, "CLC", 2 },
        { OpCode::ORA, AddressMode::ABY, "ORA", 4 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::ORA, AddressMode::ABX, "ORA", 4 },
        { OpCode::ASL, AddressMode::ABX, "ASL", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
		{ OpCode::JSR, AddressMode::ABS, "JSR", 6 },
        { OpCode::AND, AddressMode::IZX, "AND", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::BIT, AddressMode::ZP0, "BIT", 3 },
        { OpCode::AND, AddressMode::ZP0, "AND", 3 },
        { OpCode::ROL, AddressMode::ZP0, "ROL", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::PLP, AddressMode::IMP, "PLP", 4 },
        { OpCode::AND, AddressMode::IMM, "AND", 2 },
        { OpCode::ROL, AddressMode::IMP, "ROL", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::BIT, AddressMode::ABS, "BIT", 4 },
        { OpCode::AND, AddressMode::ABS, "AND", 4 },
        { OpCode::ROL, AddressMode::ABS, "ROL", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BMI, AddressMode::REL, "BMI", 2 },
        { OpCode::AND, AddressMode::IZY, "AND", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::AND, AddressMode::ZPX, "AND", 4 },
        { OpCode::ROL, AddressMode::ZPX, "ROL", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::SEC, AddressMode::IMP, "SEC", 2 },
        { OpCode::AND, AddressMode::ABY, "AND", 4 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::AND, AddressMode::ABX, "AND", 4 },
        { OpCode::ROL, AddressMode::ABX, "ROL", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
		{ OpCode::RTI, AddressMode::IMP, "RTI", 6 },
        { OpCode::EOR, AddressMode::IZX, "EOR", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 3 },
        { OpCode::EOR, AddressMode::ZP0, "EOR", 3 },
        { OpCode::LSR, AddressMode::ZP0, "LSR", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::PHA, AddressMode::IMP, "PHA", 3 },
        { OpCode::EOR, AddressMode::IMM, "EOR", 2 },
        { OpCode::LSR, AddressMode::IMP, "LSR", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::JMP, AddressMode::ABS, "JMP", 3 },
        { OpCode::EOR, AddressMode::ABS, "EOR", 4 },
        { OpCode::LSR, AddressMode::ABS, "LSR", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BVC, AddressMode::REL, "BVC", 2 },
        { OpCode::EOR, AddressMode::IZY, "EOR", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::EOR, AddressMode::ZPX, "EOR", 4 },
        { OpCode::LSR, AddressMode::ZPX, "LSR", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::CLI, AddressMode::IMP, "CLI", 2 },
        { OpCode::EOR, AddressMode::ABY, "EOR", 4 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::EOR, AddressMode::ABX, "EOR", 4 },
        { OpCode::LSR, AddressMode::ABX, "LSR", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
		{ OpCode::RTS, AddressMode::IMP, "RTS", 6 },
        { OpCode::ADC, AddressMode::IZX, "ADC", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 3 },
        { OpCode::ADC, AddressMode::ZP0, "ADC", 3 },
        { OpCode::ROR, AddressMode::ZP0, "ROR", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::PLA, AddressMode::IMP, "PLA", 4 },
        { OpCode::ADC, AddressMode::IMM, "ADC", 2 },
        { OpCode::ROR, AddressMode::IMP, "ROR", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::JMP, AddressMode::IND, "JMP", 5 },
        { OpCode::ADC, AddressMode::ABS, "ADC", 4 },
        { OpCode::ROR, AddressMode::ABS, "ROR", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BVS, AddressMode::REL, "BVS", 2 },
        { OpCode::ADC, AddressMode::IZY, "ADC", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::ADC, AddressMode::ZPX, "ADC", 4 },
        { OpCode::ROR, AddressMode::ZPX, "ROR", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::SEI, AddressMode::IMP, "SEI", 2 },
        { OpCode::ADC, AddressMode::ABY, "ADC", 4 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::ADC, AddressMode::ABX, "ADC", 4 },
        { OpCode::ROR, AddressMode::ABX, "ROR", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
		{ OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::STA, AddressMode::IZX, "STA", 6 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::STY, AddressMode::ZP0, "STY", 3 },
        { OpCode::STA, AddressMode::ZP0, "STA", 3 },
        { OpCode::STX, AddressMode::ZP0, "STX", 3 },
        { OpCode::INV, AddressMode::IMP, "INV", 3 },
        { OpCode::DEY, AddressMode::IMP, "DEY", 2 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::TXA, AddressMode::IMP, "TXA", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::STY, AddressMode::ABS, "STY", 4 },
        { OpCode::STA, AddressMode::ABS, "STA", 4 },
        { OpCode::STX, AddressMode::ABS, "STX", 4 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
		{ OpCode::BCC, AddressMode::REL, "BCC", 2 },
        { OpCode::STA, AddressMode::IZY, "STA", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::STY, AddressMode::ZPX, "STY", 4 },
        { OpCode::STA, AddressMode::ZPX, "STA", 4 },
        { OpCode::STX, AddressMode::ZPY, "STX", 4 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
        { OpCode::TYA, AddressMode::IMP, "TYA", 2 },
        { OpCode::STA, AddressMode::ABY, "STA", 5 },
        { OpCode::TXS, AddressMode::IMP, "TXS", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::NOP, AddressMode::IMP, "INV", 5 },
        { OpCode::STA, AddressMode::ABX, "STA", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
		{ OpCode::LDY, AddressMode::IMM, "LDY", 2 },
        { OpCode::LDA, AddressMode::IZX, "LDA", 6 },
        { OpCode::LDX, AddressMode::IMM, "LDX", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::LDY, AddressMode::ZP0, "LDY", 3 },
        { OpCode::LDA, AddressMode::ZP0, "LDA", 3 },
        { OpCode::LDX, AddressMode::ZP0, "LDX", 3 },
        { OpCode::INV, AddressMode::IMP, "INV", 3 },
        { OpCode::TAY, AddressMode::IMP, "TAY", 2 },
        { OpCode::LDA, AddressMode::IMM, "LDA", 2 },
        { OpCode::TAX, AddressMode::IMP, "TAX", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::LDY, AddressMode::ABS, "LDY", 4 },
        { OpCode::LDA, AddressMode::ABS, "LDA", 4 },
        { OpCode::LDX, AddressMode::ABS, "LDX", 4 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
		{ OpCode::BCS, AddressMode::REL, "BCS", 2 },
        { OpCode::LDA, AddressMode::IZY, "LDA", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::LDY, AddressMode::ZPX, "LDY", 4 },
        { OpCode::LDA, AddressMode::ZPX, "LDA", 4 },
        { OpCode::LDX, AddressMode::ZPY, "LDX", 4 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
        { OpCode::CLV, AddressMode::IMP, "CLV", 2 },
        { OpCode::LDA, AddressMode::ABY, "LDA", 4 },
        { OpCode::TSX, AddressMode::IMP, "TSX", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
        { OpCode::LDY, AddressMode::ABX, "LDY", 4 },
        { OpCode::LDA, AddressMode::ABX, "LDA", 4 },
        { OpCode::LDX, AddressMode::ABY, "LDX", 4 },
        { OpCode::INV, AddressMode::IMP, "INV", 4 },
		{ OpCode::CPY, AddressMode::IMM, "CPY", 2 },
        { OpCode::CMP, AddressMode::IZX, "CMP", 6 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::CPY, AddressMode::ZP0, "CPY", 3 },
        { OpCode::CMP, AddressMode::ZP0, "CMP", 3 },
        { OpCode::DEC, AddressMode::ZP0, "DEC", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::INY, AddressMode::IMP, "INY", 2 },
        { OpCode::CMP, AddressMode::IMM, "CMP", 2 },
        { OpCode::DEX, AddressMode::IMP, "DEX", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::CPY, AddressMode::ABS, "CPY", 4 },
        { OpCode::CMP, AddressMode::ABS, "CMP", 4 },
        { OpCode::DEC, AddressMode::ABS, "DEC", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BNE, AddressMode::REL, "BNE", 2 },
        { OpCode::CMP, AddressMode::IZY, "CMP", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::CMP, AddressMode::ZPX, "CMP", 4 },
        { OpCode::DEC, AddressMode::ZPX, "DEC", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::CLD, AddressMode::IMP, "CLD", 2 },
        { OpCode::CMP, AddressMode::ABY, "CMP", 4 },
        { OpCode::NOP, AddressMode::IMP, "NOP", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::CMP, AddressMode::ABX, "CMP", 4 },
        { OpCode::DEC, AddressMode::ABX, "DEC", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
		{ OpCode::CPX, AddressMode::IMM, "CPX", 2 },
        { OpCode::SBC, AddressMode::IZX, "SBC", 6 },
        { OpCode::NOP, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::CPX, AddressMode::ZP0, "CPX", 3 },
        { OpCode::SBC, AddressMode::ZP0, "SBC", 3 },
        { OpCode::INC, AddressMode::ZP0, "INC", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 5 },
        { OpCode::INX, AddressMode::IMP, "INX", 2 },
        { OpCode::SBC, AddressMode::IMM, "SBC", 2 },
        { OpCode::NOP, AddressMode::IMP, "NOP", 2 },
        { OpCode::SBC, AddressMode::IMP, "INV", 2 },
        { OpCode::CPX, AddressMode::ABS, "CPX", 4 },
        { OpCode::SBC, AddressMode::ABS, "SBC", 4 },
        { OpCode::INC, AddressMode::ABS, "INC", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
		{ OpCode::BEQ, AddressMode::REL, "BEQ", 2 },
        { OpCode::SBC, AddressMode::IZY, "SBC", 5 },
        { OpCode::INV, AddressMode::IMP, "INV", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 8 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::SBC, AddressMode::ZPX, "SBC", 4 },
        { OpCode::INC, AddressMode::ZPX, "INC", 6 },
        { OpCode::INV, AddressMode::IMP, "INV", 6 },
        { OpCode::SED, AddressMode::IMP, "SED", 2 },
        { OpCode::SBC, AddressMode::ABY, "SBC", 4 },
        { OpCode::NOP, AddressMode::IMP, "NOP", 2 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
        { OpCode::NOP, AddressMode::IMP, "INV", 4 },
        { OpCode::SBC, AddressMode::ABX, "SBC", 4 },
        { OpCode::INC, AddressMode::ABX, "INC", 7 },
        { OpCode::INV, AddressMode::IMP, "INV", 7 },
	};
}

Cpu::~Cpu()
{
}

// Reset Interrupt
void Cpu::resetInterrupt()
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

	// Reset Addresses/Data
	_relativeAddress = 0x0000;
	_currentAddress = 0x0000;
	_currentData = 0x00;

	// Reset cycles
	_cycles = 8;
}

// Interrupt Request
void Cpu::interruptRequest()
{
	// Check if interrupt was enabled
	if (!registers.statusFlag.disableInterrupt)
	{
        _bus->write(stackBaseAddress + registers.stackPointer,
            static_cast<uint8_t>((registers.programCounter >> 8) & 0x00FF));
		registers.stackPointer--;
		_bus->write(stackBaseAddress + registers.stackPointer,
            static_cast<uint8_t>(registers.programCounter & 0x00FF));
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
	_bus->write(stackBaseAddress + registers.stackPointer,
        static_cast<uint8_t>(registers.programCounter & 0x00FF));
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

// Execute one clock cycle
void Cpu::tick()
{
	if (_cycles == 0)
	{
		// Read next OpCode
        _bus->read(registers.programCounter, _currentOpCode);

		registers.programCounter++;

        _setStatusFlag(StatusBit::bitUnused, true);

		_cycles = _commandTable[_currentOpCode].cycles;
		// Execute AddressMode and OpCode and add cycles if needed
		if (_runAddressMode(_commandTable[_currentOpCode].addressMode) ||
            _runOpCode(_commandTable[_currentOpCode].opCode)) {
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

bool Cpu::_getCurrentData()
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
        // Page boundary hardware bug
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
    _setStatusFlag(StatusBit::bitOverflow, (~(static_cast<uint16_t>(registers.accumulator) ^ static_cast<uint16_t>(data)) &
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

    _setStatusFlag(StatusBit::bitCarry, result & 0x0001);
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

    _setStatusFlag(StatusBit::bitCarry, result & 0x0001);
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
    auto result = static_cast<uint16_t>(registers.accumulator) + inverted +
        static_cast<uint16_t>(registers.statusFlag.carry);

    _setStatusFlag(StatusBit::bitCarry, result & 0xFF00);
    _setStatusFlag(StatusBit::bitZero, !(result & 0x00FF));
    _setStatusFlag(StatusBit::bitNegative, result & 0x0080);
    _setStatusFlag(StatusBit::bitOverflow, ((result ^ static_cast<uint16_t>(registers.accumulator)) &
        (result ^ inverted) & 0x0080));

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
	if (registers.statusFlag.carry)
	{
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
	if (registers.statusFlag.zero)
	{
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
	if (registers.statusFlag.negative)
	{
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
	if (!registers.statusFlag.zero)
	{
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
	if (!registers.statusFlag.negative)
	{
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
	if (!registers.statusFlag.overflow)
	{
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
	if (registers.statusFlag.overflow)
	{
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
    _bus->write(stackBaseAddress + registers.stackPointer,
            static_cast<uint8_t>(registers.programCounter & 0x00FF));
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
    _bus->write(stackBaseAddress + registers.stackPointer,
            static_cast<uint8_t>(registers.programCounter & 0x00FF));
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
    registers.status = data;
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
	//status &= ~B;
	//status &= ~U; --> why use unused bit???

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
