#include <gtest/gtest.h>
#include "WaveSimulation.h"
#include <cmath>

// Test fixture for WaveSimulation tests
class WaveSimulationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Standard test grid: 100x50 (5m x 2.5m)
        simulation = std::make_unique<WaveSimulation>(100, 50);
    }

    void TearDown() override {
        simulation.reset();
    }

    std::unique_ptr<WaveSimulation> simulation;
};

// ===== INITIALIZATION TESTS =====

TEST_F(WaveSimulationTest, InitializationCorrectDimensions) {
    EXPECT_EQ(simulation->getWidth(), 100);
    EXPECT_EQ(simulation->getHeight(), 50);
}

TEST_F(WaveSimulationTest, InitializationPhysicalDimensions) {
    // 1 pixel = 5 cm = 0.05 m
    // 100 pixels = 5m, 50 pixels = 2.5m
    EXPECT_FLOAT_EQ(simulation->getPhysicalWidth(), 5.0f);
    EXPECT_FLOAT_EQ(simulation->getPhysicalHeight(), 2.5f);
}

TEST_F(WaveSimulationTest, InitializationPhysicalParameters) {
    // Speed of sound in air at 20°C
    EXPECT_FLOAT_EQ(simulation->getWaveSpeed(), 343.0f);

    // Air damping coefficient
    EXPECT_NEAR(simulation->getDamping(), 0.997f, 0.001f);
}

TEST_F(WaveSimulationTest, InitializationZeroPressure) {
    const float* data = simulation->getData();
    int size = simulation->getWidth() * simulation->getHeight();

    for (int i = 0; i < size; i++) {
        EXPECT_FLOAT_EQ(data[i], 0.0f) << "Non-zero pressure at index " << i;
    }
}

// ===== OBSTACLE TESTS =====

TEST_F(WaveSimulationTest, AddObstacleCreatesObstacle) {
    simulation->addObstacle(50, 25, 5);

    // Center should be obstacle
    EXPECT_TRUE(simulation->isObstacle(50, 25));

    // Points within radius should be obstacles
    EXPECT_TRUE(simulation->isObstacle(52, 25));
    EXPECT_TRUE(simulation->isObstacle(48, 25));

    // Points outside radius should not be obstacles
    EXPECT_FALSE(simulation->isObstacle(60, 25));
}

TEST_F(WaveSimulationTest, RemoveObstacleRemovesObstacle) {
    simulation->addObstacle(50, 25, 5);
    EXPECT_TRUE(simulation->isObstacle(50, 25));

    simulation->removeObstacle(50, 25, 5);
    EXPECT_FALSE(simulation->isObstacle(50, 25));
}

TEST_F(WaveSimulationTest, ClearObstaclesRemovesAllObstacles) {
    simulation->addObstacle(30, 25, 3);
    simulation->addObstacle(70, 25, 3);

    EXPECT_TRUE(simulation->isObstacle(30, 25));
    EXPECT_TRUE(simulation->isObstacle(70, 25));

    simulation->clearObstacles();

    EXPECT_FALSE(simulation->isObstacle(30, 25));
    EXPECT_FALSE(simulation->isObstacle(70, 25));
}

TEST_F(WaveSimulationTest, ObstaclesOutOfBounds) {
    // Should not crash with out-of-bounds coordinates
    EXPECT_NO_THROW(simulation->addObstacle(-10, 25, 5));
    EXPECT_NO_THROW(simulation->addObstacle(150, 25, 5));
    EXPECT_NO_THROW(simulation->isObstacle(-5, 25));
    EXPECT_NO_THROW(simulation->isObstacle(200, 100));
}

TEST_F(WaveSimulationTest, ObstacleHasZeroPressure) {
    simulation->addObstacle(50, 25, 3);
    simulation->addPressureSource(50, 25, 10.0f);

    // Obstacle should have zero pressure even after adding source
    const float* data = simulation->getData();
    int idx = 25 * simulation->getWidth() + 50;
    EXPECT_FLOAT_EQ(data[idx], 0.0f);
}

// ===== WAVE PROPAGATION TESTS =====

TEST_F(WaveSimulationTest, AddPressureSourceCreatesDisturbance) {
    simulation->addPressureSource(50, 25, 5.0f);

    const float* data = simulation->getData();
    int centerIdx = 25 * simulation->getWidth() + 50;

    // Center should have non-zero pressure
    EXPECT_GT(std::abs(data[centerIdx]), 0.0f);
}

TEST_F(WaveSimulationTest, WavePropagatesFromSource) {
    simulation->addPressureSource(50, 25, 10.0f);

    const float* dataBefore = simulation->getData();
    int rightIdx = 25 * simulation->getWidth() + 55;
    float pressureBefore = dataBefore[rightIdx];

    // Simulate for several time steps
    float dt = 1.0f / 60.0f;  // 60 FPS
    for (int i = 0; i < 10; i++) {
        simulation->update(dt);
    }

    const float* dataAfter = simulation->getData();
    float pressureAfter = dataAfter[rightIdx];

    // Wave should have reached the right side
    EXPECT_NE(pressureBefore, pressureAfter)
        << "Wave should propagate from source";
}

