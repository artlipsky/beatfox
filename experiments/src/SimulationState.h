#pragma once

#include <memory>
#include <string>

// Forward declarations
class AudioSample;

/**
 * SimulationState - Centralized UI state
 *
 * Contains all state that the UI needs to read/display.
 * This is the "model" in Model-View-Controller pattern.
 *
 * Benefits:
 * - Single source of truth for UI state
 * - Easy to serialize/deserialize (save/load sessions)
 * - Easy to test without UI
 * - Clear separation between simulation logic and UI state
 */
struct SimulationState {
    // Display settings
    bool showHelp = true;
    bool gridEnabled = true;

    // Time control
    float timeScale = 0.001f;  // 1000x slower for clear visualization

    // Interaction modes
    bool obstacleMode = false;
    int obstacleRadius = 5;
    bool listenerMode = false;
    bool sourceMode = false;

    // Audio source settings
    int selectedPreset = 0;
    float sourceVolumeDb = 0.0f;
    bool sourceLoop = true;
    std::shared_ptr<AudioSample> loadedSample = nullptr;

    // Impulse settings
    float impulsePressure = 5.0f;  // 5 Pa = typical hand clap
    int impulseRadius = 2;         // 2 pixels × 8.6mm = 17.2mm ≈ 2cm spread

    // Simulation info (read-only, updated by simulation)
    struct SimulationInfo {
        int width = 0;
        int height = 0;
        float physicalWidth = 0.0f;
        float physicalHeight = 0.0f;
        float waveSpeed = 343.0f;
        bool hasListener = false;
        int listenerX = 0;
        int listenerY = 0;
        int numAudioSources = 0;
        int numObstacles = 0;
        float activeRegionPercent = 0.0f;

        // Grid visualization config
        int gridSpacing = 10;        // Grid line every N pixels
        float pixelSize = 0.0086f;   // Physical size of 1 pixel in meters (8.6mm)
    } info;

    // Audio info (read-only, updated by audio system)
    struct AudioInfo {
        bool isInitialized = false;
        bool isMuted = false;
        float volume = 1.0f;
        int sampleRate = 48000;
        int bufferSize = 0;
    } audio;
};

/**
 * UICommand - Base class for all commands from UI to simulation
 *
 * Uses Command pattern to encapsulate all UI interactions as objects.
 * This makes it easy to:
 * - Queue commands
 * - Undo/redo
 * - Record/replay sessions
 * - Test without UI
 */
struct UICommand {
    virtual ~UICommand() = default;

    enum class Type {
        // Simulation control
        CLEAR_WAVES,
        RESET_SIMULATION,
        PAUSE_SIMULATION,
        RESUME_SIMULATION,

        // Interaction
        ADD_IMPULSE,
        ADD_OBSTACLE,
        REMOVE_OBSTACLE,
        CLEAR_OBSTACLES,
        LOAD_SVG_LAYOUT,

        // Listener
        SET_LISTENER_POSITION,
        TOGGLE_LISTENER,

        // Audio sources
        ADD_AUDIO_SOURCE,
        REMOVE_AUDIO_SOURCE,
        PLAY_AUDIO_SOURCE,
        STOP_AUDIO_SOURCE,
        CLEAR_AUDIO_SOURCES,
        LOAD_AUDIO_FILE,

        // View control
        RESIZE_GRID,
        TOGGLE_GRID_DISPLAY,

        // Audio output
        TOGGLE_MUTE,
        SET_VOLUME,

        // Settings
        SET_TIME_SCALE,
        SET_WAVE_SPEED,
        SET_AIR_ABSORPTION,
        APPLY_DAMPING_PRESET,
        TOGGLE_GPU,

        // UI-only state changes
        TOGGLE_HELP,
        SET_SHOW_HELP,
        TOGGLE_OBSTACLE_MODE,
        TOGGLE_LISTENER_MODE,
        TOGGLE_SOURCE_MODE,

        // UI parameter settings (for UI panel controls)
        SET_SELECTED_PRESET,
        SET_SOURCE_VOLUME_DB,
        SET_SOURCE_LOOP,
        SET_IMPULSE_PRESSURE,
        SET_IMPULSE_RADIUS
    };

    Type type;

    explicit UICommand(Type t) : type(t) {}
};

// Specific command types

struct AddImpulseCommand : UICommand {
    int x, y;
    float pressure;
    int radius;

    AddImpulseCommand(int x_, int y_, float p, int r)
        : UICommand(Type::ADD_IMPULSE), x(x_), y(y_), pressure(p), radius(r) {}
};

struct AddObstacleCommand : UICommand {
    int x, y;
    int radius;

    AddObstacleCommand(int x_, int y_, int r)
        : UICommand(Type::ADD_OBSTACLE), x(x_), y(y_), radius(r) {}
};

struct RemoveObstacleCommand : UICommand {
    int x, y;
    int radius;

