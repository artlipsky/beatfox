//
//  BeatfoxSimulationApp.swift
//  BeatfoxSimulation
//
//  SwiftUI app entry point
//

import SwiftUI

@main
struct BeatfoxSimulationApp: App {
    @NSApplicationDelegateAdaptor(AppDelegate.self) var appDelegate
    @StateObject private var viewModel = SimulationViewModel()

    var body: some Scene {
        WindowGroup {
            SimulationView(viewModel: viewModel)
                .onAppear {
                    // Pass viewModel to AppDelegate for keyboard handling
                    appDelegate.viewModel = viewModel
                }
        }
        .windowStyle(.hiddenTitleBar)
        .windowResizability(.contentSize)
        .commands {
            CommandMenu("Simulation") {
                Button("Toggle Help Panel") {
                    viewModel.toggleHelp()
                }
                .keyboardShortcut("h", modifiers: [.command])

                Button("Clear Waves") {
                    viewModel.clearWaves()
                }
                .keyboardShortcut(" ", modifiers: [.command])

                Divider()

                Button("Toggle Mute") {
                    viewModel.toggleMute()
                }
                .keyboardShortcut("m", modifiers: [.command])

                Divider()

                Button("Clear Obstacles") {
                    viewModel.clearObstacles()
                }
                .keyboardShortcut("c", modifiers: [.command])

                Button("Toggle Obstacle Mode") {
                    viewModel.toggleObstacleMode()
                }
                .keyboardShortcut("o", modifiers: [.command])

                Button("Toggle Listener Mode") {
                    viewModel.toggleListenerMode()
                }
                .keyboardShortcut("v", modifiers: [.command])

                Button("Toggle Source Mode") {
                    viewModel.toggleSourceMode()
                }
                .keyboardShortcut("s", modifiers: [.command])

                Divider()

                Button("Load SVG Layout...") {
                    viewModel.loadSVGLayout()
                }
                .keyboardShortcut("l", modifiers: [.command])

                Divider()

                Button("Speed Up Time") {
                    let newScale = min(2.0, viewModel.timeScale * 1.5)
                    viewModel.setTimeScale(newScale)
                }
                .keyboardShortcut("=", modifiers: [.command])

                Button("Slow Down Time") {
                    let newScale = max(0.001, viewModel.timeScale / 1.5)
                    viewModel.setTimeScale(newScale)
                }
                .keyboardShortcut("-", modifiers: [.command])

                Button("Time Scale: 0.25x") {
                    viewModel.setTimeScale(0.25)
                }
                .keyboardShortcut("0", modifiers: [.command])

                Button("Time Scale: 0.05x") {
                    viewModel.setTimeScale(0.05)
                }
                .keyboardShortcut("1", modifiers: [.command])

                Button("Time Scale: 0.001x") {
                    viewModel.setTimeScale(0.001)
                }
                .keyboardShortcut("2", modifiers: [.command])
            }
        }
    }
}
