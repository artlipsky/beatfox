//
//  WaveVisualizationView.swift
//  BeatfoxSimulation
//
//  Renders pressure field data as a visual representation
//

import SwiftUI
import CoreGraphics

struct WaveVisualizationView: View {
    @ObservedObject var viewModel: SimulationViewModel
    @State private var image: CGImage?
    @State private var updateTimer: Timer?

    var body: some View {
        GeometryReader { outerGeometry in
            // Match OpenGL background: dark gray for zero pressure
            Color(red: 0.15, green: 0.15, blue: 0.15)
                .overlay(
                    // Aspect ratio container - ensures square grid cells
                    GeometryReader { geometry in
                        ZStack {
                            if let cgImage = image {
                                Image(decorative: cgImage, scale: 1.0)
                                    .resizable()
                            }

                            // Grid overlay (using shared grid config from core)
                            GridOverlay(gridWidth: viewModel.gridWidth,
                                       gridHeight: viewModel.gridHeight,
                                       gridSpacing: viewModel.gridSpacing)

                            // Listener marker overlay
                            if viewModel.hasListener {
                                ListenerMarker(gridWidth: viewModel.gridWidth,
                                             gridHeight: viewModel.gridHeight,
                                             listenerX: viewModel.listenerX,
                                             listenerY: viewModel.listenerY)
                            }

                            // Audio source markers overlay
                            AudioSourceMarkers(gridWidth: viewModel.gridWidth,
                                             gridHeight: viewModel.gridHeight,
                                             numAudioSources: viewModel.numAudioSources,
                                             stateVersion: viewModel.audioSourceStateVersion,
                                             bridge: viewModel.bridge)

                            // Mouse event handler overlay (handles all clicks, drags, right-clicks)
                            MouseHandlerView(viewModel: viewModel, geometry: geometry)
                        }
                    }
                    .aspectRatio(CGFloat(viewModel.gridWidth) / CGFloat(viewModel.gridHeight), contentMode: .fit)
                )
                .onAppear {
                    // Update visualization at 30 FPS
                    updateTimer = Timer.scheduledTimer(withTimeInterval: 1.0 / 30.0, repeats: true) { _ in
                        updateImage()
                    }
                }
                .onDisappear {
                    updateTimer?.invalidate()
                    updateTimer = nil
                }
        }
    }

    private func updateImage() {
        // Get pressure field data from ViewModel
        guard viewModel.gridWidth > 0, viewModel.gridHeight > 0 else { return }

        let width = viewModel.gridWidth
        let height = viewModel.gridHeight
        let dataSize = width * height

        // Get raw pressure data
        let pressureData = viewModel.bridge.getPressureFieldData()

        // Create RGBA pixel buffer
        var pixelData = [UInt8](repeating: 0, count: dataSize * 4)

        for i in 0..<dataSize {
            let pressure = pressureData[i]

            // Match OpenGL shader color scheme (wave.frag)
            // Zero pressure = dark gray (0.15, 0.15, 0.15)
            // Positive = dark -> yellow/red, Negative = dark -> cyan/blue
            let normalizedPressure = pressure / 5.0  // Scale for 5 Pa (typical clap)
            let clampedPressure = max(-1.0, min(1.0, normalizedPressure))

            let darkGray: Float = 0.15
            let r: UInt8
            let g: UInt8
            let b: UInt8

            if clampedPressure > 0 {
                // Positive pressure (compression): dark gray -> yellow/red
                // mix(vec3(0.15, 0.15, 0.15), vec3(1.0, 0.3, 0.0), pressure)
                let t = clampedPressure
                r = UInt8((darkGray + (1.0 - darkGray) * t) * 255.0)
                g = UInt8((darkGray + (0.3 - darkGray) * t) * 255.0)
                b = UInt8((darkGray + (0.0 - darkGray) * t) * 255.0)
            } else {
                // Negative pressure (rarefaction): dark gray -> cyan/blue
                // mix(vec3(0.15, 0.15, 0.15), vec3(0.0, 0.6, 1.0), -pressure)
                let t = -clampedPressure
                r = UInt8((darkGray + (0.0 - darkGray) * t) * 255.0)
                g = UInt8((darkGray + (0.6 - darkGray) * t) * 255.0)
                b = UInt8((darkGray + (1.0 - darkGray) * t) * 255.0)
            }

            let pixelIndex = i * 4
            pixelData[pixelIndex] = r
            pixelData[pixelIndex + 1] = g
            pixelData[pixelIndex + 2] = b
            pixelData[pixelIndex + 3] = 255  // Alpha
        }

        // Create CGImage from pixel data
        let bytesPerPixel = 4
        let bitsPerComponent = 8
        let bytesPerRow = width * bytesPerPixel

        guard let providerRef = CGDataProvider(data: NSData(bytes: pixelData, length: pixelData.count))
        else { return }

        let cgImage = CGImage(
            width: width,
            height: height,
            bitsPerComponent: bitsPerComponent,
            bitsPerPixel: bytesPerPixel * bitsPerComponent,
            bytesPerRow: bytesPerRow,
            space: CGColorSpaceCreateDeviceRGB(),
            bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue),
            provider: providerRef,
            decode: nil,
            shouldInterpolate: false,
            intent: .defaultIntent
        )

