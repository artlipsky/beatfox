#include "WaveSimulation.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>

#include "SVGLoader.h"

WaveSimulation::WaveSimulation(int width, int height)
    : width(width),
      height(height),
      soundSpeed(343.0f)  // Speed of sound in air at 20°C (m/s)
      ,
      damping(0.997f)  // Air absorption (waves fade over time)
      ,
      wallReflection(0.85f)  // Wall reflection coefficient (15% energy loss per reflection)
      ,
      dx(0.0086f)  // Spatial grid spacing: 1 pixel = 8.6 mm = 0.0086 m (FULL AUDIO)
      ,
      currentPreset(DampingPreset::fromType(
          DampingPreset::Type::REALISTIC))  // Initialize with realistic preset
      ,
      listenerX(width / 2)  // Default listener at center
      ,
      listenerY(height / 2),
      listenerEnabled(false),
      useGPU(false)  // GPU disabled by default
{
    int size = width * height;

    // Initialize pressure fields to zero (ambient atmospheric pressure)
    pressure.resize(size, 0.0f);
    pressurePrev.resize(size, 0.0f);
    pressureNext.resize(size, 0.0f);

    // Initialize obstacle field (no obstacles initially)
    obstacles.resize(size, false);

    // Initialize Metal GPU backend (will fail gracefully if Metal unavailable)
    if (metalBackend.initialize(width, height)) {
        std::cout << "WaveSimulation: Metal GPU backend initialized successfully" << std::endl;
        std::cout << "WaveSimulation: GPU acceleration ENABLED by default" << std::endl;
        useGPU = true;  // Enable GPU automatically when available!
    } else {
        std::cout << "WaveSimulation: Metal GPU backend unavailable: " << metalBackend.getLastError() << std::endl;
        std::cout << "WaveSimulation: Using CPU-only mode" << std::endl;
        useGPU = false;
    }

    /*
     * PHYSICAL UNITS AND SCALE (SMALL ROOM + FULL AUDIO RESOLUTION):
     * -----------------------------------------------
     * Coordinate system: 1 pixel = 8.6 mm = 0.86 cm = 0.0086 m
     *
     * For 581x291 grid (W x H):
     * - Physical room size: 5m x 2.5m (width x height)
     * - Aspect ratio: 2:1 (rectangular room)
     * - Grid cells: 169,071 (~8.5× more than 20,000 baseline)
     * - Max frequency: f_max = c/(2*dx) = 343/0.0172 = 19.94 kHz (full human hearing!)
     * - Memory: ~1.9 MB for 3 pressure fields
     *
     * Physical constants (air at 20°C, 1 atm):
     * - Speed of sound: c = 343 m/s
     * - Air density: ρ = 1.204 kg/m³
     * - Atmospheric pressure: P₀ = 101325 Pa
     *
     * The pressure field represents acoustic pressure p (Pa),
     * which is the deviation from atmospheric pressure P₀.
     *
     * FULL AUDIO RESOLUTION BENEFITS:
     * - Covers entire human hearing range (20 Hz - 20 kHz)
     * - Perfect for music, voice, all acoustic content
     * - Good balance between quality and performance
     * - Engineering-grade spatial detail (8.6mm)
     * - Enables clear visualization of wave patterns
     *
     * PERFORMANCE CHARACTERISTICS:
     * - Moderate computation: 169K cells × ~940 sub-steps/frame
     * - Requires ~159 million cell updates per frame at 60 FPS
     * - Expected performance: 10-20 FPS on M3 GPU at 1× speed
     * - Use 0.1× timescale for smooth 60 FPS, or accept ~15 FPS
     * - Much more practical than 5mm while maintaining full audio range
     */
}

WaveSimulation::~WaveSimulation() {}

