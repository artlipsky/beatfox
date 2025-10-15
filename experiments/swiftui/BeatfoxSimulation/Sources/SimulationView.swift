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
            // Main simulation visualization
            WaveVisualizationView(viewModel: viewModel)

            // Help button (top-right when panel is hidden)
            if !viewModel.showHelp {
                VStack {
                    HStack {
                        Spacer()
                        Button(action: {
                            viewModel.toggleHelp()
                        }) {
                            Label("Show Controls (H)", systemImage: "sidebar.left")
                                .padding(12)
                                .background(Color(NSColor.windowBackgroundColor).opacity(0.9))
                                .cornerRadius(8)
                        }
                        .buttonStyle(.plain)
                        .padding()
                    }
                    Spacer()
                }
            }

            // Control Panel (left side with scroll)
            if viewModel.showHelp {
                HStack(spacing: 0) {
                    ScrollView {
                        VStack(alignment: .leading, spacing: 0) {
                            ControlPanelView(viewModel: viewModel)
                        }
                        .frame(maxWidth: .infinity, alignment: .top)
                    }
                    .frame(width: 320)
                    .background(Color(NSColor.windowBackgroundColor).opacity(0.95))

                    Spacer()
                }
                .transition(.move(edge: .leading))
            }
        }
        .frame(minWidth: 800, minHeight: 600)
    }
}

#Preview {
    SimulationView()
}
