//
//  SimulationViewModel.swift
//  BeatfoxSimulation
//
//  SwiftUI ViewModel that connects to C++ SimulationController
//

import Foundation
import Combine

/// Main ViewModel for the simulation
@MainActor
class SimulationViewModel: ObservableObject {
    // Bridge to C++ controller
    private let bridge: SimulationControllerBridge

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

    // Audio info
    @Published var isMuted: Bool = false
    @Published var volume: Float = 1.0

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

        isMuted = state.isMuted
        volume = state.volume
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

    func addObstacle(at point: CGPoint, radius: Int = 5) {
        let x = Int32(point.x)
        let y = Int32(point.y)

        bridge.addObstacleAt(x: x, y: y, radius: Int32(radius))
    }

    func clearObstacles() {
        bridge.clearObstacles()
    }

    func clearWaves() {
        bridge.clearWaves()
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
}
