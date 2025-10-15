//
//  SimulationControllerBridge.mm
//  BeatfoxSimulation
//
//  Objective-C++ implementation that bridges Swift to C++
//

#import "SimulationControllerBridge.h"
#include "src/SimulationController.h"
#include "src/WaveSimulation.h"
#include "src/AudioOutput.h"
#include "src/SimulationState.h"
#include <memory>

@implementation SimulationStateWrapper
@end

@interface SimulationControllerBridge () {
    std::unique_ptr<WaveSimulation> simulation;
    std::unique_ptr<AudioOutput> audioOutput;
    std::unique_ptr<SimulationController> controller;
}
@end

@implementation SimulationControllerBridge

- (instancetype)init {
    self = [super init];
    if (self) {
        // Create simulation (100x100 grid for testing)
        simulation = std::make_unique<WaveSimulation>(581, 291);  // 5m x 2.5m room

        // Initialize audio
        audioOutput = std::make_unique<AudioOutput>();
        audioOutput->initialize(48000);
        audioOutput->start();

        // Create controller (nullptr for renderer, mapper, engine - not needed for SwiftUI)
        controller = std::make_unique<SimulationController>(
            simulation.get(),
            audioOutput.get(),
            nullptr,  // renderer
            nullptr,  // mapper
            nullptr   // engine
        );

        // Set initial listener position
        simulation->setListenerPosition(290, 145);  // Center of room
        simulation->setListenerEnabled(true);
    }
    return self;
}

- (void)dealloc {
    if (audioOutput) {
        audioOutput->stop();
    }
}

- (SimulationStateWrapper *)getState {
    const auto& state = controller->getState();

    SimulationStateWrapper *wrapper = [[SimulationStateWrapper alloc] init];
    wrapper.showHelp = state.showHelp;
    wrapper.timeScale = state.timeScale;
    wrapper.obstacleMode = state.obstacleMode;
    wrapper.listenerMode = state.listenerMode;
    wrapper.sourceMode = state.sourceMode;
    wrapper.selectedPreset = state.selectedPreset;
    wrapper.sourceVolumeDb = state.sourceVolumeDb;
    wrapper.sourceLoop = state.sourceLoop;
    wrapper.impulsePressure = state.impulsePressure;
    wrapper.impulseRadius = state.impulseRadius;

    wrapper.width = state.info.width;
    wrapper.height = state.info.height;
    wrapper.physicalWidth = state.info.physicalWidth;
    wrapper.physicalHeight = state.info.physicalHeight;
    wrapper.waveSpeed = state.info.waveSpeed;
    wrapper.hasListener = state.info.hasListener;
    wrapper.listenerX = state.info.listenerX;
    wrapper.listenerY = state.info.listenerY;
    wrapper.numAudioSources = state.info.numAudioSources;
    wrapper.numObstacles = state.info.numObstacles;

    wrapper.isAudioInitialized = state.audio.isInitialized;
    wrapper.isMuted = state.audio.isMuted;
    wrapper.volume = state.audio.volume;

    return wrapper;
}

- (void)updateState {
    controller->updateState();
}

// UI Commands
- (void)setImpulsePressure:(float)pressure {
    auto cmd = std::make_unique<SetImpulsePressureCommand>(pressure);
    controller->processCommand(*cmd);
}

- (void)setImpulseRadius:(int)radius {
    auto cmd = std::make_unique<SetImpulseRadiusCommand>(radius);
    controller->processCommand(*cmd);
}

- (void)setShowHelp:(BOOL)show {
    auto cmd = std::make_unique<SetShowHelpCommand>(show);
    controller->processCommand(*cmd);
}

- (void)setSelectedPreset:(int)presetIndex {
    auto cmd = std::make_unique<SetSelectedPresetCommand>(presetIndex);
    controller->processCommand(*cmd);
}

- (void)setSourceVolumeDb:(float)volumeDb {
    auto cmd = std::make_unique<SetSourceVolumeDbCommand>(volumeDb);
    controller->processCommand(*cmd);
}

