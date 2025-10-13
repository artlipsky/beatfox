# Architecture Documentation

## Overview

This project follows **Clean Architecture** and **Domain-Driven Design (DDD)** principles to create a maintainable, testable, and extensible acoustic wave simulation system with real-time audio output.

## Clean Architecture Layers

```
┌─────────────────────────────────────────────────┐
│          UI Layer (main.cpp)                    │
│  - GLFW window management                       │
│  - ImGui UI rendering                           │
│  - Input handling (keyboard, mouse)             │
│  - Coordinate mapping                           │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│      Infrastructure Layer                       │
│  - AudioOutput: Audio device management         │
│  - Renderer: OpenGL rendering                   │
│  - SVGLoader: File parsing                      │
└─────────────────┬───────────────────────────────┘
                  │
┌─────────────────▼───────────────────────────────┐
│         Domain Layer (Core)                     │
│  - WaveSimulation: Acoustic physics             │
│  - Listener: Virtual microphone                 │
│  - Wave propagation rules                       │
│  - Obstacle interactions                        │
└─────────────────────────────────────────────────┘
```

### Dependency Rule

**Dependencies point inward**: Outer layers depend on inner layers, never the reverse.

- ✅ UI → Infrastructure → Domain
- ✅ Infrastructure can use Domain interfaces
- ❌ Domain NEVER depends on Infrastructure
- ❌ Domain NEVER depends on UI

## Domain-Driven Design (DDD)

### Ubiquitous Language

The domain model uses precise acoustic physics terminology:

- **Acoustic Pressure** (Pa) - Deviation from atmospheric pressure
- **Wave Propagation** - Governed by wave equation: ∂²p/∂t² = c² ∇²p
- **Listener** - Virtual microphone sampling acoustic field
- **Obstacle** - Rigid boundary with zero pressure
- **Damping** - Energy dissipation (air absorption)
- **Reflection** - Wave bounce at boundaries
- **Sound Speed** - Propagation velocity (m/s)

### Domain Model

**WaveSimulation** is the core domain entity:

```cpp
class WaveSimulation {
public:
    // Domain operations
    void update(float dt);
    void addPressureSource(int x, int y, float pressure);

    // Listener (Observer pattern)
    void setListenerPosition(int x, int y);
    void setListenerEnabled(bool enabled);
    float getListenerPressure() const;

    // Obstacle management
    void addObstacle(int x, int y, int radius);
    void removeObstacle(int x, int y, int radius);

    // Physics parameters
    void setWaveSpeed(float speed);
    void setDamping(float damping);

private:
    // Domain state (encapsulated)
    std::vector<float> pressure;
    std::vector<float> pressurePrev;
    std::vector<uint8_t> obstacles;
    int listenerX, listenerY;
    bool listenerEnabled;
};
```

### Domain Rules (Invariants)

1. **Pressure Conservation**: Wave equation preserves energy (minus damping)
2. **Zero Pressure at Obstacles**: Rigid boundaries enforce p = 0
3. **Listener is Observer**: Moving listener doesn't affect wave field
4. **Bounded Grid**: All coordinates clamped to valid range
5. **Physical Units**: All quantities have consistent physical units

## SOLID Principles

### Single Responsibility Principle (SRP)

Each class has one reason to change:

- **WaveSimulation**: Wave physics and domain logic
- **AudioOutput**: Audio device management and conversion
- **Renderer**: Visual rendering
- **SVGLoader**: File parsing and rasterization

Example: AudioOutput doesn't know about wave physics, WaveSimulation doesn't know about audio hardware.

### Open/Closed Principle (OCP)

- **Open for extension**: New audio backends can be added
- **Closed for modification**: Domain logic doesn't change when adding features
- Example: Adding WebAudio backend wouldn't touch WaveSimulation

### Liskov Substitution Principle (LSP)

- **Listener interface** is minimal and consistent
- **Audio output** can be replaced with different implementations
- No violations of expected behavior

### Interface Segregation Principle (ISP)

Interfaces are focused and minimal:

