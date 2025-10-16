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
#include "src/AudioSample.h"
#include <memory>

@implementation SimulationStateWrapper
@end

@implementation AudioSourceInfo
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
    wrapper.gridSpacing = state.info.gridSpacing;
    wrapper.pixelSize = state.info.pixelSize;

    wrapper.isAudioInitialized = state.audio.isInitialized;
    wrapper.isMuted = state.audio.isMuted;
    wrapper.volume = state.audio.volume;

    // GPU state
    if (simulation) {
        wrapper.isGPUEnabled = simulation->isGPUEnabled();
        wrapper.isGPUAvailable = simulation->isGPUAvailable();
    }

    // Obstacle radius from state
    wrapper.obstacleRadius = state.obstacleRadius;

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

- (BOOL)loadObstaclesFromSVG:(NSString *)filePath {
    if (!simulation) {
        return NO;
    }

    std::string path = [filePath UTF8String];
    auto cmd = std::make_unique<LoadSVGLayoutCommand>(path);
    controller->processCommand(*cmd);

    // Return success (actual SVG loading happens asynchronously)
    return YES;
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

- (BOOL)loadAudioFile:(NSString *)filePath {
    if (!simulation) {
        return NO;
    }

    std::string path = [filePath UTF8String];
    auto cmd = std::make_unique<LoadAudioFileCommand>(path);
    controller->processCommand(*cmd);

    return YES;
}

- (void)clearAudioSources {
    auto cmd = std::make_unique<ClearAudioSourcesCommand>(UICommand::Type::CLEAR_AUDIO_SOURCES);
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

// Physics control
- (void)setWaveSpeed:(float)speed {
    auto cmd = std::make_unique<SetWaveSpeedCommand>(speed);
    controller->processCommand(*cmd);
}

- (float)getDamping {
    if (simulation) {
        return simulation->getDamping();
    }
    return 0.0f;
}

- (void)setAirAbsorption:(float)damping {
    auto cmd = std::make_unique<SetAirAbsorptionCommand>(damping);
    controller->processCommand(*cmd);
}

// GPU control
- (void)toggleGPU {
    auto cmd = std::make_unique<ToggleGPUCommand>(UICommand::Type::TOGGLE_GPU);
    controller->processCommand(*cmd);
}

- (BOOL)isGPUEnabled {
    if (simulation) {
        return simulation->isGPUEnabled();
    }
    return NO;
}

- (BOOL)isGPUAvailable {
    if (simulation) {
        return simulation->isGPUAvailable();
    }
    return NO;
}

// Obstacle control
- (void)setObstacleRadius:(int)radius {
    // Directly modify state (like ImGui version does)
    controller->getStateMutable().obstacleRadius = radius;
}

- (void)removeObstacleAtX:(int)x y:(int)y radius:(int)radius {
    auto cmd = std::make_unique<RemoveObstacleCommand>(x, y, radius);
    controller->processCommand(*cmd);
}

// Audio source control (for click-to-toggle)
- (void)toggleAudioSourcePlayback:(int)index {
    auto cmd = std::make_unique<ToggleAudioSourcePlaybackCommand>(index);
    controller->processCommand(*cmd);
}

// Audio source with preset selection
- (void)addAudioSourceAtX:(int)x y:(int)y preset:(int)presetIndex volumeDb:(float)volumeDb loop:(BOOL)loop {
    if (!simulation) {
        return;
    }

    // Get sample based on preset index
    std::shared_ptr<AudioSample> sample;

    if (presetIndex == 0) {
        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateKick());
    } else if (presetIndex == 1) {
        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateSnare());
    } else if (presetIndex == 2) {
        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateTone(440.0f, 1.0f));
    } else if (presetIndex == 3) {
        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateImpulse());
    } else if (presetIndex == 4) {
        // Use loaded file (if available)
        sample = controller->getState().loadedSample;
        if (!sample) {
            NSLog(@"No audio file loaded! Please load a file first.");
            return;
        }
    } else {
        // Default to kick
        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateKick());
    }

    // Create and process AddAudioSourceCommand
    auto cmd = std::make_unique<AddAudioSourceCommand>(x, y, sample, volumeDb, loop);
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

- (const float *)getPressureFieldData {
    if (simulation) {
        return simulation->getData();
    }
    return nullptr;
}

- (NSArray<AudioSourceInfo *> *)getAudioSources {
    if (!simulation) {
        return @[];
    }

    NSMutableArray<AudioSourceInfo *> *sources = [NSMutableArray array];
    const auto& audioSources = simulation->getAudioSources();

    for (const auto& source : audioSources) {
        if (!source) continue;

        AudioSourceInfo *info = [[AudioSourceInfo alloc] init];
        info.x = source->getX();
        info.y = source->getY();
        info.isPlaying = source->isPlaying();
        [sources addObject:info];
    }

    return sources;
}

@end
