#include "math.h"

#include "Apu.hpp"

// Reference: https://wiki.nesdev.com/w/index.php/APU
constexpr auto pulse1Address0 = 0x4000;
constexpr auto pulse1Address1 = 0x4001;
constexpr auto pulse1Address2 = 0x4002;
constexpr auto pulse1Address3 = 0x4003;
constexpr auto pulse2Address0 = 0x4004;
constexpr auto pulse2Address1 = 0x4005;
constexpr auto pulse2Address2 = 0x4006;
constexpr auto pulse2Address3 = 0x4007;
constexpr auto triangleAddress0 = 0x4008;
constexpr auto triangleAddress1 = 0x400A;
constexpr auto triangleAddress2 = 0x400B;
constexpr auto noiseAddress0 = 0x400C;
constexpr auto noiseAddress1 = 0x400E;
constexpr auto noiseAddress2 = 0x400F;
constexpr auto DMCAddress0 = 0x4010;
constexpr auto DMCAddress1 = 0x4011;
constexpr auto DMCAddress2 = 0x4012;
constexpr auto DMCAddress3 = 0x4013;
constexpr auto apuControlAddress = 0x4015;

constexpr auto apuFrequency = 894886.5f;
constexpr uint8_t lengthCounterTable[] = {
    0x0A, 0xFE, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06,
    0xA0, 0x08, 0x3C, 0x0A, 0x0E, 0x0C, 0x1A, 0x0E,
    0x0C, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16,
    0xC0, 0x18, 0x48, 0x1A, 0x10, 0x1C, 0x20, 0x1E,
};
constexpr uint8_t triangleStep[] = {
    15, 14, 13, 12, 11, 10, 9,  8,
    7,  6,  5,  4,  3,  2,  1,  0,
    0,  1,  2,  3,  4,  5,  6,  7,
    8,  9,  10, 11, 12, 13, 14, 15,
};

Apu::Apu()
{
}

Apu::~Apu()
{
}

bool Apu::read(uint16_t address, uint8_t& data)
{
    data = 0x00;
    return true;
}

bool Apu::write(uint16_t address, uint8_t data)
{
    auto lock = LockGuard{_mutex};
    switch (address) {
    case pulse1Address0:
        _pulse1.register0 = data;
        _pulse1.dutyCycle = getDutyCycle(_pulse1.Register0Flag.dutyCycle);
        if (_pulse1.Register0Flag.constantEnvelopeFlag) {
            _pulse1.volume = _pulse1.Register0Flag.envelopePeriod;
        } else {
            _pulse1.envelopeDecay = 15;
            _pulse1.volume = _pulse1.envelopeDecay;
        }
        break;
    case pulse1Address1:
        _pulse1.register1 = data;
        break;
    case pulse1Address2:
        _pulse1.register2 = data;
        _pulse1.period = (_pulse1.period & 0x0700) | _pulse1.Register2Flag.pulseTimerLow;
        break;
    case pulse1Address3:
        _pulse1.register3 = data;
        _pulse1.period = (static_cast<uint16_t>(_pulse1.Register3Flag.pulseTimerHigh) << 8) | (_pulse1.period & 0xFF);
        _pulse1.sweepTarget = _pulse1.period;
        _pulse1.envelopeStart = true;
        if (_registers.controlFlag.pulse1Enable) {
            _pulse1.lengthCounter = lengthCounterTable[_pulse1.Register3Flag.lengthCounter];
        }
        break;
    case pulse2Address0:
        _pulse2.register0 = data;
        _pulse2.dutyCycle = getDutyCycle(_pulse2.Register0Flag.dutyCycle);
        if (_pulse2.Register0Flag.constantEnvelopeFlag) {
            _pulse2.volume = _pulse2.Register0Flag.envelopePeriod;
        } else {
            _pulse2.envelopeDecay = 15;
            _pulse2.volume = _pulse2.envelopeDecay;
        }
        break;
    case pulse2Address1:
        _pulse2.register1 = data;
        break;
    case pulse2Address2:
        _pulse2.register2 = data;
        _pulse2.period = (_pulse2.period & 0x0700) | _pulse2.Register2Flag.pulseTimerLow;
        break;
    case pulse2Address3:
        _pulse2.register3 = data;
        _pulse2.period = (static_cast<uint16_t>(_pulse2.Register3Flag.pulseTimerHigh) << 8) | (_pulse2.period & 0xFF);
        _pulse2.sweepTarget = _pulse2.period;
        _pulse2.envelopeStart = true;
        if (_registers.controlFlag.pulse2Enable) {
            _pulse2.lengthCounter = lengthCounterTable[_pulse2.Register3Flag.lengthCounter];
        }
        break;
    case triangleAddress0:
        _triangle.register0 = data;
        break;
    case triangleAddress1:
        _triangle.register1 = data;
        _triangle.period = (_triangle.period & 0x0700) | _triangle.Register1Flag.triangleTimerLow;
        break;
    case triangleAddress2:
        _triangle.register2 = data;
        _triangle.period = (static_cast<uint16_t>(_triangle.Register2Flag.triangleTimerHigh) << 8) | (_triangle.period & 0xFF);
        if (_registers.controlFlag.triangleEnable) {
            _triangle.lengthCounter = lengthCounterTable[_triangle.Register2Flag.lengthCounter];
        }
        _triangle.linearCounterReload = true;
        break;
    case apuControlAddress:
        _registers.control = data;
        if (!_registers.controlFlag.pulse1Enable) {
            _pulse1.lengthCounter = 0;
        }
        if (!_registers.controlFlag.pulse2Enable) {
            _pulse2.lengthCounter = 0;
        }
        if (!_registers.controlFlag.triangleEnable) {
            _triangle.lengthCounter = 0;
        }
        break;
    default:
        break;
    }

    return true;
}