```cpp
// Listener interface (5 methods, all necessary)
void setListenerPosition(int x, int y);
void getListenerPosition(int& x, int& y) const;
void setListenerEnabled(bool enabled);
bool hasListener() const;
float getListenerPressure() const;

// Audio interface (focused on audio output)
bool initialize(int sampleRate);
void start();
void stop();
void submitPressureSample(float pressure, float timeScale);
void setVolume(float volume);
void setMuted(bool muted);
```

No client is forced to depend on methods it doesn't use.

### Dependency Inversion Principle (DIP)

- **Domain** defines interfaces (conceptual)
- **Infrastructure** implements them
- Example: WaveSimulation provides `getListenerPressure()`, AudioOutput consumes it
- High-level policy (domain) doesn't depend on low-level details (audio hardware)

## Design Patterns

### Observer Pattern

**Listener** implements the Observer pattern:
- Subject: Wave field (pressure grid)
- Observer: Listener (virtual microphone)
- Notification: Pressure sampling via `getListenerPressure()`
- Decoupling: Observer doesn't affect subject

### Strategy Pattern

Audio resampling strategy can be swapped:
- Current: Linear interpolation
- Alternative: Cubic interpolation, sinc resampling
- Isolated in `submitPressureSample()` method

### Repository Pattern

SVGLoader acts as a repository:
- Abstracts file system details
- Loads obstacles from external storage
- Domain doesn't know about SVG format

## Testing Strategy

### Test Pyramid

```
    ┌─────┐
    │ UI  │  (Manual testing)
    └─────┘
   ┌───────┐
   │ Integ │  (System tests)
   └───────┘
  ┌─────────┐
  │  Unit   │  (68 automated tests)
  └─────────┘
```

### Test Coverage

**Domain Layer Tests** (`ListenerTest.cpp`):
- 17 tests covering listener functionality
- Validates domain rules and invariants
- Tests physical behavior (propagation, damping, obstacles)
- Verifies Observer pattern implementation

**Infrastructure Layer Tests** (`AudioOutputTest.cpp`):
- 32 tests covering audio output
- Validates audio conversion logic
- Tests thread safety (audio callback runs on separate thread)
- Verifies error handling and edge cases

**Total: 68 tests, 100% passing**

### Test Characteristics

- **Fast**: Most tests run in milliseconds
- **Isolated**: No external dependencies (files, network, actual audio hardware)
- **Deterministic**: Same input always produces same output
- **Readable**: Tests document behavior and intent

## Clean Code Principles

### Meaningful Names

```cpp
// Good: Expresses intent
void setListenerPosition(int x, int y);
float getListenerPressure() const;

// Bad (what we DON'T do)
void set(int a, int b);
float get();
```

### Small Functions

Functions do one thing:

```cpp
// Single responsibility: Sample pressure at listener
float WaveSimulation::getListenerPressure() const {
    if (!listenerEnabled) return 0.0f;
    if (listenerX < 0 || listenerX >= width) return 0.0f;
    if (listenerY < 0 || listenerY >= height) return 0.0f;
    return pressure[index(listenerX, listenerY)];
}
```

### No Side Effects

Pure functions where possible:

```cpp
// Pure: No hidden state changes
float getPhysicalWidth() const { return width * dx; }
bool isObstacle(int x, int y) const { return obstacles[index(x, y)]; }

// Clear side effects in name
void setListenerPosition(int x, int y);  // "set" indicates mutation
```

### DRY (Don't Repeat Yourself)

Common logic extracted:

```cpp
// Reusable index calculation
int index(int x, int y) const { return y * width + x; }

// Used everywhere instead of repeating calculation
pressure[index(x, y)] = value;
obstacles[index(x, y)] = true;
```

### Comments Explain WHY, Not WHAT

```cpp
// Good: Explains rationale
// Default time scale: 0.01x (100x slower)
// Rationale: Makes wave propagation audible
float timeScale = 0.01f;

// Bad (what we don't do)
// Set time scale to 0.01
float timeScale = 0.01f;
```

## Boundaries and Interfaces

