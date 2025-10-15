#include <metal_stdlib>
using namespace metal;

/*
 * Metal Compute Shader for 2D Acoustic Wave Equation
 *
 * FDTD (Finite Difference Time Domain) Wave Propagation on GPU
 *
 * Performance: Runs on all GPU cores simultaneously!
 * - M3 Max: 30-40 GPU cores
 * - Each core runs thousands of threads
 * - Total: 80,000 threads for 400x200 grid = massive parallelism!
 *
 * Physics: 2D wave equation with leapfrog integration
 * ∂²p/∂t² = c² ∇²p
 */

struct SimulationParams {
    int width;                  // Grid width
    int height;                 // Grid height
    float c2_dt2_dx2;          // CFL coefficient: (c*dt/dx)²
    float damping;             // Air absorption coefficient
    float wallReflection;      // Wall reflection coefficient (0-1)
    float twoOverDamping;      // Precomputed: 2.0 * damping
};

/*
 * Multi-step simulation parameters
 * Used for executing multiple sub-steps on GPU without CPU round-trip
 */
struct MultiStepParams {
    int width;                  // Grid width
    int height;                 // Grid height
    float c2_dt2_dx2;          // CFL coefficient: (c*dt/dx)²
    float damping;             // Air absorption coefficient
    float wallReflection;      // Wall reflection coefficient (0-1)
    float twoOverDamping;      // Precomputed: 2.0 * damping
    int currentIdx;            // Current buffer index (0, 1, or 2)
    int prevIdx;               // Previous buffer index (0, 1, or 2)
    int nextIdx;               // Next buffer index (0, 1, or 2)
    int listenerX;             // Listener X coordinate (-1 if disabled)
    int listenerY;             // Listener Y coordinate
    int subStepIdx;            // Current sub-step index (for listener sampling)
    int numAudioSources;       // Number of active audio sources
    // ACTIVE REGION OPTIMIZATION: Map local threads to global grid
    int offsetX;               // X offset of active region in global grid
    int offsetY;               // Y offset of active region in global grid
    int activeWidth;           // Width of active region
    int activeHeight;          // Height of active region
};

/*
 * Audio source data (per sub-step)
 * Pre-computed on CPU, passed to GPU for injection at each sub-step
 */
struct AudioSourceData {
    int x;                     // Source X position
    int y;                     // Source Y position
    float pressure;            // Pressure value to inject
};

/*
 * Wave equation compute kernel - runs on GPU
 *
 * Each thread handles ONE grid cell
 * Threadgroup size: 16x16 = 256 threads per group (optimal for M3)
 */
