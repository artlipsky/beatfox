#pragma once

#include "AudioSample.h"
#include <memory>
#include <cmath>

/*
 * AudioSource - Domain Entity
 *
 * Represents a sound source positioned in the simulation space.
 * Entity properties:
 * - Has identity (each source is unique, even with same audio)
 * - Mutable state (position, volume, playback position)
 * - Contains a value object (AudioSample)
 *
 * Clean Architecture: Domain layer
 */
class AudioSource {
public:
    /*
     * Construct audio source
     *
     * @param sample Audio sample to play
     * @param x X position in grid coordinates
     * @param y Y position in grid coordinates
     * @param volumeDb Volume in decibels (0 dB = reference level, -∞ dB = silence)
     * @param loop Whether to loop the sample
     */
    AudioSource(std::shared_ptr<AudioSample> sample, int x, int y, float volumeDb = 0.0f, bool loop = true)
        : sample(std::move(sample))
        , x(x)
        , y(y)
        , volumeDb(volumeDb)
        , loop(loop)
        , playing(false)
        , playbackPosition(0)
    {
        if (!this->sample) {
            throw std::invalid_argument("AudioSource: sample cannot be null");
        }
    }

    // Entity: movable but not copyable (has identity)
    AudioSource(const AudioSource&) = delete;
    AudioSource(AudioSource&&) = default;
    AudioSource& operator=(const AudioSource&) = delete;
    AudioSource& operator=(AudioSource&&) = default;

    // ========================================================================
    // POSITION
    // ========================================================================

    int getX() const { return x; }
    int getY() const { return y; }

    void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    // ========================================================================
    // VOLUME (Decibel Scale)
    // ========================================================================

    /*
     * Get volume in decibels
     *
     * 0 dB = reference level (100 Pa RMS = 134 dB SPL)
     * -6 dB = half amplitude
     * -∞ dB = silence
     */
    float getVolumeDb() const { return volumeDb; }

    void setVolumeDb(float db) {
        volumeDb = db;
    }

    /*
     * Get linear amplitude multiplier from dB
     *
     * Formula: amplitude = 10^(dB/20)
     * Examples:
     *   0 dB -> 1.0x
     *  -6 dB -> 0.5x
     * -12 dB -> 0.25x
     * -20 dB -> 0.1x
     */
    float getAmplitude() const {
        return std::pow(10.0f, volumeDb / 20.0f);
    }

    /*
     * Convert amplitude to dB
     *
     * Formula: dB = 20 * log10(amplitude)
     */
    static float amplitudeToDb(float amplitude) {
        if (amplitude <= 0.0f) {
            return -100.0f;  // Effectively -∞
        }
        return 20.0f * std::log10(amplitude);
    }

    // ========================================================================
    // PLAYBACK CONTROL
    // ========================================================================

    bool isPlaying() const { return playing; }
    bool isLooping() const { return loop; }

    void play() {
        playing = true;
        playbackPosition = 0;
    }

    void stop() {
        playing = false;
        playbackPosition = 0;
    }

    void pause() {
        playing = false;
    }

    void resume() {
        playing = true;
    }

    void setLoop(bool shouldLoop) {
        loop = shouldLoop;
    }

    // ========================================================================
    // AUDIO SAMPLING
    // ========================================================================

    /*
     * Get current audio sample value
     *
     * Returns the current sample value scaled by volume, then advances playback position.
     * If at end of sample:
     * - If looping: wrap to beginning
     * - If not looping: stop playback and return 0
     *
     * @return Pressure value in Pascals to add to simulation
     */
    float getCurrentSample() {
        if (!playing || !sample) {
            return 0.0f;
        }

        // Get sample at current position
        float sampleValue = sample->getSample(playbackPosition);

        // Apply volume (convert dB to linear amplitude)
        float amplitude = getAmplitude();
        float pressure = sampleValue * amplitude;

        // Advance playback position
        playbackPosition++;

        // Check if reached end of sample
        if (playbackPosition >= sample->getLength()) {
            if (loop) {
                playbackPosition = 0;  // Wrap to beginning
            } else {
                playing = false;  // Stop playback
                return 0.0f;
            }
        }

        // Scale to acoustic pressure
        // Assuming sample values [-1, 1] map to ±100 Pa (loud sound)
        // This is adjustable based on desired intensity
        const float referencePressure = 100.0f;  // Pa (about 134 dB SPL)
        return pressure * referencePressure;
    }

    /*
     * Get audio sample (const reference)
     */
    const AudioSample& getSample() const { return *sample; }

    /*
     * Get playback info
     */
    size_t getPlaybackPosition() const { return playbackPosition; }
    float getPlaybackProgress() const {
        if (!sample || sample->getLength() == 0) {
            return 0.0f;
        }
        return static_cast<float>(playbackPosition) / sample->getLength();
    }

private:
    std::shared_ptr<AudioSample> sample;  // Audio sample to play
    int x, y;                             // Position in grid coordinates
    float volumeDb;                       // Volume in decibels
    bool loop;                            // Whether to loop
    bool playing;                         // Playback state
    size_t playbackPosition;              // Current playback position (sample index)
};
