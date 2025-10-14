#include "SimulationUI.h"
#include "WaveSimulation.h"
#include "AudioOutput.h"
#include "CoordinateMapper.h"
#include "AudioSample.h"
#include "AudioFileLoader.h"
#include "DampingPreset.h"
#include "portable-file-dialogs.h"
#include <iostream>

SimulationUI::SimulationUI(
    WaveSimulation* sim,
    AudioOutput* audio,
    CoordinateMapper* mapper,
    bool& showHelp,
    float& timeScale,
    bool& obstacleMode,
    int& obstacleRadius,
    bool& listenerMode,
    bool& sourceMode,
    int& selectedPreset,
    float& sourceVolumeDb,
    bool& sourceLoop,
    std::shared_ptr<AudioSample>& loadedSample
)
    : simulation(sim)
    , audioOutput(audio)
    , coordinateMapper(mapper)
    , showHelp(showHelp)
    , timeScale(timeScale)
    , obstacleMode(obstacleMode)
    , obstacleRadius(obstacleRadius)
    , listenerMode(listenerMode)
    , sourceMode(sourceMode)
    , selectedPreset(selectedPreset)
    , sourceVolumeDb(sourceVolumeDb)
    , sourceLoop(sourceLoop)
    , loadedSample(loadedSample)
{
}

void SimulationUI::render() {
    renderListenerMarker();
    renderAudioSourceMarkers();

    if (showHelp) {
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
        showHelp = true;
    }

    ImGui::End();
}
