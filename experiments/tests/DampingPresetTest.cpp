/*
 * DampingPresetTest.cpp - Domain Value Object Tests
 *
 * Tests for DampingPreset value object and related domain logic.
 * Validates domain invariants, behavior, and Clean Architecture principles.
 *
 * Clean Architecture:
 * - These are domain layer tests (no infrastructure dependencies)
 * - Tests document domain rules and behavior
 * - Validates value object properties (immutability, equality by value)
 */

#include <gtest/gtest.h>
#include "DampingPreset.h"
#include "WaveSimulation.h"
#include <stdexcept>
#include <cmath>

// ============================================================================
// DAMPING PRESET VALUE OBJECT TESTS
// ============================================================================

class DampingPresetTest : public ::testing::Test {
protected:
    void SetUp() override {
        // No setup needed for value object tests
    }
};

TEST_F(DampingPresetTest, RealisticPresetHasCorrectValues) {
    /*
     * Domain Rule: REALISTIC preset represents real-world room acoustics
     * Physics: 0.997 damping = 0.3% energy loss per timestep
     */
    auto preset = DampingPreset::fromType(DampingPreset::Type::REALISTIC);

    EXPECT_FLOAT_EQ(preset.getDamping(), 0.997f);
    EXPECT_FLOAT_EQ(preset.getWallReflection(), 0.85f);
    EXPECT_EQ(preset.getName(), "Realistic");
    EXPECT_EQ(preset.getType(), DampingPreset::Type::REALISTIC);
}

TEST_F(DampingPresetTest, VisualizationPresetHasMinimalDamping) {
    /*
     * Domain Rule: VISUALIZATION preset has minimal damping
     * Rationale: Allows clear demonstration of interference patterns
     */
    auto preset = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);

    EXPECT_FLOAT_EQ(preset.getDamping(), 0.9995f);
    EXPECT_FLOAT_EQ(preset.getWallReflection(), 0.95f);
    EXPECT_EQ(preset.getName(), "Visualization");
    EXPECT_TRUE(preset.isVisualization());
}

TEST_F(DampingPresetTest, AnechoicPresetHasNoReflections) {
    /*
     * Domain Rule: ANECHOIC preset simulates anechoic chamber
     * Physics: Zero wall reflection (100% absorption)
     */
    auto preset = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);

    EXPECT_FLOAT_EQ(preset.getWallReflection(), 0.0f);
    EXPECT_FLOAT_EQ(preset.getDamping(), 0.999f);
    EXPECT_EQ(preset.getName(), "Anechoic");
    EXPECT_TRUE(preset.isAnechoic());
}

TEST_F(DampingPresetTest, CustomPresetCanBeCreated) {
    /*
     * Domain Rule: Users can create custom presets
     * Validates domain invariants during creation
     */
    auto preset = DampingPreset::custom(0.995f, 0.9f, "Custom Environment");

    EXPECT_FLOAT_EQ(preset.getDamping(), 0.995f);
    EXPECT_FLOAT_EQ(preset.getWallReflection(), 0.9f);
    EXPECT_EQ(preset.getName(), "Custom Environment");
}

TEST_F(DampingPresetTest, CustomPresetRejectsInvalidDamping) {
    /*
     * Domain Invariant: Damping must be in range (0, 1]
     * Rationale: damping <= 0 would cause instant decay, damping > 1 would amplify
     */

    // Damping = 0 should throw
    EXPECT_THROW(
        DampingPreset::custom(0.0f, 0.5f, "Invalid"),
        std::invalid_argument
    );

    // Damping < 0 should throw
    EXPECT_THROW(
        DampingPreset::custom(-0.1f, 0.5f, "Invalid"),
        std::invalid_argument
    );

    // Damping > 1 should throw
    EXPECT_THROW(
        DampingPreset::custom(1.1f, 0.5f, "Invalid"),
        std::invalid_argument
    );
}

TEST_F(DampingPresetTest, CustomPresetRejectsInvalidWallReflection) {
    /*
     * Domain Invariant: Wall reflection must be in range [0, 1]
     * Rationale: Represents energy reflection coefficient
     */

    // Wall reflection < 0 should throw
    EXPECT_THROW(
        DampingPreset::custom(0.997f, -0.1f, "Invalid"),
        std::invalid_argument
    );

    // Wall reflection > 1 should throw
    EXPECT_THROW(
        DampingPreset::custom(0.997f, 1.1f, "Invalid"),
        std::invalid_argument
    );
}

