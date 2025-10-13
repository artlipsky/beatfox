#pragma once

#include "AudioSample.h"
#include <string>
#include <memory>

/*
 * AudioFileLoader - Infrastructure Layer
 *
 * Loads audio files (MP3, WAV, FLAC) using miniaudio library.
 * Infrastructure concerns: file I/O, decoding, format conversion
 *
 * Clean Architecture: Infrastructure layer (depends on external library)
 * Domain layer (AudioSample) has no knowledge of file formats or miniaudio
 */
class AudioFileLoader {
public:
    /*
     * Load audio file and convert to AudioSample
     *
     * Supported formats: WAV, MP3, FLAC (via miniaudio)
     * Automatically resamples to target sample rate
     * Converts stereo to mono by averaging channels
     *
     * @param filename Path to audio file
     * @param targetSampleRate Target sample rate (default: 48000 Hz)
     * @return AudioSample on success, nullptr on failure
     *
     * Thread-safe: Each call creates independent decoder
     */
    static std::shared_ptr<AudioSample> loadFile(const std::string& filename, int targetSampleRate = 48000);

    /*
     * Get last error message
     *
     * Returns empty string if no error
     */
    static std::string getLastError();

private:
    static std::string lastError;
};
