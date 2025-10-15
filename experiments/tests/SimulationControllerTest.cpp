#include <gtest/gtest.h>
#include "SimulationController.h"
#include "WaveSimulation.h"
#include "AudioOutput.h"
#include "Renderer.h"
#include "CoordinateMapper.h"

/**
 * SimulationControllerTest - Tests for command pattern and controller logic
 *
 * These tests verify that:
 * 1. Commands are processed correctly
 * 2. State is updated properly
 * 3. Domain operations are called
 * 4. UI/domain separation is maintained
 */

class SimulationControllerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create minimal subsystems for testing
        simulation = std::make_unique<WaveSimulation>(100, 100);
        audioOutput = std::make_unique<AudioOutput>();
        audioOutput->initialize(48000);
        renderer = nullptr;  // Not needed for most controller tests
        mapper = nullptr;     // Not needed for most controller tests
        engine = nullptr;     // Not needed for most controller tests

        // Create controller
        controller = std::make_unique<SimulationController>(
            simulation.get(),
            audioOutput.get(),
            renderer,
            mapper,
            engine
        );
    }

    void TearDown() override {
        controller.reset();
        audioOutput.reset();
        simulation.reset();
    }

    std::unique_ptr<WaveSimulation> simulation;
    std::unique_ptr<AudioOutput> audioOutput;
    Renderer* renderer;
    CoordinateMapper* mapper;
    SimulationEngine* engine;
    std::unique_ptr<SimulationController> controller;
};

// ============================================================================
// State Management Tests
// ============================================================================

TEST_F(SimulationControllerTest, InitialStateIsValid) {
    const auto& state = controller->getState();

    EXPECT_TRUE(state.showHelp);  // Default: help shown
    EXPECT_FLOAT_EQ(state.timeScale, 0.001f);  // Default: 1000x slower
    EXPECT_FALSE(state.obstacleMode);
    EXPECT_FALSE(state.listenerMode);
    EXPECT_FALSE(state.sourceMode);
    EXPECT_EQ(state.selectedPreset, 0);
    EXPECT_FLOAT_EQ(state.sourceVolumeDb, 0.0f);
    EXPECT_TRUE(state.sourceLoop);
    EXPECT_FLOAT_EQ(state.impulsePressure, 5.0f);
    EXPECT_EQ(state.impulseRadius, 2);
}

TEST_F(SimulationControllerTest, UpdateStateReflectsSimulation) {
    // Modify simulation
    simulation->setListenerPosition(50, 50);
    simulation->setListenerEnabled(true);

    // Update controller state
    controller->updateState();

    const auto& state = controller->getState();
    EXPECT_TRUE(state.info.hasListener);
    EXPECT_EQ(state.info.listenerX, 50);
    EXPECT_EQ(state.info.listenerY, 50);
    EXPECT_EQ(state.info.width, 100);
    EXPECT_EQ(state.info.height, 100);
}

// ============================================================================
// UI State Command Tests
// ============================================================================

TEST_F(SimulationControllerTest, SetShowHelpCommand) {
    // Initially true
    EXPECT_TRUE(controller->getState().showHelp);

    // Hide help
    auto cmd = std::make_unique<SetShowHelpCommand>(false);
    controller->processCommand(*cmd);

    EXPECT_FALSE(controller->getState().showHelp);

    // Show help again
    cmd = std::make_unique<SetShowHelpCommand>(true);
    controller->processCommand(*cmd);

    EXPECT_TRUE(controller->getState().showHelp);
}

TEST_F(SimulationControllerTest, SetImpulsePressureCommand) {
    auto cmd = std::make_unique<SetImpulsePressureCommand>(10.0f);
    controller->processCommand(*cmd);

    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 10.0f);
}

TEST_F(SimulationControllerTest, SetImpulseRadiusCommand) {
    auto cmd = std::make_unique<SetImpulseRadiusCommand>(5);
    controller->processCommand(*cmd);

    EXPECT_EQ(controller->getState().impulseRadius, 5);
}

TEST_F(SimulationControllerTest, SetSelectedPresetCommand) {
    auto cmd = std::make_unique<SetSelectedPresetCommand>(3);
    controller->processCommand(*cmd);

    EXPECT_EQ(controller->getState().selectedPreset, 3);
}

