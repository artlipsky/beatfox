#include "SimulationUI.h"
#include "SimulationController.h"
#include "SimulationEngine.h"
#include "WaveSimulation.h"
#include "AudioOutput.h"
#include "CoordinateMapper.h"
#include "AudioSample.h"
#include "AudioFileLoader.h"
#include "DampingPreset.h"
#include "AcousticUtils.h"
#include "portable-file-dialogs.h"
#include <iostream>

SimulationUI::SimulationUI(
    SimulationController* ctrl,
    WaveSimulation* sim,
    AudioOutput* audio,
    CoordinateMapper* mapper
)
    : controller(ctrl)
    , simulation(sim)
    , audioOutput(audio)
    , coordinateMapper(mapper)
{
}

void SimulationUI::updateSimulationPointer(WaveSimulation* newSim) {
    simulation = newSim;
}

std::vector<std::unique_ptr<UICommand>> SimulationUI::collectCommands() {
    std::vector<std::unique_ptr<UICommand>> commands;
    commands.swap(pendingCommands);
    return commands;
}

void SimulationUI::render() {
    const auto& state = controller->getState();

    renderListenerMarker();
    renderAudioSourceMarkers();

    if (state.showHelp) {
        renderControlsPanel();
    } else {
        renderHelpButton();
    }
}

