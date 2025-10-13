#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <chrono>
#include "WaveSimulation.h"
#include "DampingPreset.h"
#include "Renderer.h"
#include "AudioOutput.h"
#include "AudioSource.h"
#include "AudioSample.h"
#include "AudioFileLoader.h"
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
float timeScale = 0.01f;  // Time scale: 0.01 = 100x slower (for audible sound), 1.0 = real-time

// Obstacle mode
bool obstacleMode = false;  // Toggle with 'O' key
int obstacleRadius = 5;     // Obstacle brush size (pixels)

// Listener mode (virtual microphone)
bool listenerMode = false;  // Toggle with 'V' key

// Audio source mode
bool sourceMode = false;    // Toggle with 'S' key
int selectedPreset = 0;     // 0=Kick, 1=Snare, 2=Tone, 3=Impulse, 4=LoadedFile
float sourceVolumeDb = 0.0f;  // Volume in dB
bool sourceLoop = true;     // Loop audio
std::shared_ptr<AudioSample> loadedSample;  // User-loaded audio file

// Convert screen coordinates to simulation grid coordinates
// Returns false if click is outside the room
bool screenToGrid(double screenX, double screenY, int& gridX, int& gridY) {
    if (!simulation || !renderer) return false;

    // GLFW gives window coordinates, need to convert to framebuffer coordinates
    // Get window size and framebuffer size
    int winWidth, winHeight;
    glfwGetWindowSize(glfwGetCurrentContext(), &winWidth, &winHeight);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbWidth, &fbHeight);

    // Scale factor from window to framebuffer
    float scaleX = (float)fbWidth / (float)winWidth;
    float scaleY = (float)fbHeight / (float)winHeight;

    // Convert to framebuffer coordinates
    float fbX = screenX * scaleX;
    float fbY = screenY * scaleY;

    // Get room viewport bounds in framebuffer coordinates
    float viewLeft, viewRight, viewBottom, viewTop;
    renderer->getRoomViewport(viewLeft, viewRight, viewBottom, viewTop);

    // Framebuffer Y is bottom-up, but input Y is top-down
    float fbYFlipped = fbHeight - fbY;

    // Check if click is inside the room viewport
    if (fbX < viewLeft || fbX > viewRight ||
        fbYFlipped < viewBottom || fbYFlipped > viewTop) {
        return false;  // Click outside room
    }

    // Map from room viewport to simulation grid
    float normalizedX = (fbX - viewLeft) / (viewRight - viewLeft);
    float normalizedY = (fbYFlipped - viewBottom) / (viewTop - viewBottom);

    gridX = static_cast<int>(normalizedX * simulation->getWidth());
    gridY = static_cast<int>(normalizedY * simulation->getHeight());

    // Clamp to valid range
    gridX = std::max(0, std::min(gridX, simulation->getWidth() - 1));
    gridY = std::max(0, std::min(gridY, simulation->getHeight() - 1));

    return true;  // Valid click inside room
}

// Callbacks
void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->resize(width, height);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMouseX = xpos;
            lastMouseY = ypos;

            int gridX, gridY;
            if (screenToGrid(xpos, ypos, gridX, gridY)) {
                if (listenerMode) {
                    // Place listener (virtual microphone)
                    simulation->setListenerPosition(gridX, gridY);
                    simulation->setListenerEnabled(true);
                    std::cout << "Listener placed at (" << gridX << ", " << gridY << ")" << std::endl;
                } else if (obstacleMode) {
                    // Place obstacle
                    simulation->addObstacle(gridX, gridY, obstacleRadius);
                } else if (sourceMode) {
                    // Place audio source
                    std::shared_ptr<AudioSample> sample;

                    // Get sample based on selection
                    if (selectedPreset == 0) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateKick());
                    } else if (selectedPreset == 1) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateSnare());
                    } else if (selectedPreset == 2) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateTone(440.0f, 1.0f));
                    } else if (selectedPreset == 3) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateImpulse());
                    } else if (selectedPreset == 4) {
                        // Use loaded file
                        sample = loadedSample;
                        if (!sample) {
                            std::cerr << "No audio file loaded! Please load a file first." << std::endl;
                        }
                    }

                    if (sample) {
                        auto source = std::make_unique<AudioSource>(sample, gridX, gridY, sourceVolumeDb, sourceLoop);
                        source->play();
                        simulation->addAudioSource(std::move(source));
                        std::cout << "Audio source placed at (" << gridX << ", " << gridY << "), volume: " << sourceVolumeDb << " dB" << std::endl;
                    }
                } else {
                    // Create a single impulse (like a hand clap)
                    // Brief pressure impulse (5 Pa - typical hand clap)
                    // For reference: whisper ~0.01 Pa, conversation ~0.1 Pa, clap ~5 Pa
                    simulation->addPressureSource(gridX, gridY, 5.0f);
                }
            }
        } else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Right-click to remove obstacles
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            int gridX, gridY;
            if (screenToGrid(xpos, ypos, gridX, gridY)) {
                simulation->removeObstacle(gridX, gridY, obstacleRadius);
            }
        }
    }
}

