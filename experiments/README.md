# Sound Wave Simulation in C++

A real-time 2D sound wave simulation with interactive visualization using OpenGL. This application simulates the propagation of sound waves in air using the 2D wave equation with realistic physics.

## Features

- Real-time 2D wave equation solver using finite difference methods
- Interactive wave generation with mouse input
- Beautiful color-mapped visualization showing wave amplitude
- Adjustable wave speed and damping parameters
- OpenGL-based rendering with custom shaders
- Cross-platform support (macOS, Linux, Windows)

## Physics

The simulation solves the 2D wave equation:

```
∂²u/∂t² = c² * (∂²u/∂x² + ∂²u/∂y²)
```

Where:
- `u` is the wave amplitude (displacement)
- `c` is the wave speed
- The equation is discretized using finite difference methods
- Damping is applied to simulate energy dissipation

## Prerequisites

### macOS
```bash
brew install cmake glfw glm
```

### Linux (Ubuntu/Debian)
```bash
sudo apt-get install cmake libglfw3-dev libglm-dev libgl1-mesa-dev
```

### Windows
- Install CMake: https://cmake.org/download/
- Install vcpkg: https://github.com/microsoft/vcpkg
```
vcpkg install glfw3:x64-windows glm:x64-windows
```

## Building

```bash
# Create build directory
mkdir build
cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./SoundWaveSimulation
```

## Controls

### Mouse
- **Left Click**: Create a wave disturbance at cursor position
- **Left Click + Drag**: Continuously create waves while dragging

### Keyboard
- **SPACE**: Clear all waves (reset simulation)
- **UP Arrow**: Increase wave speed
- **DOWN Arrow**: Decrease wave speed
- **RIGHT Arrow**: Increase damping (waves decay slower)
- **LEFT Arrow**: Decrease damping (waves decay faster)
- **ESC**: Exit application

## How It Works

### Wave Simulation
The `WaveSimulation` class implements a 2D grid-based wave solver:
- Uses three buffers (previous, current, next) to store wave states
- Applies the discrete wave equation at each timestep
- Implements fixed boundary conditions (edges are fixed at zero)
- Supports Gaussian-shaped disturbances for smooth wave generation

### Rendering
The `Renderer` class handles OpenGL visualization:
- Creates a vertex grid matching the simulation resolution
- Updates vertex heights each frame based on wave amplitudes
- Uses custom GLSL shaders for color mapping
- Colors range from blue (negative) → green (neutral) → red (positive)

### Shaders
- **Vertex Shader** (`shaders/wave.vert`): Passes position and height data
- **Fragment Shader** (`shaders/wave.frag`): Maps wave height to color gradient

## Project Structure

```
experiments/
├── CMakeLists.txt         # Build configuration
├── README.md              # This file
├── src/
│   ├── main.cpp          # Application entry point & GLFW setup
│   ├── WaveSimulation.h  # Wave physics simulation header
│   ├── WaveSimulation.cpp # Wave physics implementation
│   ├── Renderer.h        # OpenGL renderer header
│   ├── Renderer.cpp      # OpenGL renderer implementation
│   ├── glad.h            # OpenGL loader header
│   └── glad.c            # OpenGL loader implementation
├── shaders/
│   ├── wave.vert         # Vertex shader
│   └── wave.frag         # Fragment shader
└── include/
    └── glad/
        └── glad.h        # GLAD header for OpenGL loading
```

## Customization

### Simulation Parameters
In `WaveSimulation.cpp`:
- `waveSpeed`: Initial wave propagation speed (default: 0.5)
- `damping`: Energy dissipation factor (default: 0.995)
- `gridSize`: Resolution of simulation grid (set in `main.cpp`, default: 200x200)

### Visual Appearance
In `shaders/wave.frag`:
- Modify `heightToColor()` function to change color mapping
- Adjust brightness calculations for different visual styles

### Performance
- Decrease `gridSize` in `main.cpp` for better performance
- Increase `gridSize` for higher resolution simulation
- Adjust `fixedDt` for different simulation speeds

## Technical Details

- **Language**: C++17
- **Graphics**: OpenGL 3.3 Core Profile
- **Window Management**: GLFW 3.3+
- **Math Library**: GLM
- **OpenGL Loader**: Custom GLAD implementation
- **Build System**: CMake 3.15+

## License

This project is part of the beatfox experiments collection.

## Future Enhancements

- 3D visualization with height-mapped surface
- Multiple wave sources with different frequencies
- Save/load simulation states
- Real-time frequency analysis
- Obstacle placement for wave reflection/diffraction
- Variable wave speed zones (different materials)