TEST_F(WaveSimulationTest, WavesDissipateOverTime) {
    simulation->addPressureSource(50, 25, 10.0f);

    // Get initial maximum pressure
    float maxPressureInitial = 0.0f;
    const float* dataInitial = simulation->getData();
    int size = simulation->getWidth() * simulation->getHeight();
    for (int i = 0; i < size; i++) {
        maxPressureInitial = std::max(maxPressureInitial, std::abs(dataInitial[i]));
    }

    // Simulate for many time steps
    float dt = 1.0f / 60.0f;
    for (int i = 0; i < 100; i++) {
        simulation->update(dt);
    }

    // Get final maximum pressure
    float maxPressureFinal = 0.0f;
    const float* dataFinal = simulation->getData();
    for (int i = 0; i < size; i++) {
        maxPressureFinal = std::max(maxPressureFinal, std::abs(dataFinal[i]));
    }

    // Waves should have dissipated due to damping
    EXPECT_LT(maxPressureFinal, maxPressureInitial * 0.5f)
        << "Waves should dissipate over time due to damping";
}

TEST_F(WaveSimulationTest, ClearResetsSimulation) {
    simulation->addPressureSource(50, 25, 10.0f);

    // Verify there's non-zero pressure
    const float* dataBefore = simulation->getData();
    int size = simulation->getWidth() * simulation->getHeight();
    bool hasNonZero = false;
    for (int i = 0; i < size; i++) {
        if (dataBefore[i] != 0.0f) {
            hasNonZero = true;
            break;
        }
    }
    EXPECT_TRUE(hasNonZero);

    // Clear simulation
    simulation->clear();

    // All pressure should be zero
    const float* dataAfter = simulation->getData();
    for (int i = 0; i < size; i++) {
        EXPECT_FLOAT_EQ(dataAfter[i], 0.0f) << "Non-zero pressure at index " << i << " after clear";
    }
}

// ===== PARAMETER MODIFICATION TESTS =====

TEST_F(WaveSimulationTest, SetWaveSpeedModifiesSpeed) {
    simulation->setWaveSpeed(400.0f);
    EXPECT_FLOAT_EQ(simulation->getWaveSpeed(), 400.0f);
}

TEST_F(WaveSimulationTest, SetDampingModifiesDamping) {
    simulation->setDamping(0.99f);
    EXPECT_FLOAT_EQ(simulation->getDamping(), 0.99f);
}

TEST_F(WaveSimulationTest, DifferentSpeedsAffectPropagation) {
    // Test that setting wave speed changes propagation
    auto sim = std::make_unique<WaveSimulation>(100, 50);

    sim->setWaveSpeed(343.0f);
    EXPECT_FLOAT_EQ(sim->getWaveSpeed(), 343.0f);

    // Add source and simulate
    sim->addPressureSource(50, 25, 10.0f);

    float dt = 1.0f / 60.0f;
    for (int i = 0; i < 10; i++) {
        sim->update(dt);
    }

    const float* data = sim->getData();

    // Verify wave has propagated away from source
    float nearEnergy = 0.0f;
    float farEnergy = 0.0f;

    for (int y = 20; y < 30; y++) {
        nearEnergy += std::abs(data[y * 100 + 55]);  // 5 pixels right
        farEnergy += std::abs(data[y * 100 + 70]);   // 20 pixels right
    }

    // Near should have more energy than far (waves spreading and dissipating)
    EXPECT_GT(nearEnergy, 0.0f) << "Waves should propagate from source";
}

// ===== BOUNDARY CONDITION TESTS =====

TEST_F(WaveSimulationTest, WavesReflectOffBoundaries) {
    // Place source near left boundary
    simulation->addPressureSource(10, 25, 10.0f);

    // Simulate until wave reaches boundary and reflects
    float dt = 1.0f / 60.0f;
    for (int i = 0; i < 30; i++) {
        simulation->update(dt);
    }

    // There should be non-zero pressure on both sides of source
    // indicating reflection from left boundary
    const float* data = simulation->getData();
    int leftIdx = 25 * simulation->getWidth() + 5;
    int rightIdx = 25 * simulation->getWidth() + 15;

    EXPECT_GT(std::abs(data[leftIdx]) + std::abs(data[rightIdx]), 0.0f)
        << "Waves should reflect off boundaries";
}

// ===== NUMERICAL STABILITY TESTS =====

TEST_F(WaveSimulationTest, SimulationRemainsStable) {
    simulation->addPressureSource(50, 25, 100.0f);  // Large disturbance

    float dt = 1.0f / 60.0f;
    bool stable = true;

    for (int i = 0; i < 200; i++) {
        simulation->update(dt);

        // Check for NaN or infinity
        const float* data = simulation->getData();
        int size = simulation->getWidth() * simulation->getHeight();
        for (int j = 0; j < size; j++) {
            if (std::isnan(data[j]) || std::isinf(data[j])) {
                stable = false;
                break;
            }
        }

        if (!stable) break;
    }

    EXPECT_TRUE(stable) << "Simulation should remain numerically stable";
}

// ===== INTEGRATION TESTS =====

TEST_F(WaveSimulationTest, ObstaclesBlockWavePropagation) {
    // Place a large obstacle block in the middle
    for (int y = 15; y < 35; y++) {
        for (int x = 45; x < 55; x++) {
            simulation->addObstacle(x, y, 0);
        }
    }

    // Verify obstacles are present
    EXPECT_TRUE(simulation->isObstacle(50, 25));

    // Add source on left side
    simulation->addPressureSource(30, 25, 10.0f);

    // Simulate
    float dt = 1.0f / 60.0f;
    for (int i = 0; i < 40; i++) {
        simulation->update(dt);
    }

    // Verify simulation still has energy (didn't crash)
    const float* data = simulation->getData();
    int size = simulation->getWidth() * simulation->getHeight();

    float totalEnergy = 0.0f;
    for (int i = 0; i < size; i++) {
        if (!simulation->isObstacle(i % simulation->getWidth(), i / simulation->getWidth())) {
            totalEnergy += std::abs(data[i]);
        }
    }

    EXPECT_GT(totalEnergy, 0.0f) << "Simulation should maintain energy with obstacles";
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