        DispatchQueue.main.async {
            self.image = cgImage
        }
    }
}

/// Grid overlay matching OpenGL renderer
struct GridOverlay: View {
    let gridWidth: Int
    let gridHeight: Int
    let gridSpacing: Int  // Grid line every N simulation cells (from core config)

    var body: some View {
        GeometryReader { geometry in
            Canvas { context, size in
                // Calculate pixel per grid cell
                let pixelWidth = size.width / CGFloat(gridWidth)
                let pixelHeight = size.height / CGFloat(gridHeight)

                // Draw vertical grid lines
                for x in stride(from: 0, through: gridWidth, by: gridSpacing) {
                    let xPos = CGFloat(x) * pixelWidth
                    let isMajor = x % (gridSpacing * 10) == 0

                    var path = Path()
                    path.move(to: CGPoint(x: xPos, y: 0))
                    path.addLine(to: CGPoint(x: xPos, y: size.height))

                    if isMajor {
                        // Major lines: bright white, more opaque
                        context.stroke(path, with: .color(.white.opacity(0.35)), lineWidth: 1)
                    } else {
                        // Minor lines: subtle gray, low opacity
                        context.stroke(path, with: .color(Color(white: 0.6, opacity: 0.12)), lineWidth: 1)
                    }
                }

                // Draw horizontal grid lines
                for y in stride(from: 0, through: gridHeight, by: gridSpacing) {
                    let yPos = CGFloat(y) * pixelHeight
                    let isMajor = y % (gridSpacing * 10) == 0

                    var path = Path()
                    path.move(to: CGPoint(x: 0, y: yPos))
                    path.addLine(to: CGPoint(x: size.width, y: yPos))

                    if isMajor {
                        // Major lines: bright white, more opaque
                        context.stroke(path, with: .color(.white.opacity(0.35)), lineWidth: 1)
                    } else {
                        // Minor lines: subtle gray, low opacity
                        context.stroke(path, with: .color(Color(white: 0.6, opacity: 0.12)), lineWidth: 1)
                    }
                }
            }
        }
        .allowsHitTesting(false)  // Pass through touch events
    }
}

/// Listener (microphone) marker overlay
struct ListenerMarker: View {
    let gridWidth: Int
    let gridHeight: Int
    let listenerX: Int
    let listenerY: Int

