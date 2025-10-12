#include "WaveSimulation.h"
#include <algorithm>
#include <cstring>
#include <cmath>

WaveSimulation::WaveSimulation(int width, int height)
    : width(width)
    , height(height)
    , soundSpeed(343.0f)    // Speed of sound in air at 20°C (m/s)
    , damping(0.997f)       // Air absorption (waves fade over time)
    , wallReflection(0.85f) // Wall reflection coefficient (15% energy loss per reflection)
    , dx(0.03125f)          // Spatial grid spacing: 1 pixel = 3.125 cm = 0.03125 m
{
    int size = width * height;

    // Initialize pressure fields to zero (ambient atmospheric pressure)
    pressure.resize(size, 0.0f);
    pressurePrev.resize(size, 0.0f);
    pressureNext.resize(size, 0.0f);

    /*
     * PHYSICAL UNITS AND SCALE:
     * -------------------------
     * Coordinate system: 1 pixel = 3.125 cm = 31.25 mm = 0.03125 m
     *
     * For 640x320 grid (W x H):
     * - Physical room size: 20m x 10m (width x height)
     * - Aspect ratio: 2:1 (rectangular room)
     *
     * Physical constants (air at 20°C, 1 atm):
     * - Speed of sound: c = 343 m/s
     * - Air density: ρ = 1.204 kg/m³
     * - Atmospheric pressure: P₀ = 101325 Pa
     *
     * The pressure field represents acoustic pressure p (Pa),
     * which is the deviation from atmospheric pressure P₀.
     */
}

WaveSimulation::~WaveSimulation() {
}

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
     * With c = 343 m/s, dx = 0.01 m:
     * dt_max = 0.707 * 0.01 / 343 ≈ 2.06e-5 s
     *
     * At 60 FPS (dt_frame ≈ 0.0167 s), we need multiple sub-steps
     */

    // Calculate maximum stable time step (CFL condition)
    const float CFL_SAFETY = 0.6f;  // Safety factor (< 0.707)
    float dt_max = CFL_SAFETY * dx / soundSpeed;

    // Calculate number of sub-steps needed
    int numSteps = static_cast<int>(std::ceil(dt_frame / dt_max));
    float dt = dt_frame / numSteps;

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

    // Compute CFL coefficient: (c*dt/dx)²
    float c2_dt2_dx2 = (soundSpeed * soundSpeed * dt * dt) / (dx * dx);

    // Interior points - solve wave equation
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            int idx = index(x, y);

            // 5-point stencil Laplacian: ∇²p ≈ (p[i+1,j] + p[i-1,j] + p[i,j+1] + p[i,j-1] - 4*p[i,j]) / dx²
            float p_xp = pressure[index(x+1, y)];
            float p_xm = pressure[index(x-1, y)];
            float p_yp = pressure[index(x, y+1)];
            float p_ym = pressure[index(x, y-1)];
            float p_c  = pressure[idx];

            float laplacian = (p_xp + p_xm + p_yp + p_ym - 4.0f * p_c);

            // Leapfrog time integration
            // p^(n+1) = 2*p^n - p^(n-1) + c²*dt²/dx² * ∇²p^n
            pressureNext[idx] = 2.0f * p_c - pressurePrev[idx] + c2_dt2_dx2 * laplacian;

            // Apply damping (air absorption: viscosity + thermal losses)
            pressureNext[idx] *= damping;
        }
    }

    // Boundary conditions: REFLECTIVE WALLS with energy loss
    // Using Neumann boundary condition: ∂p/∂n = 0 (zero pressure gradient)
    // Plus reflection coefficient to simulate energy absorption by walls
    // Real walls absorb 10-20% of acoustic energy per reflection

    // Top edge (y = 0): mirror from y = 1 with energy loss
    for (int x = 0; x < width; x++) {
        pressureNext[index(x, 0)] = pressureNext[index(x, 1)] * wallReflection;
    }

    // Bottom edge (y = height-1): mirror from y = height-2 with energy loss
    for (int x = 0; x < width; x++) {
        pressureNext[index(x, height-1)] = pressureNext[index(x, height-2)] * wallReflection;
    }

    // Left edge (x = 0): mirror from x = 1 with energy loss
    for (int y = 0; y < height; y++) {
        pressureNext[index(0, y)] = pressureNext[index(1, y)] * wallReflection;
    }

    // Right edge (x = width-1): mirror from x = width-2 with energy loss
    for (int y = 0; y < height; y++) {
        pressureNext[index(width-1, y)] = pressureNext[index(width-2, y)] * wallReflection;
    }

    // Time step: rotate buffers
    std::swap(pressurePrev, pressure);
    std::swap(pressure, pressureNext);
}

void WaveSimulation::addDisturbance(int x, int y, float amplitude) {
    addPressureSource(x, y, amplitude);
}

void WaveSimulation::addPressureSource(int x, int y, float pressureAmplitude) {
    /*
     * Add a brief impulse source - like a hand clap or drum hit
     *
     * This creates a localized pressure spike that will propagate
     * outward as circular waves, reflect off walls, and interfere.
     */

    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }

    // Create a compact, smooth impulse
    // With 1 pixel = 3.125 cm, radius of 3 pixels ≈ 9.4 cm (realistic hand clap size)
    const int sourceRadius = 3;
    const float sigma = 2.5f;  // Gaussian width for smoothness

    for (int dy = -sourceRadius; dy <= sourceRadius; dy++) {
        for (int dx = -sourceRadius; dx <= sourceRadius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width-1 && py > 0 && py < height-1) {
                float r = std::sqrt(float(dx*dx + dy*dy));

                // Gaussian profile (smooth, no sharp edges)
                float profile = std::exp(-r*r / (2.0f * sigma * sigma));

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
}
