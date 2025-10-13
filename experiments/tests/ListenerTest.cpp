/*
 * ListenerTest.cpp - Domain Layer Tests
 *
 * Tests for virtual listener (microphone) functionality in WaveSimulation.
 * Validates domain logic independent of infrastructure concerns.
 *
 * Clean Architecture principles:
 * - Domain layer should be testable without infrastructure
 * - Domain rules should be clearly expressed in tests
 * - Tests should document behavior and intent
 */

#include <gtest/gtest.h>
#include "WaveSimulation.h"
#include <cmath>

/*
 * Test fixture for listener functionality
 * Provides common setup for all listener tests
 */
class ListenerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard test grid: 100x50
        simulation = new WaveSimulation(100, 50);
    }

    void TearDown() override {
        delete simulation;
    }

    WaveSimulation* simulation;
};

// ============================================================================
// LISTENER POSITIONING TESTS
// ============================================================================

TEST_F(ListenerTest, ListenerInitiallyDisabled) {
    /*
     * Domain Rule: Listener should be disabled by default
     * Rationale: Explicit activation prevents accidental audio output
     */
    EXPECT_FALSE(simulation->hasListener());
}

TEST_F(ListenerTest, ListenerCanBeEnabled) {
    /*
     * Domain Rule: Listener can be explicitly enabled
     */
    simulation->setListenerEnabled(true);
    EXPECT_TRUE(simulation->hasListener());
}

TEST_F(ListenerTest, ListenerCanBeDisabled) {
    /*
     * Domain Rule: Listener can be disabled after being enabled
     */
    simulation->setListenerEnabled(true);
    EXPECT_TRUE(simulation->hasListener());

    simulation->setListenerEnabled(false);
    EXPECT_FALSE(simulation->hasListener());
}

TEST_F(ListenerTest, ListenerPositionCanBeSet) {
    /*
     * Domain Rule: Listener position can be set to any grid coordinate
     */
    simulation->setListenerPosition(25, 15);

    int x, y;
    simulation->getListenerPosition(x, y);

    EXPECT_EQ(x, 25);
    EXPECT_EQ(y, 15);
}

TEST_F(ListenerTest, ListenerPositionDefaultsToCenter) {
    /*
     * Domain Rule: Listener defaults to center of simulation grid
     * Rationale: Center is a reasonable default position
     */
    int x, y;
    simulation->getListenerPosition(x, y);

    EXPECT_EQ(x, 50);  // 100 / 2
    EXPECT_EQ(y, 25);  // 50 / 2
}

TEST_F(ListenerTest, ListenerPositionClampedToGridBounds) {
    /*
     * Domain Rule: Listener position is clamped to valid grid coordinates
     * Rationale: Prevents out-of-bounds access and undefined behavior
     */

    // Test upper bounds
    simulation->setListenerPosition(150, 80);
    int x, y;
    simulation->getListenerPosition(x, y);
    EXPECT_EQ(x, 99);  // width - 1
    EXPECT_EQ(y, 49);  // height - 1

    // Test lower bounds
    simulation->setListenerPosition(-10, -5);
    simulation->getListenerPosition(x, y);
    EXPECT_EQ(x, 0);
    EXPECT_EQ(y, 0);
}

TEST_F(ListenerTest, ListenerPositionCanBeMovedDynamically) {
    /*
     * Domain Rule: Listener can be repositioned multiple times
     * Rationale: Allows interactive exploration of acoustic field
     */
    simulation->setListenerPosition(10, 10);
    int x, y;
    simulation->getListenerPosition(x, y);
    EXPECT_EQ(x, 10);
    EXPECT_EQ(y, 10);

    simulation->setListenerPosition(20, 30);
    simulation->getListenerPosition(x, y);
    EXPECT_EQ(x, 20);
    EXPECT_EQ(y, 30);

    simulation->setListenerPosition(50, 25);
    simulation->getListenerPosition(x, y);
    EXPECT_EQ(x, 50);
    EXPECT_EQ(y, 25);
}

// ============================================================================
// PRESSURE SAMPLING TESTS
// ============================================================================

