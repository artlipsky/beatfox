#include "InputHandler.h"
#include "SimulationController.h"
#include "WaveSimulation.h"
#include "CoordinateMapper.h"
#include "AudioSample.h"
#include "AudioSource.h"
#include "AcousticUtils.h"
#include "portable-file-dialogs.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <iostream>
#include <algorithm>
#include <cmath>

InputHandler::InputHandler(
    SimulationController* ctrl,
    WaveSimulation* sim,
    CoordinateMapper* mapper
)
    : controller(ctrl)
    , simulation(sim)
    , coordinateMapper(mapper)
{
}

void InputHandler::updateSimulationPointer(WaveSimulation* newSim) {
    simulation = newSim;
}

std::vector<std::unique_ptr<UICommand>> InputHandler::collectCommands() {
    std::vector<std::unique_ptr<UICommand>> commands;
    commands.swap(pendingCommands);
    return commands;
}

bool InputHandler::screenToGrid(double screenX, double screenY, int& gridX, int& gridY) {
    if (!coordinateMapper) return false;
    return coordinateMapper->screenToGrid(screenX, screenY, gridX, gridY);
}

void InputHandler::handleFramebufferResize(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    // Note: Renderer resize is handled by SimulationEngine directly
    // We just track window dimensions here for coordinate mapping
}

void InputHandler::handleMouseButton(GLFWwindow* window, int button, int action, int mods) {
    // Forward event to ImGui first (since we use install_callbacks=false)
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // Let ImGui handle input if it wants to capture mouse
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) {
        return;
    }

    // Get current state
    const auto& state = controller->getState();

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMouseX = xpos;
            lastMouseY = ypos;

            int gridX, gridY;
            if (screenToGrid(xpos, ypos, gridX, gridY)) {
                // Check if clicking on listener position for toggle or dragging
                if (state.info.hasListener) {
                    int listenerX = state.info.listenerX;
                    int listenerY = state.info.listenerY;

                    // Check if click is within listener radius
                    int dx = gridX - listenerX;
                    int dy = gridY - listenerY;
                    int distSquared = dx * dx + dy * dy;
                    const int listenerRadiusSquared = 10 * 10;  // 10 pixel radius

                    if (distSquared <= listenerRadiusSquared) {
                        // Start tracking for potential drag or toggle
                        draggingListener = true;
                        return;  // Don't process other actions
                    }
                }

                // Check if clicking on existing audio source for play/pause toggle
                const auto& audioSources = simulation->getAudioSources();
                for (size_t i = 0; i < audioSources.size(); i++) {
                    AudioSource* source = simulation->getAudioSource(i);
                    if (!source) continue;

                    int sourceX = source->getX();
                    int sourceY = source->getY();

                    // Check if click is within source radius
                    int dx = gridX - sourceX;
                    int dy = gridY - sourceY;
                    int distSquared = dx * dx + dy * dy;
                    const int sourceRadiusSquared = 10 * 10;  // 10 pixel radius

                    if (distSquared <= sourceRadiusSquared) {
                        // Toggle play/pause state
                        pendingCommands.push_back(
                            std::make_unique<ToggleAudioSourcePlaybackCommand>(i)
                        );
                        if (source->isPlaying()) {
                            std::cout << "Audio source " << i << " paused" << std::endl;
                        } else {
                            std::cout << "Audio source " << i << " resumed" << std::endl;
                        }
                        return;  // Don't process other actions
                    }
                }

                // Handle mode-specific clicks
                if (state.listenerMode) {
                    // Place listener (virtual microphone)
                    pendingCommands.push_back(
                        std::make_unique<SetListenerPositionCommand>(gridX, gridY)
                    );
                    std::cout << "Listener placed at (" << gridX << ", " << gridY << ")" << std::endl;
                } else if (state.obstacleMode) {
                    // Place obstacle
                    pendingCommands.push_back(
                        std::make_unique<AddObstacleCommand>(gridX, gridY, state.obstacleRadius)
                    );
                } else if (state.sourceMode) {
                    // Place audio source
                    std::shared_ptr<AudioSample> sample;

                    // Get sample based on selection
                    if (state.selectedPreset == 0) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateKick());
                    } else if (state.selectedPreset == 1) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateSnare());
                    } else if (state.selectedPreset == 2) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateTone(440.0f, 1.0f));
                    } else if (state.selectedPreset == 3) {
                        sample = std::make_shared<AudioSample>(AudioSamplePresets::generateImpulse());
                    } else if (state.selectedPreset == 4) {
                        // Use loaded file
                        sample = state.loadedSample;
                        if (!sample) {
                            std::cerr << "No audio file loaded! Please load a file first." << std::endl;
                        }
                    }

                    if (sample) {
                        pendingCommands.push_back(
                            std::make_unique<AddAudioSourceCommand>(
                                gridX, gridY, sample, state.sourceVolumeDb, state.sourceLoop
                            )
                        );
                        std::cout << "Audio source placed at (" << gridX << ", " << gridY
                                  << "), volume: " << state.sourceVolumeDb << " dB" << std::endl;
                    }
                } else {
                    // Create a single impulse using user-defined parameters
                    pendingCommands.push_back(
                        std::make_unique<AddImpulseCommand>(
                            gridX, gridY, state.impulsePressure, state.impulseRadius
                        )
                    );

                    // Log impulse creation with physical parameters
                    const float dB_SPL = AcousticUtils::pressureToDbSpl(state.impulsePressure);
                    const float spreadMM = state.impulseRadius * simulation->getPixelSize();
                    std::cout << "Created impulse at (" << gridX << ", " << gridY << "): "
                              << state.impulsePressure << " Pa (" << dB_SPL << " dB SPL), "
                              << state.impulseRadius << " px (" << spreadMM << " mm spread)" << std::endl;
                }
            }
        } else if (action == GLFW_RELEASE) {
            // Check if we were dragging the listener
            if (draggingListener && simulation) {
                // Get current mouse position
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                // Calculate distance moved (in screen pixels)
                double dx = xpos - lastMouseX;
                double dy = ypos - lastMouseY;
                double distanceMoved = std::sqrt(dx * dx + dy * dy);

                // If mouse barely moved, treat it as a click (toggle)
                const double clickThreshold = 5.0;  // 5 pixels
                if (distanceMoved < clickThreshold) {
                    // Toggle listener enabled/disabled
                    pendingCommands.push_back(
                        std::make_unique<ToggleListenerCommand>(UICommand::Type::TOGGLE_LISTENER)
                    );
                    bool currentlyEnabled = simulation->hasListener();
                    std::cout << "Listener " << (currentlyEnabled ? "disabled" : "enabled") << std::endl;
                }
            }

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
                pendingCommands.push_back(
                    std::make_unique<RemoveObstacleCommand>(gridX, gridY, state.obstacleRadius)
                );
            }
        }
    }
}

