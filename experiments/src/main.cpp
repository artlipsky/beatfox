#include <iostream>
#include <chrono>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Application.h"
#include "WaveSimulation.h"
#include "DampingPreset.h"
#include "Renderer.h"
#include "AudioOutput.h"
#include "AudioSource.h"
#include "AudioSample.h"
#include "AudioFileLoader.h"
#include "CoordinateMapper.h"
#include "SimulationUI.h"
#include "InputHandler.h"
#include "portable-file-dialogs.h"

// Window state
int windowWidth = 1280;
int windowHeight = 720;
bool mousePressed = false;
double lastMouseX = 0.0;
double lastMouseY = 0.0;
bool showHelp = true;

// Simulation
WaveSimulation* simulation = nullptr;
Renderer* renderer = nullptr;
AudioOutput* audioOutput = nullptr;
CoordinateMapper* coordinateMapper = nullptr;
float timeScale = 0.01f;  // Time scale: 0.01 = 100x slower (for audible sound), 1.0 = real-time

// Obstacle mode
bool obstacleMode = false;  // Toggle with 'O' key
int obstacleRadius = 5;     // Obstacle brush size (pixels)

// Listener mode (virtual microphone)
bool listenerMode = false;  // Toggle with 'V' key
bool draggingListener = false;  // True when dragging the listener

// Audio source mode
bool sourceMode = false;    // Toggle with 'S' key
int selectedPreset = 0;     // 0=Kick, 1=Snare, 2=Tone, 3=Impulse, 4=LoadedFile
float sourceVolumeDb = 0.0f;  // Volume in dB
bool sourceLoop = true;     // Loop audio
std::shared_ptr<AudioSample> loadedSample;  // User-loaded audio file

// Global pointer to InputHandler for GLFW callbacks
InputHandler* inputHandler = nullptr;

