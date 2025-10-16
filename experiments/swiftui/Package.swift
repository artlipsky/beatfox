// swift-tools-version: 5.9
import PackageDescription

let package = Package(
    name: "BeatfoxSimulation",
    platforms: [
        .macOS(.v13)
    ],
    products: [
        .executable(
            name: "BeatfoxSimulation",
            targets: ["BeatfoxSimulation"]
        )
    ],
    targets: [
        .executableTarget(
            name: "BeatfoxSimulation",
            dependencies: [],
            path: "BeatfoxSimulation/Sources",
            exclude: [
                "SimulationControllerBridge.mm",
                "SimulationControllerBridge.h"
            ],
            sources: [
                "BeatfoxSimulationApp.swift",
                "SimulationView.swift",
                "ControlPanelView.swift",
                "SimulationViewModel.swift",
                "WaveVisualizationView.swift",
                "MouseHandlerView.swift",
                "AppDelegate.swift"
            ],
            publicHeadersPath: ".",
            cxxSettings: [
                .headerSearchPath("../../src"),
                .headerSearchPath("../../external"),
                .headerSearchPath("../../external/nanosvg/src"),
                .define("OBJC_OLD_DISPATCH_PROTOTYPES", to: "1"),
            ],
            linkerSettings: [
                .linkedLibrary("SimulationBridge", .when(platforms: [.macOS])),
                .linkedLibrary("SimulationCoreMinimal", .when(platforms: [.macOS])),
                .unsafeFlags(["-L../../build"], .when(platforms: [.macOS])),
                .unsafeFlags(["-L../../build/swiftui"], .when(platforms: [.macOS])),
                .linkedFramework("Metal"),
                .linkedFramework("CoreAudio"),
                .linkedFramework("AudioToolbox"),
                .linkedFramework("AppKit")
            ]
        )
    ],
    cxxLanguageStandard: .cxx17
)
