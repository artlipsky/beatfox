/*
 * InterferenceTest.cpp - Wave Interference Validation
 *
 * Tests to verify that waves properly superpose (interfere).
 * This validates that our wave equation implementation correctly
 * handles constructive and destructive interference.
 */

#include <gtest/gtest.h>
#include "WaveSimulation.h"
#include <cmath>

class InterferenceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use a larger grid for interference patterns
        simulation = new WaveSimulation(200, 100);
    }

    void TearDown() override {
        delete simulation;
    }

    WaveSimulation* simulation;
};

TEST_F(InterferenceTest, TwoWavesConstructiveInterference) {
    /*
     * Test constructive interference:
     * Two waves with SAME phase should ADD together
     *
     * Physics: If wave1 has amplitude A and wave2 has amplitude A,
     * then at overlap: total amplitude = 2A (constructive)
     */

    // Add two pressure sources at the same location with same amplitude
    const float amplitude = 10.0f;
    simulation->addPressureSource(100, 50, amplitude);
    simulation->addPressureSource(100, 50, amplitude);

    // Check that pressure at center is approximately 2x amplitude
    const float* data = simulation->getData();
    int centerIdx = 50 * simulation->getWidth() + 100;
    float centerPressure = std::abs(data[centerIdx]);

    // Should be close to 2x amplitude (constructive interference)
    EXPECT_GT(centerPressure, 1.5f * amplitude)
        << "Constructive interference should produce ~2x amplitude";
    EXPECT_LT(centerPressure, 2.5f * amplitude)
        << "Pressure shouldn't exceed 2x amplitude significantly";
}

TEST_F(InterferenceTest, TwoWavesDestructiveInterference) {
    /*
     * Test destructive interference:
     * Two waves with OPPOSITE phase should CANCEL
     *
     * Physics: If wave1 = +A and wave2 = -A at same point,
     * then total = 0 (destructive interference)
     */

    // Add two pressure sources at the same location with opposite amplitudes
    const float amplitude = 10.0f;
    simulation->addPressureSource(100, 50, amplitude);
    simulation->addPressureSource(100, 50, -amplitude);

    // Check that pressure at center is nearly zero
    const float* data = simulation->getData();
    int centerIdx = 50 * simulation->getWidth() + 100;
    float centerPressure = std::abs(data[centerIdx]);

    // Should be close to zero (destructive interference)
    EXPECT_LT(centerPressure, 1.0f)
        << "Destructive interference should produce near-zero amplitude";
}