void cursorPosCallback(GLFWwindow* /*window*/, double xpos, double ypos) {
    // Track mouse position
    // Note: We don't add sources while dragging - click creates single impulses
    lastMouseX = xpos;
    lastMouseY = ypos;
}

void keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_H:
                showHelp = !showHelp;
                std::cout << "Help " << (showHelp ? "shown" : "hidden") << std::endl;
                break;
            case GLFW_KEY_SPACE:
                if (simulation) {
                    simulation->clear();
                }
                break;
            case GLFW_KEY_UP:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+UP: Increase volume
                    if (audioOutput) {
                        float vol = audioOutput->getVolume();
                        audioOutput->setVolume(std::min(2.0f, vol + 0.1f));
                    }
                } else {
                    // UP: Increase sound speed
                    if (simulation) {
                        float speed = simulation->getWaveSpeed();
                        simulation->setWaveSpeed(speed + 10.0f);  // ±10 m/s increments
                        std::cout << "Sound speed: " << simulation->getWaveSpeed() << " m/s";
                        std::cout << " (normal air: 343 m/s)" << std::endl;
                    }
                }
                break;
            case GLFW_KEY_DOWN:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+DOWN: Decrease volume
                    if (audioOutput) {
                        float vol = audioOutput->getVolume();
                        audioOutput->setVolume(std::max(0.0f, vol - 0.1f));
                    }
                } else {
                    // DOWN: Decrease sound speed
                    if (simulation) {
                        float speed = simulation->getWaveSpeed();
                        simulation->setWaveSpeed(std::max(50.0f, speed - 10.0f));  // Min 50 m/s
                        std::cout << "Sound speed: " << simulation->getWaveSpeed() << " m/s";
                        std::cout << " (normal air: 343 m/s)" << std::endl;
                    }
                }
                break;
            case GLFW_KEY_RIGHT:
                if (simulation) {
                    float damp = simulation->getDamping();
                    simulation->setDamping(std::min(0.9995f, damp + 0.0001f));
                    std::cout << "Air absorption: " << (1.0f - simulation->getDamping()) * 100.0f << "%" << std::endl;
                }
                break;
            case GLFW_KEY_LEFT:
                if (simulation) {
                    float damp = simulation->getDamping();
                    simulation->setDamping(std::max(0.99f, damp - 0.0001f));
                    std::cout << "Air absorption: " << (1.0f - simulation->getDamping()) * 100.0f << "%" << std::endl;
                }
                break;
            case GLFW_KEY_EQUAL:  // Plus/Equals key (speed up)
                timeScale = std::min(2.0f, timeScale * 1.5f);
                std::cout << "Time scale: " << timeScale << "x";
                if (timeScale < 1.0f) {
                    std::cout << " (" << (1.0f / timeScale) << "x slower)";
                }
                std::cout << std::endl;
                break;
            case GLFW_KEY_MINUS:  // Minus key (slow down)
                timeScale = std::max(0.01f, timeScale / 1.5f);
                std::cout << "Time scale: " << timeScale << "x";
                if (timeScale < 1.0f) {
                    std::cout << " (" << (1.0f / timeScale) << "x slower)";
                }
                std::cout << std::endl;
                break;
            case GLFW_KEY_RIGHT_BRACKET:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+] for obstacle radius
                    obstacleRadius = std::min(20, obstacleRadius + 1);
                    std::cout << "Obstacle radius: " << obstacleRadius << " pixels" << std::endl;
                } else {
                    // ] for time speed up
                    timeScale = std::min(2.0f, timeScale * 1.5f);
                    std::cout << "Time scale: " << timeScale << "x";
                    if (timeScale < 1.0f) {
                        std::cout << " (" << (1.0f / timeScale) << "x slower)";
                    }
                    std::cout << std::endl;
                }
                break;
            case GLFW_KEY_LEFT_BRACKET:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+[ for obstacle radius
                    obstacleRadius = std::max(1, obstacleRadius - 1);
                    std::cout << "Obstacle radius: " << obstacleRadius << " pixels" << std::endl;
                } else {
                    // [ for time slow down
                    timeScale = std::max(0.01f, timeScale / 1.5f);
                    std::cout << "Time scale: " << timeScale << "x";
                    if (timeScale < 1.0f) {
                        std::cout << " (" << (1.0f / timeScale) << "x slower)";
                    }
                    std::cout << std::endl;
                }
                break;
            case GLFW_KEY_0:  // Reset to real-time
                timeScale = 1.0f;
                std::cout << "Time scale: 1.0x (real-time)" << std::endl;
                break;
            case GLFW_KEY_1:  // 20x slower
                timeScale = 0.05f;
                std::cout << "Time scale: 0.05x (20x slower)" << std::endl;
                break;
            case GLFW_KEY_O:  // Toggle obstacle mode
                obstacleMode = !obstacleMode;
                listenerMode = false;  // Disable listener mode
                sourceMode = false;    // Disable source mode
                std::cout << "Obstacle mode: " << (obstacleMode ? "ON" : "OFF") << std::endl;
                break;
            case GLFW_KEY_V:  // Toggle listener mode
                listenerMode = !listenerMode;
                obstacleMode = false;  // Disable obstacle mode
                sourceMode = false;    // Disable source mode
                std::cout << "Listener mode: " << (listenerMode ? "ON" : "OFF") << std::endl;
                if (listenerMode) {
                    std::cout << "Click to place listener (virtual microphone)" << std::endl;
                }
                break;
            case GLFW_KEY_S:  // Toggle source mode
                sourceMode = !sourceMode;
                obstacleMode = false;  // Disable obstacle mode
                listenerMode = false;  // Disable listener mode
                std::cout << "Audio Source mode: " << (sourceMode ? "ON" : "OFF") << std::endl;
                if (sourceMode) {
                    std::cout << "Click to place audio source (current: ";
                    const char* presets[] = {"Kick", "Snare", "Tone", "Impulse", "File"};
                    std::cout << presets[selectedPreset] << ")" << std::endl;
                }
                break;
            case GLFW_KEY_M:  // Toggle mute
                if (audioOutput) {
                    bool muted = !audioOutput->isMuted();
                    audioOutput->setMuted(muted);
                }
                break;
            case GLFW_KEY_C:  // Clear obstacles
                if (simulation) {
                    simulation->clearObstacles();
                    std::cout << "Obstacles cleared" << std::endl;
                }
                break;
            case GLFW_KEY_L:  // Load SVG file
                if (simulation) {
                    std::cout << "Opening file dialog..." << std::endl;

                    // Open file dialog for SVG files
                    auto selection = pfd::open_file("Load SVG Room Layout",
                                                    "",
                                                    { "SVG Files", "*.svg",
                                                      "All Files", "*" },
                                                    pfd::opt::none).result();

                    if (!selection.empty()) {
                        std::string filename = selection[0];
                        std::cout << "Loading: " << filename << std::endl;

                        bool success = simulation->loadObstaclesFromSVG(filename);

                        if (success) {
                            std::cout << "Successfully loaded SVG layout" << std::endl;
                        } else {
                            std::cerr << "Failed to load SVG file" << std::endl;
                        }
                    } else {
                        std::cout << "File dialog cancelled" << std::endl;
                    }
                }
                break;
            case GLFW_KEY_G:  // Toggle GPU acceleration
                if (simulation) {
                    bool currentlyUsingGPU = simulation->isGPUEnabled();
                    simulation->setGPUEnabled(!currentlyUsingGPU);
                    std::cout << "GPU Acceleration: " << (simulation->isGPUEnabled() ? "ENABLED" : "DISABLED") << std::endl;
                    if (!simulation->isGPUAvailable()) {
                        std::cout << "Note: GPU not available on this system" << std::endl;
                    }
                }
                break;
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac

    // Create window
    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight,
                                          "Acoustic Pressure Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetKeyCallback(window, keyCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Print OpenGL info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Get window content scale for DPI
    float xscale, yscale;
    glfwGetWindowContentScale(window, &xscale, &yscale);
    float dpiScale = xscale; // Use x scale (typically both are the same)

    std::cout << "DPI Scale: " << dpiScale << "x" << std::endl;

    // Setup Dear ImGui style - NO scaling, keep logical sizes
    ImGui::StyleColorsDark();

    // Load font with proper DPI scaling for sharpness, but reasonable size
    float baseFontSize = 14.0f;  // Logical size in points
    ImFontConfig fontConfig;
    fontConfig.SizePixels = baseFontSize * dpiScale;  // Physical pixels for sharpness
    fontConfig.RasterizerMultiply = 1.0f;
    io.Fonts->AddFontDefault(&fontConfig);
    io.FontGlobalScale = 1.0f / dpiScale;  // Scale back to logical size

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

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

    glfwSetWindowUserPointer(window, renderer);

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

        // Render listener indicator if enabled
        if (simulation->hasListener()) {
            int listenerX, listenerY;
            simulation->getListenerPosition(listenerX, listenerY);

            // Get window and framebuffer sizes for coordinate conversion
            GLFWwindow* window = glfwGetCurrentContext();
            int winWidth, winHeight;
            glfwGetWindowSize(window, &winWidth, &winHeight);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

            // Get room viewport bounds (in framebuffer coordinates)
            float viewLeft, viewRight, viewBottom, viewTop;
            renderer->getRoomViewport(viewLeft, viewRight, viewBottom, viewTop);

            // Map grid coordinates to framebuffer coordinates (bottom-up Y)
            float normalizedX = (float)listenerX / (float)simulation->getWidth();
            float normalizedY = (float)listenerY / (float)simulation->getHeight();

            float fbX = viewLeft + normalizedX * (viewRight - viewLeft);
            float fbY = viewBottom + normalizedY * (viewTop - viewBottom);

            // Convert from framebuffer coordinates to window coordinates
            // ImGui uses window coordinates (top-down Y)
            float scaleX = (float)winWidth / (float)fbWidth;
            float scaleY = (float)winHeight / (float)fbHeight;

            float windowX = fbX * scaleX;
            float windowY = (fbHeight - fbY) * scaleY;  // Flip Y axis

            // Draw listener marker using ImGui draw list (overlay)
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            // Draw a green circle for the listener
            drawList->AddCircleFilled(ImVec2(windowX, windowY), 8.0f,
                                     IM_COL32(50, 255, 100, 200));
            drawList->AddCircle(ImVec2(windowX, windowY), 8.0f,
                               IM_COL32(255, 255, 255, 255), 0, 2.0f);

            // Draw a small microphone icon (simplified)
            drawList->AddCircle(ImVec2(windowX, windowY - 3), 3.0f,
                               IM_COL32(255, 255, 255, 255), 0, 1.5f);
        }

        // Render audio source indicators
        auto& audioSources = simulation->getAudioSources();
        if (!audioSources.empty()) {
            // Get window and framebuffer sizes for coordinate conversion
            GLFWwindow* window = glfwGetCurrentContext();
            int winWidth, winHeight;
            glfwGetWindowSize(window, &winWidth, &winHeight);
            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

            // Get room viewport bounds (in framebuffer coordinates)
            float viewLeft, viewRight, viewBottom, viewTop;
            renderer->getRoomViewport(viewLeft, viewRight, viewBottom, viewTop);

            ImDrawList* drawList = ImGui::GetBackgroundDrawList();

            for (size_t i = 0; i < audioSources.size(); i++) {
                const auto& source = audioSources[i];
                if (!source) continue;

                int sourceX = source->getX();
                int sourceY = source->getY();

                // Map grid coordinates to framebuffer coordinates (bottom-up Y)
                float normalizedX = (float)sourceX / (float)simulation->getWidth();
                float normalizedY = (float)sourceY / (float)simulation->getHeight();

                float fbX = viewLeft + normalizedX * (viewRight - viewLeft);
                float fbY = viewBottom + normalizedY * (viewTop - viewBottom);

                // Convert from framebuffer coordinates to window coordinates
                // ImGui uses window coordinates (top-down Y)
                float scaleX = (float)winWidth / (float)fbWidth;
                float scaleY = (float)winHeight / (float)fbHeight;

                float windowX = fbX * scaleX;
                float windowY = (fbHeight - fbY) * scaleY;  // Flip Y axis

                // Choose color based on playback state
                ImU32 fillColor = source->isPlaying()
                    ? IM_COL32(255, 150, 50, 200)   // Orange for playing
                    : IM_COL32(150, 150, 150, 150); // Gray for paused/stopped

                ImU32 outlineColor = source->isPlaying()
                    ? IM_COL32(255, 200, 100, 255)  // Bright orange outline
                    : IM_COL32(200, 200, 200, 255); // Gray outline

                // Draw audio source marker (circle with speaker icon)
                drawList->AddCircleFilled(ImVec2(windowX, windowY), 8.0f, fillColor);
                drawList->AddCircle(ImVec2(windowX, windowY), 8.0f, outlineColor, 0, 2.0f);

                // Draw speaker icon (simplified: three curved lines)
                if (source->isPlaying()) {
                    // Small speaker box
                    drawList->AddRectFilled(ImVec2(windowX - 3, windowY - 2),
                                          ImVec2(windowX - 1, windowY + 2),
                                          IM_COL32(255, 255, 255, 255));
                    // Sound waves
                    drawList->AddCircle(ImVec2(windowX, windowY), 3.0f,
                                       IM_COL32(255, 255, 255, 200), 12, 1.0f);
                    drawList->AddCircle(ImVec2(windowX, windowY), 5.0f,
                                       IM_COL32(255, 255, 255, 150), 12, 1.0f);
                }
            }
        }

        // Render help overlay if enabled with ImGui
        if (showHelp) {
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowBgAlpha(0.9f);
            ImGui::Begin("Controls", &showHelp, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.8f, 1.0f, 1.0f));
            ImGui::Text("ACOUSTIC SIMULATION");
            ImGui::PopStyleColor();

            ImGui::TextDisabled("20m x 10m room (1px = 5cm)");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Physical parameters:");
            ImGui::BulletText("Speed: %.0f m/s", simulation ? simulation->getWaveSpeed() : 343.0f);
            ImGui::BulletText("Scale: 1 px = 5.0 cm [optimized]");

            ImGui::Spacing();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.3f, 1.0f));
            if (timeScale < 1.0f) {
                ImGui::BulletText("Time: %.2fx (%.0fx slower)", timeScale, 1.0f / timeScale);
            } else {
                ImGui::BulletText("Time: %.2fx", timeScale);
            }

            // Volume indicator
            if (audioOutput) {
                float volume = audioOutput->getVolume();
                int volumePercent = static_cast<int>(volume * 100.0f);
                ImGui::BulletText("Volume: %.1fx (%d%%)", volume, volumePercent);
            }

            // GPU status indicator
            if (simulation) {
                if (simulation->isGPUEnabled()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));  // Green
                    ImGui::BulletText("GPU: ENABLED (Metal)");
                    ImGui::PopStyleColor();
                } else if (simulation->isGPUAvailable()) {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));  // Orange
                    ImGui::BulletText("GPU: Available (press G)");
                    ImGui::PopStyleColor();
                } else {
                    ImGui::TextDisabled("GPU: Not available");
                }
            }
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Acoustic Environment Presets (Domain-driven)
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.3f, 1.0f));
            ImGui::Text("Acoustic Environment:");
            ImGui::PopStyleColor();

            auto currentPreset = simulation ? simulation->getCurrentPreset() : DampingPreset::fromType(DampingPreset::Type::REALISTIC);

            if (simulation) {
                // Radio button for each preset type
                bool isRealistic = (currentPreset.getType() == DampingPreset::Type::REALISTIC);
                if (ImGui::RadioButton("Realistic", isRealistic)) {
                    simulation->applyDampingPreset(DampingPreset::fromType(DampingPreset::Type::REALISTIC));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Real-world room acoustics\nAir absorption: 0.3%%, Wall reflection: 85%%");
                }

                bool isVisualization = (currentPreset.getType() == DampingPreset::Type::VISUALIZATION);
                if (ImGui::RadioButton("Visualization", isVisualization)) {
                    simulation->applyDampingPreset(DampingPreset::fromType(DampingPreset::Type::VISUALIZATION));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Minimal damping for clear wave patterns\n"
                                     "Air: 0.02%% loss, Walls: 98%% reflective\n"
                                     "Waves persist long, strong reflections");
                }

                bool isAnechoic = (currentPreset.getType() == DampingPreset::Type::ANECHOIC);
                if (ImGui::RadioButton("Anechoic Chamber", isAnechoic)) {
                    simulation->applyDampingPreset(DampingPreset::fromType(DampingPreset::Type::ANECHOIC));
                }
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("No wall reflections (perfect absorption)\n"
                                     "Air: 0.2%% loss, Walls: 0%% reflective\n"
                                     "Waves absorbed at walls, no echoes");
                }

                // Show current preset info
                ImGui::TextDisabled("%s", currentPreset.getDescription().c_str());
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Audio Sources Section
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.8f, 1.0f));
            ImGui::Text("Audio Sources:");
            ImGui::PopStyleColor();

            const char* presetNames[] = {"Kick Drum", "Snare Drum", "Tone (440Hz)", "Impulse", "Loaded File"};
            ImGui::Combo("Sample", &selectedPreset, presetNames, 5);

            ImGui::SliderFloat("Volume (dB)", &sourceVolumeDb, -40.0f, 20.0f, "%.1f dB");
            ImGui::Checkbox("Loop", &sourceLoop);

            if (ImGui::Button("Load Audio File")) {
                auto selection = pfd::open_file("Load Audio Sample",
                                                "",
                                                { "Audio Files", "*.mp3 *.wav *.flac",
                                                  "All Files", "*" },
                                                pfd::opt::none).result();

                if (!selection.empty()) {
                    std::string filename = selection[0];
                    std::cout << "Loading audio file: " << filename << std::endl;

                    auto sample = AudioFileLoader::loadFile(filename, 48000);

                    if (sample) {
                        // Store loaded sample for placement
                        loadedSample = sample;
                        selectedPreset = 4;  // Switch to "Loaded File" preset
                        std::cout << "Loaded: " << sample->getName() << " (" << sample->getDuration() << "s)" << std::endl;
                        std::cout << "Switched to 'Loaded File' preset" << std::endl;
                    } else {
                        std::cerr << "Failed to load: " << AudioFileLoader::getLastError() << std::endl;
                    }
                }
            }

            // Show active sources
            auto& sources = simulation->getAudioSources();
            if (!sources.empty()) {
                ImGui::Spacing();
                ImGui::TextDisabled("Active Sources: %zu", sources.size());

                if (ImGui::Button("Clear All Sources")) {
                    simulation->clearAudioSources();
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Controls:");

            if (sourceMode) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.8f, 1.0f));
                ImGui::BulletText("Left Click: Place audio source");
                ImGui::PopStyleColor();
            } else if (listenerMode) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));
                ImGui::BulletText("Left Click: Place listener (mic)");
                ImGui::PopStyleColor();
            } else if (obstacleMode) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
                ImGui::BulletText("Left Click: Place obstacle");
                ImGui::BulletText("Right Click: Remove obstacle");
                ImGui::PopStyleColor();
            } else {
                ImGui::BulletText("Left Click: Create sound (5 Pa)");
            }
            ImGui::BulletText("S: Audio Source mode (%s)", sourceMode ? "ON" : "OFF");
            ImGui::BulletText("V: Listener mode (%s)", listenerMode ? "ON" : "OFF");
            ImGui::BulletText("O: Obstacle mode (%s)", obstacleMode ? "ON" : "OFF");
            ImGui::BulletText("C: Clear obstacles");
            ImGui::BulletText("L: Load SVG layout");
            ImGui::BulletText("Shift+[/]: Obstacle size (%d px)", obstacleRadius);
            ImGui::BulletText("SPACE: Clear waves");
            ImGui::BulletText("+/- or [/]: Time speed");
            ImGui::BulletText("1: 20x slower | 0: real-time");
            ImGui::BulletText("UP/DOWN: Sound speed");
            ImGui::BulletText("Shift+UP/DOWN: Volume");
            ImGui::BulletText("M: Mute audio (%s)", (audioOutput && audioOutput->isMuted()) ? "ON" : "OFF");
            ImGui::BulletText("G: Toggle GPU (%s)", simulation && simulation->isGPUEnabled() ? "ON" : "OFF");
            ImGui::BulletText("LEFT/RIGHT: Absorption");
            ImGui::BulletText("H: Toggle help");

            ImGui::Spacing();
            ImGui::TextDisabled("Rigid walls reflect sound");

            ImGui::End();
        } else {
            // Show a small help button when panel is closed
            ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.7f);
            ImGui::Begin("HelpButton", nullptr,
                        ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoResize |
                        ImGuiWindowFlags_AlwaysAutoResize |
                        ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoSavedSettings);

            if (ImGui::Button("? Help (H)")) {
                showHelp = true;
            }

            ImGui::End();
        }

        // Render ImGui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup
    delete simulation;
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
    }
    glfwTerminate();

    return 0;
}
