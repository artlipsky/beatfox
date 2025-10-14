#ifndef SIMULATION_ENGINE_H
#define SIMULATION_ENGINE_H

#include <memory>
#include "Application.h"
#include "WaveSimulation.h"
#include "Renderer.h"
#include "AudioOutput.h"
#include "CoordinateMapper.h"
#include "SimulationUI.h"
#include "InputHandler.h"
#include "AudioSample.h"

/**
 * SimulationEngine manages all simulation subsystems and the main game loop.
 * This encapsulates the entire simulation lifecycle:
 * - Subsystem initialization (simulation, audio, rendering, input)
 * - Main game loop (update, render)
 * - State management (time scale, modes, settings)
 * - Resource cleanup
 */
class SimulationEngine {
public:
    // Constructor takes Application reference for window/context access
    explicit SimulationEngine(Application& app);

    // Destructor handles cleanup
    ~SimulationEngine();

    // Initialize all subsystems
    bool initialize();

    // Run the main game loop (blocks until window closes)
    void run();

    // Delete copy constructor and assignment operator
    SimulationEngine(const SimulationEngine&) = delete;
    SimulationEngine& operator=(const SimulationEngine&) = delete;

private:
    // Application reference (for window access)
    Application& application;

    // Subsystems (owned by this class)
    std::unique_ptr<WaveSimulation> simulation;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<AudioOutput> audioOutput;
    std::unique_ptr<CoordinateMapper> coordinateMapper;
    std::unique_ptr<SimulationUI> simulationUI;
    std::unique_ptr<InputHandler> inputHandler;

    // Window state
    int windowWidth;
    int windowHeight;
    bool mousePressed;
    double lastMouseX;
    double lastMouseY;
    bool showHelp;

    // Simulation parameters
    float timeScale;  // Time scale: 0.001-1.0 (1000x slower to real-time), default 0.001 for visualization

    // Obstacle mode
    bool obstacleMode;  // Toggle with 'O' key
    int obstacleRadius;  // Obstacle brush size (pixels)

    // Listener mode (virtual microphone)
    bool listenerMode;  // Toggle with 'V' key
    bool draggingListener;  // True when dragging the listener

    // Audio source mode
    bool sourceMode;  // Toggle with 'S' key
    int selectedPreset;  // 0=Kick, 1=Snare, 2=Tone, 3=Impulse, 4=LoadedFile
    float sourceVolumeDb;  // Volume in dB
    bool sourceLoop;  // Loop audio
    std::shared_ptr<AudioSample> loadedSample;  // User-loaded audio file

    // Grid dimensions (const after initialization)
    int gridWidth;
    int gridHeight;

    // Performance tracking for adaptive frame skipping
    double lastFrameTime;
    double simulationTimeBudget;

    // Initialization helpers
    bool initializeSubsystems();
    void printInitializationInfo();

    // Game loop helpers
    void update();
    void render();
};

#endif // SIMULATION_ENGINE_H
