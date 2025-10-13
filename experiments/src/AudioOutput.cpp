#include "AudioOutput.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <cstring>

// miniaudio - single-header audio library
#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

AudioOutput::AudioOutput()
    : device(nullptr)
    , deviceInitialized(false)
    , bufferWritePos(0)
    , bufferReadPos(0)
    , volume(1.0f)
    , muted(false)
    , sampleRate(48000)
{
    // Allocate ring buffer (1 second worth of samples)
    audioBuffer.resize(sampleRate, 0.0f);
}

AudioOutput::~AudioOutput() {
    stop();

    if (device) {
        ma_device_uninit(device);
        delete device;
        device = nullptr;
    }
}

bool AudioOutput::initialize(int sampleRate) {
    this->sampleRate = sampleRate;
    audioBuffer.resize(sampleRate, 0.0f);

    // Allocate device
    device = new ma_device();

    // Configure device
    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_playback);
    deviceConfig.playback.format   = ma_format_f32;   // 32-bit float
    deviceConfig.playback.channels = 1;               // Mono
    deviceConfig.sampleRate        = sampleRate;
    deviceConfig.dataCallback      = audioCallback;
    deviceConfig.pUserData         = this;

    // Initialize device
    if (ma_device_init(NULL, &deviceConfig, device) != MA_SUCCESS) {
        lastError = "Failed to initialize audio device";
        std::cerr << "AudioOutput: " << lastError << std::endl;
        delete device;
        device = nullptr;
        return false;
    }

    deviceInitialized = true;
    std::cout << "AudioOutput: Initialized at " << sampleRate << " Hz" << std::endl;

    return true;
}

void AudioOutput::start() {
    if (!deviceInitialized || !device) {
        std::cerr << "AudioOutput: Cannot start - device not initialized" << std::endl;
        return;
    }

    if (ma_device_start(device) != MA_SUCCESS) {
        lastError = "Failed to start audio device";
        std::cerr << "AudioOutput: " << lastError << std::endl;
        return;
    }

    std::cout << "AudioOutput: Started" << std::endl;
}

void AudioOutput::stop() {
    if (!deviceInitialized || !device) {
        return;
    }

    ma_device_stop(device);
    std::cout << "AudioOutput: Stopped" << std::endl;
}

void AudioOutput::submitPressureSample(float pressure, float timeScale) {
    /*
     * Submit pressure sample to audio buffer
     *
     * The simulation runs at a variable time scale (e.g., 100x slower).
     * We need to map simulation time to audio time.
     *
     * Strategy:
     * - Each simulation frame provides one pressure sample
     * - We interpolate/repeat samples to match audio sample rate
     * - At 60 FPS simulation and 48000 Hz audio, we need 800 audio samples per frame
     */

    std::lock_guard<std::mutex> lock(bufferMutex);

    // Write sample to ring buffer
    audioBuffer[bufferWritePos] = pressure;
    bufferWritePos = (bufferWritePos + 1) % audioBuffer.size();

    // Prevent buffer overrun (write catching up to read)
    if (bufferWritePos == bufferReadPos) {
        bufferReadPos = (bufferReadPos + 1) % audioBuffer.size();
    }
}

void AudioOutput::audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount) {
    AudioOutput* audio = (AudioOutput*)pDevice->pUserData;
    float* outputBuffer = (float*)pOutput;

    std::lock_guard<std::mutex> lock(audio->bufferMutex);

    for (unsigned int i = 0; i < frameCount; i++) {
        // Read sample from ring buffer
        float pressure = audio->audioBuffer[audio->bufferReadPos];
        audio->bufferReadPos = (audio->bufferReadPos + 1) % audio->audioBuffer.size();

        // Convert pressure to audio sample
        float sample = audio->pressureToAudio(pressure);

        // Apply mute
        if (audio->muted.load()) {
            sample = 0.0f;
        }

        // Output sample
        outputBuffer[i] = sample;
    }
}

float AudioOutput::pressureToAudio(float pressure) {
    /*
     * Convert acoustic pressure (Pa) to audio sample (-1.0 to +1.0)
     *
     * Pressure scaling:
     * - Reference pressure: 20 Pa (loud hand clap)
     * - Human hearing range: ~0.00002 Pa (threshold) to ~20 Pa (loud)
     *
     * We normalize by reference pressure and apply volume control.
     */

    // Normalize pressure
    float normalized = pressure / REFERENCE_PRESSURE;

    // Apply volume
    float sample = normalized * volume.load();

    // Clamp to prevent clipping
    sample = std::max(-MAX_AMPLITUDE, std::min(MAX_AMPLITUDE, sample));

    return sample;
}

void AudioOutput::setVolume(float volume) {
    this->volume.store(std::max(0.0f, volume));
    std::cout << "AudioOutput: Volume set to " << volume << std::endl;
}

void AudioOutput::setMuted(bool muted) {
    this->muted.store(muted);
    std::cout << "AudioOutput: " << (muted ? "Muted" : "Unmuted") << std::endl;
}
