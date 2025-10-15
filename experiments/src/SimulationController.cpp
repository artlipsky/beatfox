#include "SimulationController.h"
#include "WaveSimulation.h"
#include "AudioOutput.h"
#include "Renderer.h"
#include "CoordinateMapper.h"
#include "SimulationEngine.h"
#include "AudioSource.h"
#include <iostream>

SimulationController::SimulationController(
    WaveSimulation* sim,
    AudioOutput* audio,
    Renderer* rend,
    CoordinateMapper* mapper,
    SimulationEngine* engine)
    : simulation(sim)
    , audioOutput(audio)
    , renderer(rend)
    , coordinateMapper(mapper)
    , simulationEngine(engine)
{
    // Initialize state with defaults from SimulationState constructor
    updateState();
}

void SimulationController::processCommand(const UICommand& command) {
    using Type = UICommand::Type;

    switch (command.type) {
        case Type::ADD_IMPULSE:
            handleAddImpulse(static_cast<const AddImpulseCommand&>(command));
            break;

        case Type::ADD_OBSTACLE:
            handleAddObstacle(static_cast<const AddObstacleCommand&>(command));
            break;

        case Type::REMOVE_OBSTACLE:
            handleRemoveObstacle(static_cast<const RemoveObstacleCommand&>(command));
            break;

        case Type::CLEAR_WAVES:
            handleClearWaves();
            break;

        case Type::CLEAR_OBSTACLES:
            handleClearObstacles();
            break;

        case Type::SET_LISTENER_POSITION:
            handleSetListenerPosition(static_cast<const SetListenerPositionCommand&>(command));
            break;

        case Type::TOGGLE_LISTENER:
            handleToggleListener();
            break;

        case Type::LOAD_SVG_LAYOUT:
            handleLoadSVGLayout(static_cast<const LoadSVGLayoutCommand&>(command));
            break;

        case Type::SET_TIME_SCALE:
            handleSetTimeScale(static_cast<const SetTimeScaleCommand&>(command));
            break;

        case Type::TOGGLE_MUTE:
            handleToggleMute();
            break;

        case Type::SET_VOLUME:
            handleSetVolume(static_cast<const SetVolumeCommand&>(command));
            break;

        case Type::RESIZE_GRID:
            handleResizeGrid(static_cast<const ResizeGridCommand&>(command));
            break;

        case Type::TOGGLE_GRID_DISPLAY:
            handleToggleGridDisplay();
            break;

        case Type::SET_WAVE_SPEED:
            handleSetWaveSpeed(static_cast<const SetWaveSpeedCommand&>(command));
            break;

        case Type::SET_AIR_ABSORPTION:
            handleSetAirAbsorption(static_cast<const SetAirAbsorptionCommand&>(command));
            break;

        case Type::TOGGLE_GPU:
            handleToggleGPU();
            break;

        case Type::ADD_AUDIO_SOURCE:
            handleAddAudioSource(static_cast<const AddAudioSourceCommand&>(command));
            break;

        case Type::PLAY_AUDIO_SOURCE:
            handleToggleAudioSourcePlayback(static_cast<const ToggleAudioSourcePlaybackCommand&>(command));
            break;

        case Type::TOGGLE_HELP:
            handleToggleHelp();
            break;

        case Type::TOGGLE_OBSTACLE_MODE:
            handleToggleObstacleMode();
            break;

        case Type::TOGGLE_LISTENER_MODE:
            handleToggleListenerMode();
            break;

        case Type::TOGGLE_SOURCE_MODE:
            handleToggleSourceMode();
            break;

        default:
            std::cerr << "SimulationController: Unknown command type: "
                      << static_cast<int>(command.type) << std::endl;
            break;
    }
}

void SimulationController::processCommands(const std::vector<std::unique_ptr<UICommand>>& commands) {
    for (const auto& cmd : commands) {
        processCommand(*cmd);
    }
}

void SimulationController::updateState() {
    if (!simulation) return;

    // Update simulation info
    state.info.width = simulation->getWidth();
    state.info.height = simulation->getHeight();
    state.info.physicalWidth = simulation->getPhysicalWidth();
    state.info.physicalHeight = simulation->getPhysicalHeight();
    state.info.waveSpeed = simulation->getWaveSpeed();
    state.info.hasListener = simulation->hasListener();

    if (state.info.hasListener) {
        simulation->getListenerPosition(state.info.listenerX, state.info.listenerY);
    }

    state.info.numAudioSources = simulation->getAudioSources().size();

    // Count obstacles
    const uint8_t* obstacles = simulation->getObstacles();
    int obstacleCount = 0;
    for (int i = 0; i < state.info.width * state.info.height; i++) {
        if (obstacles[i]) obstacleCount++;
    }
    state.info.numObstacles = obstacleCount;

    // Get active region percentage
    const auto& activeRegion = simulation->getActiveRegion();
    if (activeRegion.hasActivity) {
        int activeWidth = activeRegion.maxX - activeRegion.minX + 1;
        int activeHeight = activeRegion.maxY - activeRegion.minY + 1;
        int totalCells = state.info.width * state.info.height;
        int activeCells = activeWidth * activeHeight;
        state.info.activeRegionPercent = (totalCells > 0) ? (100.0f * activeCells / totalCells) : 0.0f;
    } else {
        state.info.activeRegionPercent = 0.0f;
    }

    // Update audio info
    if (audioOutput) {
        state.audio.isInitialized = true; // If we have audioOutput, assume it's initialized
        state.audio.isMuted = audioOutput->isMuted();
        state.audio.volume = audioOutput->getVolume();
        state.audio.sampleRate = 48000; // Fixed sample rate (hardcoded in AudioOutput constructor)
    }

    // Grid display state
    if (renderer) {
        state.gridEnabled = renderer->isGridEnabled();
    }
}

