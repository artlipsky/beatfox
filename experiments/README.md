# Acoustic Wave Simulation with Real-Time Audio

A real-time 2D acoustic wave simulation with interactive visualization and audio output using OpenGL. This application simulates the propagation of sound waves in air using the 2D wave equation with realistic physics, and converts the pressure field to audible sound through a virtual microphone (listener).

## Features

- **Real-time wave equation solver** using Finite Difference Time Domain (FDTD) methods
- **Real-time audio output** from virtual microphone with proper resampling (60 FPS → 48kHz)
- **Interactive controls** for wave generation, listener placement, and obstacle editing
- **Obstacle support** with SVG room layout loading
- **Beautiful visualization** showing wave amplitude with color mapping
- **Adjustable parameters** including wave speed, damping, time scale, and volume
- **Clean Architecture** with comprehensive test suite (68 tests, 82% domain coverage)
- **Cross-platform** support (macOS, Linux, Windows)

## Quick Start

```bash
# Install dependencies (macOS)
make install

# Build and run
make run

# Run tests with coverage
make coverage

# Full quality check (linter + tests)
make check
```

## Physics

The simulation solves the 2D acoustic wave equation:

```
∂²p/∂t² = c² ∇²p
```

Where:
- `p` is acoustic pressure (Pa) - deviation from atmospheric pressure
- `c` is the speed of sound in air (343 m/s at 20°C)
- Physical units: 1 pixel = 5 cm, room size = 20m × 10m
- FDTD method with Courant–Friedrichs–Lewy (CFL) stability condition
- Air absorption damping for realistic wave dissipation

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

## Building and Running

### Using Makefile (Recommended)

```bash
# Build and run
make run              # Debug build and run
make run-release      # Optimized release build and run

# Build only
make build            # Debug build
make build-release    # Release build (optimized)

# Testing
make test             # Run all unit tests
make test-coverage    # Run tests with coverage report
make coverage         # Alias for test-coverage

# Code quality
make check            # Run linter and tests (full quality check)
make lint             # Build with strict warnings

# Utilities
make clean            # Clean build artifacts
make rebuild          # Clean and rebuild
make help             # Show all available commands
```

### Manual Build (If no Make)

```bash
mkdir build && cd build
cmake ..
cmake --build .
./SoundWaveSimulation
```

## Controls

### Mouse
- **Left Click**: Create sound impulse (5 Pa pressure, like a hand clap)
- **V + Left Click**: Place virtual listener (microphone) for audio output
- **O + Left Click**: Place obstacles
- **O + Right Click**: Remove obstacles

### Keyboard - Modes
- **V**: Toggle listener mode (place virtual microphone)
- **O**: Toggle obstacle mode (place/remove obstacles)
- **L**: Load SVG room layout from file

### Keyboard - Audio
- **M**: Mute/Unmute audio output
- **Shift + ↑/↓**: Increase/decrease volume

### Keyboard - Simulation
- **SPACE**: Clear all waves (reset pressure field)
- **C**: Clear all obstacles
- **+/- or [/]**: Adjust time scale (slow motion)
- **1**: Set time to 20× slower (default for audible sound)
- **0**: Set time to real-time

### Keyboard - Physics
- **↑/↓**: Increase/decrease sound speed
- **←/→**: Adjust air absorption (damping)
- **Shift + [/]**: Adjust obstacle brush size

### Interface
- **H**: Toggle help overlay
- **ESC**: Exit application

## Architecture

This project follows **Clean Architecture** and **Domain-Driven Design** principles:

### Layers
```
┌─────────────────────────────────────┐
│  UI Layer (main.cpp)                │
│  - GLFW window, ImGui, input        │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Infrastructure Layer               │
│  - AudioOutput: Audio device        │
│  - Renderer: OpenGL rendering       │
│  - SVGLoader: File parsing          │
└──────────────┬──────────────────────┘
               │
┌──────────────▼──────────────────────┐
│  Domain Layer (Core)                │
│  - WaveSimulation: Wave physics     │
│  - Listener: Virtual microphone     │
│  - Obstacle management              │
└─────────────────────────────────────┘
```

### Test Coverage

Run `make coverage` to generate a comprehensive coverage report:

- **WaveSimulation**: 82% line coverage
- **AudioOutput**: 90% line coverage
- **Critical paths**: 100% covered (wave propagation, audio resampling, thread safety)
- **68 tests total**, all passing

See `COVERAGE_REPORT.md` for detailed analysis.

### Code Quality

The project enforces strict compiler warnings to prevent dead code:
- `-Wall -Wextra -Wpedantic`
- `-Wunused-function` catches unused methods
- `-Wunused-variable` catches unused variables
- **Zero warnings** in our code

Run `make check` for full quality validation (linter + tests).

