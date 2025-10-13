# Test executable
add_executable(
    simulation_tests
    WaveSimulationTest.cpp
    ListenerTest.cpp
    AudioOutputTest.cpp
    InterferenceTest.cpp
    DampingPresetTest.cpp
    WallReflectionTest.cpp
    AbsorbingBoundaryDebugTest.cpp
    CoordinateMapperTest.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/AudioOutput.cpp
)

target_include_directories(simulation_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../external
)

target_link_libraries(
    simulation_tests
    SimulationCore
    GTest::gtest_main
)

# Platform-specific audio frameworks for AudioOutputTest
if(APPLE)
    target_link_libraries(simulation_tests
        "-framework AudioToolbox"
        "-framework CoreAudio"
    )
endif()

include(GoogleTest)
gtest_discover_tests(simulation_tests)
