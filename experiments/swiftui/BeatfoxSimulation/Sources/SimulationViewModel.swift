//
//  SimulationViewModel.swift
//  BeatfoxSimulation
//
//  SwiftUI ViewModel that connects to C++ SimulationController
//

import Foundation
import Combine
import AppKit
import UniformTypeIdentifiers

/// Main ViewModel for the simulation
@MainActor
class SimulationViewModel: ObservableObject {
    // Bridge to C++ controller
    let bridge: SimulationControllerBridge  // Public so views can access pressure data

    // Timer for simulation updates
    private var updateTimer: Timer?

    // Published state properties (auto-update SwiftUI)
    @Published var impulsePressure: Float = 5.0
    @Published var impulseRadius: Int = 2
    @Published var showHelp: Bool = true
    @Published var selectedPreset: Int = 0
    @Published var sourceVolumeDb: Float = 0.0
    @Published var sourceLoop: Bool = true
    @Published var timeScale: Float = 0.001

    // Mode toggles
    @Published var obstacleMode: Bool = false
    @Published var listenerMode: Bool = false
    @Published var sourceMode: Bool = false

    // Simulation info
    @Published var gridWidth: Int = 0
    @Published var gridHeight: Int = 0
    @Published var physicalWidth: Float = 0.0
    @Published var physicalHeight: Float = 0.0
    @Published var waveSpeed: Float = 343.0
    @Published var hasListener: Bool = false
    @Published var listenerX: Int = 0
    @Published var listenerY: Int = 0
    @Published var numAudioSources: Int = 0
    @Published var numObstacles: Int = 0
    @Published var gridSpacing: Int = 10
    @Published var pixelSize: Float = 0.0086

    // Audio info
    @Published var isMuted: Bool = false
    @Published var volume: Float = 1.0

    // Obstacle control
    @Published var obstacleRadius: Int = 5

    // State version for triggering UI updates
    @Published var audioSourceStateVersion: Int = 0

    init() {
        self.bridge = SimulationControllerBridge()

        // Initial state sync
        syncStateFromBridge()

        // Start update timer (60 FPS)
        startUpdateTimer()
    }

    // MARK: - Update Loop

