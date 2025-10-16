//
//  SimulationView.swift
//  BeatfoxSimulation
//
//  Main view with simulation canvas and controls
//

import SwiftUI

struct SimulationView: View {
    @ObservedObject var viewModel: SimulationViewModel

    var body: some View {
        ZStack {
            // Main simulation visualization
            WaveVisualizationView(viewModel: viewModel)

            // Help button (top-left when panel is hidden - matching ImGui version)
            if !viewModel.showHelp {
                VStack {
                    HStack {
                        Button("? Help (H)") {
                            viewModel.toggleHelp()
                        }
                        .padding(8)
                        .background(Color(NSColor.windowBackgroundColor).opacity(0.7))
                        .cornerRadius(4)
                        .buttonStyle(.plain)
                        .padding(.leading, 20)
                        .padding(.top, 40)  // Extra padding to avoid macOS traffic light buttons

                        Spacer()
                    }
                    Spacer()
                }
            }

            // Control Panel (left side with scroll - matching ImGui position at 20, 20)
            if viewModel.showHelp {
                VStack {
                    HStack(spacing: 0) {
                        ScrollView {
                            VStack(alignment: .leading, spacing: 0) {
                                ControlPanelView(viewModel: viewModel)
                            }
                            .frame(maxWidth: .infinity, alignment: .top)
                        }
                        .frame(width: 350)
                        .background(Color(NSColor.windowBackgroundColor).opacity(0.9))
                        .cornerRadius(8)
                        .padding(.leading, 20)
                        .padding(.top, 40)  // Position to avoid traffic lights

                        Spacer()
                    }
                    Spacer()
                }
                .transition(.move(edge: .leading))
            }
        }
        .frame(minWidth: 800, minHeight: 600)
    }
}

#Preview {
    SimulationView(viewModel: SimulationViewModel())
}