// Execute one clock cycle
void Apu::tick()
{
    // Reference: https://wiki.nesdev.com/w/index.php/APU_Frame_Counter
    // The APU runs half the rate of CPU, so 2 CPU cycles = 1 APU cycle.

    auto lock = LockGuard{_mutex};

    _frameCounter++;

    /* Mode 0: 4-Step Sequence */
    switch (_frameCounter) {
    case 3729:
        doQuarterFrame();
        break;
    case 7457:
        doQuarterFrame();
        doHalfFrame();
        break;
    case 11186:
        doQuarterFrame();
        break;
    case 14916:
        doQuarterFrame();
        doHalfFrame();
        _frameCounter = 0;
        break;
    }
}

void Apu::reset()
{
}

float Apu::getMixedOutput(float time)
{
    auto lock = LockGuard{_mutex};
    auto outputPulse1 = getPulseOutput(_pulse1, time);
    auto outputPulse2 = getPulseOutput(_pulse2, time);
    auto outputTriangle = getTriangleOutput(_triangle, time);

    return (outputPulse1 * 0.10f + outputPulse2 * 0.10f + outputTriangle * 0.25f) * 0.25f;
}

float Apu::getPulseOutput(Pulse pulse, float time)
{
    auto outputPulse = 0.0f;
    if (pulse.lengthCounter > 0) {
        // Period is 8 times the period because we are shifting every bit on a byte (8-bit)
        auto period = (8.0f * static_cast<float>(pulse.period)) / apuFrequency;
        // Amplitude is scaled to 15 since that's our maximum envelope decay (4-bit)
        auto amplitude = static_cast<float>(pulse.volume) / 15.0f;
        // Compute cycle percentage based from period and time and from there we
        // know how to generate pulse wave based on the given duty cycle
        auto cyclePercentage = (time - floor(time / period) * period) / period;
        if (pulse.dutyCycle < 0.0f) {
            // This is for negative duty cycle waveform
            if (cyclePercentage <= (-1.0f * pulse.dutyCycle)) {
                outputPulse = -1.0f * amplitude;
            } else {
                outputPulse = 1.0f * amplitude;
            }
        } else {
            if (cyclePercentage <= pulse.dutyCycle) {
                outputPulse = 1.0f * amplitude;
            } else {
                outputPulse = -1.0f * amplitude;
            }
        }
    }

    return outputPulse;
}