    private func startUpdateTimer() {
        updateTimer = Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            Task { @MainActor in
                self?.update()
            }
        }
    }

    private func stopUpdateTimer() {
        updateTimer?.invalidate()
        updateTimer = nil
    }

    private func update() {
        // Update simulation with fixed timestep
        let dt: Float = 1.0 / 60.0
        bridge.update(withDeltaTime: dt)

        // Sync state back to SwiftUI
        syncStateFromBridge()

        // Increment state version to trigger UI updates (for audio source playback state)
        audioSourceStateVersion += 1
    }

    private func syncStateFromBridge() {
        let state = bridge.getState()

        impulsePressure = state.impulsePressure
        impulseRadius = Int(state.impulseRadius)
        showHelp = state.showHelp
        selectedPreset = Int(state.selectedPreset)
        sourceVolumeDb = state.sourceVolumeDb
        sourceLoop = state.sourceLoop
        timeScale = state.timeScale

        obstacleMode = state.obstacleMode
        listenerMode = state.listenerMode
        sourceMode = state.sourceMode

        gridWidth = Int(state.width)
        gridHeight = Int(state.height)
        physicalWidth = state.physicalWidth
        physicalHeight = state.physicalHeight
        waveSpeed = state.waveSpeed
        hasListener = state.hasListener
        listenerX = Int(state.listenerX)
        listenerY = Int(state.listenerY)
        numAudioSources = Int(state.numAudioSources)
        numObstacles = Int(state.numObstacles)
        gridSpacing = Int(state.gridSpacing)
        pixelSize = state.pixelSize

        isMuted = state.isMuted
        volume = state.volume
        obstacleRadius = Int(state.obstacleRadius)
    }

    // MARK: - UI Commands

    func setImpulsePressure(_ value: Float) {
        bridge.setImpulsePressure(value)
        impulsePressure = value
    }

    func setImpulseRadius(_ value: Int) {
        bridge.setImpulseRadius(Int32(value))
        impulseRadius = value
    }

    func setShowHelp(_ value: Bool) {
        bridge.setShowHelp(value)
        showHelp = value
    }

    func setSelectedPreset(_ value: Int) {
        bridge.setSelectedPreset(Int32(value))
        selectedPreset = value

        // Apply the damping preset immediately
        bridge.applyDampingPreset(Int32(value))
    }

    func setSourceVolumeDb(_ value: Float) {
        bridge.setSourceVolumeDb(value)
        sourceVolumeDb = value
    }

    func setSourceLoop(_ value: Bool) {
        bridge.setSourceLoop(value)
        sourceLoop = value
    }

    func setTimeScale(_ value: Float) {
        bridge.setTimeScale(value)
        timeScale = value
    }

    // MARK: - Mode Toggles

    func toggleHelp() {
        bridge.toggleHelp()
        showHelp.toggle()
    }

    func toggleObstacleMode() {
        bridge.toggleObstacleMode()
        obstacleMode.toggle()
        if obstacleMode {
            listenerMode = false
            sourceMode = false
        }
    }

    func toggleListenerMode() {
        bridge.toggleListenerMode()
        listenerMode.toggle()
        if listenerMode {
            obstacleMode = false
            sourceMode = false
        }
    }

    func toggleSourceMode() {
        bridge.toggleSourceMode()
        sourceMode.toggle()
        if sourceMode {
            obstacleMode = false
            listenerMode = false
        }
    }

    // MARK: - Domain Operations

    func addImpulse(at point: CGPoint) {
        // Convert view coordinates to grid coordinates
        // For now, use direct mapping (assumes 1:1 scale)
        let x = Int32(point.x)
        let y = Int32(point.y)

        bridge.addImpulseAt(x: x, y: y, pressure: impulsePressure, radius: Int32(impulseRadius))
    }

    func addObstacle(at point: CGPoint) {
        let x = Int32(point.x)
        let y = Int32(point.y)

        bridge.addObstacleAt(x: x, y: y, radius: Int32(obstacleRadius))
    }

    func addAudioSource(at point: CGPoint) {
        let x = Int32(point.x)
        let y = Int32(point.y)

        // Call bridge method with selected preset and settings
        bridge.addAudioSourceAt(x: x, y: y, preset: Int32(selectedPreset), volumeDb: sourceVolumeDb, loop: sourceLoop)
        print("Audio source placed at (\(x), \(y)) with preset \(selectedPreset)")
    }

    func clearObstacles() {
        bridge.clearObstacles()
    }

    func clearWaves() {
        bridge.clearWaves()
    }

    func loadSVGLayout() {
        // Create file picker dialog
        let panel = NSOpenPanel()
        panel.title = "Load SVG Room Layout"
        panel.allowedContentTypes = [.svg]
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.canChooseFiles = true

        // Show dialog
        panel.begin { response in
            if response == .OK, let url = panel.url {
                print("Loading SVG: \(url.path)")
                let success = self.bridge.loadObstacles(fromSVG: url.path)
                if success {
                    print("SVG loaded successfully")
                } else {
                    print("Failed to load SVG")
                }
            } else {
                print("File dialog cancelled")
            }
        }
    }

    func setListenerPosition(at point: CGPoint) {
        let x = Int32(point.x)
        let y = Int32(point.y)

        bridge.setListenerPositionX(x, y: y)
    }

    func toggleListener() {
        bridge.toggleListener()
    }

    // MARK: - Audio Operations

    func toggleMute() {
        bridge.toggleMute()
        isMuted.toggle()
    }

    func setVolume(_ value: Float) {
        bridge.setVolume(value)
        volume = value
    }

    func loadAudioFile() {
        // Create file picker dialog
        let panel = NSOpenPanel()
        panel.title = "Load Audio File"
        panel.allowedContentTypes = [
            UTType(filenameExtension: "mp3")!,
            UTType(filenameExtension: "wav")!,
            UTType(filenameExtension: "flac")!
        ]
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        panel.canChooseFiles = true

        // Show dialog
        panel.begin { response in
            if response == .OK, let url = panel.url {
                print("Loading audio file: \(url.path)")
                let success = self.bridge.loadAudioFile(url.path)
                if success {
                    print("Audio file loaded successfully")
                } else {
                    print("Failed to load audio file")
                }
            } else {
                print("File dialog cancelled")
            }
        }
    }

    func clearAudioSources() {
        bridge.clearAudioSources()
        print("Cleared all audio sources")
    }

    // MARK: - Physics Control

    func setWaveSpeed(_ speed: Float) {
        bridge.setWaveSpeed(speed)
        print("Wave speed set to \(speed) m/s")
    }

    func getAirAbsorption() -> Float {
        return bridge.getDamping()
    }

    func setAirAbsorption(_ damping: Float) {
        bridge.setAirAbsorption(damping)
        print("Air absorption set to \(damping)")
    }

    // MARK: - GPU Control

    func toggleGPU() {
        bridge.toggleGPU()
        print("GPU toggled - now \(bridge.isGPUEnabled() ? "enabled" : "disabled")")
    }

    // MARK: - Obstacle Control

    func setObstacleRadius(_ radius: Int) {
        let clampedRadius = max(1, min(20, radius))
        bridge.setObstacleRadius(Int32(clampedRadius))
        obstacleRadius = clampedRadius
        print("Obstacle radius: \(clampedRadius) pixels")
    }

    func removeObstacle(at point: CGPoint) {
        let x = Int32(point.x)
        let y = Int32(point.y)
        bridge.removeObstacleAt(x: x, y: y, radius: Int32(obstacleRadius))
    }

    // MARK: - Audio Source Control

    func toggleAudioSourcePlayback(at index: Int) {
        bridge.toggleAudioSourcePlayback(Int32(index))
    }
}
