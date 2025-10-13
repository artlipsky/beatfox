#include "DampingPreset.h"
#include <stdexcept>
#include <cmath>

/*
 * DampingPreset Implementation
 *
 * This implements the domain logic for acoustic environment presets.
 * Values are based on physical acoustics research and practical testing.
 */

DampingPreset::DampingPreset(Type type, float damping, float wallReflection,
                             const std::string& name, const std::string& description)
    : type(type)
    , damping(damping)
    , wallReflection(wallReflection)
    , name(name)
    , description(description)
{
    // Validate domain invariants
    if (damping <= 0.0f || damping > 1.0f) {
        throw std::invalid_argument("Damping must be in range (0, 1]");
    }
    if (wallReflection < 0.0f || wallReflection > 1.0f) {
        throw std::invalid_argument("Wall reflection must be in range [0, 1]");
    }
}

DampingPreset DampingPreset::fromType(Type type) {
    switch (type) {
        case Type::REALISTIC:
            /*
             * REALISTIC: Real-world room acoustics
             *
             * Physics basis:
             * - Air absorption: ~0.3% energy loss per timestep
             * - At 60 FPS with 100x slowdown: waves decay to 50% after ~137m
             * - Wall reflection: 85% (typical for concrete/drywall)
             * - This matches measured room acoustics behavior
             */
            return DampingPreset(
                Type::REALISTIC,
                0.997f,      // damping (0.3% loss per step)
                0.85f,       // wallReflection (15% absorption at walls)
                "Realistic",
                "Real-world room acoustics with air absorption and wall reflections"
            );

        case Type::VISUALIZATION:
            /*
             * VISUALIZATION: Optimized for demonstrating wave phenomena
             *
             * Physics basis:
             * - Minimal air absorption: 0.02% energy loss per timestep
             * - This allows waves to travel long distances and interfere clearly
             * - Wall reflection: 98% (highly reflective walls)
             * - Perfect for educational demonstrations and debugging
             */
            return DampingPreset(
                Type::VISUALIZATION,
                0.9998f,     // damping (0.02% loss per step - REDUCED for clearer patterns)
                0.98f,       // wallReflection (2% absorption at walls - INCREASED reflection)
                "Visualization",
                "Minimal damping for clear demonstration of interference patterns"
            );

        case Type::ANECHOIC:
            /*
             * ANECHOIC: Simulates anechoic chamber
             *
             * Physics basis:
             * - Anechoic chamber: walls absorb all sound (no reflections)
             * - Higher air absorption: 0.2% per timestep (more than visualization)
             * - Wall reflection: 0% (perfect absorption)
             * - Used for testing isolated wave behavior
             * - More damping than visualization to show clear difference
             */
            return DampingPreset(
                Type::ANECHOIC,
                0.998f,      // damping (0.2% loss per step - INCREASED for distinction)
                0.0f,        // wallReflection (100% absorption - no reflections)
                "Anechoic",
                "Anechoic chamber: no wall reflections, higher air absorption"
            );

        default:
            throw std::invalid_argument("Unknown preset type");
    }
}

DampingPreset DampingPreset::custom(float damping, float wallReflection, const std::string& name) {
    // Validate invariants (constructor will throw if invalid)
    return DampingPreset(
        Type::REALISTIC,  // Custom presets default to REALISTIC type
        damping,
        wallReflection,
        name,
        "Custom acoustic environment"
    );
}

bool DampingPreset::operator==(const DampingPreset& other) const {
    // Value objects: equality by value, not identity
    return std::abs(damping - other.damping) < 1e-6f &&
           std::abs(wallReflection - other.wallReflection) < 1e-6f &&
           type == other.type;
}

bool DampingPreset::operator!=(const DampingPreset& other) const {
    return !(*this == other);
}
