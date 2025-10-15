# UI Architecture - Pluggable View Layer

## Overview

The simulation uses a **Model-View-Controller (MVC)** architecture with a pluggable view layer. This allows you to swap out the UI framework (ImGui, SwiftUI, Qt, etc.) without modifying the core simulation logic.

## Architecture Diagram

```
┌─────────────────────────────────────────────────────────────┐
│                     UI Layer (Pluggable)                     │
│                                                              │
│  ┌────────────────────┐                                     │
│  │ ISimulationView    │  ◄─── Abstract Interface           │
│  │  (interface)       │                                     │
│  └────────────────────┘                                     │
│           ▲                                                  │
│           │                                                  │
│           ├──────────────┬──────────────┬─────────────┐    │
│           │              │              │             │    │
│  ┌────────┴──────┐  ┌───┴────────┐ ┌───┴──────┐ ┌───┴──┐ │
│  │ ImGuiView     │  │ SwiftUIView│ │ AppKitView│ │QtView│ │
│  │ (Current)     │  │ (Future)   │ │ (Future)  │ │...   │ │
│  └───────────────┘  └────────────┘ └───────────┘ └──────┘ │
└─────────────────────────────────────────────────────────────┘
                        │
                        ▼ (generates UICommands)
┌─────────────────────────────────────────────────────────────┐
│              Application Layer (Framework-agnostic)          │
│                                                              │
│  ┌──────────────────┐      ┌──────────────────────┐        │
│  │ SimulationState  │◄─────│ SimulationController │        │
│  │ (Model)          │      │ (Controller)         │        │
│  │                  │      │                      │        │
│  │ - UI state       │      │ - Processes commands │        │
│  │ - Simulation info│      │ - Updates simulation │        │
│  │ - Audio info     │      │ - Updates state      │        │
│  └──────────────────┘      └──────────────────────┘        │
│           ▲                         │                        │
│           │                         ▼ (calls methods)        │
│           └─────────────────────────┘                        │
└─────────────────────────────────────────────────────────────┘
                        │
                        ▼
┌─────────────────────────────────────────────────────────────┐
│                   Core Layer (UI-agnostic)                   │
│                                                              │
│  ┌────────────────┐  ┌────────────┐  ┌──────────────────┐ │
│  │ WaveSimulation │  │AudioOutput │  │MetalBackend      │ │
│  │ (Physics)      │  │ (Audio)    │  │ (GPU compute)    │ │
│  └────────────────┘  └────────────┘  └──────────────────┘ │
└─────────────────────────────────────────────────────────────┘
```

## Key Components

### 1. SimulationState (Model)
**File:** `src/SimulationState.h`

Centralized state container for all UI-related data.

```cpp
struct SimulationState {
    // Display settings
    bool showHelp;
    bool gridEnabled;

    // Interaction modes
    bool obstacleMode;
    float timeScale;

    // Read-only simulation info
    struct SimulationInfo {
        int width, height;
        float waveSpeed;
        bool hasListener;
        // ...
    } info;

    // Read-only audio info
    struct AudioInfo {
        bool isMuted;
        float volume;
        // ...
    } audio;
};
```

**Benefits:**
- Single source of truth for UI state
- Easy to serialize/deserialize (save/load sessions)
- Easy to test without UI
- Clear API for what UI can read/write

### 2. UICommand (Events)
**File:** `src/SimulationState.h`

Command pattern for all UI interactions.

```cpp
struct UICommand {
    enum class Type {
        ADD_IMPULSE,
        CLEAR_WAVES,
        TOGGLE_LISTENER,
        // ...
    };
};

struct AddImpulseCommand : UICommand {
    int x, y;
    float pressure;
    int radius;
};
```

**Benefits:**
- Decouples UI from simulation
- Easy to add undo/redo
- Can log/replay commands
- Testable without UI

### 3. ISimulationView (View Interface)
**File:** `src/ISimulationView.h`

Abstract interface that all UI implementations must satisfy.

```cpp
class ISimulationView {
public:
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;

    virtual void beginFrame() = 0;
    virtual std::vector<std::unique_ptr<UICommand>>
        render(const SimulationState& state) = 0;
    virtual void endFrame() = 0;

    virtual void onWindowResize(int w, int h) = 0;
    virtual const char* getViewName() const = 0;
};
```

**Benefits:**
- Framework-agnostic API
- Easy to swap implementations
- Forces clean separation of concerns

### 4. SimulationController (Controller)
**Files:** `src/SimulationController.h`, `src/SimulationController.cpp`

Processes commands and coordinates between UI and simulation.

```cpp
class SimulationController {
public:
    void processCommand(const UICommand& command);
    void updateState();  // Updates SimulationState from simulation
    const SimulationState& getState() const;

private:
    SimulationState state;
    WaveSimulation* simulation;
    AudioOutput* audioOutput;
    // ...
};
```

**Benefits:**
- Single place for all application logic
- No UI code in simulation
- No simulation code in UI
- Easy to test business logic

## Data Flow

### 1. Initialization
```
Application
  ↓
Creates: WaveSimulation, AudioOutput, Renderer
  ↓
Creates: SimulationController(sim, audio, renderer)
  ↓
Creates: ISimulationView* view = createDefaultView()
  ↓
view->initialize()
```

