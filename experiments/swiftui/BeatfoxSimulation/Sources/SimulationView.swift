//
//  SimulationView.swift
//  BeatfoxSimulation
//
//  Main view with simulation canvas and controls
//

import SwiftUI

struct SimulationView: View {
    @StateObject private var viewModel = SimulationViewModel()

    var body: some View {
        ZStack {
            // Main simulation area (placeholder for now - will add Metal rendering later)
            Color.black
                .overlay(
                    Text("Acoustic Wave Simulation\n\nClick to add impulse")
                        .foregroundColor(.white.opacity(0.5))
                        .multilineTextAlignment(.center)
                        .font(.title3)
                )
                .contentShape(Rectangle())
                .onTapGesture { location in
                    viewModel.addImpulse(at: location)
                }

            // Help button (top-left)
            if !viewModel.showHelp {
                VStack {
                    HStack {
                        Button(action: {
                            viewModel.toggleHelp()
                        }) {
                            Label("Help (H)", systemImage: "questionmark.circle")
                                .padding(12)
                                .background(Color(NSColor.windowBackgroundColor).opacity(0.9))
                                .cornerRadius(8)
                        }
                        .buttonStyle(.plain)
                        .padding()

                        Spacer()
                    }
                    Spacer()
                }
            }

            // Control Panel (right side)
            if viewModel.showHelp {
                HStack {
                    Spacer()
                    ControlPanelView(viewModel: viewModel)
                }
                .transition(.move(edge: .trailing))
            }
        }
        .frame(minWidth: 800, minHeight: 600)
    }
}

#Preview {
    SimulationView()
}