TEST_F(SimulationControllerTest, SetSourceVolumeDbCommand) {
    auto cmd = std::make_unique<SetSourceVolumeDbCommand>(-6.0f);
    controller->processCommand(*cmd);

    EXPECT_FLOAT_EQ(controller->getState().sourceVolumeDb, -6.0f);
}

TEST_F(SimulationControllerTest, SetSourceLoopCommand) {
    auto cmd = std::make_unique<SetSourceLoopCommand>(false);
    controller->processCommand(*cmd);

    EXPECT_FALSE(controller->getState().sourceLoop);
}

// ============================================================================
// Mode Toggle Command Tests
// ============================================================================

TEST_F(SimulationControllerTest, ToggleHelpCommand) {
    bool initialHelp = controller->getState().showHelp;

    auto cmd = std::make_unique<ToggleHelpCommand>(UICommand::Type::TOGGLE_HELP);
    controller->processCommand(*cmd);

    EXPECT_EQ(controller->getState().showHelp, !initialHelp);
}

TEST_F(SimulationControllerTest, ToggleObstacleModeCommand) {
    auto cmd = std::make_unique<ToggleObstacleModeCommand>(UICommand::Type::TOGGLE_OBSTACLE_MODE);
    controller->processCommand(*cmd);

    EXPECT_TRUE(controller->getState().obstacleMode);
    EXPECT_FALSE(controller->getState().listenerMode);
    EXPECT_FALSE(controller->getState().sourceMode);
}

TEST_F(SimulationControllerTest, ToggleListenerModeCommand) {
    auto cmd = std::make_unique<ToggleListenerModeCommand>(UICommand::Type::TOGGLE_LISTENER_MODE);
    controller->processCommand(*cmd);

    EXPECT_TRUE(controller->getState().listenerMode);
    EXPECT_FALSE(controller->getState().obstacleMode);
    EXPECT_FALSE(controller->getState().sourceMode);
}

TEST_F(SimulationControllerTest, ToggleSourceModeCommand) {
    auto cmd = std::make_unique<ToggleSourceModeCommand>(UICommand::Type::TOGGLE_SOURCE_MODE);
    controller->processCommand(*cmd);

    EXPECT_TRUE(controller->getState().sourceMode);
    EXPECT_FALSE(controller->getState().obstacleMode);
    EXPECT_FALSE(controller->getState().listenerMode);
}

TEST_F(SimulationControllerTest, ModeTogglesAreMutuallyExclusive) {
    // Turn on obstacle mode
    auto cmd1 = std::make_unique<ToggleObstacleModeCommand>(UICommand::Type::TOGGLE_OBSTACLE_MODE);
    controller->processCommand(*cmd1);
    EXPECT_TRUE(controller->getState().obstacleMode);

    // Turn on listener mode - should turn off obstacle mode
    auto cmd2 = std::make_unique<ToggleListenerModeCommand>(UICommand::Type::TOGGLE_LISTENER_MODE);
    controller->processCommand(*cmd2);
    EXPECT_TRUE(controller->getState().listenerMode);
    EXPECT_FALSE(controller->getState().obstacleMode);

    // Turn on source mode - should turn off listener mode
    auto cmd3 = std::make_unique<ToggleSourceModeCommand>(UICommand::Type::TOGGLE_SOURCE_MODE);
    controller->processCommand(*cmd3);
    EXPECT_TRUE(controller->getState().sourceMode);
    EXPECT_FALSE(controller->getState().listenerMode);
}

// ============================================================================
// Domain Operation Command Tests
// ============================================================================

TEST_F(SimulationControllerTest, AddImpulseCommand) {
    auto cmd = std::make_unique<AddImpulseCommand>(50, 50, 10.0f, 3);
    controller->processCommand(*cmd);

    // Can't directly verify impulse was added, but we can verify no crash
    SUCCEED();
}

TEST_F(SimulationControllerTest, AddObstacleCommand) {
    auto cmd = std::make_unique<AddObstacleCommand>(25, 25, 5);
    controller->processCommand(*cmd);

    // Update state to reflect changes
    controller->updateState();

    // Verify obstacles were added (count > 0)
    EXPECT_GT(controller->getState().info.numObstacles, 0);
}

