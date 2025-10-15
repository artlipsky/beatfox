cmake_minimum_required(VERSION 3.15)
project(SoundWaveSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aggressive performance optimization
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -ffast-math -DNDEBUG")
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
endif()

# Strict compiler warnings to catch dead code and unused variables
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang|AppleClang")
    add_compile_options(
        -Wall              # Enable all standard warnings
        -Wextra            # Enable extra warnings
        -Wpedantic         # Strict ISO C++ compliance
        -Wunused           # Warn about unused entities
        -Wunused-function  # Warn about unused functions (catches dead methods)
        -Wunused-parameter # Warn about unused parameters
        -Wunused-variable  # Warn about unused variables
    )
endif()

# Find required packages
find_package(OpenGL REQUIRED)

# GLFW
find_package(glfw3 3.3 REQUIRED)

# GLM
find_package(glm REQUIRED)

# Dear ImGui sources
set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    src/Application.cpp
    src/SimulationEngine.cpp
    src/Renderer.cpp
    src/AudioOutput.cpp
    src/glad.c
    ${IMGUI_SOURCES}
)

set(HEADERS
    src/Application.h
    src/SimulationEngine.h
    src/Renderer.h
    src/AudioOutput.h
    src/glad.h
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OPENGL_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${nanosvg_SOURCE_DIR}/src
    src
)

target_link_libraries(${PROJECT_NAME}
    SimulationCore
    OpenGL::GL
    glfw
    glm::glm
)

# Platform-specific settings
if(APPLE)
    target_link_libraries(${PROJECT_NAME}
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
        "-framework AudioToolbox"
        "-framework CoreAudio"
    )
endif()

# Copy shaders to build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

# Copy Metal shaders to build directory (for GPU acceleration)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src/shaders
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/src)

# Enable testing
enable_testing()

# External dependencies
include(FetchContent)

# Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# nanosvg - Single-header SVG parser and rasterizer
FetchContent_Declare(
    nanosvg
    GIT_REPOSITORY https://github.com/memononen/nanosvg.git
    GIT_TAG master
)
FetchContent_MakeAvailable(nanosvg)

# Create library from core logic (for testing)
add_library(SimulationCore STATIC
    src/WaveSimulation.cpp
    src/WaveSimulation.h
    src/DampingPreset.cpp
    src/DampingPreset.h
    src/SVGLoader.cpp
    src/SVGLoader.h
    src/AudioSample.cpp
    src/AudioSample.h
    src/AudioSource.h
    src/AudioFileLoader.cpp
    src/AudioFileLoader.h
    src/MetalSimulationBackend.mm
    src/MetalSimulationBackend.h
    src/CoordinateMapper.cpp
    src/CoordinateMapper.h
    src/SimulationController.cpp
    src/SimulationController.h
    src/SimulationState.h
    src/ISimulationView.h
    src/SimulationUI.cpp
    src/SimulationUI.h
    src/InputHandler.cpp
    src/InputHandler.h
    src/SimulationEngine.cpp
    src/SimulationEngine.h
    src/Renderer.cpp
    src/Renderer.h
    src/glad.c
    ${IMGUI_SOURCES}
)

target_include_directories(SimulationCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${nanosvg_SOURCE_DIR}/src
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(SimulationCore
    OpenGL::GL
    glfw
    glm::glm
)

# Apple-specific: Enable Objective-C++ for Metal backend and link Metal framework
if(APPLE)
    set_source_files_properties(src/MetalSimulationBackend.mm PROPERTIES
        COMPILE_FLAGS "-x objective-c++ -fobjc-arc"
    )
    target_link_libraries(SimulationCore
        "-framework Metal"
        "-framework Foundation"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()

# Minimal library for SwiftUI (no UI dependencies)
add_library(SimulationCoreMinimal STATIC
    src/WaveSimulation.cpp
    src/WaveSimulation.h
    src/DampingPreset.cpp
    src/DampingPreset.h
    src/SVGLoader.cpp
    src/SVGLoader.h
    src/AudioSample.cpp
    src/AudioSample.h
    src/AudioSource.h
    src/AudioFileLoader.cpp
    src/AudioFileLoader.h
    src/MetalSimulationBackend.mm
    src/MetalSimulationBackend.h
    src/SimulationController.cpp
    src/SimulationController.h
    src/SimulationState.h
    src/AudioOutput.cpp
    src/AudioOutput.h
)

target_include_directories(SimulationCoreMinimal PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    ${nanosvg_SOURCE_DIR}/src
)

target_compile_definitions(SimulationCoreMinimal PRIVATE
    BEATFOX_MINIMAL_BUILD
)

if(APPLE)
    set_source_files_properties(src/MetalSimulationBackend.mm PROPERTIES
        COMPILE_FLAGS "-x objective-c++ -fobjc-arc"
    )
    target_link_libraries(SimulationCoreMinimal
        "-framework Metal"
        "-framework Foundation"
        "-framework CoreAudio"
        "-framework AudioToolbox"
    )
endif()

# SwiftUI App (macOS only)
if(APPLE)
    add_subdirectory(swiftui)
endif()

# Tests
add_subdirectory(tests)
