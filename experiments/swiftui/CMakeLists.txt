# SwiftUI Application build configuration

# Build Obj-C++ bridge as a library
add_library(SimulationBridge STATIC
    BeatfoxSimulation/Sources/SimulationControllerBridge.mm
)

target_include_directories(SimulationBridge PUBLIC
    BeatfoxSimulation/Sources
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(SimulationBridge
    SimulationCoreMinimal
    "-framework Foundation"
    "-framework Metal"
    "-framework CoreAudio"
    "-framework AudioToolbox"
)

set_source_files_properties(
    BeatfoxSimulation/Sources/SimulationControllerBridge.mm
    PROPERTIES
    COMPILE_FLAGS "-x objective-c++ -fobjc-arc"
)

# List Swift source files
set(SWIFT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/BeatfoxSimulationApp.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/SimulationView.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/ControlPanelView.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/SimulationViewModel.swift
    ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/WaveVisualizationView.swift
)

# Custom target to build SwiftUI app using swiftc directly
add_custom_target(BeatfoxSimulationApp ALL
    COMMAND ${CMAKE_COMMAND} -E echo "Building SwiftUI app with swiftc..."
    COMMAND swiftc ${SWIFT_SOURCES}
        -o ${CMAKE_CURRENT_BINARY_DIR}/BeatfoxSimulation
        -import-objc-header ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources/SimulationControllerBridge.h
        -I ${CMAKE_CURRENT_SOURCE_DIR}/BeatfoxSimulation/Sources
        -I ${CMAKE_CURRENT_SOURCE_DIR}/../src
        -L ${CMAKE_CURRENT_BINARY_DIR}
        -L ${CMAKE_CURRENT_BINARY_DIR}/..
        -lSimulationBridge
        -lSimulationCoreMinimal
        -lc++
        -framework SwiftUI
        -framework AppKit
        -framework Metal
        -framework CoreAudio
        -framework AudioToolbox
        -framework Foundation
        -O
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS SimulationBridge SimulationCoreMinimal
    COMMENT "Building SwiftUI application with swiftc"
)

# Custom target to run SwiftUI app
add_custom_target(run-swiftui
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/BeatfoxSimulation
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS BeatfoxSimulationApp
    COMMENT "Running SwiftUI application"
)