void WaveSimulation::update(float dt_frame) {
    /*
     * 2D Acoustic Wave Equation with Real Physical Units
     *
     * Physical model: Linear acoustics
     * ∂²p/∂t² = c² ∇²p
     *
     * Numerical stability (CFL condition):
     * c * dt / dx < 1/√2 ≈ 0.707 (in 2D)
     *
     * With c = 343 m/s, dx = 0.0086 m (FULL AUDIO):
     * dt_max = 0.707 * 0.0086 / 343 ≈ 1.77e-5 s ≈ 17.7 μs
     *
     * At 60 FPS (dt_frame ≈ 0.0167 s), we need ~940 sub-steps
     */

    // Clear listener sample buffer at start of frame
    listenerSampleBuffer.clear();

    // Calculate maximum stable time step (CFL condition)
    const float CFL_SAFETY = 0.6f;  // Safety factor (< 0.707)
    float dt_max = CFL_SAFETY * dx / soundSpeed;

    // Calculate number of sub-steps needed
    int numSteps = static_cast<int>(std::ceil(dt_frame / dt_max));
    float dt = dt_frame / numSteps;

    // Grow active region based on wave propagation
    // This expands the region where we need to compute wave updates
    growActiveRegionForFrame(dt_frame);

    // ========================================================================
    // OPTIMIZED GPU PATH: Execute all sub-steps on GPU without CPU round-trip
    // ========================================================================
    if (useGPU && metalBackend.isAvailable()) {
        // CRITICAL OPTIMIZATION:
        // Instead of copying CPU↔GPU for each sub-step (~477 times per frame),
        // we keep data on GPU for the entire frame and copy only twice:
        // 1. Initial state → GPU
        // 2. Final state + listener samples ← GPU
        //
        // Performance: 382× reduction in memory bandwidth!
        // NEW: Supports continuous audio injection on GPU!

        // Pre-sample audio sources for ALL sub-steps (on CPU)
        // This is done once per frame, then passed to GPU for injection at each sub-step
        std::vector<std::vector<MetalSimulationBackend::AudioSourceData>> audioSourcesPerStep(numSteps);

        for (int step = 0; step < numSteps; step++) {
            for (auto& source : audioSources) {
                if (source && source->isPlaying()) {
                    float pressureValue = source->getCurrentSample(dt);
                    int x = source->getX();
                    int y = source->getY();

                    if (x > 0 && x < width - 1 && y > 0 && y < height - 1) {
                        if (!obstacles[index(x, y)]) {
                            MetalSimulationBackend::AudioSourceData sourceData;
                            sourceData.x = x;
                            sourceData.y = y;
                            sourceData.pressure = pressureValue;
                            audioSourcesPerStep[step].push_back(sourceData);
                        }
                    }
                }
            }
        }

        // Execute all sub-steps on GPU with audio injection
        const float c2_dt2_dx2 = (soundSpeed * soundSpeed * dt * dt) / (dx * dx);

        std::vector<float> finalPressure;
        std::vector<float> finalPressurePrev;

        metalBackend.executeFrame(
            pressure,             // Initial current pressure
            pressurePrev,         // Initial previous pressure
            finalPressure,        // Output: final current pressure
            finalPressurePrev,    // Output: final previous pressure
            obstacles,            // Obstacle mask
            listenerSampleBuffer, // Output: listener samples (all sub-steps)
            audioSourcesPerStep,  // Audio source data for each sub-step
            listenerEnabled ? listenerX : -1,  // Listener X (-1 if disabled)
            listenerY,            // Listener Y
            numSteps,             // Number of sub-steps
            c2_dt2_dx2,           // CFL coefficient
            damping,              // Air absorption
            wallReflection,       // Wall reflection
            // ACTIVE REGION OPTIMIZATION: Only compute where waves are active!
            activeRegion.hasActivity ? activeRegion.minX : 0,
            activeRegion.hasActivity ? activeRegion.minY : 0,
            activeRegion.hasActivity ? activeRegion.maxX : width - 1,
            activeRegion.hasActivity ? activeRegion.maxY : height - 1
        );

        // Update simulation state with GPU results
        pressure = std::move(finalPressure);
        pressurePrev = std::move(finalPressurePrev);

        return;  // GPU path complete - skip CPU code
    }

    // ========================================================================
    // CPU FALLBACK PATH: Sub-step loop with CPU computation
    // ========================================================================
    // Perform sub-stepping for numerical stability
    for (int step = 0; step < numSteps; step++) {
        updateStep(dt);
    }
}

