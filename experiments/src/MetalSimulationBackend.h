#pragma once

#include <vector>
#include <string>

/*
 * MetalSimulationBackend - GPU Acceleration via Metal
 *
 * Leverages Apple's Metal framework for massive parallel processing
 * on M1/M2/M3 GPUs (30-40 GPU cores on M3 Max!)
 *
 * Architecture:
 * - Opaque implementation (Pimpl pattern)
 * - CPU fallback if Metal unavailable
 * - Zero-copy unified memory on Apple Silicon
 *
 * Performance Expected:
 * - M3 Max: 20-50x speedup over single-threaded CPU
 * - 80,000 threads running simultaneously (one per grid cell)
 */

class MetalSimulationBackend {
public:
    MetalSimulationBackend();
    ~MetalSimulationBackend();

    // Prevent copying (Metal resources are not copyable)
    MetalSimulationBackend(const MetalSimulationBackend&) = delete;
    MetalSimulationBackend& operator=(const MetalSimulationBackend&) = delete;

    /*
     * Initialize Metal backend
     *
     * @param width Grid width
     * @param height Grid height
     * @return true if initialization successful, false otherwise
     */
    bool initialize(int width, int height);

    /*
     * Check if Metal is available and initialized
     */
    bool isAvailable() const;

    /*
     * Execute one wave equation time step on GPU
     *
     * @param pressure Current pressure field (device or host memory)
     * @param pressurePrev Previous pressure field
     * @param pressureNext Output pressure field
     * @param obstacles Obstacle mask
     * @param c2_dt2_dx2 CFL coefficient
     * @param damping Air absorption coefficient
     * @param wallReflection Wall reflection coefficient
     */
    void executeStep(
        const std::vector<float>& pressure,
        const std::vector<float>& pressurePrev,
        std::vector<float>& pressureNext,
        const std::vector<uint8_t>& obstacles,
        float c2_dt2_dx2,
        float damping,
        float wallReflection
    );

    /*
     * Audio source data for GPU injection
     */
    struct AudioSourceData {
        int x;
        int y;
        float pressure;
    };

    /*
     * Execute multiple wave equation time steps on GPU (OPTIMIZED)
     *
     * KEY OPTIMIZATION: Data stays on GPU for entire frame!
     * - Copy initial state to GPU: ~1.84 MB
     * - Execute all sub-steps on GPU (no CPU round-trip)
     * - Copy final state + listener samples back: ~1.84 MB
     *
     * Performance: 382x less memory bandwidth than per-step execution
     * - Before: ~703 MB per frame (191 copies × 3.68 MB)
     * - After: ~3.68 MB per frame (2 copies × 1.84 MB)
     *
     * NEW: Supports continuous audio injection on GPU!
     * - Audio sources are pre-sampled on CPU for all sub-steps
     * - GPU injects audio at each sub-step for continuous sound
     *
     * @param initialPressure Initial current pressure field
     * @param initialPressurePrev Initial previous pressure field
     * @param finalPressure Output: final current pressure field
     * @param finalPressurePrev Output: final previous pressure field
     * @param obstacles Obstacle mask
     * @param listenerSamples Output: pressure samples at listener position
     * @param audioSourcesPerStep Audio source data for each sub-step (numSubSteps × numSources)
     * @param listenerX Listener X coordinate (-1 if disabled)
     * @param listenerY Listener Y coordinate
     * @param numSubSteps Number of sub-steps to execute
     * @param c2_dt2_dx2 CFL coefficient
     * @param damping Air absorption coefficient
     * @param wallReflection Wall reflection coefficient
     */
    void executeFrame(
        const std::vector<float>& initialPressure,
        const std::vector<float>& initialPressurePrev,
        std::vector<float>& finalPressure,
        std::vector<float>& finalPressurePrev,
        const std::vector<uint8_t>& obstacles,
        std::vector<float>& listenerSamples,
        const std::vector<std::vector<AudioSourceData>>& audioSourcesPerStep,
        int listenerX,
        int listenerY,
        int numSubSteps,
        float c2_dt2_dx2,
        float damping,
        float wallReflection
    );

    /*
     * Get last error message
     */
    const std::string& getLastError() const;

    /*
     * Get performance statistics
     */
    struct PerformanceStats {
        double lastStepTimeMs;      // Last step execution time (ms)
        double avgStepTimeMs;        // Average step time (ms)
        uint64_t totalSteps;         // Total steps executed
        double totalTimeMs;          // Total execution time (ms)
    };

    PerformanceStats getPerformanceStats() const;

    /*
     * Reset performance statistics
     */
    void resetPerformanceStats();

private:
    // Pimpl pattern - hide Metal implementation details
    class Impl;
    Impl* pImpl;

    std::string lastError;
};
