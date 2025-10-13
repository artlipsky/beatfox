#include "Application.h"
#include "SimulationEngine.h"

int main() {
    // Initialize application (GLFW, OpenGL, ImGui)
    Application app(1280, 720, "Acoustic Pressure Simulation");
    if (!app.initialize()) {
        return -1;
    }

    // Initialize simulation engine (subsystems, game loop)
    SimulationEngine engine(app);
    if (!engine.initialize()) {
        return -1;
    }

    // Run the simulation (blocks until window closes)
    engine.run();

    // Application and engine destructors handle cleanup
    return 0;
}