    var body: some View {
        GeometryReader { geometry in
            Canvas { context, size in
                // Calculate pixel per grid cell
                let pixelWidth = size.width / CGFloat(gridWidth)
                let pixelHeight = size.height / CGFloat(gridHeight)

                // Convert grid coordinates to screen coordinates
                let x = CGFloat(listenerX) * pixelWidth
                let y = CGFloat(listenerY) * pixelHeight

                // Draw green circle (matching ImGui: RGB(50, 255, 100, 200))
                let fillColor = Color(red: 50/255.0, green: 255/255.0, blue: 100/255.0, opacity: 200/255.0)
                context.fill(
                    Circle().path(in: CGRect(x: x - 8, y: y - 8, width: 16, height: 16)),
                    with: .color(fillColor)
                )

                // White outline (thickness 2px)
                context.stroke(
                    Circle().path(in: CGRect(x: x - 8, y: y - 8, width: 16, height: 16)),
                    with: .color(.white),
                    lineWidth: 2
                )

                // Microphone icon - small white circle at (x, y-3)
                context.stroke(
                    Circle().path(in: CGRect(x: x - 3, y: y - 6, width: 6, height: 6)),
                    with: .color(.white),
                    lineWidth: 1.5
                )
            }
        }
        .allowsHitTesting(false)
    }
}

/// Audio source markers overlay
struct AudioSourceMarkers: View {
    let gridWidth: Int
    let gridHeight: Int
    let numAudioSources: Int  // Triggers re-render when count changes
    let stateVersion: Int      // Triggers re-render when playback state changes
    let bridge: SimulationControllerBridge

    var body: some View {
        GeometryReader { geometry in
            Canvas { context, size in
                // Calculate pixel per grid cell
                let pixelWidth = size.width / CGFloat(gridWidth)
                let pixelHeight = size.height / CGFloat(gridHeight)

                // Get audio sources
                let sources = bridge.getAudioSources()

                for source in sources {
                    // Convert grid coordinates to screen coordinates
                    let x = CGFloat(source.x) * pixelWidth
                    let y = CGFloat(source.y) * pixelHeight

                    // Choose color based on playback state
                    let fillColor: Color
                    let outlineColor: Color

                    if source.isPlaying {
                        // Orange for playing (matching ImGui: RGB(255, 150, 50, 200))
                        fillColor = Color(red: 255/255.0, green: 150/255.0, blue: 50/255.0, opacity: 200/255.0)
                        outlineColor = Color(red: 255/255.0, green: 200/255.0, blue: 100/255.0)
                    } else {
                        // Gray for paused/stopped (matching ImGui: RGB(150, 150, 150, 150))
                        fillColor = Color(red: 150/255.0, green: 150/255.0, blue: 150/255.0, opacity: 150/255.0)
                        outlineColor = Color(red: 200/255.0, green: 200/255.0, blue: 200/255.0)
                    }

                    // Draw filled circle
                    context.fill(
                        Circle().path(in: CGRect(x: x - 8, y: y - 8, width: 16, height: 16)),
                        with: .color(fillColor)
                    )

                    // Draw outline
                    context.stroke(
                        Circle().path(in: CGRect(x: x - 8, y: y - 8, width: 16, height: 16)),
                        with: .color(outlineColor),
                        lineWidth: 2
                    )

                    // Draw speaker icon if playing
                    if source.isPlaying {
                        // Small speaker box (white rectangle)
                        var speakerBox = Path()
                        speakerBox.addRect(CGRect(x: x - 3, y: y - 2, width: 2, height: 4))
                        context.fill(speakerBox, with: .color(.white))

                        // Sound waves (concentric circles)
                        context.stroke(
                            Circle().path(in: CGRect(x: x - 3, y: y - 3, width: 6, height: 6)),
                            with: .color(.white.opacity(200/255.0)),
                            lineWidth: 1
                        )
                        context.stroke(
                            Circle().path(in: CGRect(x: x - 5, y: y - 5, width: 10, height: 10)),
                            with: .color(.white.opacity(150/255.0)),
                            lineWidth: 1
                        )
                    }
                }
            }
        }
        .allowsHitTesting(false)
    }
}
