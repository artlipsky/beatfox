#pragma once

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

/*
 * AudioSample - Domain Value Object
 *
 * Represents an immutable audio sample with PCM data.
 * Value object properties:
 * - Immutable after construction
 * - Equality by value (same data = same sample)
 * - No identity (samples with same data are interchangeable)
 *
 * Clean Architecture: Domain layer (no infrastructure dependencies)
 */
class AudioSample {
public:
    /*
     * Construct audio sample from PCM data
     *
     * @param data PCM audio data (mono, float, normalized to [-1, 1])
     * @param sampleRate Sample rate in Hz (e.g., 48000, 44100)
     * @param name Human-readable name for the sample
     *
     * Throws std::invalid_argument if data is empty or sampleRate invalid
     */
    AudioSample(std::vector<float> data, int sampleRate, std::string name = "")
        : data(std::move(data))
        , sampleRate(sampleRate)
        , name(std::move(name))
    {
        // Domain invariants
        if (this->data.empty()) {
            throw std::invalid_argument("AudioSample: data cannot be empty");
        }
        if (sampleRate <= 0) {
            throw std::invalid_argument("AudioSample: sample rate must be positive");
        }
    }

    // Value object: copyable and movable
    AudioSample(const AudioSample&) = default;
    AudioSample(AudioSample&&) = default;
    AudioSample& operator=(const AudioSample&) = default;
    AudioSample& operator=(AudioSample&&) = default;

    // Getters (immutable interface)
    const std::vector<float>& getData() const { return data; }
    int getSampleRate() const { return sampleRate; }
    const std::string& getName() const { return name; }

    // Derived properties
    size_t getLength() const { return data.size(); }
    float getDuration() const { return static_cast<float>(data.size()) / sampleRate; }

    // Get single sample at index (clamped to valid range)
    float getSample(size_t index) const {
        if (index >= data.size()) {
            return 0.0f;  // Past end of sample
        }
        return data[index];
    }

    // Value object: equality by value
    bool operator==(const AudioSample& other) const {
        return sampleRate == other.sampleRate && data == other.data;
    }

    bool operator!=(const AudioSample& other) const {
        return !(*this == other);
    }

private:
    std::vector<float> data;  // PCM audio data (mono, float, [-1, 1])
    int sampleRate;           // Sample rate in Hz
    std::string name;         // Human-readable name
};

/*
 * AudioSample Presets - Domain Factory
 *
 * Factory methods for creating common audio sample types.
 * This follows the Factory pattern for value object creation.
 */
class AudioSamplePresets {
public:
    /*
     * Generate a kick drum sound (synthetic)
     *
     * Physics: Low-frequency (50-60 Hz) sine wave with exponential decay
     * Typical kick drum has fast attack, exponential decay over ~300ms
     */
    static AudioSample generateKick(int sampleRate = 48000);

    /*
     * Generate a snare drum sound (synthetic)
     *
     * Physics: Filtered white noise + transient tone (200 Hz)
     * Snare has sharp attack, decays over ~150ms
     */
    static AudioSample generateSnare(int sampleRate = 48000);

    /*
     * Generate a simple sine wave tone
     *
     * @param frequency Frequency in Hz (e.g., 440 for A4)
     * @param duration Duration in seconds
     */
    static AudioSample generateTone(float frequency, float duration, int sampleRate = 48000);

    /*
     * Generate an impulse (single spike) for testing room acoustics
     *
     * Useful for measuring room impulse response (RIR)
     * Duration: typically 1-10 ms
     */
    static AudioSample generateImpulse(float duration = 0.005f, int sampleRate = 48000);
};
