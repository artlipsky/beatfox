//
//  ControlPanelView.swift
//  BeatfoxSimulation
//
//  SwiftUI control panel for simulation parameters
//

import SwiftUI

struct ControlPanelView: View {
    @ObservedObject var viewModel: SimulationViewModel

    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            // Header
            HStack {
                Text("Control Panel")
                    .font(.title2)
                    .fontWeight(.bold)

                Spacer()

                Button(action: {
                    viewModel.toggleHelp()
                }) {
                    Image(systemName: "xmark.circle.fill")
                        .font(.title2)
                        .foregroundColor(.secondary)
                }
                .buttonStyle(.plain)
            }

            Divider()

            // Impulse Parameters
            GroupBox(label: Label("Impulse Parameters", systemImage: "waveform")) {
                VStack(alignment: .leading, spacing: 12) {
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Pressure (Pa)")
                            Spacer()
                            Text(String(format: "%.2f", viewModel.impulsePressure))
                                .foregroundColor(.secondary)
                        }
                        Slider(value: Binding(
                            get: { viewModel.impulsePressure },
                            set: { viewModel.setImpulsePressure($0) }
                        ), in: 0.01...100.0)
                    }

                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Radius (pixels)")
                            Spacer()
                            Text("\(viewModel.impulseRadius)")
                                .foregroundColor(.secondary)
                        }
                        Slider(value: Binding(
                            get: { Double(viewModel.impulseRadius) },
                            set: { viewModel.setImpulseRadius(Int($0)) }
                        ), in: 1...10, step: 1)
                    }
                }
                .padding(.vertical, 8)
            }

            // Acoustic Presets
            GroupBox(label: Label("Acoustic Environment", systemImage: "speaker.wave.3")) {
                VStack(alignment: .leading, spacing: 8) {
                    ForEach(0..<3) { index in
                        Button(action: {
                            viewModel.setSelectedPreset(index)
                        }) {
                            HStack {
                                Image(systemName: viewModel.selectedPreset == index ? "checkmark.circle.fill" : "circle")
                                    .foregroundColor(viewModel.selectedPreset == index ? .blue : .secondary)
                                Text(presetName(for: index))
                                    .foregroundColor(.primary)
                                Spacer()
                            }
                            .padding(.vertical, 4)
                        }
                        .buttonStyle(.plain)
                    }
                }
                .padding(.vertical, 4)
            }

            // Time Scale
            GroupBox(label: Label("Time Scale", systemImage: "clock")) {
                VStack(alignment: .leading, spacing: 4) {
                    HStack {
                        Text("Speed")
                        Spacer()
                        Text(timeScaleDescription(viewModel.timeScale))
                            .foregroundColor(.secondary)
                    }
                    Slider(value: Binding(
                        get: { log10(viewModel.timeScale) },
                        set: { viewModel.setTimeScale(pow(10, $0)) }
                    ), in: -3...(-0.6))  // 0.001 to 0.25
                }
                .padding(.vertical, 8)
            }

            // Audio Controls
            GroupBox(label: Label("Audio", systemImage: "speaker.3")) {
                VStack(alignment: .leading, spacing: 12) {
                    Toggle(isOn: Binding(
                        get: { !viewModel.isMuted },
                        set: { _ in viewModel.toggleMute() }
                    )) {
                        Label("Audio Enabled", systemImage: viewModel.isMuted ? "speaker.slash" : "speaker.wave.2")
                    }

                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Text("Volume")
                            Spacer()
                            Text(String(format: "%.0f%%", viewModel.volume * 100))
                                .foregroundColor(.secondary)
                        }
                        Slider(value: Binding(
                            get: { viewModel.volume },
                            set: { viewModel.setVolume($0) }
                        ), in: 0...1)
                    }
                }
                .padding(.vertical, 8)
            }

            // Simulation Info
            GroupBox(label: Label("Simulation Info", systemImage: "info.circle")) {
                VStack(alignment: .leading, spacing: 6) {
                    InfoRow(label: "Grid Size", value: "\(viewModel.gridWidth) × \(viewModel.gridHeight)")
                    InfoRow(label: "Room Size", value: String(format: "%.1fm × %.1fm", viewModel.physicalWidth, viewModel.physicalHeight))
                    InfoRow(label: "Wave Speed", value: String(format: "%.0f m/s", viewModel.waveSpeed))
                    InfoRow(label: "Obstacles", value: "\(viewModel.numObstacles)")
                    InfoRow(label: "Audio Sources", value: "\(viewModel.numAudioSources)")
                    InfoRow(label: "Listener", value: viewModel.hasListener ? "Active (\(viewModel.listenerX), \(viewModel.listenerY))" : "Inactive")
                }
                .padding(.vertical, 4)
                .font(.system(.body, design: .monospaced))
            }

            Spacer()

            // Action Buttons
            HStack(spacing: 12) {
                Button(action: {
                    viewModel.clearWaves()
                }) {
                    Label("Clear Waves", systemImage: "clear")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)

                Button(action: {
                    viewModel.clearObstacles()
                }) {
                    Label("Clear Obstacles", systemImage: "trash")
                        .frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)
            }
        }
        .padding()
        .frame(width: 350)
        .background(Color(NSColor.windowBackgroundColor))
    }

    private func presetName(for index: Int) -> String {
        switch index {
        case 0: return "Realistic"
        case 1: return "Visualization"
        case 2: return "Anechoic"
        default: return "Unknown"
        }
    }

    private func timeScaleDescription(_ scale: Float) -> String {
        let multiplier = 1.0 / scale
        if multiplier >= 1000 {
            return "\(Int(multiplier))× slower"
        } else if multiplier > 1 {
            return String(format: "%.0f× slower", multiplier)
        } else {
            return String(format: "%.2f× speed", scale)
        }
    }
}

struct InfoRow: View {
    let label: String
    let value: String

    var body: some View {
        HStack {
            Text(label + ":")
                .foregroundColor(.secondary)
            Spacer()
            Text(value)
        }
    }
}

#Preview {
    ControlPanelView(viewModel: SimulationViewModel())
}