void WaveSimulation::updateStep(float dt) {
    /*
     * EULER'S EQUATIONS FOR ACOUSTICS (Уравнение Эйлера)
     *
     * We solve the linearized Euler equations for a compressible fluid (air):
     *
     * Conservation of mass:     ∂ρ/∂t + ρ₀∇·v = 0
     * Conservation of momentum: ρ₀ ∂v/∂t + ∇p = 0
     *
     * Combined with the equation of state p = c²ρ, these yield the wave equation:
     * ∂²p/∂t² = c² ∇²p
     *
     * Where:
     * - p = acoustic pressure (Pa) - deviation from atmospheric pressure
     * - ρ = density perturbation (kg/m³)
     * - v = particle velocity field (m/s)
     * - c = speed of sound (m/s)
     * - ρ₀ = ambient air density (1.204 kg/m³ at 20°C)
     *
     * Numerical method: FDTD with leapfrog time integration (2nd order accurate)
     */

    // Sample audio sources and inject into pressure field
    // This must happen BEFORE the wave propagation step
    // OPTIMIZED: Single-point injection at sub-step rate for continuous audio
    // With balanced resolution (2.5 cm/pixel), wave propagation naturally handles spreading
    for (auto& source : audioSources) {
        if (source && source->isPlaying()) {
            // Pass simulation timestep - audio speed matches simulation speed
            float pressureValue = source->getCurrentSample(dt);

            int x = source->getX();
            int y = source->getY();

            // Check bounds
            if (x > 0 && x < width - 1 && y > 0 && y < height - 1) {
                // Single-point injection: fast and effective with high resolution
                if (!obstacles[index(x, y)]) {
                    pressure[index(x, y)] += pressureValue;
                }
            }
        }
    }

    // Compute CFL coefficient: (c*dt/dx)²
    const float c2_dt2_dx2 = (soundSpeed * soundSpeed * dt * dt) / (dx * dx);
    const float twoOverDamping = 2.0f * damping;

    // ========================================================================
    // GPU ACCELERATION PATH (Metal)
    // ========================================================================
    if (useGPU && metalBackend.isAvailable()) {
        // Execute wave equation on GPU - all grid cells in parallel!
        metalBackend.executeStep(pressure, pressurePrev, pressureNext, obstacles,
                                 c2_dt2_dx2, damping, wallReflection);

        // Time step: rotate buffers
        std::swap(pressurePrev, pressure);
        std::swap(pressure, pressureNext);

        // Collect listener sample at sub-step rate
        if (listenerEnabled) {
            listenerSampleBuffer.push_back(getListenerPressure());
        }

        return;  // GPU path complete - skip CPU code
    }

    // ========================================================================
    // CPU FALLBACK PATH
    // ========================================================================

    // Interior points - solve wave equation with optimized memory access
    // Process row by row for better cache locality
    for (int y = 1; y < height - 1; y++) {
        const int rowOffset = y * width;
        const int rowAbove = (y - 1) * width;
        const int rowBelow = (y + 1) * width;

        // Cache-friendly inner loop
        for (int x = 1; x < width - 1; x++) {
            const int idx = rowOffset + x;

            // Skip obstacle cells - they act as rigid boundaries (zero pressure)
            if (obstacles[idx]) {
                pressureNext[idx] = 0.0f;
                continue;
            }

            // Load current and neighbors with minimal index calculations
            const float p_c = pressure[idx];
            const float p_xp = pressure[idx + 1];
            const float p_xm = pressure[idx - 1];
            const float p_yp = pressure[rowBelow + x];
            const float p_ym = pressure[rowAbove + x];

            // Compute Laplacian (5-point stencil)
            const float laplacian = (p_xp + p_xm + p_yp + p_ym - 4.0f * p_c);

            // Leapfrog integration with combined damping
            // p^(n+1) = 2*damping*p^n - damping*p^(n-1) + damping*c²*dt²/dx² * ∇²p^n
            pressureNext[idx] = twoOverDamping * p_c - damping * pressurePrev[idx] +
                                damping * c2_dt2_dx2 * laplacian;
        }
    }

    // Boundary conditions: REFLECTIVE or ABSORBING walls
    //
    // Two types:
    // 1. Reflective (wallReflection > 0.1):
    //    Neumann condition (∂p/∂n = 0) with attenuation
    //    Waves reflect back with reduced energy
    //
    // 2. Absorbing (wallReflection ≈ 0):
    //    Zero boundaries - waves absorbed without propagation
    //    Simulates anechoic chamber (no reflections)

    const int lastRow = (height - 1) * width;
    const int lastCol = width - 1;

    const bool absorbingWalls = (wallReflection < 0.1f);

    if (absorbingWalls) {
        // ONE-WAY WAVE EQUATION BOUNDARY (Engquist-Majda ABC)
        // Only allows outward-traveling waves: ∂p/∂t + c*∂p/∂n = 0
        // Discretized: p^(n+1) = p^n - (c*dt/dx)*(p^n - p_interior^n)

        const float cfl = soundSpeed * dt / dx;        // CFL number
        const float absorption = std::min(1.0f, cfl);  // Clamp for stability

        for (int x = 1; x < width - 1; x++) {
            // Top boundary (y=0): wave traveling in +y direction
            int idx = x;
            pressureNext[idx] =
                pressure[idx] - absorption * (pressure[idx] - pressure[idx + width]);

            // Bottom boundary (y=height-1): wave traveling in -y direction
            idx = lastRow + x;
            pressureNext[idx] =
                pressure[idx] - absorption * (pressure[idx] - pressure[idx - width]);
        }

        for (int y = 1; y < height - 1; y++) {
            const int rowOffset = y * width;

            // Left boundary (x=0): wave traveling in +x direction
            pressureNext[rowOffset] =
                pressure[rowOffset] - absorption * (pressure[rowOffset] - pressure[rowOffset + 1]);

            // Right boundary (x=width-1): wave traveling in -x direction
            int idx = rowOffset + lastCol;
            pressureNext[idx] = pressure[idx] - absorption * (pressure[idx] - pressure[idx - 1]);
        }

        // Corners: zero (simplest stable choice)
        pressureNext[0] = 0.0f;
        pressureNext[lastCol] = 0.0f;
        pressureNext[lastRow] = 0.0f;
        pressureNext[lastRow + lastCol] = 0.0f;
    } else {
        // REFLECTIVE BOUNDARY: Neumann condition with attenuation
        // Waves reflect back with energy loss based on wallReflection coefficient

        for (int x = 0; x < width; x++) {
            pressureNext[x] = pressureNext[width + x] * wallReflection;  // Top
            pressureNext[lastRow + x] =
                pressureNext[lastRow - width + x] * wallReflection;  // Bottom
        }

        for (int y = 0; y < height; y++) {
            const int rowOffset = y * width;
            pressureNext[rowOffset] = pressureNext[rowOffset + 1] * wallReflection;  // Left
            pressureNext[rowOffset + lastCol] =
                pressureNext[rowOffset + lastCol - 1] * wallReflection;  // Right
        }
    }

    // Time step: rotate buffers
    std::swap(pressurePrev, pressure);
    std::swap(pressure, pressureNext);

    // Collect listener sample at sub-step rate
    // CRITICAL FIX: Sample listener at same rate as audio injection (~11 kHz)
    // This captures all high-frequency content, not just low-frequency envelope
    if (listenerEnabled) {
        listenerSampleBuffer.push_back(getListenerPressure());
    }
}

