#include "AudioSample.h"
#include <cmath>
#include <random>
#include <algorithm>

// ============================================================================
// AUDIO SAMPLE PRESETS IMPLEMENTATION
// ============================================================================

AudioSample AudioSamplePresets::generateKick(int sampleRate) {
    /*
     * Kick Drum Synthesis
     *
     * Physics: Real kick drums produce a rapidly decaying low-frequency tone
     * - Initial pitch: ~150 Hz, quickly drops to ~50 Hz (pitch envelope)
     * - Amplitude: Exponential decay, ~300ms duration
     * - Contains some harmonics for "click" attack
     */

    const float duration = 0.4f;  // 400ms
    const int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> data(numSamples);

    const float startFreq = 150.0f;  // Starting frequency (Hz)
    const float endFreq = 50.0f;     // Ending frequency (Hz)
    const float decayTime = 0.3f;    // Amplitude decay time

    for (int i = 0; i < numSamples; i++) {
        float t = static_cast<float>(i) / sampleRate;

        // Exponential frequency sweep (pitch envelope)
        float freqRatio = std::exp(-t * 8.0f);  // Fast decay
        float freq = endFreq + (startFreq - endFreq) * freqRatio;

        // Exponential amplitude envelope
        float amplitude = std::exp(-t / decayTime);

        // Generate tone
        float phase = 2.0f * M_PI * freq * t;
        float sample = amplitude * std::sin(phase);

        // Add small amount of click for attack (high-frequency transient)
        float click = 0.3f * std::exp(-t * 100.0f) * (std::rand() / static_cast<float>(RAND_MAX) - 0.5f);

        data[i] = std::clamp(sample + click, -1.0f, 1.0f);
    }

    return AudioSample(std::move(data), sampleRate, "Kick Drum");
}

AudioSample AudioSamplePresets::generateSnare(int sampleRate) {
    /*
     * Snare Drum Synthesis
     *
     * Physics: Snare consists of:
     * 1. Tonal component: ~200 Hz (drum head)
     * 2. Noise component: Filtered white noise (snare wires)
     * Short decay: ~150ms
     */

    const float duration = 0.2f;  // 200ms
    const int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> data(numSamples);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    const float toneFreq = 200.0f;   // Snare drum tone
    const float decayTime = 0.15f;   // Decay time

    for (int i = 0; i < numSamples; i++) {
        float t = static_cast<float>(i) / sampleRate;

        // Exponential amplitude envelope
        float envelope = std::exp(-t / decayTime);

        // Tonal component (drum head)
        float phase = 2.0f * M_PI * toneFreq * t;
        float tone = 0.3f * std::sin(phase);

        // Noise component (snare wires) - white noise
        float noise = 0.7f * dist(gen);

        // Simple bandpass filter for noise (emphasize 1-5 kHz)
        // In reality, should use proper IIR filter, but this approximation works
        float filtered = noise * envelope;

        data[i] = std::clamp(envelope * (tone + filtered), -1.0f, 1.0f);
    }

    return AudioSample(std::move(data), sampleRate, "Snare Drum");
}

AudioSample AudioSamplePresets::generateTone(float frequency, float duration, int sampleRate) {
    /*
     * Pure Sine Wave Tone
     *
     * @param frequency Frequency in Hz (e.g., 440 = A4, 261.63 = C4)
     * @param duration Duration in seconds
     */

    const int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> data(numSamples);

    // Apply fade in/out to avoid clicks
    const int fadeLength = std::min(static_cast<int>(0.01f * sampleRate), numSamples / 4);  // 10ms or 25% of duration

    for (int i = 0; i < numSamples; i++) {
        float t = static_cast<float>(i) / sampleRate;

        // Sine wave
        float phase = 2.0f * M_PI * frequency * t;
        float sample = std::sin(phase);

        // Fade envelope (prevent clicks)
        float envelope = 1.0f;
        if (i < fadeLength) {
            envelope = static_cast<float>(i) / fadeLength;  // Fade in
        } else if (i > numSamples - fadeLength) {
            envelope = static_cast<float>(numSamples - i) / fadeLength;  // Fade out
        }

        data[i] = sample * envelope;
    }

    char name[64];
    snprintf(name, sizeof(name), "Tone %.1f Hz", frequency);
    return AudioSample(std::move(data), sampleRate, name);
}

AudioSample AudioSamplePresets::generateImpulse(float duration, int sampleRate) {
    /*
     * Impulse (Dirac-like) for Room Impulse Response
     *
     * Used for measuring acoustic properties of rooms.
     * A very short, sharp transient (like a hand clap or starter pistol).
     *
     * Physics: Impulse contains all frequencies (white spectrum)
     */

    const int numSamples = static_cast<int>(duration * sampleRate);
    std::vector<float> data(numSamples);

    // Create a Gaussian-windowed impulse (smoother than pure spike)
    const float center = numSamples / 2.0f;
    const float width = numSamples / 8.0f;  // Narrow Gaussian

    for (int i = 0; i < numSamples; i++) {
        float t = (i - center) / width;
        data[i] = std::exp(-t * t);  // Gaussian shape
    }

    // Normalize to peak amplitude 1.0
    float maxVal = *std::max_element(data.begin(), data.end());
    if (maxVal > 0.0f) {
        for (float& sample : data) {
            sample /= maxVal;
        }
    }

    return AudioSample(std::move(data), sampleRate, "Impulse");
}