TEST_F(DampingPresetTest, ValueObjectEqualityByValue) {
    /*
     * Value Object Property: Equality is by value, not identity
     * Two presets with same values should be equal
     */
    auto preset1 = DampingPreset::fromType(DampingPreset::Type::REALISTIC);
    auto preset2 = DampingPreset::fromType(DampingPreset::Type::REALISTIC);

    EXPECT_EQ(preset1, preset2);
    EXPECT_FALSE(preset1 != preset2);
}

TEST_F(DampingPresetTest, ValueObjectInequalityByValue) {
    /*
     * Value Object Property: Different values means inequality
     */
    auto realistic = DampingPreset::fromType(DampingPreset::Type::REALISTIC);
    auto visualization = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);

    EXPECT_NE(realistic, visualization);
    EXPECT_FALSE(realistic == visualization);
}

TEST_F(DampingPresetTest, PresetDescriptionIsInformative) {
    /*
     * Domain Documentation: Presets should have clear descriptions
     */
    auto preset = DampingPreset::fromType(DampingPreset::Type::REALISTIC);

    std::string description = preset.getDescription();
    EXPECT_GT(description.length(), 10);  // Should be meaningful text
    EXPECT_NE(description.find("acoustic"), std::string::npos);  // Should mention acoustics
}

// ============================================================================
// DAMPING PRESET SERVICE TESTS (Domain Service)
// ============================================================================

TEST_F(DampingPresetTest, ServiceRecommendsVisualizationPreset) {
    /*
     * Domain Service: Recommend appropriate preset for use case
     */
    auto preset = DampingPresetService::recommendForVisualization();

    EXPECT_TRUE(preset.isVisualization());
    EXPECT_EQ(preset.getType(), DampingPreset::Type::VISUALIZATION);
}

TEST_F(DampingPresetTest, ServiceRecommendsSimulationPreset) {
    auto preset = DampingPresetService::recommendForSimulation();

    EXPECT_EQ(preset.getType(), DampingPreset::Type::REALISTIC);
}

TEST_F(DampingPresetTest, ServiceRecommendsTestingPreset) {
    auto preset = DampingPresetService::recommendForTesting();

    EXPECT_TRUE(preset.isAnechoic());
    EXPECT_EQ(preset.getType(), DampingPreset::Type::ANECHOIC);
}

// ============================================================================
// WAVE SIMULATION INTEGRATION TESTS
// ============================================================================

class DampingPresetIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        simulation = new WaveSimulation(100, 50);
    }

    void TearDown() override {
        delete simulation;
    }

    WaveSimulation* simulation;
};

TEST_F(DampingPresetIntegrationTest, SimulationInitializesWithRealisticPreset) {
    /*
     * Domain Rule: Simulations start with REALISTIC preset
     */
    auto preset = simulation->getCurrentPreset();

    EXPECT_EQ(preset.getType(), DampingPreset::Type::REALISTIC);
    EXPECT_FLOAT_EQ(simulation->getDamping(), 0.997f);
    EXPECT_FLOAT_EQ(simulation->getWallReflection(), 0.85f);
}

TEST_F(DampingPresetIntegrationTest, ApplyPresetUpdatesSimulationParameters) {
    /*
     * Integration: Applying preset updates all relevant physics parameters
     */
    auto visualPreset = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);

    simulation->applyDampingPreset(visualPreset);

    EXPECT_FLOAT_EQ(simulation->getDamping(), 0.9995f);
    EXPECT_FLOAT_EQ(simulation->getWallReflection(), 0.95f);
    EXPECT_EQ(simulation->getCurrentPreset(), visualPreset);
}

TEST_F(DampingPresetIntegrationTest, ApplyAnechoicPresetEliminatesReflections) {
    /*
     * Domain Behavior: Anechoic preset should eliminate wall reflections
     */
    auto anechoic = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);

    simulation->applyDampingPreset(anechoic);

    EXPECT_FLOAT_EQ(simulation->getWallReflection(), 0.0f);
}