void WaveSimulation::addPressureSource(int x, int y, float pressureAmplitude, int radius) {
    /*
     * Add a brief impulse source - like a hand clap or drum hit
     *
     * This creates a localized pressure spike that will propagate
     * outward as circular waves, reflect off walls, and interfere.
     *
     * @param radius Spatial spread in pixels (default: 2)
     *               At 8.6mm/pixel: radius pixels × 8.6mm = actual spatial spread
     *               Example: radius=2 → 17.2mm ≈ 2cm (typical hand clap)
     */

    // Validate grid coordinates
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // Expand active region to include this impulse
    // Start with radius, will grow as waves propagate
    expandActiveRegion(x, y, radius * 2);  // 2x for initial wave spread

    // Validate pressure amplitude (reasonable physical range)
    // Max 1000 Pa ≈ 134 dB SPL (threshold of pain is ~120 dB)
    if (pressureAmplitude <= 0.0f || pressureAmplitude > 1000.0f) {
        std::cerr << "WaveSimulation: Invalid pressure amplitude: " << pressureAmplitude
                  << " Pa (must be 0 < p <= 1000)" << std::endl;
        return;
    }

    // Validate radius (must be positive and reasonable for grid)
    // Max 50 pixels to prevent excessive computation or grid overflow
    if (radius < 1 || radius > 50) {
        std::cerr << "WaveSimulation: Invalid radius: " << radius
                  << " pixels (must be 1 <= r <= 50)" << std::endl;
        return;
    }

    // Gaussian width coefficient for impulse smoothness
    // Value of 1.25 ensures smooth falloff while maintaining spatial localization
    // Empirically determined for realistic impulse response (no sharp edges)
    constexpr float GAUSSIAN_WIDTH_FACTOR = 1.25f;

    // Create a compact, smooth impulse with user-defined spatial spread
    // The Gaussian width scales with the radius for consistent smoothness
    const float sigma = radius * GAUSSIAN_WIDTH_FACTOR;

    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width - 1 && py > 0 && py < height - 1) {
                // Don't add pressure to obstacle cells
                if (obstacles[index(px, py)]) {
                    continue;
                }

                float r = std::sqrt(float(dx * dx + dy * dy));

                // Gaussian profile (smooth, no sharp edges)
                float profile = std::exp(-r * r / (2.0f * sigma * sigma));

                // Add impulse to current pressure field
                // This is a brief "kick" that will propagate away
                pressure[index(px, py)] += pressureAmplitude * profile;
            }
        }
    }
}

