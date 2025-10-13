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
