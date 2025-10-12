# Test executable
add_executable(
    simulation_tests
    WaveSimulationTest.cpp
)

target_link_libraries(
    simulation_tests
    SimulationCore
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(simulation_tests)
