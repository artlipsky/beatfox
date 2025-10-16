//
//  KeyboardMonitor.swift
//  BeatfoxSimulation
//
//  Monitors keyboard events at the app level using NSEvent
//

import Cocoa
import SwiftUI

class KeyboardMonitor: ObservableObject {
    private var monitor: Any?
    private weak var viewModel: SimulationViewModel?

    init(viewModel: SimulationViewModel) {
        self.viewModel = viewModel
        startMonitoring()
    }

    deinit {
        stopMonitoring()
    }

    private func startMonitoring() {
        // Monitor local keyboard events for the app
        monitor = NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self = self, let viewModel = self.viewModel else { return event }

            // Get the key without modifiers
            let key = event.charactersIgnoringModifiers?.lowercased() ?? ""

            // Only handle events without modifier keys (except shift for uppercase)
            let hasModifiers = event.modifierFlags.intersection([.command, .option, .control]).isEmpty == false
            if hasModifiers {
                return event  // Let system handle command/option/control shortcuts
            }

            // Handle key presses - dispatch to main actor
            switch key {
            case " ":  // Space
                Task { @MainActor in
                    viewModel.clearWaves()
                }
                return nil  // Event handled

            case "h":
                Task { @MainActor in
                    viewModel.toggleHelp()
                }
                return nil

            case "m":
                Task { @MainActor in
                    viewModel.toggleMute()
                }
                return nil

            case "c":
                Task { @MainActor in
                    viewModel.clearObstacles()
                }
                return nil

            case "o":
                Task { @MainActor in
                    viewModel.toggleObstacleMode()
                }
                return nil

            case "v":
                Task { @MainActor in
                    viewModel.toggleListenerMode()
                }
                return nil

            case "s":
                Task { @MainActor in
                    viewModel.toggleSourceMode()
                }
                return nil

            case "l":
                Task { @MainActor in
                    viewModel.loadSVGLayout()
                }
                return nil

            case "=", "+":
                Task { @MainActor in
                    let newScale = min(2.0, viewModel.timeScale * 1.5)
                    viewModel.setTimeScale(newScale)
                }
                return nil

            case "-":
                Task { @MainActor in
                    let newScale = max(0.001, viewModel.timeScale / 1.5)
                    viewModel.setTimeScale(newScale)
                }
                return nil

            case "0":
                Task { @MainActor in
                    viewModel.setTimeScale(0.25)
                }
                return nil

            case "1":
                Task { @MainActor in
                    viewModel.setTimeScale(0.05)
                }
                return nil

            case "2":
                Task { @MainActor in
                    viewModel.setTimeScale(0.001)
                }
                return nil

            default:
                return event  // Let system handle other keys
            }
        }
    }

    private func stopMonitoring() {
        if let monitor = monitor {
            NSEvent.removeMonitor(monitor)
            self.monitor = nil
        }
    }
}