float Apu::getTriangleOutput(Triangle triangle, float time)
{
    auto outputTriangle = 0.0f;
    if ((_triangle.lengthCounter > 0) && (_triangle.linearCounter > 0)) {
        // Period is 32 times the period because we have a 32-step sequence
        auto period = (32.0f * static_cast<float>(_triangle.period)) / apuFrequency;
        // Compute cycle percentage based from period and time
        auto cyclePercentage = (time - floor(time / period) * period) / period;
        // Convert cycle percentage to which step we are in triangle sequence
        auto index = static_cast<uint8_t>(floor(cyclePercentage * 32.0f));
        // Normalized to 1.0f and centered at zero
        outputTriangle = (static_cast<float>(triangleStep[index]) / 15.0f) - 0.5f;
    }

    return outputTriangle;
}
float Apu::getDutyCycle(uint8_t dutyCycle)
{
    auto dutyCycleValue = float{0.0f};

    switch (dutyCycle) {
    case 0x00:
        // 12.5% waveform
        dutyCycleValue = 0.125f;
        break;
    case 0x01:
        // 25% waveform
        dutyCycleValue = 0.250f;
        break;
    case 0x02:
        // 50% waveform
        dutyCycleValue = 0.500f;
        break;
    case 0x03:
        // 25% negated waveform
        dutyCycleValue = -0.250f;
        break;
    default:
        break;
    }

    return dutyCycleValue;
}

void Apu::doQuarterFrame()
{
    doEnvelope(_pulse1);
    doEnvelope(_pulse2);
    doLinearCounters(_triangle);
}

void Apu::doHalfFrame()
{
    doLengthCounters(_pulse1, _registers.controlFlag.pulse1Enable);
    doLengthCounters(_pulse2, _registers.controlFlag.pulse2Enable);
    doLengthCounters(_triangle, _registers.controlFlag.triangleEnable);
    doSweep(_pulse1);
    doSweep(_pulse2);
}

void Apu::doEnvelope(Pulse& pulse)
{
    if (pulse.envelopeStart) {
        pulse.envelopeStart = false;
        pulse.envelopeDecay = 15;
        pulse.envelopeCounter = pulse.Register0Flag.envelopePeriod;
    } else {
        if (pulse.envelopeCounter > 0) {
            pulse.envelopeCounter--;
        } else {
            pulse.envelopeCounter = pulse.Register0Flag.envelopePeriod;

            if (pulse.envelopeDecay > 0) {
                pulse.envelopeDecay--;
            }

            if (pulse.Register0Flag.lengthCounterHalt) {
                pulse.envelopeDecay = 15;
            }
        }
    }

    if (pulse.Register0Flag.constantEnvelopeFlag) {
        pulse.volume = pulse.Register0Flag.envelopePeriod;
    } else {
        pulse.volume = pulse.envelopeDecay;
    }
}

void Apu::doSweep(Pulse& pulse)
{
    if (pulse.Register1Flag.sweepEnableFlag) {
        if (!pulse.sweepDone && pulse.sweepCounter == 0) {
            auto periodDelta = pulse.sweepTarget >> pulse.Register1Flag.sweepShiftCount;
            if (pulse.Register1Flag.sweepNegateFlag) {
                pulse.sweepTarget -= periodDelta - 1;
            } else {
                pulse.sweepTarget += periodDelta;
            }

            pulse.period = pulse.sweepTarget;
        }
    }

    if (pulse.sweepCounter == 0) {
        pulse.sweepCounter = pulse.Register1Flag.sweepPeriod;
    } else {
        pulse.sweepCounter--;
    }
    if (pulse.sweepTarget > 0x7FF || pulse.period < 8) {
        pulse.sweepDone = true;
    } else {
        pulse.sweepDone = false;
    }
}

void Apu::doLengthCounters(Pulse& pulse, bool enable)
{
    if (pulse.lengthCounter > 0) {
        if (!enable) {
            pulse.lengthCounter = 0;
        } else if (!pulse.Register0Flag.lengthCounterHalt) {
            pulse.lengthCounter--;
        }
    }
}

void Apu::doLengthCounters(Triangle& triangle, bool enable)
{
    if (triangle.lengthCounter > 0) {
        if (!enable) {
            triangle.lengthCounter = 0;
        } else if (!triangle.Register0Flag.lengthCounterHalt) {
            triangle.lengthCounter--;
        }
    }
}

void Apu::doLinearCounters(Triangle& triangle)
{
    if (triangle.linearCounterReload) {
        triangle.linearCounter = triangle.Register0Flag.linearCounterReload;
    } else if (triangle.linearCounter > 0) {
        triangle.linearCounter--;
    }

    if (!triangle.Register0Flag.lengthCounterHalt) {
        triangle.linearCounterReload = false;
    }
}