- (void)setSourceLoop:(BOOL)loop {
    auto cmd = std::make_unique<SetSourceLoopCommand>(loop);
    controller->processCommand(*cmd);
}

// Mode toggles
- (void)toggleHelp {
    auto cmd = std::make_unique<ToggleHelpCommand>(UICommand::Type::TOGGLE_HELP);
    controller->processCommand(*cmd);
}

- (void)toggleObstacleMode {
    auto cmd = std::make_unique<ToggleObstacleModeCommand>(UICommand::Type::TOGGLE_OBSTACLE_MODE);
    controller->processCommand(*cmd);
}

- (void)toggleListenerMode {
    auto cmd = std::make_unique<ToggleListenerModeCommand>(UICommand::Type::TOGGLE_LISTENER_MODE);
    controller->processCommand(*cmd);
}

- (void)toggleSourceMode {
    auto cmd = std::make_unique<ToggleSourceModeCommand>(UICommand::Type::TOGGLE_SOURCE_MODE);
    controller->processCommand(*cmd);
}

// Domain operations
- (void)addImpulseAtX:(int)x y:(int)y pressure:(float)pressure radius:(int)radius {
    auto cmd = std::make_unique<AddImpulseCommand>(x, y, pressure, radius);
    controller->processCommand(*cmd);
}

- (void)addObstacleAtX:(int)x y:(int)y radius:(int)radius {
    auto cmd = std::make_unique<AddObstacleCommand>(x, y, radius);
    controller->processCommand(*cmd);
}

- (void)clearObstacles {
    auto cmd = std::make_unique<ClearObstaclesCommand>(UICommand::Type::CLEAR_OBSTACLES);
    controller->processCommand(*cmd);
}

- (void)clearWaves {
    auto cmd = std::make_unique<ClearWavesCommand>(UICommand::Type::CLEAR_WAVES);
    controller->processCommand(*cmd);
}

- (void)setListenerPositionX:(int)x y:(int)y {
    auto cmd = std::make_unique<SetListenerPositionCommand>(x, y);
    controller->processCommand(*cmd);
}

- (void)toggleListener {
    auto cmd = std::make_unique<ToggleListenerCommand>(UICommand::Type::TOGGLE_LISTENER);
    controller->processCommand(*cmd);
}

- (void)setTimeScale:(float)scale {
    auto cmd = std::make_unique<SetTimeScaleCommand>(scale);
    controller->processCommand(*cmd);
}

// Audio operations
- (void)toggleMute {
    auto cmd = std::make_unique<ToggleMuteCommand>(UICommand::Type::TOGGLE_MUTE);
    controller->processCommand(*cmd);
}

- (void)setVolume:(float)volume {
    auto cmd = std::make_unique<SetVolumeCommand>(volume);
    controller->processCommand(*cmd);
}

- (void)applyDampingPreset:(int)presetType {
    ApplyDampingPresetCommand::PresetType type;
    switch (presetType) {
        case 0: type = ApplyDampingPresetCommand::PresetType::REALISTIC; break;
        case 1: type = ApplyDampingPresetCommand::PresetType::VISUALIZATION; break;
        case 2: type = ApplyDampingPresetCommand::PresetType::ANECHOIC; break;
        default: type = ApplyDampingPresetCommand::PresetType::REALISTIC; break;
    }

    auto cmd = std::make_unique<ApplyDampingPresetCommand>(type);
    controller->processCommand(*cmd);
}

- (void)updateWithDeltaTime:(float)dt {
    // Get time scale from state
    float timeScale = controller->getState().timeScale;

    // Update simulation
    simulation->update(dt * timeScale);

    // Submit audio samples
    if (simulation->hasListener() && audioOutput) {
        std::vector<float> samples = simulation->getListenerSamples();
        audioOutput->submitPressureSamples(samples, timeScale);
    }

    // Update controller state
    controller->updateState();
}

@end
