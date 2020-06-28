#pragma once

#include <queue>
#include <atomic>
#include <functional>
#include <thread>
#include <cmath>

#include <AL/al.h>
#include <AL/alc.h>

class AudioHw {
public:
    AudioHw(uint32_t sampleRate,
            uint32_t numBlocks,
            uint32_t numSamples);
    ~AudioHw();

    void setReadSampleCallback(std::function<float(float)> func);

private:
    void audioThread();

    uint32_t _sampleRate{0};
    uint32_t _numBlocks{0};
    uint32_t _numSamples{0};

    ALCdevice* _device;
    ALCcontext* _context;
    ALuint*_buffers;
    ALuint _source;
    std::queue<ALuint> _availableBuffers;

    short* _sampleMemory;
    std::atomic<bool> _isRunning;
    std::unique_ptr<std::thread> _audioThread;
    std::function<float(float)> _readSample{nullptr};
};
