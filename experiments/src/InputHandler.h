#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>

// Forward declarations
class WaveSimulation;
class AudioOutput;
class CoordinateMapper;
class Renderer;
class AudioSample;

/*
 * InputHandler - Input Event Processing for Acoustic Simulation
 *
 * Encapsulates all GLFW input callback logic for the simulation.
 * Follows Single Responsibility Principle by separating input handling
 * from application logic.
 *
 * Responsibilities:
 * - Process framebuffer resize events
 * - Process mouse button events (click, drag, release)
 * - Process cursor position events (movement, dragging)
 * - Process keyboard events (shortcuts, mode toggles)
 *
 * Does NOT handle:
 * - Application state ownership (managed by main.cpp)
 * - UI rendering (managed by SimulationUI)
 * - Simulation updates (managed by WaveSimulation)
 *
 * Design Pattern:
 * - Non-owning: Uses pointers/references to external state
 * - Stateless: Does not maintain internal state beyond references
 * - Pure event processor: Responds to input events only
 */
class InputHandler {
public:
    /*
     * Constructor
     *
     * @param sim Pointer to wave simulation (not owned)
     * @param audio Pointer to audio output (not owned)
     * @param mapper Pointer to coordinate mapper (not owned)
     * @param rend Pointer to renderer (not owned)
     * @param showHelp Reference to showHelp state variable
     * @param timeScale Reference to timeScale state variable
     * @param obstacleMode Reference to obstacleMode state variable
     * @param obstacleRadius Reference to obstacleRadius state variable
     * @param listenerMode Reference to listenerMode state variable
     * @param draggingListener Reference to draggingListener state variable
     * @param sourceMode Reference to sourceMode state variable
     * @param selectedPreset Reference to selectedPreset state variable
     * @param sourceVolumeDb Reference to sourceVolumeDb state variable
     * @param sourceLoop Reference to sourceLoop state variable
     * @param loadedSample Reference to loadedSample shared_ptr
     * @param mousePressed Reference to mousePressed state variable
     * @param lastMouseX Reference to lastMouseX state variable
     * @param lastMouseY Reference to lastMouseY state variable
     * @param windowWidth Reference to windowWidth state variable
     * @param windowHeight Reference to windowHeight state variable
     */
    InputHandler(
        WaveSimulation* sim,
        AudioOutput* audio,
        CoordinateMapper* mapper,
        Renderer* rend,
        bool& showHelp,
        float& timeScale,
        bool& obstacleMode,
        int& obstacleRadius,
        bool& listenerMode,
        bool& draggingListener,
        bool& sourceMode,
        int& selectedPreset,
        float& sourceVolumeDb,
        bool& sourceLoop,
        std::shared_ptr<AudioSample>& loadedSample,
        bool& mousePressed,
        double& lastMouseX,
        double& lastMouseY,
        int& windowWidth,
        int& windowHeight
    );

    ~InputHandler() = default;

    /*
     * Handle framebuffer resize events
     *
     * Updates window dimensions, renderer viewport, and coordinate mapper.
     * Should be registered with glfwSetFramebufferSizeCallback.
     *
     * @param window GLFW window pointer
     * @param width New framebuffer width in pixels
     * @param height New framebuffer height in pixels
     */
    void handleFramebufferResize(GLFWwindow* window, int width, int height);

    /*
     * Handle mouse button events
     *
     * Processes clicks for:
     * - Creating sound impulses (normal mode)
     * - Placing/dragging listener (listener mode)
     * - Placing/removing obstacles (obstacle mode)
     * - Placing audio sources (source mode)
     *
     * Should be registered with glfwSetMouseButtonCallback.
     *
     * @param window GLFW window pointer
     * @param button Mouse button (GLFW_MOUSE_BUTTON_LEFT, etc.)
     * @param action Button action (GLFW_PRESS or GLFW_RELEASE)
     * @param mods Modifier keys (GLFW_MOD_SHIFT, etc.)
     */
    void handleMouseButton(GLFWwindow* window, int button, int action, int mods);

    /*
     * Handle cursor position events
     *
     * Processes mouse movement for:
     * - Tracking cursor position
     * - Dragging listener
     *
     * Should be registered with glfwSetCursorPosCallback.
     *
     * @param window GLFW window pointer
     * @param xpos Cursor X position in screen coordinates
     * @param ypos Cursor Y position in screen coordinates
     */
    void handleCursorPos(GLFWwindow* window, double xpos, double ypos);

    /*
     * Handle keyboard events
     *
     * Processes keyboard shortcuts for:
     * - Window control (ESC to exit)
     * - UI toggles (H for help, O/V/S for modes)
     * - Simulation control (SPACE to clear, C to clear obstacles)
     * - Parameter adjustment (arrow keys, +/-, brackets, numbers)
     * - Audio control (M to mute, Shift+UP/DOWN for volume)
     * - File operations (L to load SVG)
     * - GPU toggle (G for GPU acceleration)
     *
     * Should be registered with glfwSetKeyCallback.
     *
     * @param window GLFW window pointer
     * @param key Keyboard key code
     * @param scancode Platform-specific scancode
     * @param action Key action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
     * @param mods Modifier keys (GLFW_MOD_SHIFT, etc.)
     */
    void handleKey(GLFWwindow* window, int key, int scancode, int action, int mods);

private:
    /*
     * Convert screen coordinates to grid coordinates
     *
     * Helper method for mouse event processing.
     * Delegates to CoordinateMapper for the actual conversion.
     *
     * @param screenX Screen X coordinate (window coords)
     * @param screenY Screen Y coordinate (window coords)
     * @param[out] gridX Output grid X coordinate
     * @param[out] gridY Output grid Y coordinate
     * @return true if point is inside room viewport, false if outside
     */
    bool screenToGrid(double screenX, double screenY, int& gridX, int& gridY);

    // References to simulation components (not owned)
    WaveSimulation* simulation;
    AudioOutput* audioOutput;
    CoordinateMapper* coordinateMapper;
    Renderer* renderer;

    // References to state variables (managed by main.cpp)
    bool& showHelp;
    float& timeScale;
    bool& obstacleMode;
    int& obstacleRadius;
    bool& listenerMode;
    bool& draggingListener;
    bool& sourceMode;
    int& selectedPreset;
    float& sourceVolumeDb;
    bool& sourceLoop;
    std::shared_ptr<AudioSample>& loadedSample;
    bool& mousePressed;
    double& lastMouseX;
    double& lastMouseY;
    int& windowWidth;
    int& windowHeight;
};
