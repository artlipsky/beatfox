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

    var body: some View {
        GeometryReader { geometry in
            ZStack {
                Color.black

                if let cgImage = image {
                    Image(decorative: cgImage, scale: 1.0)
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                }
            }
            .onTapGesture { location in
                // Scale tap location to grid coordinates
                let scaleX = CGFloat(viewModel.gridWidth) / geometry.size.width
                let scaleY = CGFloat(viewModel.gridHeight) / geometry.size.height
                let gridPoint = CGPoint(
                    x: location.x * scaleX,
                    y: location.y * scaleY
                )
                viewModel.addImpulse(at: gridPoint)
            }
        }
        .onReceive(viewModel.$gridWidth) { _ in
            updateImage()
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

            // Map pressure to color
            // Blue for negative (rarefaction), red for positive (compression)
            let normalizedPressure = pressure / 10.0  // Adjust scale as needed
            let clampedPressure = max(-1.0, min(1.0, normalizedPressure))

            let r: UInt8
            let g: UInt8
            let b: UInt8

            if clampedPressure > 0 {
                // Positive pressure: white -> red
                let intensity = UInt8(clampedPressure * 255.0)
                r = 255
                g = UInt8(255 - intensity)
                b = UInt8(255 - intensity)
            } else {
                // Negative pressure: white -> blue
                let intensity = UInt8(-clampedPressure * 255.0)
                r = UInt8(255 - intensity)
                g = UInt8(255 - intensity)
                b = 255
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
