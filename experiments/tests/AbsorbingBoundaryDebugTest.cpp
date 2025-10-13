/*
 * AbsorbingBoundaryDebugTest.cpp - Debug test for absorbing boundary triggering
 *
 * This test explicitly verifies that the absorbing boundary code path
 * is being executed when wallReflection < 0.1
 */

#include <gtest/gtest.h>

#include <cmath>
#include <iostream>

#include "DampingPreset.h"
#include "WaveSimulation.h"

TEST(AbsorbingBoundaryDebugTest, AnechoicPresetTriggersAbsorbingBoundary) {
    /*
     * Debug Test: Verify that anechoic preset wallReflection < 0.1
     * which should trigger the absorbing boundary code path
     */

    WaveSimulation sim(100, 50);

    // Apply anechoic preset
    auto anechoic = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);
    sim.applyDampingPreset(anechoic);

    // Verify wallReflection is 0.0 (should trigger absorbingWalls = true)
    EXPECT_FLOAT_EQ(sim.getWallReflection(), 0.0f);

    // Verify condition: wallReflection < 0.1f should be TRUE
    EXPECT_TRUE(sim.getWallReflection() < 0.1f)
        << "wallReflection=" << sim.getWallReflection()
        << " should be < 0.1 to trigger absorbing boundaries";

    std::cout << "Anechoic preset wallReflection: " << sim.getWallReflection() << std::endl;
    std::cout << "absorbingWalls condition (wallReflection < 0.1): "
              << (sim.getWallReflection() < 0.1f ? "TRUE" : "FALSE") << std::endl;
}

TEST(AbsorbingBoundaryDebugTest, VisualizationPresetTriggersReflectiveBoundary) {
    /*
     * Debug Test: Verify that visualization preset wallReflection >= 0.1
     * which should trigger the reflective boundary code path
     */

    WaveSimulation sim(100, 50);

    // Apply visualization preset
    auto viz = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);
    sim.applyDampingPreset(viz);

    // Verify wallReflection is 0.98 (should trigger absorbingWalls = false)
    EXPECT_FLOAT_EQ(sim.getWallReflection(), 0.98f);

    // Verify condition: wallReflection >= 0.1f should be TRUE
    EXPECT_TRUE(sim.getWallReflection() >= 0.1f)
        << "wallReflection=" << sim.getWallReflection()
        << " should be >= 0.1 to trigger reflective boundaries";

    std::cout << "Visualization preset wallReflection: " << sim.getWallReflection() << std::endl;
    std::cout << "absorbingWalls condition (wallReflection < 0.1): "
              << (sim.getWallReflection() < 0.1f ? "TRUE" : "FALSE") << std::endl;
}

TEST(AbsorbingBoundaryDebugTest, AbsorbingBoundaryZeroesBoundaryValues) {
    /*
     * Debug Test: Verify that absorbing boundaries set boundary cells to zero
     */

    WaveSimulation sim(100, 50);

    // Apply anechoic preset
    auto anechoic = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);
    sim.applyDampingPreset(anechoic);

    // Add pressure in center
    sim.addPressureSource(50, 25, 100.0f);

    // Run a few steps
    for (int i = 0; i < 5; i++) {
        sim.update(1.0f / 60.0f);
    }

    // Check that boundary values are near zero (or heavily damped)
    const float* data = sim.getData();

    // Check all four boundaries
    float topBoundarySum = 0.0f;
    float bottomBoundarySum = 0.0f;
    for (int x = 0; x < 100; x++) {
        topBoundarySum += std::abs(data[x]);                // Top row
        bottomBoundarySum += std::abs(data[49 * 100 + x]);  // Bottom row
    }

    float leftBoundarySum = 0.0f;
    float rightBoundarySum = 0.0f;
    for (int y = 0; y < 50; y++) {
        leftBoundarySum += std::abs(data[y * 100 + 0]);    // Left column
        rightBoundarySum += std::abs(data[y * 100 + 99]);  // Right column
    }

    std::cout << "Boundary sums (should be very small for absorbing):" << std::endl;
    std::cout << "  Top: " << topBoundarySum << std::endl;
    std::cout << "  Bottom: " << bottomBoundarySum << std::endl;
    std::cout << "  Left: " << leftBoundarySum << std::endl;
    std::cout << "  Right: " << rightBoundarySum << std::endl;

    // With absorbing boundaries, these should be very small (< 5) compared to reflective (> 100)
    // One-way wave equation BC doesn't set boundaries to zero; it allows outward propagation
    EXPECT_LT(topBoundarySum, 5.0f) << "Top boundary should have very low energy";
    EXPECT_LT(bottomBoundarySum, 5.0f) << "Bottom boundary should have very low energy";
    EXPECT_LT(leftBoundarySum, 5.0f) << "Left boundary should have very low energy";
    EXPECT_LT(rightBoundarySum, 5.0f) << "Right boundary should have very low energy";
}

TEST(AbsorbingBoundaryDebugTest, ReflectiveBoundaryMaintainsBoundaryValues) {
    /*
     * Debug Test: Verify that reflective boundaries maintain non-zero values
     */

    WaveSimulation sim(100, 50);

    // Apply visualization preset (reflective)
    auto viz = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);
    sim.applyDampingPreset(viz);

    // Add pressure in center
    sim.addPressureSource(50, 25, 100.0f);

    // Run many steps to let waves reach boundaries
    for (int i = 0; i < 50; i++) {
        sim.update(1.0f / 60.0f);
    }

    // Check that boundary values are NON-ZERO (waves reflecting)
    const float* data = sim.getData();

    // Check all four boundaries
    float topBoundarySum = 0.0f;
    float bottomBoundarySum = 0.0f;
    for (int x = 0; x < 100; x++) {
        topBoundarySum += std::abs(data[x]);                // Top row
        bottomBoundarySum += std::abs(data[49 * 100 + x]);  // Bottom row
    }

    float leftBoundarySum = 0.0f;
    float rightBoundarySum = 0.0f;
    for (int y = 0; y < 50; y++) {
        leftBoundarySum += std::abs(data[y * 100 + 0]);    // Left column
        rightBoundarySum += std::abs(data[y * 100 + 99]);  // Right column
    }

    std::cout << "Boundary sums (should be significant for reflective):" << std::endl;
    std::cout << "  Top: " << topBoundarySum << std::endl;
    std::cout << "  Bottom: " << bottomBoundarySum << std::endl;
    std::cout << "  Left: " << leftBoundarySum << std::endl;
    std::cout << "  Right: " << rightBoundarySum << std::endl;

    // With reflective boundaries, at least one should have significant energy
    float totalBoundaryEnergy =
        topBoundarySum + bottomBoundarySum + leftBoundarySum + rightBoundarySum;
    EXPECT_GT(totalBoundaryEnergy, 5.0f) << "Reflective boundaries should have significant energy";
}