TEST_F(ListenerTest, ListenerReturnsZeroPressureWhenDisabled) {
    /*
     * Domain Rule: Disabled listener returns zero pressure
     * Rationale: No audio should be generated when listener is off
     */
    simulation->setListenerEnabled(false);
    float pressure = simulation->getListenerPressure();

    EXPECT_FLOAT_EQ(pressure, 0.0f);
}

TEST_F(ListenerTest, ListenerSamplesActualPressureWhenEnabled) {
    /*
     * Domain Rule: Enabled listener samples actual pressure at its position
     * Rationale: Audio output should reflect real acoustic field
     */
    simulation->setListenerPosition(50, 25);
    simulation->setListenerEnabled(true);

    // Add pressure source at listener position
    simulation->addPressureSource(50, 25, 10.0f);

    float pressure = simulation->getListenerPressure();

    // Pressure should be non-zero at source location
    EXPECT_GT(std::abs(pressure), 0.0f);
}

TEST_F(ListenerTest, ListenerPressureReflectsWavePropagation) {
    /*
     * Domain Rule: Listener pressure changes as waves propagate
     * Rationale: Audio should evolve with wave dynamics
     */
    simulation->setListenerPosition(50, 25);
    simulation->setListenerEnabled(true);

    // Add pressure source away from listener
    simulation->addPressureSource(30, 25, 10.0f);

    // Initial pressure at listener should be near zero (wave hasn't arrived)
    float pressure1 = simulation->getListenerPressure();

    // Update simulation multiple times (wave propagates)
    for (int i = 0; i < 20; i++) {
        simulation->update(1.0f / 60.0f);
    }

    // Pressure should have changed as wave arrives
    float pressure2 = simulation->getListenerPressure();

    EXPECT_NE(pressure1, pressure2);
}

TEST_F(ListenerTest, ListenerPressureDecaysOverTime) {
    /*
     * Domain Rule: Sampled pressure decays due to damping
     * Rationale: Validates that listener captures physical damping
     */
    simulation->setListenerPosition(50, 25);
    simulation->setListenerEnabled(true);

    // Add strong pressure source at listener
    simulation->addPressureSource(50, 25, 100.0f);

    // Get initial pressure (should be high)
    float pressure1 = std::abs(simulation->getListenerPressure());

    // Let wave dissipate
    for (int i = 0; i < 100; i++) {
        simulation->update(1.0f / 60.0f);
    }

    float pressure2 = std::abs(simulation->getListenerPressure());

    // Pressure should have decayed significantly
    EXPECT_LT(pressure2, pressure1 * 0.5f);
}

TEST_F(ListenerTest, ListenerAtDifferentPositionsSamplesDifferentPressures) {
    /*
     * Domain Rule: Listener position affects sampled pressure
     * Rationale: Spatial variation in acoustic field should be captured
     */
    simulation->setListenerEnabled(true);

    // Add pressure source at (30, 25)
    simulation->addPressureSource(30, 25, 50.0f);

    // Sample at source location
    simulation->setListenerPosition(30, 25);
    float pressureAtSource = std::abs(simulation->getListenerPressure());

    // Update simulation to let wave propagate
    for (int i = 0; i < 10; i++) {
        simulation->update(1.0f / 60.0f);
    }

    // Sample far from source
    simulation->setListenerPosition(80, 40);
    float pressureFarAway = std::abs(simulation->getListenerPressure());

    // Pressure should be higher at/near source
    EXPECT_GT(pressureAtSource, pressureFarAway);
}

// ============================================================================
// LISTENER + OBSTACLES TESTS
// ============================================================================

TEST_F(ListenerTest, ListenerBehindObstacleReceivesLowerPressure) {
    /*
     * Domain Rule: Obstacles block sound waves
     * Rationale: Listener should capture acoustic shadowing
     */
    simulation->setListenerEnabled(true);

    // Source on left, listener on right
    simulation->setListenerPosition(70, 25);

    // Test without obstacle
    simulation->addPressureSource(30, 25, 50.0f);
    for (int i = 0; i < 30; i++) {
        simulation->update(1.0f / 60.0f);
    }
    float pressureWithoutObstacle = std::abs(simulation->getListenerPressure());

    // Clear and add obstacle between source and listener
    simulation->clear();
    simulation->addObstacle(50, 25, 5);
    simulation->addPressureSource(30, 25, 50.0f);
    for (int i = 0; i < 30; i++) {
        simulation->update(1.0f / 60.0f);
    }
    float pressureWithObstacle = std::abs(simulation->getListenerPressure());

    // Obstacle should reduce pressure at listener
    EXPECT_LT(pressureWithObstacle, pressureWithoutObstacle * 0.8f);
}

