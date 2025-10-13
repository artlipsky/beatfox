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
