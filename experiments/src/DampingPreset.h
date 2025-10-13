#pragma once

#include <string>

/*
 * DampingPreset - Domain Value Object (DDD)
 *
 * Represents acoustic environment presets with domain-specific behavior.
 * This is an immutable value object that encapsulates the physics parameters
 * for different acoustic scenarios.
 *
 * Clean Architecture: This belongs to the Domain layer (innermost circle).
 * It has no dependencies on infrastructure or frameworks.
 */
class DampingPreset {
public:
    /*
     * Preset types representing different acoustic environments
     *
     * Domain Knowledge:
     * - REALISTIC: Models real-world room acoustics with air absorption
     * - VISUALIZATION: Optimized for demonstrating wave phenomena clearly
     * - ANECHOIC: Simulates anechoic chamber (no reflections, pure absorption)
     */
    enum class Type {
        REALISTIC,      // Real-world air absorption (0.997 = 0.3% loss/step)
        VISUALIZATION,  // Minimal damping for clear interference patterns
        ANECHOIC       // No reflections, maximum absorption
    };

    /*
     * Factory method: Create preset from type (Domain Logic)
     */
    static DampingPreset fromType(Type type);

    /*
     * Factory method: Create custom preset with validation
     *
     * Domain invariants:
     * - damping must be in (0, 1] (0 = instant decay, 1 = no decay)
     * - wallReflection must be in [0, 1] (0 = full absorption, 1 = perfect reflection)
     *
     * @throws std::invalid_argument if invariants violated
     */
    static DampingPreset custom(float damping, float wallReflection, const std::string& name);

    // Value object: equality by value
    bool operator==(const DampingPreset& other) const;
    bool operator!=(const DampingPreset& other) const;

    // Getters (value objects are immutable)
    float getDamping() const { return damping; }
    float getWallReflection() const { return wallReflection; }
    Type getType() const { return type; }
    std::string getName() const { return name; }
    std::string getDescription() const { return description; }

    // Domain query: Is this an anechoic environment?
    bool isAnechoic() const { return type == Type::ANECHOIC; }

    // Domain query: Is this optimized for visualization?
    bool isVisualization() const { return type == Type::VISUALIZATION; }

private:
    // Private constructor: enforce creation through factory methods
    DampingPreset(Type type, float damping, float wallReflection,
                  const std::string& name, const std::string& description);

    // Domain state (immutable)
    Type type;
    float damping;           // Air absorption coefficient (0, 1]
    float wallReflection;    // Wall reflection coefficient [0, 1]
    std::string name;        // Human-readable name
    std::string description; // Domain-specific description
};

/*
 * Domain Service: Preset recommendations based on use case
 *
 * This encapsulates domain knowledge about which preset to use when.
 */
class DampingPresetService {
public:
    /*
     * Recommend preset for interactive visualization
     *
     * Domain Rule: For demonstrating wave phenomena, use low damping
     * so interference patterns are clearly visible.
     */
    static DampingPreset recommendForVisualization() {
        return DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);
    }

    /*
     * Recommend preset for acoustic simulation
     *
     * Domain Rule: For realistic room acoustics modeling, use
     * physically accurate air absorption and wall reflection.
     */
    static DampingPreset recommendForSimulation() {
        return DampingPreset::fromType(DampingPreset::Type::REALISTIC);
    }

    /*
     * Recommend preset for testing
     *
     * Domain Rule: For unit tests, use anechoic to isolate
     * wave behavior without environmental effects.
     */
    static DampingPreset recommendForTesting() {
        return DampingPreset::fromType(DampingPreset::Type::ANECHOIC);
    }
};
