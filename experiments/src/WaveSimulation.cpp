#include "WaveSimulation.h"
#include "SVGLoader.h"
#include <algorithm>
#include <cstring>
#include <cmath>
#include <iostream>

WaveSimulation::WaveSimulation(int width, int height)
    : width(width)
    , height(height)
    , soundSpeed(343.0f)    // Speed of sound in air at 20°C (m/s)
    , damping(0.997f)       // Air absorption (waves fade over time)
    , wallReflection(0.85f) // Wall reflection coefficient (15% energy loss per reflection)
    , dx(0.05f)             // Spatial grid spacing: 1 pixel = 5 cm = 0.05 m
{
    int size = width * height;

    // Initialize pressure fields to zero (ambient atmospheric pressure)
    pressure.resize(size, 0.0f);
    pressurePrev.resize(size, 0.0f);
    pressureNext.resize(size, 0.0f);

    // Initialize obstacle field (no obstacles initially)
    obstacles.resize(size, false);

    /*
     * PHYSICAL UNITS AND SCALE:
     * -------------------------
     * Coordinate system: 1 pixel = 5 cm = 50 mm = 0.05 m
     *
     * For 400x200 grid (W x H):
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
    const float c2_dt2_dx2 = (soundSpeed * soundSpeed * dt * dt) / (dx * dx);
    const float twoOverDamping = 2.0f * damping;

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
            const float p_c  = pressure[idx];
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

    // Boundary conditions: REFLECTIVE WALLS with energy loss
    // Using Neumann boundary condition: ∂p/∂n = 0 (zero pressure gradient)
    // Plus reflection coefficient to simulate energy absorption by walls

    // Top and bottom edges (optimized horizontal loops)
    const int lastRow = (height - 1) * width;
    for (int x = 0; x < width; x++) {
        pressureNext[x] = pressureNext[width + x] * wallReflection;  // Top
        pressureNext[lastRow + x] = pressureNext[lastRow - width + x] * wallReflection;  // Bottom
    }

    // Left and right edges (vertical loops)
    const int lastCol = width - 1;
    for (int y = 0; y < height; y++) {
        const int rowOffset = y * width;
        pressureNext[rowOffset] = pressureNext[rowOffset + 1] * wallReflection;  // Left
        pressureNext[rowOffset + lastCol] = pressureNext[rowOffset + lastCol - 1] * wallReflection;  // Right
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
    // With 1 pixel = 5 cm, radius of 2 pixels = 10 cm (realistic hand clap size)
    const int sourceRadius = 2;
    const float sigma = 2.5f;  // Gaussian width for smoothness

    for (int dy = -sourceRadius; dy <= sourceRadius; dy++) {
        for (int dx = -sourceRadius; dx <= sourceRadius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width-1 && py > 0 && py < height-1) {
                // Don't add pressure to obstacle cells
                if (obstacles[index(px, py)]) {
                    continue;
                }

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

void WaveSimulation::addObstacle(int x, int y, int radius) {
    /*
     * Add a circular obstacle (solid object that blocks sound)
     * Obstacles act as rigid boundaries with zero pressure
     */
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx;
            int py = y + dy;

            if (px > 0 && px < width-1 && py > 0 && py < height-1) {
                float r = std::sqrt(float(dx*dx + dy*dy));
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

            if (px > 0 && px < width-1 && py > 0 && py < height-1) {
                float r = std::sqrt(float(dx*dx + dy*dy));
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