int main() {
    // Initialize application (GLFW, OpenGL, ImGui)
    Application app(windowWidth, windowHeight, "Acoustic Pressure Simulation");
    if (!app.initialize()) {
        return -1;
    }

    GLFWwindow* window = app.getWindow();

    // Create simulation and renderer
    // Room: 10m (height) x 20m (width)
    // Optimized resolution for maximum performance
    // Scale: 1 pixel = 5 cm = 0.05 m
    const int gridHeight = 200;   // 10m / 0.05m = 200 pixels
    const int gridWidth = 400;    // 20m / 0.05m = 400 pixels

    simulation = new WaveSimulation(gridWidth, gridHeight);

    // Initialize audio output
    audioOutput = new AudioOutput();
    if (!audioOutput->initialize(48000)) {
        std::cerr << "Warning: Failed to initialize audio output" << std::endl;
        std::cerr << audioOutput->getLastError() << std::endl;
    } else {
        audioOutput->start();
        std::cout << "Audio output: Initialized and started" << std::endl;
    }

    // Get actual framebuffer size (accounts for DPI scaling)
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    windowWidth = fbWidth;
    windowHeight = fbHeight;

    Renderer rendererObj(fbWidth, fbHeight);
    renderer = &rendererObj;  // Store global pointer for mouse handlers

    // Print physical dimensions
    std::cout << "\nPhysical dimensions:" << std::endl;
    std::cout << "  Window: " << winWidth << " x " << winHeight << " (window coords)" << std::endl;
    std::cout << "  Framebuffer: " << fbWidth << " x " << fbHeight << " (framebuffer coords)" << std::endl;
    std::cout << "  Grid: " << gridWidth << " x " << gridHeight << " pixels (W x H) [OPTIMIZED]" << std::endl;
    std::cout << "  Scale: 1 pixel = 5.0 cm = 50 mm" << std::endl;
    std::cout << "  Room size: " << simulation->getPhysicalWidth() << " m x "
              << simulation->getPhysicalHeight() << " m (W x H)" << std::endl;
    std::cout << "  Speed of sound: " << simulation->getWaveSpeed() << " m/s" << std::endl;

    // Print viewport info
    float vLeft, vRight, vBottom, vTop;
    renderer->getRoomViewport(vLeft, vRight, vBottom, vTop);
    std::cout << "  Viewport: (" << vLeft << ", " << vBottom << ") to ("
              << vRight << ", " << vTop << ")" << std::endl;

    if (!rendererObj.initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        delete simulation;
        glfwTerminate();
        return -1;
    }

    // Initialize coordinate mapper for centralized coordinate transformations
    CoordinateMapper mapperObj;
    mapperObj.updateViewport(
        winWidth, winHeight,
        fbWidth, fbHeight,
        gridWidth, gridHeight,
        vLeft, vRight,
        vBottom, vTop
    );
    coordinateMapper = &mapperObj;

    // Place listener at center of room by default
    simulation->setListenerPosition(gridWidth / 2, gridHeight / 2);
    simulation->setListenerEnabled(true);
    std::cout << "Listener initialized at center: (" << (gridWidth / 2) << ", " << (gridHeight / 2) << ")" << std::endl;

    // Initialize UI presentation layer
    SimulationUI simulationUI(
        simulation, audioOutput, coordinateMapper,
        showHelp, timeScale, obstacleMode, obstacleRadius,
        listenerMode, sourceMode, selectedPreset, sourceVolumeDb, sourceLoop, loadedSample
    );

    // Initialize input handler
    InputHandler inputHandlerObj(
        simulation, audioOutput, coordinateMapper, renderer,
        showHelp, timeScale, obstacleMode, obstacleRadius,
        listenerMode, draggingListener, sourceMode, selectedPreset,
        sourceVolumeDb, sourceLoop, loadedSample,
        mousePressed, lastMouseX, lastMouseY, windowWidth, windowHeight
    );
    inputHandler = &inputHandlerObj;

    // Set up GLFW input callbacks using lambda wrappers
    glfwSetWindowUserPointer(window, inputHandler);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow* w, int width, int height) {
        InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        if (handler) handler->handleFramebufferResize(w, width, height);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
        InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        if (handler) handler->handleMouseButton(w, button, action, mods);
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
        InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        if (handler) handler->handleCursorPos(w, xpos, ypos);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        InputHandler* handler = static_cast<InputHandler*>(glfwGetWindowUserPointer(w));
        if (handler) handler->handleKey(w, key, scancode, action, mods);
    });

    std::cout << "\n=== Acoustic Pressure Simulation ===" << std::endl;
    std::cout << "20m x 10m closed room with reflective walls" << std::endl;
    std::cout << "Real-time audio output enabled!" << std::endl;
    std::cout << "\nControls:" << std::endl;
    std::cout << "  Left Click: Create sound impulse (clap)" << std::endl;
    std::cout << "  V: Toggle listener mode (virtual microphone)" << std::endl;
    std::cout << "  M: Mute/Unmute audio" << std::endl;
    std::cout << "  Shift+UP/DOWN: Volume control" << std::endl;
    std::cout << "  O: Toggle obstacle mode" << std::endl;
    std::cout << "  Right Click: Remove obstacles" << std::endl;
    std::cout << "  C: Clear obstacles | Shift+[/]: Obstacle size" << std::endl;
    std::cout << "  L: Load SVG room layout" << std::endl;
    std::cout << "  SPACE: Clear waves" << std::endl;
    std::cout << "  +/- or [/]: Adjust time scale (slow motion)" << std::endl;
    std::cout << "  1: 20x slower | 0: real-time" << std::endl;
    std::cout << "  UP/DOWN: Adjust sound speed" << std::endl;
    std::cout << "  LEFT/RIGHT: Adjust air absorption" << std::endl;
    std::cout << "  H: Toggle help overlay" << std::endl;
    std::cout << "  ESC: Exit" << std::endl;
    std::cout << "=========================================\n" << std::endl;
    std::cout << "Starting at 100x slower for audible sound (press '0' for real-time)\n" << std::endl;

    // Main loop
    auto lastTime = std::chrono::high_resolution_clock::now();
    const float fixedDt = 1.0f / 60.0f; // Fixed timestep for physics

    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        // Limit delta time to avoid large jumps
        deltaTime = std::min(deltaTime, 0.1f);

        // Update simulation with scaled time step (for slow motion)
        simulation->update(fixedDt * timeScale);

        // Submit all listener samples collected during sub-stepping
        // CRITICAL FIX: This preserves all high-frequency audio content!
        // Previously sampled once per frame (60 Hz) - missing 99% of audio
        // Now submits all ~191 sub-step samples (~11 kHz effective rate)
        // Uses bulk submission with proper upsampling to avoid buffer overflow
        if (simulation->hasListener() && audioOutput) {
            std::vector<float> samples = simulation->getListenerSamples();
            audioOutput->submitPressureSamples(samples, timeScale);
        }

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render
        rendererObj.render(*simulation);

        // Render UI (listener marker, audio source markers, help overlay)
        simulationUI.render();

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete simulation;
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
    }

    // Application destructor handles ImGui and GLFW cleanup
    return 0;
}