### 2. Each Frame
```
controller->updateState()          // Read from simulation
  ↓
view->beginFrame()                 // UI framework setup
  ↓
commands = view->render(state)     // UI reads state, returns commands
  ↓
controller->processCommands(commands)  // Execute commands
  ↓
view->endFrame()                   // UI framework teardown
```

### 3. User Interaction Example
```
User clicks mouse
  ↓
ImGuiView::render() detects click
  ↓
Creates: AddImpulseCommand(x, y, pressure, radius)
  ↓
Returns command in vector
  ↓
Controller::processCommand(cmd)
  ↓
Controller::handleAddImpulse()
  ↓
simulation->addImpulse(x, y, pressure, radius)
```

## How to Implement a New UI

### Example: SwiftUI View

#### Step 1: Create SwiftUI Bridge
```swift
// SwiftUISimulationView.swift
class SwiftUISimulationView: ObservableObject {
    @Published var state: SimulationState
    private let controller: UnsafeMutableRawPointer  // C++ controller

    func render() {
        // SwiftUI views automatically update when state changes
    }

    func sendCommand(_ command: UICommand) {
        // Call C++ controller
        controller.processCommand(command)
    }
}
```

#### Step 2: Implement ISimulationView
```cpp
// SwiftUIBridge.mm (Objective-C++)
class SwiftUIBridge : public ISimulationView {
    SwiftUISimulationView* swiftView;

    bool initialize() override {
        swiftView = [[SwiftUISimulationView alloc] init];
        return true;
    }

    std::vector<std::unique_ptr<UICommand>>
    render(const SimulationState& state) override {
        // Update Swift state
        [swiftView updateState:state];

        // Get commands generated by SwiftUI interactions
        return [swiftView getCommands];
    }

    const char* getViewName() const override {
        return "SwiftUI";
    }
};
```

#### Step 3: Register in Factory
```cpp
// ISimulationView.cpp
std::unique_ptr<ISimulationView> createDefaultView() {
#ifdef __APPLE__
    return std::make_unique<SwiftUIBridge>();
#else
    return std::make_unique<ImGuiSimulationView>();
#endif
}
```

### Example: Qt View

```cpp
// QtSimulationView.h
class QtSimulationView : public QWidget, public ISimulationView {
    Q_OBJECT

public:
    bool initialize() override;
    std::vector<std::unique_ptr<UICommand>>
        render(const SimulationState& state) override;

private slots:
    void onClearButtonClicked();
    void onSliderMoved(int value);

private:
    QVBoxLayout* layout;
    QPushButton* clearButton;
    QSlider* timeScaleSlider;

    std::vector<std::unique_ptr<UICommand>> pendingCommands;
};
```

## Testing Without UI

The architecture makes it easy to test simulation logic without running the UI:

```cpp
// test/SimulationControllerTest.cpp
TEST(SimulationController, AddImpulse) {
    WaveSimulation sim(100, 100);
    AudioOutput audio;
    SimulationController controller(&sim, &audio, nullptr, nullptr);

    // Execute command without UI
    AddImpulseCommand cmd(50, 50, 5.0f, 2);
    controller.processCommand(cmd);

    // Verify result
    EXPECT_GT(sim.getData()[50 * 100 + 50], 0.0f);
}

TEST(SimulationController, StateUpdate) {
    WaveSimulation sim(200, 100);
    SimulationController controller(&sim, nullptr, nullptr, nullptr);

    controller.updateState();
    const auto& state = controller.getState();

    EXPECT_EQ(state.info.width, 200);
    EXPECT_EQ(state.info.height, 100);
}
```

## Migration Strategy

To integrate this architecture into existing code:

### Phase 1: Add New Classes (Non-breaking)
- [x] Add SimulationState.h
- [x] Add ISimulationView.h
- [x] Add SimulationController.h/cpp
- [ ] Build succeeds with new files

### Phase 2: Implement ImGui View
- [ ] Create ImGuiSimulationView : ISimulationView
- [ ] Move ImGui code from SimulationUI to ImGuiSimulationView
- [ ] Implement render() to return UICommands

### Phase 3: Refactor SimulationEngine
- [ ] Add SimulationController member
- [ ] Replace direct simulation calls with controller->processCommand()
- [ ] Update render loop to use view->render()

### Phase 4: Remove Old Code
- [ ] Remove SimulationUI (superseded by ImGuiSimulationView)
- [ ] Remove InputHandler (logic moved to controller)
- [ ] Clean up state scattered across SimulationEngine

## Benefits

### For Development
- **Separation of Concerns**: UI, logic, and simulation are independent
- **Testability**: Can test logic without running UI
- **Maintainability**: Changes to UI don't affect simulation
- **Flexibility**: Easy to add new UI frameworks

### For Users
- **Choice**: Can choose UI framework (ImGui, native, etc.)
- **Platform-specific UIs**: Native AppKit on macOS, Qt on Linux
- **Accessibility**: Easier to add accessibility features
- **Customization**: Can create custom UIs for specific needs

### For Future
- **Web UI**: Could add web-based UI (WebAssembly + JavaScript)
- **Mobile**: Could port to iOS/Android with native UI
- **Scripting**: Commands can be scripted (Python, Lua)
- **Networking**: Could add remote control via network commands

## See Also

- `src/SimulationState.h` - State definition
- `src/ISimulationView.h` - View interface
- `src/SimulationController.h` - Controller interface
- `examples/CustomUIExample.md` - Full example of custom UI