## How It Works

### Wave Simulation
The `WaveSimulation` class implements:
- FDTD solver with leapfrog time integration (2nd order accurate)
- CFL stability condition for numerical stability
- Reflective boundary conditions with energy loss
- Gaussian-shaped pressure sources for smooth wave generation
- Rigid obstacle boundaries (zero pressure)

### Audio Output
The `AudioOutput` class converts pressure to sound:
- Samples pressure at listener position (60 FPS)
- Resamples to 48kHz audio using linear interpolation (800 samples/frame)
- Thread-safe ring buffer with mutex protection
- Real-time playback via miniaudio library

### Rendering
The `Renderer` class handles OpenGL visualization:
- Vertex grid matching simulation resolution (400×200)
- Updates vertex heights each frame from pressure field
- Custom GLSL shaders for color mapping
- Colors: blue (compression) → green (neutral) → red (rarefaction)

## Project Structure

```
experiments/
├── Makefile              # Build automation
├── CMakeLists.txt        # CMake configuration
├── README.md             # This file
├── ARCHITECTURE.md       # Architecture documentation
├── COVERAGE_REPORT.md    # Test coverage analysis
├── src/
│   ├── main.cpp          # Application entry, UI integration
│   ├── WaveSimulation.h  # Wave physics (domain layer)
│   ├── WaveSimulation.cpp
│   ├── AudioOutput.h     # Audio output (infrastructure)
│   ├── AudioOutput.cpp
│   ├── Renderer.h        # OpenGL renderer (infrastructure)
│   ├── Renderer.cpp
│   ├── SVGLoader.h       # SVG parsing (infrastructure)
│   └── SVGLoader.cpp
├── tests/
│   ├── WaveSimulationTest.cpp  # Domain layer tests
│   ├── ListenerTest.cpp        # Listener functionality tests
│   └── AudioOutputTest.cpp     # Infrastructure tests
├── shaders/
│   ├── wave.vert         # Vertex shader
│   └── wave.frag         # Fragment shader
└── layouts/              # Sample SVG room layouts
```

## Technical Details

- **Language**: C++17
- **Graphics**: OpenGL 3.3 Core Profile
- **Window**: GLFW 3.3+
- **Math**: GLM
- **Audio**: miniaudio (single-header library)
- **SVG**: nanosvg
- **Testing**: Google Test
- **Build**: CMake 3.15+ / Make
- **Architecture**: Clean Architecture with DDD principles
- **Design Patterns**: Observer (Listener), Strategy (Resampling), Repository (SVGLoader)

## Development

### Running Tests

```bash
make test              # Run all tests
make test-verbose      # Run with verbose output
make test-coverage     # Run with coverage report
```

### Code Quality

```bash
make check             # Run linter and tests
make lint              # Build with strict warnings
make format            # Auto-format code with clang-format
make format-check      # Check formatting without changes
```

### Performance Profiling

The simulation is optimized for real-time performance:
- Cache-friendly memory layout (row-major iteration)
- Optimized FDTD stencil computation
- Parallel compilation (`-j4` flag)
- Release build with `-O3 -ffast-math`

For profiling:
```bash
make build-release
cd build-release
./SoundWaveSimulation
```

## Customization

### Simulation Parameters
In `src/WaveSimulation.cpp`:
- `soundSpeed`: Speed of sound (default: 343 m/s for air at 20°C)
- `damping`: Air absorption (default: 0.997 = 0.3% loss per step)
- `wallReflection`: Wall energy loss (default: 0.85 = 15% loss per reflection)
- `dx`: Spatial resolution (default: 0.05 m = 5 cm per pixel)

### Visual Appearance
In `shaders/wave.frag`:
- Modify color mapping functions
- Adjust brightness and contrast
- Change color palette

### Grid Resolution
In `src/main.cpp`:
- Adjust `gridWidth` and `gridHeight` for performance vs quality tradeoff
- Current: 400×200 (20m × 10m room at 5 cm/pixel)

## Documentation

- **README.md**: Quick start and usage (this file)
- **ARCHITECTURE.md**: Detailed architecture, design patterns, and principles
- **COVERAGE_REPORT.md**: Test coverage analysis and recommendations

## Contributing

When contributing code:
1. Run `make check` to verify quality (linter + tests)
2. Run `make coverage` to ensure adequate test coverage
3. Follow Clean Architecture principles (see ARCHITECTURE.md)
4. Add tests for new functionality
5. Ensure zero compiler warnings

## License

This project is part of the beatfox experiments collection.

## Future Enhancements

- Stereo audio output (two listeners)
- Frequency domain visualization (FFT)
- Multiple sound sources with different frequencies
- Variable wave speed zones (different materials)
- 3D visualization option
- Save/replay simulations
- GPU acceleration for larger grids
