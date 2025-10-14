#pragma once

#include <imgui.h>
#include <memory>
#include <functional>

// Forward declarations
class WaveSimulation;
class AudioOutput;
class CoordinateMapper;
class AudioSample;
class SimulationEngine;

/*
 * SimulationUI - Presentation layer for acoustic simulation
 *
 * Encapsulates all ImGui rendering logic for the simulation UI.
 * Follows Single Responsibility Principle by separating presentation
 * from application logic.
 *
 * Responsibilities:
 * - Render listener position marker (virtual microphone)
 * - Render audio source position markers
 * - Render controls panel (help overlay)
 * - Render help button (when panel is closed)
 *
 * Does NOT handle:
 * - Application logic (managed by main.cpp)
 * - Event handling (managed by GLFW callbacks in main.cpp)
 * - Simulation state updates (managed by WaveSimulation)
 */
class SimulationUI {
public:
    /*
     * Constructor
     *
     * @param sim Pointer to wave simulation (not owned)
     * @param audio Pointer to audio output (not owned)
     * @param mapper Pointer to coordinate mapper (not owned)
     * @param showHelp Reference to showHelp state variable
     * @param timeScale Reference to timeScale state variable
     * @param obstacleMode Reference to obstacleMode state variable
     * @param obstacleRadius Reference to obstacleRadius state variable
     * @param listenerMode Reference to listenerMode state variable
     * @param sourceMode Reference to sourceMode state variable
     * @param selectedPreset Reference to selectedPreset state variable
     * @param sourceVolumeDb Reference to sourceVolumeDb state variable
     * @param sourceLoop Reference to sourceLoop state variable
     * @param loadedSample Reference to loadedSample shared_ptr
     */
    SimulationUI(
        SimulationEngine* engine,
        WaveSimulation* sim,
        AudioOutput* audio,
        CoordinateMapper* mapper,
        bool& showHelp,
        float& timeScale,
        bool& obstacleMode,
        int& obstacleRadius,
        bool& listenerMode,
        bool& sourceMode,
        int& selectedPreset,
        float& sourceVolumeDb,
        bool& sourceLoop,
        std::shared_ptr<AudioSample>& loadedSample,
        float& impulsePressure,
        int& impulseRadius
    );

    ~SimulationUI() = default;

    /*
     * Render all UI elements
     *
     * Should be called after ImGui::NewFrame() and before ImGui::Render()
     * in the main render loop.
     */
    void render();

    /*
     * Render listener marker (virtual microphone indicator)
     *
     * Draws a green circle with microphone icon at the listener position.
     */
    void renderListenerMarker();

    /*
     * Render audio source markers
     *
     * Draws orange circles (playing) or gray circles (stopped) at each
     * audio source position with speaker icons.
     */
    void renderAudioSourceMarkers();

    /*
     * Render controls panel (help overlay)
     *
     * Full panel showing simulation parameters, acoustic presets,
     * audio source controls, and keyboard shortcuts.
     */
    void renderControlsPanel();

    /*
     * Render help button (when panel is closed)
     *
     * Small "? Help (H)" button in top-left corner.
     */
    void renderHelpButton();

private:
    // References to simulation components (not owned)
    SimulationEngine* simulationEngine;
    WaveSimulation* simulation;
    AudioOutput* audioOutput;
    CoordinateMapper* coordinateMapper;

    // References to UI state variables (managed by main.cpp)
    bool& showHelp;
    float& timeScale;
    bool& obstacleMode;
    int& obstacleRadius;
    bool& listenerMode;
    bool& sourceMode;
    int& selectedPreset;
    float& sourceVolumeDb;
    bool& sourceLoop;
    std::shared_ptr<AudioSample>& loadedSample;
    float& impulsePressure;
    int& impulseRadius;
};