kernel void waveEquationStep(
    device const float* pressure       [[buffer(0)]],    // Current pressure field
    device const float* pressurePrev   [[buffer(1)]],    // Previous pressure field
    device float* pressureNext         [[buffer(2)]],    // Output: next pressure field
    device const uint8_t* obstacles    [[buffer(3)]],    // Obstacle mask
    constant SimulationParams& params  [[buffer(4)]],    // Simulation parameters
    uint2 gid                          [[thread_position_in_grid]])  // Thread position
{
    // Thread position = grid cell coordinates
    const int x = gid.x;
    const int y = gid.y;

    // Boundary check
    if (x >= params.width || y >= params.height) {
        return;
    }

    const int idx = y * params.width + x;

    // Skip obstacle cells - rigid boundaries (zero pressure)
    if (obstacles[idx] != 0) {
        pressureNext[idx] = 0.0f;
        return;
    }

    // ========================================================================
    // INTERIOR POINTS - Wave Equation with 5-point stencil
    // ========================================================================

    if (x > 0 && x < params.width - 1 && y > 0 && y < params.height - 1) {
        // Load current pressure and neighbors
        const float p_c = pressure[idx];
        const float p_xp = pressure[idx + 1];           // Right
        const float p_xm = pressure[idx - 1];           // Left
        const float p_yp = pressure[idx + params.width];  // Down
        const float p_ym = pressure[idx - params.width];  // Up

        // Compute Laplacian (5-point stencil)
        // ∇²p ≈ (p_xp + p_xm + p_yp + p_ym - 4*p_c) / dx²
        const float laplacian = (p_xp + p_xm + p_yp + p_ym - 4.0f * p_c);

        // Leapfrog time integration with damping
        // p^(n+1) = 2*damping*p^n - damping*p^(n-1) + damping*c²*dt²/dx² * ∇²p^n
        pressureNext[idx] = params.twoOverDamping * p_c
                          - params.damping * pressurePrev[idx]
                          + params.damping * params.c2_dt2_dx2 * laplacian;
        return;
    }

    // ========================================================================
    // BOUNDARY CONDITIONS
    // ========================================================================

    const bool absorbingWalls = (params.wallReflection < 0.1f);
    const int lastRow = (params.height - 1) * params.width;
    const int lastCol = params.width - 1;

    if (absorbingWalls) {
        // ONE-WAY WAVE EQUATION (Engquist-Majda ABC)
        // Allows only outward-traveling waves
        const float cfl = sqrt(params.c2_dt2_dx2);  // c*dt/dx
        const float absorption = min(1.0f, cfl);

        // Top/bottom boundaries
        if (x > 0 && x < params.width - 1) {
            if (y == 0) {
                // Top boundary
                pressureNext[idx] = pressure[idx] - absorption * (pressure[idx] - pressure[idx + params.width]);
                return;
            }
            if (y == params.height - 1) {
                // Bottom boundary
                pressureNext[idx] = pressure[idx] - absorption * (pressure[idx] - pressure[idx - params.width]);
                return;
            }
        }

        // Left/right boundaries
        if (y > 0 && y < params.height - 1) {
            if (x == 0) {
                // Left boundary
                pressureNext[idx] = pressure[idx] - absorption * (pressure[idx] - pressure[idx + 1]);
                return;
            }
            if (x == params.width - 1) {
                // Right boundary
                pressureNext[idx] = pressure[idx] - absorption * (pressure[idx] - pressure[idx - 1]);
                return;
            }
        }

        // Corners - zero (simplest stable choice)
        if ((x == 0 || x == lastCol) && (y == 0 || y == params.height - 1)) {
            pressureNext[idx] = 0.0f;
            return;
        }
    } else {
        // REFLECTIVE BOUNDARIES (Neumann condition with attenuation)

        // Top/bottom walls
        if (y == 0 && x >= 0 && x < params.width) {
            pressureNext[idx] = pressureNext[params.width + x] * params.wallReflection;
            return;
        }
        if (y == params.height - 1 && x >= 0 && x < params.width) {
            pressureNext[idx] = pressureNext[lastRow - params.width + x] * params.wallReflection;
            return;
        }

        // Left/right walls
        const int rowOffset = y * params.width;
        if (x == 0 && y >= 0 && y < params.height) {
            pressureNext[idx] = pressureNext[rowOffset + 1] * params.wallReflection;
            return;
        }
        if (x == params.width - 1 && y >= 0 && y < params.height) {
            pressureNext[idx] = pressureNext[rowOffset + lastCol - 1] * params.wallReflection;
            return;
        }
    }
}

/*
 * Multi-step wave equation kernel
 *
 * Executes a single sub-step using index-based triple buffering.
 * This kernel is called in a loop for all sub-steps without CPU round-trip.
 *
 * KEY OPTIMIZATION: All 3 pressure buffers are concatenated into one buffer:
 * - Buffer 0: pressureBuffers[0 * gridSize ... 1 * gridSize)
 * - Buffer 1: pressureBuffers[1 * gridSize ... 2 * gridSize)
 * - Buffer 2: pressureBuffers[2 * gridSize ... 3 * gridSize)
 *
 * Buffer indices rotate each step: (current, prev, next) → (next, current, prev)
 */
