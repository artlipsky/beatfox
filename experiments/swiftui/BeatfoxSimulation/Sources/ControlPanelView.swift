//
//  ControlPanelView.swift
//  BeatfoxSimulation
//
//  SwiftUI control panel - matches ImGui version exactly
//

import SwiftUI

struct ControlPanelView: View {
    @ObservedObject var viewModel: SimulationViewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            // Header with close button - matches ImGui window behavior
            HStack {
                VStack(alignment: .leading, spacing: 2) {
                    Text("ACOUSTIC SIMULATION")
                        .foregroundColor(Color(red: 0.5, green: 0.8, blue: 1.0))
                        .font(.headline)

                    Text(String(format: "%.1fm x %.1fm room", viewModel.physicalWidth, viewModel.physicalHeight))
                        .foregroundColor(.secondary)
                        .font(.caption)
                }

                Spacer()

                Button(action: {
                    viewModel.toggleHelp()
                }) {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundColor(.secondary)
                        .font(.title2)
                }
                .buttonStyle(.plain)
                .help("Close panel (H)")
            }

            Divider()

            // Physical parameters
            Text("Physical parameters:")
            Text("• Speed: \(Int(viewModel.waveSpeed)) m/s")
                .font(.body)
            Text("• Scale: 1 px = 8.6 mm")
                .font(.body)

            // Time and volume status
            if viewModel.timeScale < 1.0 {
                Text(String(format: "• Time: %.2fx (%.0fx slower)", viewModel.timeScale, 1.0 / viewModel.timeScale))
                    .foregroundColor(Color(red: 1.0, green: 0.8, blue: 0.3))
            } else {
                Text(String(format: "• Time: %.2fx", viewModel.timeScale))
                    .foregroundColor(Color(red: 1.0, green: 0.8, blue: 0.3))
            }

            Text(String(format: "• Volume: %.1fx (%d%%)", viewModel.volume, Int(viewModel.volume * 100)))
                .foregroundColor(Color(red: 1.0, green: 0.8, blue: 0.3))

            Divider()

            // Acoustic Environment Presets
            Text("Acoustic Environment:")
                .foregroundColor(Color(red: 1.0, green: 0.7, blue: 0.3))

            RadioButton(title: "Realistic", isSelected: viewModel.selectedPreset == 0) {
                viewModel.setSelectedPreset(0)
            }
            .help("Real-world room acoustics\nAir absorption: 0.3%, Wall reflection: 85%")

            RadioButton(title: "Visualization", isSelected: viewModel.selectedPreset == 1) {
                viewModel.setSelectedPreset(1)
            }
            .help("Minimal damping for clear wave patterns\nAir: 0.02% loss, Walls: 98% reflective\nWaves persist long, strong reflections")

            RadioButton(title: "Anechoic Chamber", isSelected: viewModel.selectedPreset == 2) {
                viewModel.setSelectedPreset(2)
            }
            .help("No wall reflections (perfect absorption)\nAir: 0.2% loss, Walls: 0% reflective\nWaves absorbed at walls, no echoes")

            Divider()

            // Room Size
            Text("Room Size (Grid Resolution):")
                .foregroundColor(Color(red: 0.6, green: 0.9, blue: 1.0))

            Text("Use UI menu or keyboard to resize grid")
                .foregroundColor(.secondary)
                .font(.caption)
            Text(String(format: "Grid: %d × %d px, %.1f × %.1f m",
                       viewModel.gridWidth, viewModel.gridHeight,
                       viewModel.physicalWidth, viewModel.physicalHeight))
                .foregroundColor(.secondary)
                .font(.caption)
            Text("Scale: 1 pixel = 8.6 mm (constant)")
                .foregroundColor(.secondary)
                .font(.caption)

            Divider()

            // Audio Sources
            Text("Audio Sources:")
                .foregroundColor(Color(red: 1.0, green: 0.7, blue: 0.8))

            Picker("Sample", selection: Binding(
                get: { viewModel.selectedPreset },
                set: { viewModel.setSelectedPreset($0) }
            )) {
                Text("Kick Drum").tag(0)
                Text("Snare Drum").tag(1)
                Text("Tone (440Hz)").tag(2)
                Text("Impulse").tag(3)
                Text("Loaded File").tag(4)
            }

            HStack {
                Text("Volume (dB)")
                Slider(value: Binding(
                    get: { viewModel.sourceVolumeDb },
                    set: { viewModel.setSourceVolumeDb($0) }
                ), in: -40...20)
                Text(String(format: "%.1f dB", viewModel.sourceVolumeDb))
                    .frame(width: 60, alignment: .trailing)
            }

            Toggle("Loop", isOn: Binding(
                get: { viewModel.sourceLoop },
                set: { viewModel.setSourceLoop($0) }
            ))

            Button("Load Audio File") {
                viewModel.loadAudioFile()
            }

            if viewModel.numAudioSources > 0 {
                Text("Active Sources: \(viewModel.numAudioSources)")
                    .foregroundColor(.secondary)
                    .font(.caption)

                Button("Clear All Sources") {
                    viewModel.clearAudioSources()
                }
            }

