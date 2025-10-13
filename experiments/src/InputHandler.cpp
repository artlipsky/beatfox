#include "InputHandler.h"
#include "WaveSimulation.h"
#include "AudioOutput.h"
#include "CoordinateMapper.h"
#include "Renderer.h"
#include "AudioSample.h"
#include "AudioSource.h"
#include "portable-file-dialogs.h"
#include <iostream>
#include <algorithm>

InputHandler::InputHandler(
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
)
    : simulation(sim)
    , audioOutput(audio)
    , coordinateMapper(mapper)
    , renderer(rend)
    , showHelp(showHelp)
    , timeScale(timeScale)
    , obstacleMode(obstacleMode)
    , obstacleRadius(obstacleRadius)
    , listenerMode(listenerMode)
    , draggingListener(draggingListener)
    , sourceMode(sourceMode)
    , selectedPreset(selectedPreset)
    , sourceVolumeDb(sourceVolumeDb)
    , sourceLoop(sourceLoop)
    , loadedSample(loadedSample)
    , mousePressed(mousePressed)
    , lastMouseX(lastMouseX)
    , lastMouseY(lastMouseY)
    , windowWidth(windowWidth)
    , windowHeight(windowHeight)
{
}

bool InputHandler::screenToGrid(double screenX, double screenY, int& gridX, int& gridY) {
    if (!coordinateMapper) return false;
    return coordinateMapper->screenToGrid(screenX, screenY, gridX, gridY);
}

void InputHandler::handleFramebufferResize(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    if (renderer) {
        renderer->resize(width, height);

        // Update coordinate mapper with new window dimensions
        if (coordinateMapper && simulation) {
            int winWidth, winHeight;
            glfwGetWindowSize(window, &winWidth, &winHeight);

            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

            float vLeft, vRight, vBottom, vTop;
            renderer->getRoomViewport(vLeft, vRight, vBottom, vTop);

            coordinateMapper->updateViewport(
                winWidth, winHeight,
                fbWidth, fbHeight,
                simulation->getWidth(), simulation->getHeight(),
                vLeft, vRight,
                vBottom, vTop
            );
        }
    }
}

void InputHandler::handleMouseButton(GLFWwindow* window, int button, int action, int /*mods*/) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMouseX = xpos;
            lastMouseY = ypos;

            int gridX, gridY;
            if (screenToGrid(xpos, ypos, gridX, gridY)) {
                // Check if clicking on existing listener for dragging
                if (simulation->hasListener()) {
                    int listenerX, listenerY;
                    simulation->getListenerPosition(listenerX, listenerY);

                    // Check if click is within listener radius (use Manhattan distance for simplicity)
                    int dx = gridX - listenerX;
                    int dy = gridY - listenerY;
                    int distSquared = dx * dx + dy * dy;
                    const int listenerRadiusSquared = 10 * 10;  // 10 pixel radius

                    if (distSquared <= listenerRadiusSquared) {
                        // Start dragging the listener
                        draggingListener = true;
                        return;  // Don't process other actions
                    }
                }

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
            draggingListener = false;  // Stop dragging listener
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

void InputHandler::handleCursorPos(GLFWwindow* /*window*/, double xpos, double ypos) {
    // Track mouse position
    lastMouseX = xpos;
    lastMouseY = ypos;

    // Update listener position if dragging
    if (draggingListener && simulation) {
        int gridX, gridY;
        if (screenToGrid(xpos, ypos, gridX, gridY)) {
            simulation->setListenerPosition(gridX, gridY);
        }
    }
}

void InputHandler::handleKey(GLFWwindow* window, int key, int /*scancode*/, int action, int mods) {
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
