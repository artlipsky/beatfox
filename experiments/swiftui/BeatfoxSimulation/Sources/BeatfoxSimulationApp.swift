//
//  BeatfoxSimulationApp.swift
//  BeatfoxSimulation
//
//  SwiftUI app entry point
//

import SwiftUI

@main
struct BeatfoxSimulationApp: App {
    var body: some Scene {
        WindowGroup {
            SimulationView()
        }
        .windowStyle(.hiddenTitleBar)
        .windowResizability(.contentSize)
        .commands {
            // Add keyboard shortcuts
            CommandGroup(replacing: .help) {
                Button("Toggle Help") {
                    // Send notification to toggle help
                    NotificationCenter.default.post(name: NSNotification.Name("ToggleHelp"), object: nil)
                }
                .keyboardShortcut("h")
            }
        }
    }
}
