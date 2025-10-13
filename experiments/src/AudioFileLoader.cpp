#define MINIAUDIO_IMPLEMENTATION
#include "../external/miniaudio.h"
#include "AudioFileLoader.h"
#include <iostream>
#include <cstring>

// Static member initialization
std::string AudioFileLoader::lastError = "";

std::shared_ptr<AudioSample> AudioFileLoader::loadFile(const std::string& filename, int targetSampleRate) {
    /*
     * Audio File Loading with miniaudio
     *
     * Process:
     * 1. Decode audio file (any format supported by miniaudio)
     * 2. Convert to mono (average channels if stereo)
     * 3. Resample to target sample rate if needed
     * 4. Normalize to float [-1, 1]
     * 5. Create AudioSample value object
     */

    lastError = "";

    // Configure decoder
    ma_decoder_config config = ma_decoder_config_init(ma_format_f32, 1, targetSampleRate);

    ma_decoder decoder;
    ma_result result = ma_decoder_init_file(filename.c_str(), &config, &decoder);

    if (result != MA_SUCCESS) {
        lastError = "Failed to open audio file: " + filename;
        std::cerr << "AudioFileLoader: " << lastError << std::endl;
        return nullptr;
    }

    // Get audio length
    ma_uint64 totalFrames;
    result = ma_decoder_get_length_in_pcm_frames(&decoder, &totalFrames);

    if (result != MA_SUCCESS) {
        // Some formats don't support length queries, decode everything
        totalFrames = 0;  // Will grow dynamically
    }

    // Read audio data
    std::vector<float> audioData;

    if (totalFrames > 0) {
        // Known length: pre-allocate
        audioData.resize(static_cast<size_t>(totalFrames));

        ma_uint64 framesRead = 0;
        ma_decoder_read_pcm_frames(&decoder, audioData.data(), totalFrames, &framesRead);

        if (framesRead < totalFrames) {
            audioData.resize(static_cast<size_t>(framesRead));
        }
    } else {
        // Unknown length: decode in chunks
        const size_t chunkSize = 4096;
        std::vector<float> chunk(chunkSize);

        while (true) {
            ma_uint64 framesRead = 0;
            ma_decoder_read_pcm_frames(&decoder, chunk.data(), chunkSize, &framesRead);

            if (framesRead == 0) {
                break;  // End of file
            }

            audioData.insert(audioData.end(), chunk.begin(), chunk.begin() + framesRead);
        }
    }

    ma_decoder_uninit(&decoder);

    // Check if we got any data
    if (audioData.empty()) {
        lastError = "Audio file is empty or could not be decoded: " + filename;
        std::cerr << "AudioFileLoader: " << lastError << std::endl;
        return nullptr;
    }

    // Extract filename (without path) for sample name
    size_t lastSlash = filename.find_last_of("/\\");
    std::string sampleName = (lastSlash != std::string::npos)
        ? filename.substr(lastSlash + 1)
        : filename;

    // Remove extension
    size_t lastDot = sampleName.find_last_of('.');
    if (lastDot != std::string::npos) {
        sampleName = sampleName.substr(0, lastDot);
    }

    std::cout << "AudioFileLoader: Loaded \"" << sampleName << "\" - "
              << audioData.size() << " samples at " << targetSampleRate << " Hz ("
              << (static_cast<float>(audioData.size()) / targetSampleRate) << " seconds)"
              << std::endl;

    // Create AudioSample (domain object)
    try {
        return std::make_shared<AudioSample>(std::move(audioData), targetSampleRate, sampleName);
    } catch (const std::exception& e) {
        lastError = "Failed to create AudioSample: " + std::string(e.what());
        std::cerr << "AudioFileLoader: " << lastError << std::endl;
        return nullptr;
    }
}

std::string AudioFileLoader::getLastError() {
    return lastError;
}
