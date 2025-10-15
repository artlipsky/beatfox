#pragma once

#include <imgui.h>
#include <memory>
#include <functional>

// Forward declarations
class SimulationController;
class WaveSimulation;
class AudioOutput;
class CoordinateMapper;
class SimulationEngine;

/*
 * SimulationUI - Presentation layer with clean MVC architecture
 *
 * Refactored to read state from SimulationController instead of
 * accepting 17 parameters by reference.
 *
 * Responsibilities:
 * - Render listener position marker (virtual microphone)
 * - Render audio source position markers
 * - Render controls panel (help overlay)
 * - Render help button (when panel is closed)
 * - Read UI state from controller->getState()
 *
 * Does NOT handle:
 * - State ownership (managed by SimulationController)
 * - Event handling (managed by InputHandler)
 * - Simulation updates (managed by WaveSimulation)
 */
class SimulationUI {
public:
    /*
     * Constructor - Clean 4-parameter design
     *
     * @param controller Pointer to simulation controller (not owned)
     * @param sim Pointer to wave simulation (not owned, for direct queries)
     * @param audio Pointer to audio output (not owned, for status queries)
     * @param mapper Pointer to coordinate mapper (not owned, for coordinate conversion)
     */
    SimulationUI(
        SimulationController* controller,
        WaveSimulation* sim,
        AudioOutput* audio,
        CoordinateMapper* mapper
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

    /*
     * Update simulation pointer after resize
     *
     * Called by SimulationEngine::resizeSimulation() to update
     * the internal simulation pointer without destroying the UI object.
     *
     * @param newSim Pointer to the new WaveSimulation instance
     */
    void updateSimulationPointer(WaveSimulation* newSim);

private:
    // Pointers to subsystems (not owned)
    SimulationController* controller;
    WaveSimulation* simulation;
    AudioOutput* audioOutput;
    CoordinateMapper* coordinateMapper;
};