TEST_F(SimulationControllerTest, ClearObstaclesCommand) {
    // First add some obstacles
    auto addCmd = std::make_unique<AddObstacleCommand>(25, 25, 5);
    controller->processCommand(*addCmd);
    controller->updateState();
    EXPECT_GT(controller->getState().info.numObstacles, 0);

    // Now clear them
    auto clearCmd = std::make_unique<ClearObstaclesCommand>(UICommand::Type::CLEAR_OBSTACLES);
    controller->processCommand(*clearCmd);
    controller->updateState();

    EXPECT_EQ(controller->getState().info.numObstacles, 0);
}

TEST_F(SimulationControllerTest, SetListenerPositionCommand) {
    auto cmd = std::make_unique<SetListenerPositionCommand>(75, 75);
    controller->processCommand(*cmd);
    controller->updateState();

    const auto& state = controller->getState();
    EXPECT_TRUE(state.info.hasListener);
    EXPECT_EQ(state.info.listenerX, 75);
    EXPECT_EQ(state.info.listenerY, 75);
}

TEST_F(SimulationControllerTest, ToggleListenerCommand) {
    // Initially listener may or may not be enabled
    bool initialState = simulation->hasListener();

    auto cmd = std::make_unique<ToggleListenerCommand>(UICommand::Type::TOGGLE_LISTENER);
    controller->processCommand(*cmd);
    controller->updateState();

    EXPECT_EQ(controller->getState().info.hasListener, !initialState);
}

TEST_F(SimulationControllerTest, ClearWavesCommand) {
    // Add impulse to create waves
    auto impulseCmd = std::make_unique<AddImpulseCommand>(50, 50, 10.0f, 2);
    controller->processCommand(*impulseCmd);

    // Clear waves
    auto clearCmd = std::make_unique<ClearWavesCommand>(UICommand::Type::CLEAR_WAVES);
    controller->processCommand(*clearCmd);

    // Can't directly verify, but ensure no crash
    SUCCEED();
}

TEST_F(SimulationControllerTest, SetTimeScaleCommand) {
    auto cmd = std::make_unique<SetTimeScaleCommand>(0.5f);
    controller->processCommand(*cmd);

    EXPECT_FLOAT_EQ(controller->getState().timeScale, 0.5f);
}

TEST_F(SimulationControllerTest, ApplyDampingPresetCommand) {
    // Test each preset type
    auto realistic = std::make_unique<ApplyDampingPresetCommand>(
        ApplyDampingPresetCommand::PresetType::REALISTIC
    );
    controller->processCommand(*realistic);
    SUCCEED();  // Verify no crash

    auto viz = std::make_unique<ApplyDampingPresetCommand>(
        ApplyDampingPresetCommand::PresetType::VISUALIZATION
    );
    controller->processCommand(*viz);
    SUCCEED();

    auto anechoic = std::make_unique<ApplyDampingPresetCommand>(
        ApplyDampingPresetCommand::PresetType::ANECHOIC
    );
    controller->processCommand(*anechoic);
    SUCCEED();
}

TEST_F(SimulationControllerTest, ToggleMuteCommand) {
    bool initialMuted = audioOutput->isMuted();

    auto cmd = std::make_unique<ToggleMuteCommand>(UICommand::Type::TOGGLE_MUTE);
    controller->processCommand(*cmd);

    EXPECT_EQ(audioOutput->isMuted(), !initialMuted);
}

TEST_F(SimulationControllerTest, SetVolumeCommand) {
    auto cmd = std::make_unique<SetVolumeCommand>(0.5f);
    controller->processCommand(*cmd);

    EXPECT_FLOAT_EQ(audioOutput->getVolume(), 0.5f);
}

// ============================================================================
// Batch Command Processing Tests
// ============================================================================

TEST_F(SimulationControllerTest, ProcessMultipleCommandsInSequence) {
    std::vector<std::unique_ptr<UICommand>> commands;

    commands.push_back(std::make_unique<SetImpulsePressureCommand>(20.0f));
    commands.push_back(std::make_unique<SetImpulseRadiusCommand>(7));
    commands.push_back(std::make_unique<SetShowHelpCommand>(false));

    controller->processCommands(commands);

    const auto& state = controller->getState();
    EXPECT_FLOAT_EQ(state.impulsePressure, 20.0f);
    EXPECT_EQ(state.impulseRadius, 7);
    EXPECT_FALSE(state.showHelp);
}