void WaveSimulation::clear() {
    std::fill(pressure.begin(), pressure.end(), 0.0f);
    std::fill(pressurePrev.begin(), pressurePrev.end(), 0.0f);
    std::fill(pressureNext.begin(), pressureNext.end(), 0.0f);

    // Clear active region when simulation is cleared
    activeRegion.clear();
}

void WaveSimulation::addObstacle(int x, int y, int radius) {
    /*
     * Add a circular obstacle (solid object that blocks sound)
     * Obstacles act as rigid boundaries with zero pressure
     */
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width - 1 && py > 0 && py < height - 1) {
                float r = std::sqrt(float(dx * dx + dy * dy));
                if (r <= radius) {
                    obstacles[index(px, py)] = 1;
                    // Set pressure to zero at obstacle
                    pressure[index(px, py)] = 0.0f;
                    pressurePrev[index(px, py)] = 0.0f;
                    pressureNext[index(px, py)] = 0.0f;
                }
            }
        }
    }
}

void WaveSimulation::removeObstacle(int x, int y, int radius) {
    /*
     * Remove obstacles in a circular area
     */
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width - 1 && py > 0 && py < height - 1) {
                float r = std::sqrt(float(dx * dx + dy * dy));
                if (r <= radius) {
                    obstacles[index(px, py)] = 0;
                }
            }
        }
    }
}

void WaveSimulation::clearObstacles() {
    std::fill(obstacles.begin(), obstacles.end(), false);
}

bool WaveSimulation::isObstacle(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }
    return obstacles[index(x, y)];
}