    RemoveObstacleCommand(int x_, int y_, int r)
        : UICommand(Type::REMOVE_OBSTACLE), x(x_), y(y_), radius(r) {}
};

struct SetListenerPositionCommand : UICommand {
    int x, y;

    SetListenerPositionCommand(int x_, int y_)
        : UICommand(Type::SET_LISTENER_POSITION), x(x_), y(y_) {}
};

struct LoadSVGLayoutCommand : UICommand {
    std::string filename;

    explicit LoadSVGLayoutCommand(const std::string& file)
        : UICommand(Type::LOAD_SVG_LAYOUT), filename(file) {}
};

struct SetTimeScaleCommand : UICommand {
    float scale;

    explicit SetTimeScaleCommand(float s)
        : UICommand(Type::SET_TIME_SCALE), scale(s) {}
};

struct SetVolumeCommand : UICommand {
    float volume;

    explicit SetVolumeCommand(float v)
        : UICommand(Type::SET_VOLUME), volume(v) {}
};

struct ResizeGridCommand : UICommand {
    enum class GridSize { SMALL, MEDIUM, LARGE, XLARGE };
    GridSize size;

    explicit ResizeGridCommand(GridSize s)
        : UICommand(Type::RESIZE_GRID), size(s) {}
};

struct SetWaveSpeedCommand : UICommand {
    float speed;

    explicit SetWaveSpeedCommand(float s)
        : UICommand(Type::SET_WAVE_SPEED), speed(s) {}
};

struct SetAirAbsorptionCommand : UICommand {
    float damping;

    explicit SetAirAbsorptionCommand(float d)
        : UICommand(Type::SET_AIR_ABSORPTION), damping(d) {}
};

struct AddAudioSourceCommand : UICommand {
    int x, y;
    std::shared_ptr<AudioSample> sample;
    float volumeDb;
    bool loop;

    AddAudioSourceCommand(int x_, int y_, std::shared_ptr<AudioSample> s, float vol, bool l)
        : UICommand(Type::ADD_AUDIO_SOURCE), x(x_), y(y_), sample(s), volumeDb(vol), loop(l) {}
};

struct ToggleAudioSourcePlaybackCommand : UICommand {
    size_t sourceIndex;

    explicit ToggleAudioSourcePlaybackCommand(size_t idx)
        : UICommand(Type::PLAY_AUDIO_SOURCE), sourceIndex(idx) {}
};

struct ApplyDampingPresetCommand : UICommand {
    enum class PresetType { REALISTIC, VISUALIZATION, ANECHOIC };
    PresetType presetType;

    explicit ApplyDampingPresetCommand(PresetType pt)
        : UICommand(Type::APPLY_DAMPING_PRESET), presetType(pt) {}
};

struct LoadAudioFileCommand : UICommand {
    std::string filename;

    explicit LoadAudioFileCommand(const std::string& file)
        : UICommand(Type::LOAD_AUDIO_FILE), filename(file) {}
};

struct SetSelectedPresetCommand : UICommand {
    int presetIndex;

    explicit SetSelectedPresetCommand(int idx)
        : UICommand(Type::SET_SELECTED_PRESET), presetIndex(idx) {}
};

struct SetSourceVolumeDbCommand : UICommand {
    float volumeDb;

    explicit SetSourceVolumeDbCommand(float vol)
        : UICommand(Type::SET_SOURCE_VOLUME_DB), volumeDb(vol) {}
};

struct SetSourceLoopCommand : UICommand {
    bool loop;

    explicit SetSourceLoopCommand(bool l)
        : UICommand(Type::SET_SOURCE_LOOP), loop(l) {}
};

struct SetImpulsePressureCommand : UICommand {
    float pressure;

    explicit SetImpulsePressureCommand(float p)
        : UICommand(Type::SET_IMPULSE_PRESSURE), pressure(p) {}
};

struct SetImpulseRadiusCommand : UICommand {
    int radius;

    explicit SetImpulseRadiusCommand(int r)
        : UICommand(Type::SET_IMPULSE_RADIUS), radius(r) {}
};

struct SetShowHelpCommand : UICommand {
    bool show;

    explicit SetShowHelpCommand(bool s)
        : UICommand(Type::SET_SHOW_HELP), show(s) {}
};

// Simple commands (no parameters)
using ClearWavesCommand = UICommand;
using ClearObstaclesCommand = UICommand;
using ClearAudioSourcesCommand = UICommand;
using ToggleListenerCommand = UICommand;
using ToggleMuteCommand = UICommand;
using ToggleGridDisplayCommand = UICommand;
using ToggleGPUCommand = UICommand;
using ToggleHelpCommand = UICommand;
using ToggleObstacleModeCommand = UICommand;
using ToggleListenerModeCommand = UICommand;
using ToggleSourceModeCommand = UICommand;