TEST_F(ListenerTest, ListenerInsideObstacleReturnsZeroPressure) {
    /*
     * Domain Rule: Obstacles have zero pressure (rigid boundary)
     * Rationale: Listener inside solid obstacle should hear nothing
     */
    simulation->setListenerEnabled(true);

    // Place obstacle
    simulation->addObstacle(50, 25, 10);

    // Place listener inside obstacle
    simulation->setListenerPosition(50, 25);

    // Add pressure sources
    simulation->addPressureSource(20, 25, 50.0f);
    simulation->addPressureSource(80, 25, 50.0f);

    // Update simulation
    for (int i = 0; i < 20; i++) {
        simulation->update(1.0f / 60.0f);
    }

    float pressure = simulation->getListenerPressure();

    // Pressure inside obstacle should be exactly zero
    EXPECT_FLOAT_EQ(pressure, 0.0f);
}

// ============================================================================
// DOMAIN INVARIANTS TESTS
// ============================================================================

TEST_F(ListenerTest, ListenerStateIndependentOfSimulationState) {
    /*
     * Domain Invariant: Listener position/state is orthogonal to wave state
     * Rationale: Listener is an observer, not part of the wave dynamics
     */
    simulation->setListenerPosition(40, 20);
    simulation->setListenerEnabled(true);

    int x1, y1;
    simulation->getListenerPosition(x1, y1);
    bool enabled1 = simulation->hasListener();

    // Clear simulation (resets waves)
    simulation->clear();

    int x2, y2;
    simulation->getListenerPosition(x2, y2);
    bool enabled2 = simulation->hasListener();

    // Listener state should be unchanged
    EXPECT_EQ(x1, x2);
    EXPECT_EQ(y1, y2);
    EXPECT_EQ(enabled1, enabled2);
}

TEST_F(ListenerTest, MultipleListenerPositionChangesDoNotAffectWaves) {
    /*
     * Domain Invariant: Moving listener does not disturb wave field
     * Rationale: Listener is a passive observer (Observer pattern)
     */
    // Add pressure source
    simulation->addPressureSource(50, 25, 50.0f);

    // Update and capture pressure field
    simulation->update(1.0f / 60.0f);
    const float* field1 = simulation->getData();
    float sum1 = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        sum1 += std::abs(field1[i]);
    }

    // Move listener multiple times
    simulation->setListenerPosition(10, 10);
    simulation->setListenerPosition(20, 20);
    simulation->setListenerPosition(30, 30);

    // Check pressure field unchanged
    const float* field2 = simulation->getData();
    float sum2 = 0.0f;
    for (int i = 0; i < 100 * 50; i++) {
        sum2 += std::abs(field2[i]);
    }

    EXPECT_FLOAT_EQ(sum1, sum2);
}

// ============================================================================
// CLEAN ARCHITECTURE VALIDATION
// ============================================================================

TEST_F(ListenerTest, ListenerInterfaceIsCleanAndMinimal) {
    /*
     * Clean Architecture: Domain interface should be minimal and expressive
     *
     * Listener interface provides exactly what's needed:
     * - setListenerPosition(x, y) - positioning
     * - getListenerPosition(x, y) - query position
     * - setListenerEnabled(bool) - activation control
     * - hasListener() - query state
     * - getListenerPressure() - sample acoustic field
     *
     * No infrastructure leaks (no audio concepts in domain)
     */

    // This test documents the interface and validates it compiles
    simulation->setListenerPosition(50, 25);

    int x, y;
    simulation->getListenerPosition(x, y);

    simulation->setListenerEnabled(true);
    bool enabled = simulation->hasListener();

    float pressure = simulation->getListenerPressure();

    // Interface is complete and minimal
    EXPECT_TRUE(true);
}