void InputHandler::handleCursorPos(GLFWwindow* window, double xpos, double ypos) {
    // Forward event to ImGui first (since we use install_callbacks=false)
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    // Track mouse position
    lastMouseX = xpos;
    lastMouseY = ypos;

    // Update listener position if dragging
    if (draggingListener && simulation) {
        int gridX, gridY;
        if (screenToGrid(xpos, ypos, gridX, gridY)) {
            pendingCommands.push_back(
                std::make_unique<SetListenerPositionCommand>(gridX, gridY)
            );
        }
    }
}

void InputHandler::handleKey(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Forward event to ImGui first (since we use install_callbacks=false)
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    // Let ImGui handle input if it wants to capture keyboard
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard) {
        return;
    }

    // Get current state
    const auto& state = controller->getState();

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;

            case GLFW_KEY_H:
                pendingCommands.push_back(
                    std::make_unique<ToggleHelpCommand>(UICommand::Type::TOGGLE_HELP)
                );
                std::cout << "Help " << (!state.showHelp ? "shown" : "hidden") << std::endl;
                break;

            case GLFW_KEY_SPACE:
                pendingCommands.push_back(
                    std::make_unique<ClearWavesCommand>(UICommand::Type::CLEAR_WAVES)
                );
                break;

            case GLFW_KEY_UP:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+UP: Increase volume
                    float newVol = std::min(2.0f, state.audio.volume + 0.1f);
                    pendingCommands.push_back(
                        std::make_unique<SetVolumeCommand>(newVol)
                    );
                } else {
                    // UP: Increase sound speed
                    float newSpeed = state.info.waveSpeed + 10.0f;
                    pendingCommands.push_back(
                        std::make_unique<SetWaveSpeedCommand>(newSpeed)
                    );
                    std::cout << "Sound speed: " << newSpeed << " m/s (normal air: 343 m/s)" << std::endl;
                }
                break;

            case GLFW_KEY_DOWN:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+DOWN: Decrease volume
                    float newVol = std::max(0.0f, state.audio.volume - 0.1f);
                    pendingCommands.push_back(
                        std::make_unique<SetVolumeCommand>(newVol)
                    );
                } else {
                    // DOWN: Decrease sound speed
                    float newSpeed = std::max(50.0f, state.info.waveSpeed - 10.0f);
                    pendingCommands.push_back(
                        std::make_unique<SetWaveSpeedCommand>(newSpeed)
                    );
                    std::cout << "Sound speed: " << newSpeed << " m/s (normal air: 343 m/s)" << std::endl;
                }
                break;

            case GLFW_KEY_RIGHT:
                if (simulation) {
                    float damp = simulation->getDamping();
                    float newDamp = std::min(0.9995f, damp + 0.0001f);
                    pendingCommands.push_back(
                        std::make_unique<SetAirAbsorptionCommand>(newDamp)
                    );
                    std::cout << "Air absorption: " << (1.0f - newDamp) * 100.0f << "%" << std::endl;
                }
                break;

            case GLFW_KEY_LEFT:
                if (simulation) {
                    float damp = simulation->getDamping();
                    float newDamp = std::max(0.99f, damp - 0.0001f);
                    pendingCommands.push_back(
                        std::make_unique<SetAirAbsorptionCommand>(newDamp)
                    );
                    std::cout << "Air absorption: " << (1.0f - newDamp) * 100.0f << "%" << std::endl;
                }
                break;

            case GLFW_KEY_EQUAL: {  // Plus/Equals key (speed up)
                float newScale = std::min(2.0f, state.timeScale * 1.5f);
                pendingCommands.push_back(
                    std::make_unique<SetTimeScaleCommand>(newScale)
                );
                std::cout << "Time scale: " << newScale << "x";
                if (newScale < 1.0f) {
                    std::cout << " (" << (1.0f / newScale) << "x slower)";
                }
                std::cout << std::endl;
                break;
            }

            case GLFW_KEY_MINUS: {  // Minus key (slow down)
                float newScale = std::max(0.001f, state.timeScale / 1.5f);
                pendingCommands.push_back(
                    std::make_unique<SetTimeScaleCommand>(newScale)
                );
                std::cout << "Time scale: " << newScale << "x";
                if (newScale < 1.0f) {
                    std::cout << " (" << (1.0f / newScale) << "x slower)";
                }
                std::cout << std::endl;
                break;
            }

            case GLFW_KEY_RIGHT_BRACKET:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+] for obstacle radius
                    int newRadius = std::min(20, state.obstacleRadius + 1);
                    controller->getStateMutable().obstacleRadius = newRadius;
                    std::cout << "Obstacle radius: " << newRadius << " pixels" << std::endl;
                } else {
                    // ] for time speed up
                    float newScale = std::min(2.0f, state.timeScale * 1.5f);
                    pendingCommands.push_back(
                        std::make_unique<SetTimeScaleCommand>(newScale)
                    );
                    std::cout << "Time scale: " << newScale << "x";
                    if (newScale < 1.0f) {
                        std::cout << " (" << (1.0f / newScale) << "x slower)";
                    }
                    std::cout << std::endl;
                }
                break;

            case GLFW_KEY_LEFT_BRACKET:
                if (mods & GLFW_MOD_SHIFT) {
                    // Shift+[ for obstacle radius
                    int newRadius = std::max(1, state.obstacleRadius - 1);
                    controller->getStateMutable().obstacleRadius = newRadius;
                    std::cout << "Obstacle radius: " << newRadius << " pixels" << std::endl;
                } else {
                    // [ for time slow down
                    float newScale = std::max(0.001f, state.timeScale / 1.5f);
                    pendingCommands.push_back(
                        std::make_unique<SetTimeScaleCommand>(newScale)
                    );
                    std::cout << "Time scale: " << newScale << "x";
                    if (newScale < 1.0f) {
                        std::cout << " (" << (1.0f / newScale) << "x slower)";
                    }
                    std::cout << std::endl;
                }
                break;

            case GLFW_KEY_0:  // Maximum speed (limited for stability)
                pendingCommands.push_back(
                    std::make_unique<SetTimeScaleCommand>(0.25f)
                );
                std::cout << "Time scale: 0.25x (4x slower - max speed)" << std::endl;
                break;

            case GLFW_KEY_1:  // 20x slower
                pendingCommands.push_back(
                    std::make_unique<SetTimeScaleCommand>(0.05f)
                );
                std::cout << "Time scale: 0.05x (20x slower)" << std::endl;
                break;

            case GLFW_KEY_2:  // 1000x slower
                pendingCommands.push_back(
                    std::make_unique<SetTimeScaleCommand>(0.001f)
                );
                std::cout << "Time scale: 0.001x (1000x slower)" << std::endl;
                break;

            case GLFW_KEY_O:  // Toggle obstacle mode
                pendingCommands.push_back(
                    std::make_unique<ToggleObstacleModeCommand>(UICommand::Type::TOGGLE_OBSTACLE_MODE)
                );
                std::cout << "Obstacle mode: " << (!state.obstacleMode ? "ON" : "OFF") << std::endl;
                break;

            case GLFW_KEY_V:  // Toggle listener mode
                pendingCommands.push_back(
                    std::make_unique<ToggleListenerModeCommand>(UICommand::Type::TOGGLE_LISTENER_MODE)
                );
                std::cout << "Listener mode: " << (!state.listenerMode ? "ON" : "OFF") << std::endl;
                if (!state.listenerMode) {
                    std::cout << "Click to place listener (virtual microphone)" << std::endl;
                }
                break;

            case GLFW_KEY_S:  // Toggle source mode
                pendingCommands.push_back(
                    std::make_unique<ToggleSourceModeCommand>(UICommand::Type::TOGGLE_SOURCE_MODE)
                );
                std::cout << "Audio Source mode: " << (!state.sourceMode ? "ON" : "OFF") << std::endl;
                if (!state.sourceMode) {
                    std::cout << "Click to place audio source (current: ";
                    const char* presets[] = {"Kick", "Snare", "Tone", "Impulse", "File"};
                    std::cout << presets[state.selectedPreset] << ")" << std::endl;
                }
                break;

            case GLFW_KEY_M:  // Toggle mute
                pendingCommands.push_back(
                    std::make_unique<ToggleMuteCommand>(UICommand::Type::TOGGLE_MUTE)
                );
                break;

            case GLFW_KEY_C:  // Clear obstacles
                pendingCommands.push_back(
                    std::make_unique<ClearObstaclesCommand>(UICommand::Type::CLEAR_OBSTACLES)
                );
                std::cout << "Obstacles cleared" << std::endl;
                break;

            case GLFW_KEY_L:  // Load SVG file
                std::cout << "Opening file dialog..." << std::endl;
                {
                    // Open file dialog for SVG files
                    auto selection = pfd::open_file("Load SVG Room Layout",
                                                    "",
                                                    { "SVG Files", "*.svg",
                                                      "All Files", "*" },
                                                    pfd::opt::none).result();

                    if (!selection.empty()) {
                        std::string filename = selection[0];
                        std::cout << "Loading: " << filename << std::endl;
                        pendingCommands.push_back(
                            std::make_unique<LoadSVGLayoutCommand>(filename)
                        );
                    } else {
                        std::cout << "File dialog cancelled" << std::endl;
                    }
                }
                break;

            case GLFW_KEY_G:  // Toggle GPU acceleration
                pendingCommands.push_back(
                    std::make_unique<ToggleGPUCommand>(UICommand::Type::TOGGLE_GPU)
                );
                if (simulation) {
                    bool willBeEnabled = !simulation->isGPUEnabled();
                    std::cout << "GPU Acceleration: " << (willBeEnabled ? "ENABLED" : "DISABLED") << std::endl;
                    if (!simulation->isGPUAvailable()) {
                        std::cout << "Note: GPU not available on this system" << std::endl;
                    }
                }
                break;
        }
    }
}