bool WaveSimulation::loadObstaclesFromSVG(const std::string& filename) {
    /*
     * Load and rasterize SVG file to obstacle grid
     *
     * Uses SVGLoader (infrastructure layer) to parse and rasterize the SVG,
     * then applies the result to the domain model (obstacle grid).
     *
     * This maintains clean architecture: domain doesn't know about SVG,
     * it just receives an array of obstacles.
     */

    std::cout << "WaveSimulation: Loading obstacles from " << filename << std::endl;

    // Clear existing obstacles and pressure field
    clearObstacles();
    clear();

    // Use SVGLoader to load and rasterize
    SVGLoader loader;
    std::vector<uint8_t> loadedObstacles;

    bool success = loader.loadSVG(filename, width, height, loadedObstacles);

    if (!success) {
        std::cerr << "WaveSimulation: Failed to load SVG: " << loader.getLastError() << std::endl;
        return false;
    }

    // Verify size matches
    if (static_cast<int>(loadedObstacles.size()) != width * height) {
        std::cerr << "WaveSimulation: Obstacle grid size mismatch" << std::endl;
        return false;
    }

    // Apply loaded obstacles to simulation
    obstacles = std::move(loadedObstacles);

    // Ensure pressure is zero at all obstacle cells
    for (int i = 0; i < width * height; i++) {
        if (obstacles[i]) {
            pressure[i] = 0.0f;
            pressurePrev[i] = 0.0f;
            pressureNext[i] = 0.0f;
        }
    }

    std::cout << "WaveSimulation: Successfully loaded obstacles from SVG" << std::endl;
    return true;
}

void WaveSimulation::setListenerPosition(int x, int y) {
    // Clamp to valid range
    listenerX = std::max(0, std::min(x, width - 1));
    listenerY = std::max(0, std::min(y, height - 1));
}

void WaveSimulation::getListenerPosition(int& x, int& y) const {
    x = listenerX;
    y = listenerY;
}

void WaveSimulation::setListenerEnabled(bool enabled) {
    listenerEnabled = enabled;
    if (enabled) {
        std::cout << "WaveSimulation: Listener enabled at (" << listenerX << ", " << listenerY
                  << ")" << std::endl;
    } else {
        std::cout << "WaveSimulation: Listener disabled" << std::endl;
    }
}

float WaveSimulation::getListenerPressure() const {
    /*
     * Get pressure at listener position
     *
     * Returns the acoustic pressure (Pa) at the listener's location.
     * This pressure value is then converted to audio by AudioOutput.
     */

    if (!listenerEnabled) {
        return 0.0f;
    }

    // Check bounds
    if (listenerX < 0 || listenerX >= width || listenerY < 0 || listenerY >= height) {
        return 0.0f;
    }

    // Sample pressure at listener position
    return pressure[index(listenerX, listenerY)];
}

std::vector<float> WaveSimulation::getListenerSamples() {
    /*
     * Get all listener samples collected during last update
     *
     * CRITICAL FIX for audio quality:
     * Returns all ~191 samples collected per frame at sub-step rate (~11 kHz).
     * This preserves all high-frequency audio content instead of just
     * sampling once per frame at 60 Hz (which caused "boom boom" bass sounds).
     *
     * The buffer is moved (not copied) for efficiency and is automatically
     * cleared, ready for the next frame.
     */
    return std::move(listenerSampleBuffer);
}

void WaveSimulation::applyDampingPreset(const DampingPreset& preset) {
    /*
     * Apply Damping Preset (Domain Logic)
     *
     * This method encapsulates the domain rule for applying acoustic
     * environment presets. It updates the simulation parameters according
     * to the domain-defined preset values.
     *
     * Clean Architecture: This is a domain service method that coordinates
     * the application of a value object (DampingPreset) to the entity
     * (WaveSimulation).
     */

    // Update physics parameters from preset
    damping = preset.getDamping();
    wallReflection = preset.getWallReflection();

    // Store current preset for querying
    currentPreset = preset;

    // Log preset application (domain event)
    std::cout << "WaveSimulation: Applied preset '" << preset.getName() << "' - damping=" << damping
              << ", wallReflection=" << wallReflection << std::endl;
}

// ============================================================================
// AUDIO SOURCE MANAGEMENT
// ============================================================================

size_t WaveSimulation::addAudioSource(std::unique_ptr<AudioSource> source) {
    /*
     * Add audio source to simulation
     *
     * Audio sources continuously emit sound based on their audio sample data.
     * Each frame, the source's current sample is added to the pressure field.
     */

    if (!source) {
        return SIZE_MAX;  // Invalid ID
    }

    // Expand active region to include this audio source
    // Use a reasonable radius for initial expansion
    int sourceX = source->getX();
    int sourceY = source->getY();
    expandActiveRegion(sourceX, sourceY, 10);  // Initial 10-pixel radius

    size_t id = audioSources.size();
    audioSources.push_back(std::move(source));

    std::cout << "WaveSimulation: Added audio source " << id
              << " at (" << audioSources[id]->getX() << ", " << audioSources[id]->getY() << ")"
              << std::endl;

    return id;
}

