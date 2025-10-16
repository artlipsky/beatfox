//
//  AppDelegate.swift
//  BeatfoxSimulation
//
//  AppDelegate for global keyboard event handling
//

import Cocoa
import SwiftUI

class AppDelegate: NSObject, NSApplicationDelegate {
    var viewModel: SimulationViewModel?

    func applicationWillFinishLaunching(_ notification: Notification) {
        print("🔥 AppDelegate: applicationWillFinishLaunching")

        // Ensure app can be activated
        NSApplication.shared.setActivationPolicy(.regular)
    }

    func applicationDidFinishLaunching(_ notification: Notification) {
        print("🔥 AppDelegate: applicationDidFinishLaunching - Setting up keyboard monitor")

        // Activate the app and bring window to front
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) {
            NSApplication.shared.activate(ignoringOtherApps: true)

            // Make sure window can become key and main
            if let window = NSApplication.shared.windows.first {
                print("🔥 AppDelegate: Making window key and front")
                window.makeKeyAndOrderFront(nil)
            }
        }

        // Set up global keyboard monitoring
        NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            return self?.handleKeyDown(event) ?? event
        }

        print("🔥 AppDelegate: Keyboard monitor setup complete")
    }

    func applicationShouldHandleReopen(_ sender: NSApplication, hasVisibleWindows flag: Bool) -> Bool {
        // Ensure window comes to front when app is clicked
        NSApplication.shared.activate(ignoringOtherApps: true)
        if let window = NSApplication.shared.windows.first {
            window.makeKeyAndOrderFront(nil)
        }
        return true
    }

    private func handleKeyDown(_ event: NSEvent) -> NSEvent? {
        let key = event.charactersIgnoringModifiers?.lowercased() ?? ""
        print("🔥 AppDelegate: Key pressed: '\(key)'")

        guard let viewModel = viewModel else {
            print("🔥 AppDelegate: No viewModel - returning event")
            return event
        }

        // Ignore if modifier keys are pressed (let system handle Cmd shortcuts)
        if event.modifierFlags.intersection([.command, .option, .control]).isEmpty == false {
            print("🔥 AppDelegate: Modifier key pressed - returning event")
            return event
        }

        print("🔥 AppDelegate: Handling key: '\(key)'")

        switch key {
        case " ":  // Space
            print("🔥 AppDelegate: SPACE - Clearing waves")
            Task { @MainActor in
                viewModel.clearWaves()
            }
            return nil

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

        // Arrow keys
        case String(UnicodeScalar(NSUpArrowFunctionKey)!):
            Task { @MainActor in
                if event.modifierFlags.contains(.shift) {
                    // Shift+UP: Increase volume
                    let newVol = min(2.0, viewModel.volume + 0.1)
                    viewModel.setVolume(newVol)
                    print("Volume: \(newVol)")
                } else {
                    // UP: Increase sound speed
                    let newSpeed = viewModel.waveSpeed + 10.0
                    viewModel.setWaveSpeed(newSpeed)
                    print("Sound speed: \(newSpeed) m/s")
                }
            }
            return nil

        case String(UnicodeScalar(NSDownArrowFunctionKey)!):
            Task { @MainActor in
                if event.modifierFlags.contains(.shift) {
                    // Shift+DOWN: Decrease volume
                    let newVol = max(0.0, viewModel.volume - 0.1)
                    viewModel.setVolume(newVol)
                    print("Volume: \(newVol)")
                } else {
                    // DOWN: Decrease sound speed
                    let newSpeed = max(50.0, viewModel.waveSpeed - 10.0)
                    viewModel.setWaveSpeed(newSpeed)
                    print("Sound speed: \(newSpeed) m/s")
                }
            }
            return nil

        case String(UnicodeScalar(NSRightArrowFunctionKey)!):
            Task { @MainActor in
                // RIGHT: Increase air absorption
                let currentDamping = viewModel.getAirAbsorption()
                let newDamping = currentDamping + 0.0001
                viewModel.setAirAbsorption(newDamping)
                print("Air absorption: \(newDamping)")
            }
            return nil

        case String(UnicodeScalar(NSLeftArrowFunctionKey)!):
            Task { @MainActor in
                // LEFT: Decrease air absorption
                let currentDamping = viewModel.getAirAbsorption()
                let newDamping = max(0.0, currentDamping - 0.0001)
                viewModel.setAirAbsorption(newDamping)
                print("Air absorption: \(newDamping)")
            }
            return nil

        // Bracket keys for time control and obstacle radius
        case "[":
            Task { @MainActor in
                if event.modifierFlags.contains(.shift) {
                    // Shift+[: Decrease obstacle radius
                    let newRadius = max(1, viewModel.obstacleRadius - 1)
                    viewModel.setObstacleRadius(newRadius)
                } else {
                    // [: Slow down time
                    let newScale = max(0.001, viewModel.timeScale / 1.5)
                    viewModel.setTimeScale(newScale)
                    print("Time scale: \(newScale)x")
                }
            }
            return nil

        case "]":
            Task { @MainActor in
                if event.modifierFlags.contains(.shift) {
                    // Shift+]: Increase obstacle radius
                    let newRadius = min(20, viewModel.obstacleRadius + 1)
                    viewModel.setObstacleRadius(newRadius)
                } else {
                    // ]: Speed up time
                    let newScale = min(2.0, viewModel.timeScale * 1.5)
                    viewModel.setTimeScale(newScale)
                    print("Time scale: \(newScale)x")
                }
            }
            return nil

        // GPU toggle
        case "g":
            Task { @MainActor in
                viewModel.toggleGPU()
            }
            return nil

        // ESC to exit
        case String(UnicodeScalar(0x1B)!):  // ESC key
            NSApplication.shared.terminate(nil)
            return nil

        default:
            return event
        }
    }
}