### Domain ↔ Infrastructure Boundary

**Domain exposes**:
```cpp
float getListenerPressure() const;  // Physical quantity (Pascals)
```

**Infrastructure consumes**:
```cpp
void AudioOutput::submitPressureSample(float pressure, float timeScale);
```

**Key insight**: Domain speaks in physical units, infrastructure translates to audio samples.

### UI ↔ Domain Boundary

**UI provides**:
```cpp
int gridX, gridY;  // Grid coordinates from mouse click
```

**Domain accepts**:
```cpp
void setListenerPosition(int x, int y);  // Domain coordinates
```

**Key insight**: UI handles coordinate mapping, domain operates on logical grid.

## Thread Safety

### Audio Callback Thread

AudioOutput runs on separate thread:

```cpp
// Thread-safe with mutex
std::lock_guard<std::mutex> lock(bufferMutex);

// Atomic for lockless reads
std::atomic<float> volume;
std::atomic<bool> muted;
```

### Guarantee

Domain operations are NOT thread-safe (by design):
- Domain runs on main thread only
- AudioOutput safely consumes domain output
- Clear ownership prevents data races

## Error Handling

### Defensive Programming

```cpp
// Clamp to valid range
listenerX = std::max(0, std::min(x, width - 1));
listenerY = std::max(0, std::min(y, height - 1));

// Validate before use
if (!listenerEnabled) return 0.0f;
if (x < 0 || x >= width) return false;
```

### Fail-Safe Defaults

- Listener disabled by default → No accidental audio
- Volume = 1.0 by default → Reasonable output
- Zero pressure in obstacles → Physics consistency

## Performance Considerations

### Hot Path Optimization

Wave update loop (called 60× per second):
```cpp
// Cache-friendly memory access
for (int y = 1; y < height - 1; y++) {
    const int rowOffset = y * width;      // Precompute
    for (int x = 1; x < width - 1; x++) {
        const int idx = rowOffset + x;    // Sequential access
        // ... computation
    }
}
```

### Audio Resampling

Critical fix for continuous audio:
```cpp
// Generate 800 samples per frame (48000 Hz / 60 FPS)
int samplesPerFrame = sampleRate / simulationFrameRate;

// Linear interpolation for smoothness
for (int i = 0; i < samplesPerFrame; i++) {
    float t = float(i) / float(samplesPerFrame);
    float sample = previousPressure + t * (pressure - previousPressure);
    audioBuffer[bufferWritePos++] = sample;
}
```

## Extensibility

### Adding New Features

Examples of easy extensions:

**New audio backend**:
```cpp
class WebAudioOutput : public AudioOutputBase {
    // Implement audio interface
    // Domain code unchanged
};
```

**New visualization**:
```cpp
class VectorFieldRenderer : public RendererBase {
    // Render velocity vectors
    // Domain code unchanged
};
```

**New listener modes**:
```cpp
// Stereo listener (two positions)
void setListenerPosition(int leftX, int leftY, int rightX, int rightY);

// Domain easily extended
```

## Documentation Standards

Every class has:
1. **Purpose**: What it does
2. **Responsibilities**: What it owns
3. **Layer**: Where it sits in architecture
4. **Dependencies**: What it depends on
5. **Thread safety**: Guarantees provided

Example:
```cpp
/*
 * AudioOutput - Infrastructure layer component
 *
 * Converts pressure samples from acoustic simulation to real-time audio.
 * Uses miniaudio for cross-platform audio playback.
 *
 * Thread safety: submitPressureSample() is thread-safe.
 * Audio callback runs on separate thread.
 */
```

## Conclusion

This architecture achieves:

✅ **Testability**: 68 automated tests, fast and reliable
✅ **Maintainability**: Clear separation of concerns
✅ **Extensibility**: Easy to add features without breaking existing code
✅ **Understandability**: Code reads like the problem domain
✅ **Correctness**: Domain rules enforced, physics preserved
✅ **Performance**: Optimized hot paths, efficient memory layout

The architecture scales from this acoustic simulation to much larger domains while maintaining clarity and flexibility.
