#include "AudioHw.hpp"

constexpr auto sampleMaxResolution = 32760.0f;

AudioHw::AudioHw(unsigned int sampleRate,
        unsigned int numBlocks,
        unsigned int numSamples)
: _sampleRate{sampleRate}
, _numBlocks{numBlocks}
, _numSamples{numSamples}
{
    // Open Audio device
    const char* name = alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    _device = alcOpenDevice(name);
    _context = alcCreateContext(_device, NULL);
    alcMakeContextCurrent(_context);

    // Setup buffers and source
    _buffers = new ALuint[_numBlocks];
    alGenBuffers(_numBlocks, _buffers);
    alGenSources(1, &_source);

    for (auto i = 0u; i < _numBlocks; i++) {
        _availableBuffers.push(_buffers[i]);
    }

    // Allocate Memory
    _sampleMemory = new short[_numSamples];
    std::fill(_sampleMemory, _sampleMemory + _numSamples, 0);

    _isRunning = true;
    _audioThread = std::make_unique<std::thread>([this] { audioThread(); });
}

AudioHw::~AudioHw()
{
    _isRunning = false;
    _audioThread->join();

    alDeleteBuffers(_numBlocks, _buffers);
    delete[] _buffers;
    alDeleteSources(1, &_source);

    alcMakeContextCurrent(NULL);
    alcDestroyContext(_context);
    alcCloseDevice(_device);
}

void AudioHw::setReadSampleCallback(std::function<float(float)> callback)
{
    _readSample = callback;
}

void AudioHw::audioThread()
{
    auto currentTime = 0.0f;
    auto timeStep = 1.0f / static_cast<float>(_sampleRate);
    std::vector<ALuint> processedBuffers;

    while (_isRunning)
    {
        ALint currentState, numProcessed;
        alGetSourcei(_source, AL_SOURCE_STATE, &currentState);
        alGetSourcei(_source, AL_BUFFERS_PROCESSED, &numProcessed);

        // Add processed buffers to our queue
        processedBuffers.resize(numProcessed);
        alSourceUnqueueBuffers(_source, numProcessed, processedBuffers.data());
        for (ALint buffer : processedBuffers) {
            _availableBuffers.push(buffer);
        }

        // Make sure we have available buffers
        if (_availableBuffers.empty()) {
            continue;
        }

        // Read Audio samples
        auto sample = short{0};
        for (auto index = 0u; index < _numSamples; index++)
        {
            auto sampleFloat = _readSample(currentTime);
            if (sampleFloat >= 0.0) {
                sample = static_cast<short>(fmin(sampleFloat, 1.0) * sampleMaxResolution);
            } else {
                sample = static_cast<short>(fmax(sampleFloat, -1.0) * sampleMaxResolution);
            }
            _sampleMemory[index] = sample;

            currentTime += timeStep;
        }

        // Upload buffer to OpenAL
        alBufferData(_availableBuffers.front(), AL_FORMAT_MONO16, _sampleMemory, 2 * _numSamples, _sampleRate);

        // Queue buffer to OpenAL buffer
        alSourceQueueBuffers(_source, 1, &_availableBuffers.front());
        _availableBuffers.pop();

        // Play source when we're still not playing
        if (currentState != AL_PLAYING) {
            alSourcePlay(_source);
        }
    }
}
