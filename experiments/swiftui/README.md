# BeatFox SwiftUI - macOS Native UI

Beautiful native macOS interface for the acoustic wave simulation using SwiftUI.

## Architecture

```
SwiftUI (View) → SimulationViewModel → Bridge (Obj-C++) → SimulationController (C++)
```

## Setup Instructions

### 1. Create Xcode Project

```bash
cd swiftui
```

Open Xcode and create a new project:
- File → New → Project
- Choose "macOS" → "App"
- Product Name: `BeatfoxSimulation`
- Interface: SwiftUI
- Language: Swift
- Save in: `swiftui/` directory

### 2. Add Source Files

In Xcode project navigator:
1. Delete the default `BeatfoxSimulationApp.swift` and `ContentView.swift`
2. Right-click project → Add Files to "BeatfoxSimulation"
3. Add all files from `BeatfoxSimulation/Sources/`:
   - `BeatfoxSimulationApp.swift`
   - `SimulationView.swift`
   - `ControlPanelView.swift`
   - `SimulationViewModel.swift`
   - `SimulationControllerBridge.h`
   - `SimulationControllerBridge.mm`

### 3. Configure C++ Integration

#### a) Add Bridging Header

1. File → New → File
2. Choose "Header File"
3. Name: `BeatfoxSimulation-Bridging-Header.h`
4. Content:
```objc
#import "SimulationControllerBridge.h"
```

#### b) Update Build Settings

In project settings → Build Settings:

1. **Objective-C Bridging Header**:
   ```
   BeatfoxSimulation/BeatfoxSimulation-Bridging-Header.h
   ```

2. **Header Search Paths**:
   ```
   $(SRCROOT)/../../src
   $(SRCROOT)/../../external
   ```

3. **Library Search Paths**:
   ```
   $(SRCROOT)/../../build
   ```

4. **Other Linker Flags**:
   ```
   -lSimulationCore
   -framework Metal
   -framework Foundation
   -framework CoreAudio
   -framework AudioToolbox
   ```

5. **Clang Language Features**:
   - C++ Language Dialect: `GNU++17`
   - Enable Modules (C and Objective-C): `Yes`

### 4. Link C++ Library

1. In project settings → "Build Phases" → "Link Binary With Libraries"
2. Click "+" → "Add Other..." → "Add Files..."
3. Navigate to `../../build/libSimulationCore.a`
4. Add the following system frameworks:
   - Metal.framework
   - Foundation.framework
   - CoreAudio.framework
   - AudioToolbox.framework

### 5. Build and Run

1. Make sure the C++ library is built:
```bash
cd ../..
cmake --build build --parallel 8
```

2. In Xcode:
   - Product → Build (⌘B)
   - Product → Run (⌘R)

## Features

✓ Native macOS SwiftUI interface
✓ Full command pattern integration
✓ Real-time audio output
✓ Reactive UI with @Published properties
✓ Clean C++/Swift bridge
✓ 60 FPS simulation updates

## UI Features

- **Control Panel**: Slide-in from right (press H to toggle)
- **Impulse Parameters**: Pressure and radius sliders
- **Acoustic Presets**: Realistic, Visualization, Anechoic
- **Time Scale**: Slow motion control (1000× slower to 0.25× speed)
- **Audio Controls**: Mute/unmute, volume control
- **Simulation Info**: Grid size, room dimensions, wave speed, etc.
- **Action Buttons**: Clear waves, clear obstacles

## Keyboard Shortcuts

- `H` - Toggle help panel
- Click on simulation canvas - Add impulse at cursor position

## Next Steps

### Add Metal Rendering

To visualize the wave simulation, add a Metal view:

1. Create `MetalSimulationRenderer.swift`
2. Use `MTKView` to render simulation data
3. Update `SimulationView.swift` to use Metal renderer instead of placeholder

### Add Gesture Support

- Pan to add obstacles
- Pinch to zoom
- Two-finger click for listener placement

## Troubleshooting

### Build Errors

**"Cannot find 'SimulationControllerBridge' in scope"**
- Make sure bridging header is configured correctly
- Check Header Search Paths include `../../src`

**"Undefined symbols for architecture x86_64"**
- Rebuild C++ library: `cmake --build build`
- Check that `libSimulationCore.a` is linked in Build Phases

**"Module 'BeatfoxSimulation' not found"**
- Clean build folder (Product → Clean Build Folder)
- Restart Xcode

### Runtime Issues

**App crashes on launch**
- Check Console.app for error messages
- Verify Metal backend initialized (should see "MetalBackend: Using GPU: Apple M3 Max")

**No audio output**
- Check System Settings → Privacy & Security → Microphone (may need audio permissions)
- Verify AudioOutput initialized in console logs

## Architecture Benefits

✅ **Pluggable UI**: Can swap SwiftUI for Qt/React/Web without touching C++ domain logic
✅ **Testable**: All business logic tested independently (29 tests)
✅ **Native Performance**: Direct Metal rendering, no overhead
✅ **Modern**: SwiftUI reactive patterns with Combine
✅ **Maintainable**: Clear separation of concerns

## File Structure

```
swiftui/
└── BeatfoxSimulation/
    ├── Sources/
    │   ├── BeatfoxSimulationApp.swift      # App entry point
    │   ├── SimulationView.swift             # Main view
    │   ├── ControlPanelView.swift           # Control panel UI
    │   ├── SimulationViewModel.swift        # SwiftUI ViewModel
    │   ├── SimulationControllerBridge.h     # Obj-C++ header
    │   └── SimulationControllerBridge.mm    # Obj-C++ implementation
    └── README.md (this file)
```

---

🎉 **Enjoy beautiful native macOS UI for your acoustic simulation!**
