#pragma once

#include <stdio.h>
#include <string.h>
#include <functional>
#include <vector>
#include <string>
#include <memory>

#include "IDevice.hpp"
#include "Ppu.hpp"

enum class AddressMode {
    IMP, IMM, ZP0, ZPX,
    ZPY, REL, ABS, ABX,
    ABY, IND, IZX, IZY,
};

enum class OpCode {
    ADC, AND, ASL, BCC,
    BCS, BEQ, BIT, BMI,
    BNE, BPL, BRK, BVC,
    BVS, CLC, CLD, CLI,
    CLV, CMP, CPX, CPY,
    DEC, DEX, DEY, EOR,
    INC, INX, INY, JMP,
    JSR, LDA, LDX, LDY,
    LSR, NOP, ORA, PHA,
    PHP, PLA, PLP, ROL,
    ROR, RTI, RTS, SBC,
    SEC, SED, SEI, STA,
    STX, STY, TAX, TAY,
    TSX, TXA, TXS, TYA,
    INV,
};

enum class StatusBit {
    bitCarry,
    bitZero,
    bitDisableInterrupt,
    bitDecimalMode,
    bitBreakCommand,
    bitUnused,
    bitOverflow,
    bitNegative,
};

struct StatusFlags {
    bool carry : 1;
    bool zero : 1;
    bool disableInterrupt : 1;
    bool decimalMode : 1;
    bool breakCommand : 1;
    bool unused : 1;
    bool overflow : 1;
    bool negative : 1;
};

struct CpuRegister {
    uint16_t programCounter;
    uint8_t stackPointer;
    uint8_t accumulator;
    uint8_t registerX;
    uint8_t registerY;
    union {
        uint8_t status;
        StatusFlags statusFlag;
    };
};

struct Command {
    uint8_t opCodeId;
    OpCode opCode;
    uint8_t opCodeLength;
    AddressMode addressMode;
    std::string name;
    uint8_t cycles;
};

struct DMA {
    bool mode;
    bool startTransfer;
    uint8_t addressLow;
    uint8_t addressHigh;
    uint8_t data;
};

class Cpu {
public:
    Cpu(std::shared_ptr<IDevice> bus, std::shared_ptr<Ppu> ppu);
    ~Cpu();

    // CPU registers
    CpuRegister registers;

    // CPU interrupts
    void reset();
    void interruptRequest();
    void nonMaskableInterruptRequest();

    // Execute one clock cycle
    void tick(bool isOddCycle);

private:
    uint16_t _currentAddress = 0x0000;
    uint16_t _relativeAddress = 0x00;
    uint8_t _currentOpCode = 0x00;
    uint8_t _currentData = 0x00;
    uint8_t _cycles = 0;

    // Bus device attached to this Cpu
    std::shared_ptr<IDevice> _bus;

    std::vector<Command> _commandTable;

    // Wrapper functions to the Cpu Bus
    bool _read(uint16_t address, uint8_t& data);
    bool _write(uint16_t address, uint8_t data);

    // PPU Interface for DMA function
    std::shared_ptr<Ppu> _ppu;
    DMA _dma;

    void _disassemble();
    bool _runAddressMode(AddressMode addressMode);
    bool _runOpCode(OpCode opCode);
    void _setStatusFlag(StatusBit statusBit, bool value);
    uint8_t _getCurrentData();

    // Address Mode implementations
    bool _modeIMP(); bool _modeIMM(); bool _modeZP0(); bool _modeZPX();
    bool _modeZPY(); bool _modeREL(); bool _modeABS(); bool _modeABX();
    bool _modeABY(); bool _modeIND(); bool _modeIZX(); bool _modeIZY();

    // Opcode instructions
    bool _codeADC(); bool _codeAND(); bool _codeASL(); bool _codeBCC();
    bool _codeBCS(); bool _codeBEQ(); bool _codeBIT(); bool _codeBMI();
    bool _codeBNE(); bool _codeBPL(); bool _codeBRK(); bool _codeBVC();
    bool _codeBVS(); bool _codeCLC(); bool _codeCLD(); bool _codeCLI();
    bool _codeCLV(); bool _codeCMP(); bool _codeCPX(); bool _codeCPY();
    bool _codeDEC(); bool _codeDEX(); bool _codeDEY(); bool _codeEOR();
    bool _codeINC(); bool _codeINX(); bool _codeINY(); bool _codeJMP();
    bool _codeJSR(); bool _codeLDA(); bool _codeLDX(); bool _codeLDY();
    bool _codeLSR(); bool _codeNOP(); bool _codeORA(); bool _codePHA();
    bool _codePHP(); bool _codePLA(); bool _codePLP(); bool _codeROL();
    bool _codeROR(); bool _codeRTI(); bool _codeRTS(); bool _codeSBC();
    bool _codeSEC(); bool _codeSED(); bool _codeSEI(); bool _codeSTA();
    bool _codeSTX(); bool _codeSTY(); bool _codeTAX(); bool _codeTAY();
    bool _codeTSX(); bool _codeTXA(); bool _codeTXS(); bool _codeTYA();
    bool _codeINV();
};
