#pragma once

#include <vector>
#include <cmath>
#include <string>
#include <memory>
#include "DampingPreset.h"
#include "AudioSource.h"

class WaveSimulation {
public:
    WaveSimulation(int width, int height);
    ~WaveSimulation();

    void update(float dt);
    void addPressureSource(int x, int y, float pressure);
    void clear();

    // Getters
    const float* getData() const { return pressure.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }

    // Parameters
    void setWaveSpeed(float speed) { soundSpeed = speed; }
    void setDamping(float damp) { damping = damp; }
    float getWaveSpeed() const { return soundSpeed; }
    float getDamping() const { return damping; }

    /*
     * Damping Presets (Domain-driven design)
     *
     * Apply acoustic environment presets following domain logic.
     * This allows users to switch between different acoustic scenarios
     * without understanding the underlying physics parameters.
     */
    void applyDampingPreset(const DampingPreset& preset);
    DampingPreset getCurrentPreset() const { return currentPreset; }
    float getWallReflection() const { return wallReflection; }
    void setWallReflection(float reflection) { wallReflection = reflection; }

    // Physical dimensions
    float getPhysicalWidth() const { return width * dx; }  // meters
    float getPhysicalHeight() const { return height * dx; } // meters

    // Obstacles
    void addObstacle(int x, int y, int radius);
    void removeObstacle(int x, int y, int radius);
    void clearObstacles();
    bool isObstacle(int x, int y) const;
    const uint8_t* getObstacles() const { return obstacles.data(); }

    /*
     * Load obstacles from SVG file
     *
     * @param filename Path to SVG file
     * @return true if successful, false on error
     *
     * The SVG is rasterized to match the simulation grid dimensions.
     * Black/dark shapes become obstacles, white/transparent areas become empty space.
     * Existing obstacles are cleared before loading.
     */
    bool loadObstaclesFromSVG(const std::string& filename);

    // Listener (virtual microphone for audio output)
    void setListenerPosition(int x, int y);
    void getListenerPosition(int& x, int& y) const;
    bool hasListener() const { return listenerEnabled; }
    void setListenerEnabled(bool enabled);

    /*
     * Get pressure at listener position
     *
     * @return Acoustic pressure in Pascals at listener location
     */
    float getListenerPressure() const;

    /*
     * Get all listener samples collected during last update
     *
     * Returns all pressure samples collected at the listener position
     * during sub-stepping (typically ~191 samples per frame).
     * The buffer is cleared after retrieval.
     *
     * @return Vector of pressure samples in Pascals
     */
    std::vector<float> getListenerSamples();

    // ========================================================================
    // AUDIO SOURCES (Continuous sound sources)
    // ========================================================================

    /*
     * Add audio source to simulation
     *
     * @param source Audio source to add (ownership transferred)
     * @return ID of added source (index in sources list)
     */
    size_t addAudioSource(std::unique_ptr<AudioSource> source);

    /*
     * Remove audio source by ID
     *
     * @param sourceId ID returned by addAudioSource
     */
    void removeAudioSource(size_t sourceId);

    /*
     * Get audio source by ID
     *
     * @return Pointer to source, or nullptr if invalid ID
     */
    AudioSource* getAudioSource(size_t sourceId);

    /*
     * Get all audio sources (const)
     */
    const std::vector<std::unique_ptr<AudioSource>>& getAudioSources() const {
        return audioSources;
    }

    /*
     * Clear all audio sources
     */
    void clearAudioSources();

private:
    int width;              // Grid width (pixels)
    int height;             // Grid height (pixels)
    float soundSpeed;       // Speed of sound in air (m/s)
    float damping;          // Air absorption coefficient (dimensionless)
    float wallReflection;   // Wall reflection coefficient (0-1, energy loss at walls)
    float dx;               // Spatial step: 1 pixel = 1 cm = 0.01 m
    DampingPreset currentPreset;  // Current acoustic environment preset

    // Acoustic pressure field (deviation from ambient pressure)
    std::vector<float> pressure;      // Current pressure field (Pa)
    std::vector<float> pressurePrev;  // Previous pressure field (Pa)
    std::vector<float> pressureNext;  // Next pressure field (Pa)

    // Obstacle field (solid objects that block sound)
    std::vector<uint8_t> obstacles;   // 1 if cell contains an obstacle, 0 otherwise

    // Listener (virtual microphone)
    int listenerX;              // Listener x position (grid coordinates)
    int listenerY;              // Listener y position (grid coordinates)
    bool listenerEnabled;       // Listener enabled flag
    std::vector<float> listenerSampleBuffer;  // Buffer for sub-step samples

    // Audio sources (continuous sound playback)
    std::vector<std::unique_ptr<AudioSource>> audioSources;

    void updateStep(float dt);  // Single time step

    int index(int x, int y) const {
        return y * width + x;
    }
};