void SimulationUI::renderListenerMarker() {
    if (!simulation->hasListener() || !coordinateMapper) {
        return;
    }

    int listenerX, listenerY;
    simulation->getListenerPosition(listenerX, listenerY);

    // Convert grid coordinates to window coordinates using CoordinateMapper
    float windowX, windowY;
    coordinateMapper->gridToWindow(listenerX, listenerY, windowX, windowY);

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

void SimulationUI::renderAudioSourceMarkers() {
    if (!coordinateMapper) {
        return;
    }

    auto& audioSources = simulation->getAudioSources();
    if (audioSources.empty()) {
        return;
    }

    ImDrawList* drawList = ImGui::GetBackgroundDrawList();

    for (size_t i = 0; i < audioSources.size(); i++) {
        const auto& source = audioSources[i];
        if (!source) continue;

        int sourceX = source->getX();
        int sourceY = source->getY();

        // Convert grid coordinates to window coordinates using CoordinateMapper
        float windowX, windowY;
        coordinateMapper->gridToWindow(sourceX, sourceY, windowX, windowY);

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

void SimulationUI::renderControlsPanel() {
    // Read state (no mutations!)
    const auto& state = controller->getState();

    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowBgAlpha(0.9f);

    bool tempShowHelp = state.showHelp;
    ImGui::Begin("Controls", &tempShowHelp, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);

    // Check if close button was clicked
    if (tempShowHelp != state.showHelp) {
        pendingCommands.push_back(std::make_unique<SetShowHelpCommand>(tempShowHelp));
    }

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
    if (state.timeScale < 1.0f) {
        ImGui::BulletText("Time: %.2fx (%.0fx slower)", state.timeScale, 1.0f / state.timeScale);
    } else {
        ImGui::BulletText("Time: %.2fx", state.timeScale);
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
            pendingCommands.push_back(
                std::make_unique<ApplyDampingPresetCommand>(ApplyDampingPresetCommand::PresetType::REALISTIC)
            );
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Real-world room acoustics\nAir absorption: 0.3%%, Wall reflection: 85%%");
        }

        bool isVisualization = (currentPreset.getType() == DampingPreset::Type::VISUALIZATION);
        if (ImGui::RadioButton("Visualization", isVisualization)) {
            pendingCommands.push_back(
                std::make_unique<ApplyDampingPresetCommand>(ApplyDampingPresetCommand::PresetType::VISUALIZATION)
            );
        }
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Minimal damping for clear wave patterns\n"
                             "Air: 0.02%% loss, Walls: 98%% reflective\n"
                             "Waves persist long, strong reflections");
        }

        bool isAnechoic = (currentPreset.getType() == DampingPreset::Type::ANECHOIC);
        if (ImGui::RadioButton("Anechoic Chamber", isAnechoic)) {
            pendingCommands.push_back(
                std::make_unique<ApplyDampingPresetCommand>(ApplyDampingPresetCommand::PresetType::ANECHOIC)
            );
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

    // Grid Size / Room Scale Section - Get SimulationEngine for grid operations
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.9f, 1.0f, 1.0f));
    ImGui::Text("Room Size (Grid Resolution):");
    ImGui::PopStyleColor();

    // Note: Grid resizing is handled via keyboard shortcuts (InputHandler),
    // so we just display current size information here
    ImGui::TextDisabled("Use UI menu or keyboard to resize grid");
    ImGui::TextDisabled("Grid: %d × %d px, %.1f × %.1f m",
                      simulation ? simulation->getWidth() : 0,
                      simulation ? simulation->getHeight() : 0,
                      simulation ? simulation->getPhysicalWidth() : 0.0f,
                      simulation ? simulation->getPhysicalHeight() : 0.0f);
    ImGui::TextDisabled("Scale: 1 pixel = 8.6 mm (constant)");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Audio Sources Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.8f, 1.0f));
    ImGui::Text("Audio Sources:");
    ImGui::PopStyleColor();

    const char* presetNames[] = {"Kick Drum", "Snare Drum", "Tone (440Hz)", "Impulse", "Loaded File"};
    int tempPreset = state.selectedPreset;
    if (ImGui::Combo("Sample", &tempPreset, presetNames, 5)) {
        pendingCommands.push_back(std::make_unique<SetSelectedPresetCommand>(tempPreset));
    }

    float tempVolumeDb = state.sourceVolumeDb;
    if (ImGui::SliderFloat("Volume (dB)", &tempVolumeDb, -40.0f, 20.0f, "%.1f dB")) {
        pendingCommands.push_back(std::make_unique<SetSourceVolumeDbCommand>(tempVolumeDb));
    }

    bool tempLoop = state.sourceLoop;
    if (ImGui::Checkbox("Loop", &tempLoop)) {
        pendingCommands.push_back(std::make_unique<SetSourceLoopCommand>(tempLoop));
    }

    if (ImGui::Button("Load Audio File")) {
        auto selection = pfd::open_file("Load Audio Sample",
                                        "",
                                        { "Audio Files", "*.mp3 *.wav *.flac",
                                          "All Files", "*" },
                                        pfd::opt::none).result();

        if (!selection.empty()) {
            std::string filename = selection[0];
            pendingCommands.push_back(std::make_unique<LoadAudioFileCommand>(filename));
        }
    }

    // Show active sources
    auto& sources = simulation->getAudioSources();
    if (!sources.empty()) {
        ImGui::Spacing();
        ImGui::TextDisabled("Active Sources: %zu", sources.size());

        if (ImGui::Button("Clear All Sources")) {
            pendingCommands.push_back(
                std::make_unique<ClearAudioSourcesCommand>(UICommand::Type::CLEAR_AUDIO_SOURCES)
            );
        }
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Impulse (Click) Parameters Section
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 1.0f, 0.7f, 1.0f));
    ImGui::Text("Impulse (Click) Parameters:");
    ImGui::PopStyleColor();

    // Pressure amplitude slider with dB SPL conversion
    float tempPressure = state.impulsePressure;
    if (ImGui::SliderFloat("Pressure (Pa)", &tempPressure, 0.01f, 100.0f, "%.2f Pa", ImGuiSliderFlags_Logarithmic)) {
        pendingCommands.push_back(std::make_unique<SetImpulsePressureCommand>(tempPressure));
    }
    if (ImGui::IsItemHovered()) {
        const float dB_SPL = AcousticUtils::pressureToDbSpl(tempPressure);
        ImGui::SetTooltip("Acoustic pressure amplitude\n"
                         "%.2f Pa ≈ %.0f dB SPL\n\n"
                         "Reference:\n"
                         "0.02 Pa = whisper (30 dB)\n"
                         "0.2 Pa = conversation (60 dB)\n"
                         "2 Pa = loud talking (80 dB)\n"
                         "5 Pa = hand clap (94 dB)\n"
                         "20 Pa = shout (100 dB)\n"
                         "100 Pa = very loud (114 dB)",
                         tempPressure, dB_SPL);
    }

    // Impulse radius slider
    int tempRadius = state.impulseRadius;
    if (ImGui::SliderInt("Spread (pixels)", &tempRadius, 1, 10, "%d px")) {
        pendingCommands.push_back(std::make_unique<SetImpulseRadiusCommand>(tempRadius));
    }
    if (ImGui::IsItemHovered()) {
        const float pixelSizeMM = simulation ? simulation->getPixelSize() : 8.6f;
        const float spreadMM = tempRadius * pixelSizeMM;
        ImGui::SetTooltip("Spatial spread of impulse\n"
                         "%d pixels ≈ %.1f mm\n\n"
                         "Smaller = point source (sharp wave)\n"
                         "Larger = diffuse source (smooth wave)",
                         tempRadius, spreadMM);
    }

    // Show current impulse info
    const float dB_SPL = AcousticUtils::pressureToDbSpl(state.impulsePressure);
    const float pixelSizeMM = simulation ? simulation->getPixelSize() : 8.6f;
    ImGui::TextDisabled("Click impulse: %.2f Pa (%.0f dB SPL), %.1f mm spread",
                       state.impulsePressure, dB_SPL, state.impulseRadius * pixelSizeMM);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Controls:");

    if (state.sourceMode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.7f, 0.8f, 1.0f));
        ImGui::BulletText("Left Click: Place audio source");
        ImGui::PopStyleColor();
    } else if (state.listenerMode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 1.0f, 0.5f, 1.0f));
        ImGui::BulletText("Left Click: Place listener (mic)");
        ImGui::PopStyleColor();
    } else if (state.obstacleMode) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.2f, 1.0f));
        ImGui::BulletText("Left Click: Place obstacle");
        ImGui::BulletText("Right Click: Remove obstacle");
        ImGui::PopStyleColor();
    } else {
        const float dB_SPL_display = AcousticUtils::pressureToDbSpl(state.impulsePressure);
        ImGui::BulletText("Left Click: Create impulse (%.1f Pa, %.0f dB)", state.impulsePressure, dB_SPL_display);
    }
    ImGui::BulletText("S: Audio Source mode (%s)", state.sourceMode ? "ON" : "OFF");
    ImGui::BulletText("V: Listener mode (%s)", state.listenerMode ? "ON" : "OFF");
    ImGui::BulletText("O: Obstacle mode (%s)", state.obstacleMode ? "ON" : "OFF");
    ImGui::BulletText("C: Clear obstacles");
    ImGui::BulletText("L: Load SVG layout");
    ImGui::BulletText("Shift+[/]: Obstacle size (%d px)", state.obstacleRadius);
    ImGui::BulletText("SPACE: Clear waves");
    ImGui::BulletText("+/- or [/]: Time speed");
    ImGui::BulletText("2: 1000x slower | 1: 20x | 0: max (0.25x)");
    ImGui::BulletText("UP/DOWN: Sound speed");
    ImGui::BulletText("Shift+UP/DOWN: Volume");
    ImGui::BulletText("M: Mute audio (%s)", (audioOutput && audioOutput->isMuted()) ? "ON" : "OFF");
    ImGui::BulletText("G: Toggle GPU (%s)", simulation && simulation->isGPUEnabled() ? "ON" : "OFF");
    ImGui::BulletText("LEFT/RIGHT: Absorption");
    ImGui::BulletText("H: Toggle help");

    ImGui::Spacing();
    ImGui::TextDisabled("Rigid walls reflect sound");

    ImGui::End();
}

void SimulationUI::renderHelpButton() {
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
        pendingCommands.push_back(std::make_unique<SetShowHelpCommand>(true));
    }

    ImGui::End();
}
