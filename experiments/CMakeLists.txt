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
    src/Renderer.cpp
    src/AudioOutput.cpp
    src/glad.c
    ${IMGUI_SOURCES}
)

set(HEADERS
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
    src/SVGLoader.cpp
    src/SVGLoader.h
)

target_include_directories(SimulationCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${nanosvg_SOURCE_DIR}/src
)

# Tests
add_subdirectory(tests)
