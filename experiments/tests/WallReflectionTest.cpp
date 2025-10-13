/*
 * WallReflectionTest.cpp - Verify wall reflection differences
 *
 * This test verifies that different wall reflection coefficients
 * produce measurably different results.
 */

#include <gtest/gtest.h>
#include "WaveSimulation.h"
#include "DampingPreset.h"
#include <cmath>
#include <iostream>

TEST(WallReflectionTest, AnechoicVsReflectiveShowsDifference) {
    /*
     * Test Strategy: Place source near left wall, measure energy
     * after wave hits wall and returns.
     *
     * Expected: Reflective walls return energy, anechoic walls absorb it
     */

    // Test 1: Reflective walls (Visualization preset - 95% reflection)
    WaveSimulation simReflective(100, 50);
    simReflective.applyDampingPreset(
        DampingPreset::fromType(DampingPreset::Type::VISUALIZATION)
    );

    // Place source 10 pixels from left wall
    simReflective.addPressureSource(10, 25, 50.0f);

    // Let wave travel to wall and back (enough time for round trip)
    for (int i = 0; i < 60; i++) {
        simReflective.update(1.0f / 60.0f);
    }

    // Measure energy near source (where reflected wave returns)
    const float* dataReflective = simReflective.getData();
    float energyReflectiveNearSource = 0.0f;
    for (int y = 20; y < 30; y++) {
        for (int x = 5; x < 15; x++) {
            energyReflectiveNearSource += std::abs(dataReflective[y * 100 + x]);
        }
    }

    // Test 2: Anechoic walls (0% reflection)
    WaveSimulation simAnechoic(100, 50);
    simAnechoic.applyDampingPreset(
        DampingPreset::fromType(DampingPreset::Type::ANECHOIC)
    );

    // Same source position
    simAnechoic.addPressureSource(10, 25, 50.0f);

    // Same propagation time
    for (int i = 0; i < 60; i++) {
        simAnechoic.update(1.0f / 60.0f);
    }

    // Measure energy near source
    const float* dataAnechoic = simAnechoic.getData();
    float energyAnechoicNearSource = 0.0f;
    for (int y = 20; y < 30; y++) {
        for (int x = 5; x < 15; x++) {
            energyAnechoicNearSource += std::abs(dataAnechoic[y * 100 + x]);
        }
    }

    std::cout << "Reflective energy near source: " << energyReflectiveNearSource << std::endl;
    std::cout << "Anechoic energy near source: " << energyAnechoicNearSource << std::endl;
    std::cout << "Ratio (Reflective/Anechoic): " << (energyReflectiveNearSource / energyAnechoicNearSource) << std::endl;

    // Reflective should have MORE energy near source due to reflected waves
    EXPECT_GT(energyReflectiveNearSource, energyAnechoicNearSource * 1.1f)
        << "Reflective walls should return energy to source area, "
        << "anechoic walls should absorb it";
}

TEST(WallReflectionTest, AnechoicAbsorbsAtBoundary) {
    /*
     * Direct test: Verify anechoic preset sets wallReflection=0
     */
    WaveSimulation sim(100, 50);

    auto anechoic = DampingPreset::fromType(DampingPreset::Type::ANECHOIC);
    sim.applyDampingPreset(anechoic);

    EXPECT_FLOAT_EQ(sim.getWallReflection(), 0.0f)
        << "Anechoic preset must have zero wall reflection";
}

TEST(WallReflectionTest, VisualizationHasHighReflection) {
    /*
     * Direct test: Verify visualization preset has high wall reflection
     */
    WaveSimulation sim(100, 50);

    auto viz = DampingPreset::fromType(DampingPreset::Type::VISUALIZATION);
    sim.applyDampingPreset(viz);

    EXPECT_FLOAT_EQ(sim.getWallReflection(), 0.98f)
        << "Visualization preset must have 98% wall reflection";
}