            Divider()

            // Impulse (Click) Parameters
            Text("Impulse (Click) Parameters:")
                .foregroundColor(Color(red: 0.8, green: 1.0, blue: 0.7))

            HStack {
                Text("Pressure (Pa)")
                Slider(value: Binding(
                    get: { viewModel.impulsePressure },
                    set: { viewModel.setImpulsePressure($0) }
                ), in: 0.01...100, onEditingChanged: { _ in })
                Text(String(format: "%.2f Pa", viewModel.impulsePressure))
                    .frame(width: 60, alignment: .trailing)
            }
            .help(pressureTooltip(viewModel.impulsePressure))

            HStack {
                Text("Spread (pixels)")
                Slider(value: Binding(
                    get: { Double(viewModel.impulseRadius) },
                    set: { viewModel.setImpulseRadius(Int($0)) }
                ), in: 1...10, step: 1)
                Text("\(viewModel.impulseRadius) px")
                    .frame(width: 60, alignment: .trailing)
            }
            .help(spreadTooltip(viewModel.impulseRadius))

            let dBSPL = pressureToDbSpl(viewModel.impulsePressure)
            Text(String(format: "Click impulse: %.2f Pa (%.0f dB SPL), %.1f mm spread",
                       viewModel.impulsePressure, dBSPL, Float(viewModel.impulseRadius) * 8.6))
                .foregroundColor(.secondary)
                .font(.caption)

            Divider()

            // Controls
            Text("Controls:")

            if viewModel.sourceMode {
                Text("• Left Click: Place audio source")
                    .foregroundColor(Color(red: 1.0, green: 0.7, blue: 0.8))
            } else if viewModel.listenerMode {
                Text("• Left Click: Place listener (mic)")
                    .foregroundColor(Color(red: 0.3, green: 1.0, blue: 0.5))
            } else if viewModel.obstacleMode {
                Text("• Left Click: Place obstacle")
                    .foregroundColor(Color(red: 1.0, green: 0.5, blue: 0.2))
                Text("• Right Click: Remove obstacle")
                    .foregroundColor(Color(red: 1.0, green: 0.5, blue: 0.2))
            } else {
                let dBSPL = pressureToDbSpl(viewModel.impulsePressure)
                Text(String(format: "• Left Click: Create impulse (%.1f Pa, %.0f dB)", viewModel.impulsePressure, dBSPL))
            }

            Text("• S: Audio Source mode (\(viewModel.sourceMode ? "ON" : "OFF"))")
            Text("• V: Listener mode (\(viewModel.listenerMode ? "ON" : "OFF"))")
            Text("• O: Obstacle mode (\(viewModel.obstacleMode ? "ON" : "OFF"))")
            Text("• C: Clear obstacles")
            Text("• L: Load SVG layout")
            Text("• Shift+[/]: Obstacle size")
            Text("• SPACE: Clear waves")
            Text("• +/- or [/]: Time speed")
            Text("• 2: 1000x slower | 1: 20x | 0: max (0.25x)")
            Text("• UP/DOWN: Sound speed")
            Text("• Shift+UP/DOWN: Volume")
            Text("• M: Mute audio (\(viewModel.isMuted ? "ON" : "OFF"))")
            Text("• LEFT/RIGHT: Absorption")
            Text("• H: Toggle help")

            Text("Rigid walls reflect sound")
                .foregroundColor(.secondary)
                .font(.caption)
                .padding(.top, 10)
        }
        .padding()
        .font(.system(size: 13))
    }

    private func pressureToDbSpl(_ pressure: Float) -> Float {
        return 20.0 * log10(pressure / 0.00002)
    }

    private func pressureTooltip(_ pressure: Float) -> String {
        let dB = pressureToDbSpl(pressure)
        return """
        Acoustic pressure amplitude
        \(String(format: "%.2f", pressure)) Pa ≈ \(String(format: "%.0f", dB)) dB SPL

        Reference:
        0.02 Pa = whisper (30 dB)
        0.2 Pa = conversation (60 dB)
        2 Pa = loud talking (80 dB)
        5 Pa = hand clap (94 dB)
        20 Pa = shout (100 dB)
        100 Pa = very loud (114 dB)
        """
    }

    private func spreadTooltip(_ radius: Int) -> String {
        let spreadMM = Float(radius) * 8.6
        return """
        Spatial spread of impulse
        \(radius) pixels ≈ \(String(format: "%.1f", spreadMM)) mm

        Smaller = point source (sharp wave)
        Larger = diffuse source (smooth wave)
        """
    }
}

struct RadioButton: View {
    let title: String
    let isSelected: Bool
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack {
                Image(systemName: isSelected ? "largecircle.fill.circle" : "circle")
                    .foregroundColor(isSelected ? .blue : .gray)
                Text(title)
                    .foregroundColor(.primary)
            }
        }
        .buttonStyle(.plain)
    }
}

#Preview {
    ControlPanelView(viewModel: SimulationViewModel())
}
