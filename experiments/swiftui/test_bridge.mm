//
//  test_bridge.mm
//  Quick test to verify bridge compiles
//

#include "BeatfoxSimulation/Sources/SimulationControllerBridge.h"
#include <iostream>

int main() {
    std::cout << "Testing SimulationControllerBridge..." << std::endl;

    @autoreleasepool {
        // Create bridge
        SimulationControllerBridge *bridge = [[SimulationControllerBridge alloc] init];

        // Get initial state
        SimulationStateWrapper *state = [bridge getState];

        std::cout << "Bridge created successfully!" << std::endl;
        std::cout << "Grid size: " << state.width << " x " << state.height << std::endl;
        std::cout << "Room size: " << state.physicalWidth << "m x " << state.physicalHeight << "m" << std::endl;
        std::cout << "Impulse pressure: " << state.impulsePressure << " Pa" << std::endl;

        // Test command
        [bridge setImpulsePressure:10.0f];
        state = [bridge getState];
        std::cout << "After setting pressure to 10: " << state.impulsePressure << " Pa" << std::endl;

        // Test impulse
        std::cout << "Adding impulse at (100, 100)..." << std::endl;
        [bridge addImpulseAtX:100 y:100 pressure:5.0f radius:2];

        // Update simulation
        std::cout << "Updating simulation..." << std::endl;
        for (int i = 0; i < 10; i++) {
            [bridge updateWithDeltaTime:1.0f/60.0f];
        }

        std::cout << "✓ All tests passed!" << std::endl;
    }

    return 0;
}
