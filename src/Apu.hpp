#pragma once

#include <cstdint>
#include <array>
#include <atomic>
#include <mutex>
#include <stdio.h>

#include "IDevice.hpp"

typedef struct PulseRegister0Flags {
    uint8_t envelopePeriod : 4;
    bool constantEnvelopeFlag : 1;
    bool lengthCounterHalt : 1;
    uint8_t dutyCycle : 2;
} PulseRegister0Flags;

typedef struct PulseRegister1Flags {
    uint8_t sweepShiftCount : 3;
    bool sweepNegateFlag : 1;
    uint8_t sweepPeriod : 3;
    bool sweepEnableFlag : 1;
} PulseRegister1Flags;

typedef struct PulseRegister2Flags {
    uint8_t pulseTimerLow;
} PulseRegister2Flags;

typedef struct PulseRegister3Flags {
    uint8_t pulseTimerHigh : 3;
    uint8_t lengthCounter : 5;
} PulseRegister3Flags;

typedef struct Pulse {
    // Registers
    union {
        uint8_t register0;
        PulseRegister0Flags Register0Flag;
    };
    union {
        uint8_t register1;
        PulseRegister1Flags Register1Flag;
    };
    union {
        uint8_t register2;
        PulseRegister2Flags Register2Flag;
    };
    union {
        uint8_t register3;
        PulseRegister3Flags Register3Flag;
    };

    // Data
    uint8_t volume{0x00};
    uint16_t period{0x0000};
    float dutyCycle{0.0f};
    uint8_t envelopeDecay{0x00};
    uint8_t envelopeCounter{0x00};
    bool envelopeStart{false};
    uint16_t sweepTarget{0x0000};
    uint8_t sweepCounter{0x00};
    bool sweepDone{false};
    uint8_t lengthCounter{0x00};
} Pulse;

typedef struct TriangleRegister0Flags {
    uint8_t linearCounterReload : 7;
    bool lengthCounterHalt : 1;
} TriangleRegister0Flags;

typedef struct TriangleRegister1Flags {
    uint8_t triangleTimerLow;
} TriangleRegister1Flags;

typedef struct TriangleRegister2Flags {
    uint8_t triangleTimerHigh : 3;
    uint8_t lengthCounter : 5;
} TriangleRegister2Flags;

typedef struct Triangle {
    // Registers
    union {
        uint8_t register0;
        TriangleRegister0Flags Register0Flag;
    };
    union {
        uint8_t register1;
        TriangleRegister1Flags Register1Flag;
    };
    union {
        uint8_t register2;
        TriangleRegister2Flags Register2Flag;
    };

    // Data
    uint16_t period{0x0000};
    uint8_t lengthCounter{0x00};
    bool linearCounterReload{false};
    uint8_t linearCounter{0x00};
} Triangle;

typedef struct ControlRegisterFlags {
    bool pulse1Enable : 1;
    bool pulse2Enable : 1;
    bool triangleEnable : 1;
    bool noiseEnable : 1;
    bool dmcEnable : 1;
    uint8_t unused : 3;
} ControlRegisterFlags;

typedef struct ApuRegister {
    union {
        uint8_t control;
        ControlRegisterFlags controlFlag;
    };
} ApuRegister;

class Apu : public IDevice {
public:
    Apu();
    ~Apu();

    /// @name Implementation IDevice
    /// @[
    bool read(uint16_t address, uint8_t& data);
    bool write(uint16_t address, uint8_t data);
    /// @]

    // Execute one clock cycle
    void tick();
    void reset();

    float getMixedOutput(float time);

private:
    using LockGuard = std::unique_lock<std::mutex>;
    std::mutex _mutex;

    void doQuarterFrame();
    void doHalfFrame();

    void doEnvelope(Pulse& pulse);
    void doSweep(Pulse& pulse);
    void doLengthCounters(Pulse& pulse, bool enable);
    void doLengthCounters(Triangle& triangle, bool enable);
    void doLinearCounters(Triangle& triangle);

    float getPulseOutput(Pulse pulse, float time);
    float getTriangleOutput(Triangle triangle, float time);
    float getDutyCycle(uint8_t dutyCycle);

    uint32_t _frameCounter{0};

    // APU registers
    ApuRegister registers;

    // Pulse data
    Pulse pulse1;
    Pulse pulse2;

    // Triangle data
    Triangle triangle;
};
