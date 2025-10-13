#pragma once

#include <vector>
#include <atomic>
#include <mutex>

// Forward declaration for miniaudio
typedef struct ma_device ma_device;
typedef struct ma_device_config ma_device_config;

/*
 * AudioOutput - Infrastructure layer component
 *
 * Converts pressure samples from the acoustic simulation to real-time audio output.
 * Uses miniaudio for cross-platform audio playback.
 *
 * Design principles:
 * - Single Responsibility: Only handles audio output
 * - Clean Architecture: Infrastructure layer component
 * - Thread-safe: Audio callback runs on separate thread
 */

class AudioOutput {
public:
    AudioOutput();
    ~AudioOutput();

    /*
     * Initialize audio device
     *
     * @param sampleRate Audio sample rate (e.g., 48000 Hz)
     * @return true if successful, false on error
     */
    bool initialize(int sampleRate = 48000);

    /*
     * Start audio playback
     */
    void start();

    /*
     * Stop audio playback
     */
    void stop();

    /*
     * Submit a pressure sample to the audio buffer
     *
     * @param pressure Acoustic pressure in Pascals
     * @param timeScale Simulation time scale (e.g., 0.01 for 100x slower)
     *
     * This function is called every simulation frame to feed pressure
     * samples into the audio output pipeline.
     */
    void submitPressureSample(float pressure, float timeScale);

    /*
     * Set audio volume (0.0 = silent, 1.0 = normal, > 1.0 = amplified)
     */
    void setVolume(float volume);

    /*
     * Get current volume
     */
    float getVolume() const { return volume; }

    /*
     * Mute/unmute audio
     */
    void setMuted(bool muted);

    /*
     * Check if audio is muted
     */
    bool isMuted() const { return muted; }

    /*
     * Check if audio device is initialized
     */
    bool isInitialized() const { return deviceInitialized; }

    /*
     * Get last error message
     */
    const std::string& getLastError() const { return lastError; }

private:
    /*
     * Audio callback (called by miniaudio from audio thread)
     */
    static void audioCallback(ma_device* pDevice, void* pOutput, const void* pInput, unsigned int frameCount);

    /*
     * Convert pressure (Pa) to audio sample (float -1.0 to +1.0)
     *
     * Applies volume control and prevents clipping.
     */
    float pressureToAudio(float pressure);

    ma_device* device;                  // miniaudio device
    bool deviceInitialized;             // Device initialization status

    std::vector<float> audioBuffer;     // Ring buffer for pressure samples
    size_t bufferWritePos;              // Write position in ring buffer
    size_t bufferReadPos;               // Read position in ring buffer
    std::mutex bufferMutex;             // Mutex for thread-safe access

    std::atomic<float> volume;          // Audio volume (0.0 to 1.0+)
    std::atomic<bool> muted;            // Mute flag

    int sampleRate;                     // Audio sample rate (Hz)
    float previousPressure;             // Previous pressure sample for interpolation
    float simulationFrameRate;          // Simulation update rate (Hz)

    std::string lastError;              // Last error message

    // Pressure scaling constants
    static constexpr float REFERENCE_PRESSURE = 20.0f;  // Reference pressure for normalization (Pa)
    static constexpr float MAX_AMPLITUDE = 0.95f;       // Maximum audio amplitude (prevent clipping)
};