kernel void multiStepWaveEquation(
    device float* pressureBuffers          [[buffer(0)]],    // 3 concatenated buffers
    device const uint8_t* obstacles        [[buffer(1)]],    // Obstacle mask
    device float* listenerSamples          [[buffer(2)]],    // Output: listener samples
    constant MultiStepParams& params       [[buffer(3)]],    // Multi-step parameters
    device const AudioSourceData* audioSources [[buffer(4)]], // Audio source data for this sub-step
    uint2 gid                              [[thread_position_in_grid]])
{
    // ACTIVE REGION OPTIMIZATION: Map local thread position to global grid coordinates
    // Local coordinates (gid.x, gid.y) are in [0, activeWidth) × [0, activeHeight)
    // Global coordinates (x, y) are in [0, width) × [0, height)
    const int localX = gid.x;
    const int localY = gid.y;

    // Boundary check for active region
    if (localX >= params.activeWidth || localY >= params.activeHeight) {
        return;
    }

    // Map to global grid coordinates
    const int x = localX + params.offsetX;
    const int y = localY + params.offsetY;

    // Boundary check for full grid (safety check)
    if (x >= params.width || y >= params.height) {
        return;
    }

    const int idx = y * params.width + x;
    const int gridSize = params.width * params.height;

    // Calculate buffer offsets based on indices
    const int offset_current = params.currentIdx * gridSize;
    const int offset_prev = params.prevIdx * gridSize;
    const int offset_next = params.nextIdx * gridSize;

    // Inject audio sources into current pressure field
    // This happens BEFORE wave propagation for continuous sound
    if (params.numAudioSources > 0) {
        for (int i = 0; i < params.numAudioSources; i++) {
            if (audioSources[i].x == x && audioSources[i].y == y) {
                // Add pressure from this audio source
                pressureBuffers[offset_current + idx] += audioSources[i].pressure;
            }
        }
    }

    // Sample listener position BEFORE computing wave step
    // This captures the current pressure at the listener location
    if (params.listenerX >= 0 && x == params.listenerX && y == params.listenerY) {
        listenerSamples[params.subStepIdx] = pressureBuffers[offset_current + idx];
    }

    // Skip obstacle cells - rigid boundaries (zero pressure)
    if (obstacles[idx] != 0) {
        pressureBuffers[offset_next + idx] = 0.0f;
        return;
    }

    // ========================================================================
    // INTERIOR POINTS - Wave Equation with 5-point stencil
    // ========================================================================

    if (x > 0 && x < params.width - 1 && y > 0 && y < params.height - 1) {
        // Load current pressure and neighbors
        const float p_c = pressureBuffers[offset_current + idx];
        const float p_xp = pressureBuffers[offset_current + idx + 1];           // Right
        const float p_xm = pressureBuffers[offset_current + idx - 1];           // Left
        const float p_yp = pressureBuffers[offset_current + idx + params.width];  // Down
        const float p_ym = pressureBuffers[offset_current + idx - params.width];  // Up

        // Compute Laplacian (5-point stencil)
        const float laplacian = (p_xp + p_xm + p_yp + p_ym - 4.0f * p_c);

        // Leapfrog time integration with damping
        pressureBuffers[offset_next + idx] = params.twoOverDamping * p_c
                                           - params.damping * pressureBuffers[offset_prev + idx]
                                           + params.damping * params.c2_dt2_dx2 * laplacian;
        return;
    }

    // ========================================================================
    // BOUNDARY CONDITIONS
    // ========================================================================

    const bool absorbingWalls = (params.wallReflection < 0.1f);
    const int lastRow = (params.height - 1) * params.width;
    const int lastCol = params.width - 1;

    if (absorbingWalls) {
        // ONE-WAY WAVE EQUATION (Engquist-Majda ABC)
        const float cfl = sqrt(params.c2_dt2_dx2);
        const float absorption = min(1.0f, cfl);

        // Top/bottom boundaries
        if (x > 0 && x < params.width - 1) {
            if (y == 0) {
                pressureBuffers[offset_next + idx] = pressureBuffers[offset_current + idx]
                    - absorption * (pressureBuffers[offset_current + idx]
                                  - pressureBuffers[offset_current + idx + params.width]);
                return;
            }
            if (y == params.height - 1) {
                pressureBuffers[offset_next + idx] = pressureBuffers[offset_current + idx]
                    - absorption * (pressureBuffers[offset_current + idx]
                                  - pressureBuffers[offset_current + idx - params.width]);
                return;
            }
        }

        // Left/right boundaries
        if (y > 0 && y < params.height - 1) {
            if (x == 0) {
                pressureBuffers[offset_next + idx] = pressureBuffers[offset_current + idx]
                    - absorption * (pressureBuffers[offset_current + idx]
                                  - pressureBuffers[offset_current + idx + 1]);
                return;
            }
            if (x == params.width - 1) {
                pressureBuffers[offset_next + idx] = pressureBuffers[offset_current + idx]
                    - absorption * (pressureBuffers[offset_current + idx]
                                  - pressureBuffers[offset_current + idx - 1]);
                return;
            }
        }

        // Corners - zero
        if ((x == 0 || x == lastCol) && (y == 0 || y == params.height - 1)) {
            pressureBuffers[offset_next + idx] = 0.0f;
            return;
        }
    } else {
        // REFLECTIVE BOUNDARIES (Neumann condition with attenuation)

        // Top/bottom walls
        if (y == 0 && x >= 0 && x < params.width) {
            pressureBuffers[offset_next + idx] = pressureBuffers[offset_next + params.width + x] * params.wallReflection;
            return;
        }
        if (y == params.height - 1 && x >= 0 && x < params.width) {
            pressureBuffers[offset_next + idx] = pressureBuffers[offset_next + lastRow - params.width + x] * params.wallReflection;
            return;
        }

        // Left/right walls
        const int rowOffset = y * params.width;
        if (x == 0 && y >= 0 && y < params.height) {
            pressureBuffers[offset_next + idx] = pressureBuffers[offset_next + rowOffset + 1] * params.wallReflection;
            return;
        }
        if (x == params.width - 1 && y >= 0 && y < params.height) {
            pressureBuffers[offset_next + idx] = pressureBuffers[offset_next + rowOffset + lastCol - 1] * params.wallReflection;
            return;
        }
    }
}
