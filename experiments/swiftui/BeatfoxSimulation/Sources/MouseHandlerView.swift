//
//  MouseHandlerView.swift
//  BeatfoxSimulation
//
//  NSView-based mouse event handler for comprehensive gesture support
//

import SwiftUI
import AppKit

struct MouseHandlerView: NSViewRepresentable {
    @ObservedObject var viewModel: SimulationViewModel
    let geometry: GeometryProxy

    func makeNSView(context: Context) -> NSView {
        let view = MouseEventView()
        view.viewModel = viewModel
        view.geometry = geometry
        return view
    }

    func updateNSView(_ nsView: NSView, context: Context) {
        if let mouseView = nsView as? MouseEventView {
            mouseView.viewModel = viewModel
            mouseView.geometry = geometry
        }
    }
}

class MouseEventView: NSView {
    var viewModel: SimulationViewModel?
    var geometry: GeometryProxy?

    // Track dragging state
    var dragStartLocation: NSPoint?
    var isDraggingListener: Bool = false

    override var acceptsFirstResponder: Bool { true }

    override func mouseDown(with event: NSEvent) {
        guard let viewModel = viewModel, let geometry = geometry else { return }

        let location = convert(event.locationInWindow, from: nil)
        let gridPoint = screenToGrid(location, geometry: geometry, viewModel: viewModel)

        // Check if clicking on listener
        if viewModel.hasListener {
            let listenerGridX = viewModel.listenerX
            let listenerGridY = viewModel.listenerY

            let dx = Int(gridPoint.x) - listenerGridX
            let dy = Int(gridPoint.y) - listenerGridY
            let distSquared = dx * dx + dy * dy

            if distSquared <= 100 {  // 10 pixel radius
                isDraggingListener = true
                dragStartLocation = location
                return
            }
        }

        // Check if clicking on audio source
        let sources = viewModel.bridge.getAudioSources()
        for (index, source) in sources.enumerated() {
            let dx = Int(gridPoint.x) - Int(source.x)
            let dy = Int(gridPoint.y) - Int(source.y)
            let distSquared = dx * dx + dy * dy

            if distSquared <= 100 {  // 10 pixel radius
                viewModel.toggleAudioSourcePlayback(at: index)
                let state = source.isPlaying ? "paused" : "resumed"
                print("Audio source \(index) \(state)")
                return
            }
        }

        // Mode-specific handling
        if viewModel.listenerMode {
            viewModel.setListenerPosition(at: gridPoint)
            print("Listener placed at (\(Int(gridPoint.x)), \(Int(gridPoint.y)))")
        } else if viewModel.obstacleMode {
            viewModel.addObstacle(at: gridPoint)
            print("Obstacle placed at (\(Int(gridPoint.x)), \(Int(gridPoint.y)))")
        } else if viewModel.sourceMode {
            viewModel.addAudioSource(at: gridPoint)
            print("Audio source placed at (\(Int(gridPoint.x)), \(Int(gridPoint.y)))")
        } else {
            viewModel.addImpulse(at: gridPoint)
        }
    }

    override func mouseDragged(with event: NSEvent) {
        guard let viewModel = viewModel, let geometry = geometry else { return }

        if isDraggingListener {
            let location = convert(event.locationInWindow, from: nil)
            let gridPoint = screenToGrid(location, geometry: geometry, viewModel: viewModel)
            viewModel.setListenerPosition(at: gridPoint)
        }
    }

    override func mouseUp(with event: NSEvent) {
        guard let viewModel = viewModel, let _ = geometry else { return }

        if isDraggingListener, let startLoc = dragStartLocation {
            let endLoc = convert(event.locationInWindow, from: nil)
            let dx = endLoc.x - startLoc.x
            let dy = endLoc.y - startLoc.y
            let distance = sqrt(dx * dx + dy * dy)

            // If barely moved (< 5 pixels), treat as click to toggle
            if distance < 5.0 {
                viewModel.toggleListener()
                let enabled = viewModel.hasListener
                print("Listener \(enabled ? "enabled" : "disabled")")
            }
        }

        isDraggingListener = false
        dragStartLocation = nil
    }

    override func rightMouseDown(with event: NSEvent) {
        guard let viewModel = viewModel, let geometry = geometry else { return }

        let location = convert(event.locationInWindow, from: nil)
        let gridPoint = screenToGrid(location, geometry: geometry, viewModel: viewModel)

        viewModel.removeObstacle(at: gridPoint)
        print("Remove obstacle at (\(Int(gridPoint.x)), \(Int(gridPoint.y)))")
    }

    private func screenToGrid(_ location: NSPoint, geometry: GeometryProxy, viewModel: SimulationViewModel) -> CGPoint {
        let scaleX = CGFloat(viewModel.gridWidth) / geometry.size.width
        let scaleY = CGFloat(viewModel.gridHeight) / geometry.size.height

        // Flip Y coordinate (NSView has origin at bottom-left, SwiftUI at top-left)
        let flippedY = geometry.size.height - location.y

        return CGPoint(
            x: location.x * scaleX,
            y: flippedY * scaleY
        )
    }
}
