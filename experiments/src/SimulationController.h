#pragma once

#include "SimulationState.h"
#include "ISimulationView.h"
#include <memory>
#include <vector>

// Forward declarations
class WaveSimulation;
class AudioOutput;
class Renderer;
class CoordinateMapper;
class SimulationEngine;

/**
 * SimulationController - Application logic layer
 *
 * Handles all commands from the UI and updates the simulation accordingly.
 * This is the "Controller" in Model-View-Controller pattern.
 *
 * Responsibilities:
 * - Process UICommands from the view
 * - Update WaveSimulation, AudioOutput, and other core components
 * - Update SimulationState for the view to display
 * - Coordinate between different subsystems
 *
 * Benefits:
 * - Decouples UI from simulation logic
 * - Makes business logic testable without UI
 * - Single place to handle all user actions
 * - Easy to add command logging, undo/redo, macros, etc.
 */
class SimulationController {
public:
    /**
     * Constructor
     *
     * @param sim Pointer to wave simulation (not owned)
     * @param audio Pointer to audio output (not owned)
     * @param renderer Pointer to renderer (not owned)
     * @param mapper Pointer to coordinate mapper (not owned)
     * @param engine Pointer to simulation engine (not owned, for grid resizing)
     */
    SimulationController(
        WaveSimulation* sim,
        AudioOutput* audio,
        Renderer* renderer,
        CoordinateMapper* mapper,
        SimulationEngine* engine
    );

    ~SimulationController() = default;

    /**
     * Process a single command
     *
     * Executes the command and updates simulation state accordingly.
     *
     * @param command The command to process
     */
    void processCommand(const UICommand& command);

    /**
     * Process multiple commands
     *
     * Convenience method to process a batch of commands.
     *
     * @param commands Vector of commands to process
     */
    void processCommands(const std::vector<std::unique_ptr<UICommand>>& commands);

    /**
     * Update simulation state
     *
     * Queries current state from simulation, audio, etc. and updates
     * the SimulationState struct for the view to read.
     *
     * Should be called once per frame before rendering the UI.
     */
    void updateState();

    /**
     * Get current simulation state
     *
     * @return Reference to current state (read-only)
     */
    const SimulationState& getState() const { return state; }

    /**
     * Get mutable simulation state
     *
     * For direct state modification when needed.
     * Prefer using processCommand() when possible.
     *
     * @return Reference to current state
     */
    SimulationState& getStateMutable() { return state; }

private:
    // Pointers to subsystems (not owned)
    WaveSimulation* simulation;
    AudioOutput* audioOutput;
    Renderer* renderer;
    CoordinateMapper* coordinateMapper;
    SimulationEngine* simulationEngine;

    // Current state
    SimulationState state;

    // Command handlers (one per command type)
    void handleAddImpulse(const AddImpulseCommand& cmd);
    void handleAddObstacle(const AddObstacleCommand& cmd);
    void handleRemoveObstacle(const RemoveObstacleCommand& cmd);
    void handleSetListenerPosition(const SetListenerPositionCommand& cmd);
    void handleLoadSVGLayout(const LoadSVGLayoutCommand& cmd);
    void handleSetTimeScale(const SetTimeScaleCommand& cmd);
    void handleSetVolume(const SetVolumeCommand& cmd);
    void handleResizeGrid(const ResizeGridCommand& cmd);
    void handleClearWaves();
    void handleClearObstacles();
    void handleToggleListener();
    void handleToggleMute();
    void handleToggleGridDisplay();
};