TEST_F(SimulationControllerTest, CommandsModifyIndependentState) {
    // Set multiple independent state values
    auto cmd1 = std::make_unique<SetImpulsePressureCommand>(15.0f);
    auto cmd2 = std::make_unique<SetSourceVolumeDbCommand>(-3.0f);
    auto cmd3 = std::make_unique<SetSelectedPresetCommand>(2);

    controller->processCommand(*cmd1);
    controller->processCommand(*cmd2);
    controller->processCommand(*cmd3);

    const auto& state = controller->getState();
    EXPECT_FLOAT_EQ(state.impulsePressure, 15.0f);
    EXPECT_FLOAT_EQ(state.sourceVolumeDb, -3.0f);
    EXPECT_EQ(state.selectedPreset, 2);
}

// ============================================================================
// Edge Cases and Error Handling
// ============================================================================

TEST_F(SimulationControllerTest, CommandsWithNullptrSimulation) {
    // Create controller with null simulation (shouldn't crash)
    auto nullController = std::make_unique<SimulationController>(
        nullptr, nullptr, nullptr, nullptr, nullptr
    );

    // These should not crash even with null simulation
    auto cmd1 = std::make_unique<AddImpulseCommand>(50, 50, 10.0f, 2);
    auto cmd2 = std::make_unique<ClearWavesCommand>(UICommand::Type::CLEAR_WAVES);
    auto cmd3 = std::make_unique<SetTimeScaleCommand>(0.1f);

    EXPECT_NO_THROW(nullController->processCommand(*cmd1));
    EXPECT_NO_THROW(nullController->processCommand(*cmd2));
    EXPECT_NO_THROW(nullController->processCommand(*cmd3));
}

TEST_F(SimulationControllerTest, ExtremeParameterValues) {
    // Test with extreme but valid values
    auto cmd1 = std::make_unique<SetImpulsePressureCommand>(0.01f);  // Min
    controller->processCommand(*cmd1);
    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 0.01f);

    auto cmd2 = std::make_unique<SetImpulsePressureCommand>(100.0f);  // Max
    controller->processCommand(*cmd2);
    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 100.0f);

    auto cmd3 = std::make_unique<SetImpulseRadiusCommand>(1);  // Min
    controller->processCommand(*cmd3);
    EXPECT_EQ(controller->getState().impulseRadius, 1);

    auto cmd4 = std::make_unique<SetImpulseRadiusCommand>(10);  // Max
    controller->processCommand(*cmd4);
    EXPECT_EQ(controller->getState().impulseRadius, 10);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(SimulationControllerTest, FullWorkflowSimulation) {
    // Simulate a complete user workflow

    // 1. User adjusts impulse parameters
    controller->processCommand(*std::make_unique<SetImpulsePressureCommand>(7.5f));
    controller->processCommand(*std::make_unique<SetImpulseRadiusCommand>(3));

    // 2. User creates impulse
    controller->processCommand(*std::make_unique<AddImpulseCommand>(50, 50, 7.5f, 3));

    // 3. User places listener
    controller->processCommand(*std::make_unique<SetListenerPositionCommand>(30, 30));

    // 4. User adds obstacles
    controller->processCommand(*std::make_unique<AddObstacleCommand>(60, 60, 10));

    // 5. User changes acoustic environment
    controller->processCommand(*std::make_unique<ApplyDampingPresetCommand>(
        ApplyDampingPresetCommand::PresetType::ANECHOIC
    ));

    // 6. User adjusts audio
    controller->processCommand(*std::make_unique<SetVolumeCommand>(0.7f));

    // Update and verify state
    controller->updateState();
    const auto& state = controller->getState();

    EXPECT_FLOAT_EQ(state.impulsePressure, 7.5f);
    EXPECT_EQ(state.impulseRadius, 3);
    EXPECT_TRUE(state.info.hasListener);
    EXPECT_GT(state.info.numObstacles, 0);
    EXPECT_FLOAT_EQ(audioOutput->getVolume(), 0.7f);
}

TEST_F(SimulationControllerTest, CommandIdempotency) {
    // Applying the same command multiple times should give consistent results

    auto cmd1 = std::make_unique<SetImpulsePressureCommand>(12.5f);
    controller->processCommand(*cmd1);
    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 12.5f);

    auto cmd2 = std::make_unique<SetImpulsePressureCommand>(12.5f);
    controller->processCommand(*cmd2);
    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 12.5f);

    auto cmd3 = std::make_unique<SetImpulsePressureCommand>(12.5f);
    controller->processCommand(*cmd3);
    EXPECT_FLOAT_EQ(controller->getState().impulsePressure, 12.5f);
}
