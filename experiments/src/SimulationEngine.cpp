#include "SimulationEngine.h"
#include "AudioFileLoader.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include <algorithm>

SimulationEngine::SimulationEngine(Application& app)
    : application(app)
    , windowWidth(1280)
    , windowHeight(720)
    , mousePressed(false)
    , lastMouseX(0.0)
    , lastMouseY(0.0)
    , showHelp(true)
    , timeScale(0.01f)  // 100x slower for audible sound
    , obstacleMode(false)
    , obstacleRadius(5)
    , listenerMode(false)
    , draggingListener(false)
    , sourceMode(false)
    , selectedPreset(0)
    , sourceVolumeDb(0.0f)
    , sourceLoop(true)
    , loadedSample(nullptr)
    , gridWidth(800)   // 20m / 0.025m = 800 pixels (BALANCED: 6.8 kHz support)
    , gridHeight(400)  // 10m / 0.025m = 400 pixels
    , lastFrameTime(0.0)
    , simulationTimeBudget(0.014)  // 14ms budget for simulation (leaves 2.7ms for rendering at 60 FPS)
{
}

SimulationEngine::~SimulationEngine() {
    // Stop and cleanup audio
    if (audioOutput) {
        audioOutput->stop();
    }

    // unique_ptr handles automatic cleanup of all subsystems
}

bool SimulationEngine::initialize() {
    GLFWwindow* window = application.getWindow();

    // Get actual framebuffer size (accounts for DPI scaling)
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    windowWidth = fbWidth;
    windowHeight = fbHeight;

    // Initialize subsystems
    if (!initializeSubsystems()) {
        return false;
    }

    // Set up input callbacks
    glfwSetWindowUserPointer(window, inputHandler.get());

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

    // Print initialization info
    printInitializationInfo();

    return true;
}

bool SimulationEngine::initializeSubsystems() {
    GLFWwindow* window = application.getWindow();

    // Create simulation (BALANCED RESOLUTION)
    // Room: 10m (height) x 20m (width)
    // Scale: 1 pixel = 2.5 cm = 0.025 m
    // Max frequency: f_max = c / (2*dx) = 343 / 0.050 = 6.86 kHz
    simulation = std::make_unique<WaveSimulation>(gridWidth, gridHeight);

    // Initialize audio output
    audioOutput = std::make_unique<AudioOutput>();
    if (!audioOutput->initialize(48000)) {
        std::cerr << "Warning: Failed to initialize audio output" << std::endl;
        std::cerr << audioOutput->getLastError() << std::endl;
    } else {
        audioOutput->start();
        std::cout << "Audio output: Initialized and started" << std::endl;
    }

    // Initialize renderer
    renderer = std::make_unique<Renderer>(windowWidth, windowHeight);
    if (!renderer->initialize()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return false;
    }

    // Get window dimensions for coordinate mapper
    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    // Get viewport info from renderer
    float vLeft, vRight, vBottom, vTop;
    renderer->getRoomViewport(vLeft, vRight, vBottom, vTop);

    // Initialize coordinate mapper
    coordinateMapper = std::make_unique<CoordinateMapper>();
    coordinateMapper->updateViewport(
        winWidth, winHeight,
        windowWidth, windowHeight,
        gridWidth, gridHeight,
        vLeft, vRight,
        vBottom, vTop
    );

    // Place listener at center of room by default
    simulation->setListenerPosition(gridWidth / 2, gridHeight / 2);
    simulation->setListenerEnabled(true);
    std::cout << "Listener initialized at center: (" << (gridWidth / 2) << ", " << (gridHeight / 2) << ")" << std::endl;

    // Initialize UI
    simulationUI = std::make_unique<SimulationUI>(
        simulation.get(), audioOutput.get(), coordinateMapper.get(),
        showHelp, timeScale, obstacleMode, obstacleRadius,
        listenerMode, sourceMode, selectedPreset, sourceVolumeDb, sourceLoop, loadedSample
    );

    // Initialize input handler
    inputHandler = std::make_unique<InputHandler>(
        simulation.get(), audioOutput.get(), coordinateMapper.get(), renderer.get(),
        showHelp, timeScale, obstacleMode, obstacleRadius,
        listenerMode, draggingListener, sourceMode, selectedPreset,
        sourceVolumeDb, sourceLoop, loadedSample,
        mousePressed, lastMouseX, lastMouseY, windowWidth, windowHeight
    );

    return true;
}

void SimulationEngine::printInitializationInfo() {
    GLFWwindow* window = application.getWindow();

    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);

    // Print physical dimensions
    std::cout << "\nPhysical dimensions:" << std::endl;
    std::cout << "  Window: " << winWidth << " x " << winHeight << " (window coords)" << std::endl;
    std::cout << "  Framebuffer: " << windowWidth << " x " << windowHeight << " (framebuffer coords)" << std::endl;
    std::cout << "  Grid: " << gridWidth << " x " << gridHeight << " pixels (W x H) [BALANCED]" << std::endl;
    std::cout << "  Scale: 1 pixel = 2.5 cm = 25 mm" << std::endl;
    std::cout << "  Max frequency: ~6.8 kHz (Nyquist limit - good music quality)" << std::endl;
    std::cout << "  Memory: ~" << (gridWidth * gridHeight * 3 * 4 / 1024 / 1024) << " MB for pressure fields" << std::endl;
    std::cout << "  Room size: " << simulation->getPhysicalWidth() << " m x "
              << simulation->getPhysicalHeight() << " m (W x H)" << std::endl;
    std::cout << "  Speed of sound: " << simulation->getWaveSpeed() << " m/s" << std::endl;

    // Print viewport info
    float vLeft, vRight, vBottom, vTop;
    renderer->getRoomViewport(vLeft, vRight, vBottom, vTop);
    std::cout << "  Viewport: (" << vLeft << ", " << vBottom << ") to ("
              << vRight << ", " << vTop << ")" << std::endl;

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
}

void SimulationEngine::run() {
    GLFWwindow* window = application.getWindow();

    while (!glfwWindowShouldClose(window)) {
        // Adaptive frame skipping: Only update simulation if we have time budget
        // This keeps UI responsive even when simulation is slow
        if (lastFrameTime < simulationTimeBudget) {
            auto simStart = std::chrono::high_resolution_clock::now();
            update();
            auto simEnd = std::chrono::high_resolution_clock::now();
            lastFrameTime = std::chrono::duration<double>(simEnd - simStart).count();
        } else {
            // Skip simulation update to maintain UI responsiveness
            // Still consume frame time for smoother behavior
            lastFrameTime *= 0.95;  // Decay to allow recovery
        }

        // Always render UI (keeps it responsive)
        render();

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void SimulationEngine::update() {
    const float fixedDt = 1.0f / 60.0f; // Fixed timestep for physics

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
}

void SimulationEngine::render() {
    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Render simulation
    renderer->render(*simulation);

    // Render UI (listener marker, audio source markers, help overlay)
    simulationUI->render();

    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