// Command handlers

void SimulationController::handleAddImpulse(const AddImpulseCommand& cmd) {
    if (simulation) {
        simulation->addPressureSource(cmd.x, cmd.y, cmd.pressure, cmd.radius);
    }
}

void SimulationController::handleAddObstacle(const AddObstacleCommand& cmd) {
    if (simulation) {
        simulation->addObstacle(cmd.x, cmd.y, cmd.radius);
    }
}

void SimulationController::handleRemoveObstacle(const RemoveObstacleCommand& cmd) {
    if (simulation) {
        simulation->removeObstacle(cmd.x, cmd.y, cmd.radius);
    }
}

void SimulationController::handleSetListenerPosition(const SetListenerPositionCommand& cmd) {
    if (simulation) {
        simulation->setListenerPosition(cmd.x, cmd.y);
        simulation->setListenerEnabled(true);
    }
}

void SimulationController::handleLoadSVGLayout(const LoadSVGLayoutCommand& cmd) {
    if (simulation) {
        simulation->loadObstaclesFromSVG(cmd.filename);
    }
}

void SimulationController::handleSetTimeScale(const SetTimeScaleCommand& cmd) {
    state.timeScale = cmd.scale;
}

void SimulationController::handleSetVolume(const SetVolumeCommand& cmd) {
    if (audioOutput) {
        audioOutput->setVolume(cmd.volume);
    }
}

void SimulationController::handleResizeGrid(const ResizeGridCommand& cmd) {
    if (simulationEngine) {
        // Map command enum to SimulationEngine enum
        using EngineSize = SimulationEngine::GridSize;
        EngineSize size;

        switch (cmd.size) {
            case ResizeGridCommand::GridSize::SMALL:
                size = EngineSize::SMALL;
                break;
            case ResizeGridCommand::GridSize::MEDIUM:
                size = EngineSize::MEDIUM;
                break;
            case ResizeGridCommand::GridSize::LARGE:
                size = EngineSize::LARGE;
                break;
            case ResizeGridCommand::GridSize::XLARGE:
                size = EngineSize::XLARGE;
                break;
            default:
                size = EngineSize::SMALL;
        }

        simulationEngine->resizeSimulation(size);
    }
}

void SimulationController::handleClearWaves() {
    if (simulation) {
        simulation->clear();
    }
}

void SimulationController::handleClearObstacles() {
    if (simulation) {
        simulation->clearObstacles();
    }
}

void SimulationController::handleToggleListener() {
    if (simulation) {
        simulation->setListenerEnabled(!simulation->hasListener());
    }
}

void SimulationController::handleToggleMute() {
    if (audioOutput) {
        audioOutput->setMuted(!audioOutput->isMuted());
    }
}

void SimulationController::handleToggleGridDisplay() {
    if (renderer) {
        renderer->setGridEnabled(!renderer->isGridEnabled());
    }
}

void SimulationController::handleSetWaveSpeed(const SetWaveSpeedCommand& cmd) {
    if (simulation) {
        simulation->setWaveSpeed(cmd.speed);
    }
}

void SimulationController::handleSetAirAbsorption(const SetAirAbsorptionCommand& cmd) {
    if (simulation) {
        simulation->setDamping(cmd.damping);
    }
}

void SimulationController::handleToggleGPU() {
    if (simulation) {
        simulation->setGPUEnabled(!simulation->isGPUEnabled());
    }
}

void SimulationController::handleAddAudioSource(const AddAudioSourceCommand& cmd) {
    if (simulation && cmd.sample) {
        auto source = std::make_unique<AudioSource>(cmd.sample, cmd.x, cmd.y, cmd.volumeDb, cmd.loop);
        source->play();
        simulation->addAudioSource(std::move(source));
    }
}

void SimulationController::handleToggleAudioSourcePlayback(const ToggleAudioSourcePlaybackCommand& cmd) {
    if (simulation) {
        AudioSource* source = simulation->getAudioSource(cmd.sourceIndex);
        if (source) {
            if (source->isPlaying()) {
                source->pause();
            } else {
                source->resume();
            }
        }
    }
}

void SimulationController::handleToggleHelp() {
    state.showHelp = !state.showHelp;
}

void SimulationController::handleToggleObstacleMode() {
    state.obstacleMode = !state.obstacleMode;
    if (state.obstacleMode) {
        state.listenerMode = false;
        state.sourceMode = false;
    }
}

void SimulationController::handleToggleListenerMode() {
    state.listenerMode = !state.listenerMode;
    if (state.listenerMode) {
        state.obstacleMode = false;
        state.sourceMode = false;
    }
}

void SimulationController::handleToggleSourceMode() {
    state.sourceMode = !state.sourceMode;
    if (state.sourceMode) {
        state.obstacleMode = false;
        state.listenerMode = false;
    }
}