void WaveSimulation::removeAudioSource(size_t sourceId) {
    if (sourceId < audioSources.size()) {
        audioSources.erase(audioSources.begin() + sourceId);
        std::cout << "WaveSimulation: Removed audio source " << sourceId << std::endl;
    }
}

AudioSource* WaveSimulation::getAudioSource(size_t sourceId) {
    if (sourceId < audioSources.size()) {
        return audioSources[sourceId].get();
    }
    return nullptr;
}

void WaveSimulation::clearAudioSources() {
    audioSources.clear();
    std::cout << "WaveSimulation: Cleared all audio sources" << std::endl;
}

// ============================================================================
// GPU ACCELERATION
// ============================================================================

void WaveSimulation::setGPUEnabled(bool enabled) {
    if (enabled && !metalBackend.isAvailable()) {
        std::cout << "WaveSimulation: Cannot enable GPU - Metal not available" << std::endl;
        useGPU = false;
        return;
    }

    useGPU = enabled;

    if (useGPU) {
        std::cout << "WaveSimulation: GPU acceleration ENABLED (Metal)" << std::endl;
        std::cout << "WaveSimulation: Wave equation will run on " << width << "x" << height
                  << " = " << (width * height) << " GPU threads" << std::endl;
    } else {
        std::cout << "WaveSimulation: GPU acceleration DISABLED (CPU fallback)" << std::endl;
    }
}

// ============================================================================
// ACTIVE REGION OPTIMIZATION
// ============================================================================

void WaveSimulation::expandActiveRegion(int centerX, int centerY, int radius) {
    /*
     * Expand active region to include a circular area of activity
     *
     * Called when:
     * - Adding an impulse source
     * - Placing an audio source
     * - Any localized event that creates waves
     *
     * @param centerX X coordinate of activity center
     * @param centerY Y coordinate of activity center
     * @param radius Radius of activity in pixels
     */

    if (!activeRegion.hasActivity) {
        // First activity - initialize region
        activeRegion.minX = std::max(0, centerX - radius);
        activeRegion.maxX = std::min(width - 1, centerX + radius);
        activeRegion.minY = std::max(0, centerY - radius);
        activeRegion.maxY = std::min(height - 1, centerY + radius);
        activeRegion.hasActivity = true;
    } else {
        // Expand existing region
        activeRegion.minX = std::max(0, std::min(activeRegion.minX, centerX - radius));
        activeRegion.maxX = std::min(width - 1, std::max(activeRegion.maxX, centerX + radius));
        activeRegion.minY = std::max(0, std::min(activeRegion.minY, centerY - radius));
        activeRegion.maxY = std::min(height - 1, std::max(activeRegion.maxY, centerY + radius));
    }
}

void WaveSimulation::growActiveRegionForFrame(float dt) {
    /*
     * Grow active region based on wave propagation distance
     *
     * Waves travel at soundSpeed (m/s), so expand region by:
     * expansion = soundSpeed * dt / dx (in pixels)
     *
     * Add safety margin of 2x to ensure we don't miss wave fronts
     *
     * @param dt Time step for this frame (seconds)
     */

    if (!activeRegion.hasActivity) {
        return;  // No activity to grow
    }

    // Calculate how far waves can travel in one frame
    float propagationDistance = soundSpeed * dt / dx;  // in pixels
    int expansion = static_cast<int>(std::ceil(propagationDistance * 2.0f));  // 2x safety factor

    // Expand region in all directions
    activeRegion.minX = std::max(0, activeRegion.minX - expansion);
    activeRegion.maxX = std::min(width - 1, activeRegion.maxX + expansion);
    activeRegion.minY = std::max(0, activeRegion.minY - expansion);
    activeRegion.maxY = std::min(height - 1, activeRegion.maxY + expansion);
}