TEST_F(InterferenceTest, SuperpositionAfterPropagation) {
    /*
     * Test superposition after waves propagate:
     * Create two separate waves close together, verify they interfere
     *
     * Note: With realistic damping (0.997), waves decay relatively quickly.
     * This is CORRECT physics - air absorbs sound energy.
     */

    // Set lower damping for this test to see propagation
    simulation->setDamping(0.9995f);  // Much lower absorption

    // Create two pressure sources closer together
    const float amplitude = 15.0f;
    simulation->addPressureSource(80, 50, amplitude);   // Left source
    simulation->addPressureSource(120, 50, amplitude);  // Right source

    // Let waves propagate (fewer steps needed with lower damping)
    for (int i = 0; i < 15; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* data = simulation->getData();

    // Check the midpoint between sources (should have clear interference)
    int midIdx = 50 * simulation->getWidth() + 100;
    float midPressure = std::abs(data[midIdx]);

    // With lower damping and closer sources, waves should clearly interfere
    EXPECT_GT(midPressure, 0.5f)
        << "Waves should have propagated to midpoint and interfered";

    // Verify simulation maintains physical behavior
    float totalEnergy = 0.0f;
    for (int i = 0; i < simulation->getWidth() * simulation->getHeight(); i++) {
        totalEnergy += std::abs(data[i]);
    }
    EXPECT_GT(totalEnergy, 5.0f)
        << "Waves should maintain significant energy with low damping";
}

TEST_F(InterferenceTest, StandingWavePattern) {
    /*
     * Test standing wave formation:
     * Continuous sources at same frequency should create standing waves
     *
     * This is a classic interference pattern - demonstrates that
     * our simulation handles continuous interference correctly.
     */

    // Create two sources at opposite ends
    const float amplitude = 5.0f;
    const int numFrames = 50;

    for (int frame = 0; frame < numFrames; frame++) {
        // Add pulses at regular intervals (simulating continuous sources)
        if (frame % 5 == 0) {
            simulation->addPressureSource(40, 50, amplitude);
            simulation->addPressureSource(160, 50, amplitude);
        }
        simulation->update(1.0f / 60.0f);
    }

    const float* data = simulation->getData();

    // Check that interference pattern exists along the line between sources
    // We should see variation in amplitude (standing wave nodes and antinodes)
    float minPressure = 1000.0f;
    float maxPressure = 0.0f;

    for (int x = 60; x < 140; x++) {
        int idx = 50 * simulation->getWidth() + x;
        float pressure = std::abs(data[idx]);
        minPressure = std::min(minPressure, pressure);
        maxPressure = std::max(maxPressure, pressure);
    }

    // Standing waves should show clear amplitude variation
    EXPECT_GT(maxPressure, 2.0f * minPressure)
        << "Standing waves should show amplitude variation (nodes and antinodes)";
}

TEST_F(InterferenceTest, LinearSuperpositionProperty) {
    /*
     * Test linearity: p(source1 + source2) = p(source1) + p(source2)
     *
     * This is a fundamental property of linear wave equations.
     * If this fails, our physics model is incorrect.
     */

    const float amp1 = 7.0f;
    const float amp2 = 3.0f;

    // Test 1: Add both sources together
    simulation->addPressureSource(100, 50, amp1);
    simulation->addPressureSource(100, 50, amp2);

    simulation->update(1.0f / 60.0f);
    const float* dataBoth = simulation->getData();
    int centerIdx = 50 * simulation->getWidth() + 100;
    float pressureBoth = dataBoth[centerIdx];

    // Test 2: Add sources separately
    simulation->clear();
    simulation->addPressureSource(100, 50, amp1);
    simulation->update(1.0f / 60.0f);
    const float* data1 = simulation->getData();
    float pressure1 = data1[centerIdx];

    simulation->clear();
    simulation->addPressureSource(100, 50, amp2);
    simulation->update(1.0f / 60.0f);
    const float* data2 = simulation->getData();
    float pressure2 = data2[centerIdx];

    // Verify linearity: p(a+b) ≈ p(a) + p(b)
    float expectedSum = pressure1 + pressure2;
    float tolerance = 0.5f;  // Allow small numerical error

    EXPECT_NEAR(pressureBoth, expectedSum, tolerance)
        << "Linear superposition principle should hold";
}

TEST_F(InterferenceTest, WavesDontDissipateInstantly) {
    /*
     * Verify waves maintain coherence as they propagate
     * (they don't just vanish instantly)
     *
     * With realistic damping, energy decreases exponentially.
     * This is CORRECT - air absorbs sound energy over distance.
     */

    // Use reduced damping to observe propagation clearly
    simulation->setDamping(0.9995f);

    simulation->addPressureSource(80, 50, 20.0f);
    simulation->addPressureSource(120, 50, 20.0f);

    // Early measurement
    for (int i = 0; i < 10; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* dataEarly = simulation->getData();
    float energyEarly = 0.0f;
    for (int i = 0; i < simulation->getWidth() * simulation->getHeight(); i++) {
        energyEarly += std::abs(dataEarly[i]);
    }

    // Later measurement (waves have met and passed through)
    for (int i = 0; i < 10; i++) {
        simulation->update(1.0f / 60.0f);
    }

    const float* dataLater = simulation->getData();
    float energyLater = 0.0f;
    for (int i = 0; i < simulation->getWidth() * simulation->getHeight(); i++) {
        energyLater += std::abs(dataLater[i]);
    }

    // With reduced damping, energy should decay gradually (not vanish)
    // After interference, waves continue propagating
    EXPECT_GT(energyLater, energyEarly * 0.5f)
        << "With low damping, waves should maintain energy through interference";

    // Verify waves still exist
    EXPECT_GT(energyLater, 10.0f)
        << "Waves should still have significant energy after propagating";
}
