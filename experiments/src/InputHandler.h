#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include "SimulationState.h"

// Forward declarations
class SimulationController;
class WaveSimulation;
class CoordinateMapper;

/*
 * InputHandler - Input Event Processing with Command Pattern
 *
 * Encapsulates all GLFW input callback logic for the simulation.
 * Uses the Command pattern to decouple input handling from simulation logic.
 *
 * Responsibilities:
 * - Process framebuffer resize events
 * - Process mouse button events (click, drag, release)
 * - Process cursor position events (movement, dragging)
 * - Process keyboard events (shortcuts, mode toggles)
 * - Generate UICommand objects for simulation controller
 *
 * Does NOT handle:
 * - Simulation updates (delegated to SimulationController)
 * - UI rendering (managed by SimulationUI)
 * - State ownership (managed by SimulationController)
 *
 * Design Pattern:
 * - Command Pattern: Accumulates UICommands during event processing
 * - Non-owning: Uses pointer to controller (not owned)
 * - Event-driven: Responds to GLFW callbacks
 */
class InputHandler {
public:
    /*
     * Constructor
     *
     * @param ctrl Pointer to simulation controller (not owned)
     * @param sim Pointer to wave simulation (not owned, for direct queries)
     * @param mapper Pointer to coordinate mapper (not owned)
     */
    InputHandler(
        SimulationController* ctrl,
        WaveSimulation* sim,
        CoordinateMapper* mapper
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

    /*
     * Update simulation pointer after resize
     *
     * Called by SimulationEngine::resizeSimulation() to update
     * the internal simulation pointer without destroying the InputHandler object.
     *
     * @param newSim Pointer to the new WaveSimulation instance
     */
    void updateSimulationPointer(WaveSimulation* newSim);

    /*
     * Collect accumulated commands
     *
     * Returns all commands generated during this frame and clears
     * the internal command queue. Should be called once per frame
     * after all input events have been processed.
     *
     * @return Vector of commands to be processed by controller
     */
    std::vector<std::unique_ptr<UICommand>> collectCommands();

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

    // Pointers to subsystems (not owned)
    SimulationController* controller;
    WaveSimulation* simulation;
    CoordinateMapper* coordinateMapper;

    // Accumulated commands during frame
    std::vector<std::unique_ptr<UICommand>> pendingCommands;

    // Local tracking state (for drag operations)
    bool mousePressed = false;
    bool draggingListener = false;
    double lastMouseX = 0.0;
    double lastMouseY = 0.0;
    int windowWidth = 0;
    int windowHeight = 0;
};