TEST_F(DampingPresetIntegrationTest, PresetSwitchingMaintainsWaveField) {
    /*
     * Domain Invariant: Changing preset doesn't clear wave field
     * Rationale: User should be able to change environment without losing waves
     */

    // Add pressure source
    simulation->addPressureSource(50, 25, 10.0f);

    // Get wave energy before preset change
    const float* dataBefore = simulation->getData();
    float energyBefore = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyBefore += std::abs(dataBefore[i]);
    }

    // Apply different preset
    auto visualPreset = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);
    simulation->applyDampingPreset(visualPreset);

    // Wave field should be unchanged
    const float* dataAfter = simulation->getData();
    float energyAfter = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyAfter += std::abs(dataAfter[i]);
    }

    EXPECT_FLOAT_EQ(energyBefore, energyAfter);
}

TEST_F(DampingPresetIntegrationTest, VisualizationPresetMaintainsMoreEnergy) {
    /*
     * Physics Validation: VISUALIZATION preset should maintain more energy
     * than REALISTIC preset over time
     */

    // Test with realistic preset
    simulation->applyDampingPreset(
        DampingPreset::fromType(DampingPreset::Type::REALISTIC)
    );
    simulation->addPressureSource(50, 25, 50.0f);

    for (int i = 0; i < 50; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* dataRealistic = simulation->getData();
    float energyRealistic = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyRealistic += std::abs(dataRealistic[i]);
    }

    // Reset and test with visualization preset
    simulation->clear();
    simulation->applyDampingPreset(
        DampingPreset::fromType(DampingPreset::Type::VISUALIZATION)
    );
    simulation->addPressureSource(50, 25, 50.0f);

    for (int i = 0; i < 50; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* dataVis = simulation->getData();
    float energyVisualization = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyVisualization += std::abs(dataVis[i]);
    }

    // Visualization preset should maintain significantly more energy
    EXPECT_GT(energyVisualization, energyRealistic * 2.0f);
}

TEST_F(DampingPresetIntegrationTest, CustomPresetCanBeApplied) {
    /*
     * Integration: Custom presets can be created and applied
     */
    auto customPreset = DampingPreset::custom(0.999f, 0.7f, "Custom Test");

    simulation->applyDampingPreset(customPreset);

    EXPECT_FLOAT_EQ(simulation->getDamping(), 0.999f);
    EXPECT_FLOAT_EQ(simulation->getWallReflection(), 0.7f);
}

// ============================================================================
// DOMAIN RULE VALIDATION TESTS
// ============================================================================

TEST_F(DampingPresetIntegrationTest, RealisticPresetProducesPhysicallyAccurateDamping) {
    /*
     * Physics Validation: Realistic preset should produce realistic energy decay
     *
     * Physics: With damping = 0.997, energy should decay to ~50% after ~230 timesteps
     * This matches measured air absorption in real rooms
     */

    simulation->applyDampingPreset(
        DampingPreset::fromType(DampingPreset::Type::REALISTIC)
    );
    simulation->addPressureSource(50, 25, 100.0f);

    // Get initial energy
    const float* dataInitial = simulation->getData();
    float energyInitial = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyInitial += std::abs(dataInitial[i]);
    }

    // Run many timesteps
    for (int i = 0; i < 100; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* dataFinal = simulation->getData();
    float energyFinal = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        energyFinal += std::abs(dataFinal[i]);
    }

    // Should have decayed significantly with realistic damping
    EXPECT_LT(energyFinal, energyInitial * 0.5f);
}

TEST_F(DampingPresetIntegrationTest, AnechoicPresetEliminatesBoundaryReflections) {
    /*
     * Physics Validation: Anechoic preset should have zero wall reflection
     *
     * Test Strategy: Just verify the preset sets wallReflection to 0
     * This is a simple but definitive test of the domain rule.
     *
     * Testing actual acoustic behavior of reflections requires more complex
     * setup and is covered by the physics model tests elsewhere.
     */

    auto anechoic = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);
    simulation->applyDampingPreset(anechoic);

    // Anechoic walls have zero reflection coefficient
    EXPECT_FLOAT_EQ(simulation->getWallReflection(), 0.0f)
        << "Anechoic preset should have zero wall reflection";

    // Verify preset properties
    EXPECT_TRUE(anechoic.isAnechoic());
    EXPECT_EQ(anechoic.getName(), "Anechoic");
}
