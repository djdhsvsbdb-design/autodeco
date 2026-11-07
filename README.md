# Alt3rDECO

An intelligent AI-powered decoration system for Geometry Dash 2.2081 (PC) using the Geode modding framework.

## Overview

Alt3rDECO analyzes selected level geometry and automatically generates human-like decoration patterns using adaptive procedural rules and pattern recognition. The system behaves like a junior human designer assistant, avoiding mechanical repetition and introducing slight imperfections for realism.

## Features

### Core AI Engine
- **Geometry Understanding Layer**: Detects object clusters, shape types (linear, vertical, enclosed, sparse), density maps, and visual importance zones
- **Pattern Recognition System**: Detects symmetry (horizontal, vertical, radial, bilateral), repetition, flow direction, and density imbalances
- **Adaptive Decoration Logic**: Edge emphasis, visual hierarchy, style adaptation based on detected geometry

### Built-in AI Presets
1. **Adaptive Neon Intelligence**: High contrast outlines with layered lighting objects
2. **Minimal Structural Balance**: Clean, symmetric decoration with low clutter
3. **High Detail Demon Flow**: High density spikes with chaotic micro detail
4. **Clean Architectural Mode**: Grid-based placement with strict spacing
5. **Retro Grid Symmetry**: Pixel-style grid placement with retro aesthetics

### Editor Integration
- Toolbar button "Alt3rDECO" that opens a persistent overlay window
- Real-time analysis of selected objects
- Adjustable parameters: Density, Chaos, Symmetry, Detail
- Toggle switches: Edge Emphasis, Flow Following, Symmetry Mirroring, Anti-Clutter Mode
- Custom preset creation and JSON persistence

## Architecture

```
/src
├── ModMain.cpp              # Hooks + initialization
├── ui
│   ├── AIDecorateLayer.hpp  # UI layer header
│   └── AIDecorateLayer.cpp  # UI layer implementation
├── engine
│   ├── AIDecorationEngine.hpp  # Core engine header
│   ├── AIDecorationEngine.cpp  # Core engine implementation
│   ├── GeometryAnalyzer.hpp   # Geometry analysis header
│   ├── GeometryAnalyzer.cpp   # Geometry analysis implementation
│   ├── PatternDetector.hpp    # Pattern detection header
│   └── PatternDetector.cpp    # Pattern detection implementation
└── presets
    ├── AIPresetData.hpp       # Data structures
    ├── AIPresetData.cpp       # JSON serialization
    ├── AIPresetManager.hpp    # Preset management header
    └── AIPresetManager.cpp    # Preset management implementation
```

## AI Logic Pipeline

1. **Selection Analysis**: Read selected objects and cluster into structural groups
2. **Geometry Classification**: Run geometry classification and generate density map
3. **Pattern Detection**: Detect symmetry, repetition, flow, and density imbalances
4. **Preset Application**: Apply preset behavior model with user parameter overrides
5. **Zone Generation**: Decide decoration zones (edges, fills, accents, micro details)
6. **Decoration Generation**: Generate decoration objects with adaptive logic
7. **Validation**: Validate placements (no overlap, no invalid positions)
8. **Application**: Commit changes to editor with undo support

## Build Instructions

### Prerequisites
- Windows 10/11
- Visual Studio 2019 or later with C++ development tools
- CMake 3.15 or later
- Geode SDK 2.0.0 or later
- Geometry Dash 2.2081 (PC)

### Setup

1. **Clone or download this mod repository**

2. **Install Geode**:
   - Download Geode installer from https://geode-sdk.org/
   - Install Geode to your Geometry Dash directory

3. **Set up build environment**:
   ```bash
   # Navigate to the mod directory
   cd "c:/Users/Reid/Downloads/gd mod"
   
   # Create build directory
   mkdir build
   cd build
   ```

4. **Configure with CMake**:
   ```bash
   cmake .. -G "Visual Studio 16 2019" -A x64
   ```
   (Adjust generator for your Visual Studio version)

5. **Build the mod**:
   ```bash
   cmake --build . --config Release
   ```

6. **Install the mod**:
   - Copy the built `.geode` file from `build/Release/` to your Geode mods directory:
   ```
   %LOCALAPPDATA%\GeometryDash\geode\mods\
   ```

7. **Launch Geometry Dash**:
   - The mod will load automatically when you open the editor

## Usage

### Basic Usage

1. Open the Level Editor in Geometry Dash
2. Select the objects you want to decorate
3. Click the "Alt3rDECO" button in the toolbar
4. Select an AI preset from the dropdown
5. Adjust parameters as needed:
   - **Density**: Overall decoration density (0.0 - 1.0)
   - **Chaos**: Randomness and imperfection level (0.0 - 1.0)
   - **Symmetry**: Symmetry strength (0.0 - 1.0)
   - **Detail**: Micro decoration level (0.0 - 1.0)
6. Toggle options:
   - **Edge Emphasis**: Increase decoration near edges
   - **Flow Following**: Follow level progression direction
   - **Symmetry Mirroring**: Mirror decorations in symmetric regions
   - **Anti-Clutter**: Prevent over-decoration
7. Click "Analyze Selection" to preview geometry analysis
8. Click "Apply AI Decoration" to generate decorations

### Custom Presets

1. Configure your desired parameters in the UI
2. Click "Save AI Preset"
3. Enter a name for your preset
4. The preset will be saved as JSON and persist across sessions
5. Custom presets appear in the preset dropdown

### JSON Preset Format

Custom presets are saved as JSON files in:
```
%LOCALAPPDATA%\GeometryDash\geode\mods\Alt3rDECO\presets\
```

Example preset structure:
```json
{
    "id": "custom_my_preset",
    "name": "My Custom Preset",
    "description": "Description of my preset",
    "styleBias": "CleanModern",
    "densityCurve": {
        "baseDensity": 0.5,
        "edgeMultiplier": 1.5,
        "cornerMultiplier": 2.0,
        "centerMultiplier": 0.8,
        "flowMultiplier": 1.2
    },
    "symmetryPreference": 0.7,
    "chaosTolerance": 0.3,
    "clusteringThreshold": 0.5,
    "microDetailLevel": 0.4,
    "edgeEmphasis": true,
    "flowFollowing": true,
    "symmetryMirroring": true,
    "antiClutterMode": false,
    "decorationPool": [...],
    "styleWeights": {...}
}
```

## Technical Details

### Geometry Analysis

The system analyzes selected objects using:
- **Clustering**: Groups nearby objects into logical structures
- **Shape Detection**: Identifies linear, vertical, enclosed, and sparse structures
- **Density Mapping**: Computes object density across the selection
- **Visual Importance**: Assigns importance weights to edges, corners, and centers

### Pattern Recognition

- **Symmetry Detection**: Analyzes object positions to detect horizontal, vertical, radial, and bilateral symmetry
- **Repetition Detection**: Identifies repeated segments with consistent spacing
- **Flow Detection**: Determines the direction of level progression
- **Density Analysis**: Finds empty and dense zones for balanced decoration

### Decoration Generation

The system uses adaptive logic:
- **Edge Emphasis**: Increases decoration density near edges and silhouettes
- **Visual Hierarchy**: Assigns importance weights (primary geometry 100%, structural outlines 70%, fill 30-50%, micro detail 10-20%)
- **Style Adaptation**: Auto-adjusts based on detected geometry type
- **Imperfection Injection**: Adds slight variations for human-like results

### Safety Features

- Never crashes the editor
- Supports empty selection safely
- Does not modify original geometry
- Integrates with undo/redo system
- Batches object creation to prevent frame freezing
- Validates all placements before application

## Troubleshooting

### Mod not loading
- Ensure Geode is properly installed
- Check that the `.geode` file is in the correct mods directory
- Verify Geometry Dash version compatibility (2.2081)

### Decorations not appearing
- Ensure objects are selected in the editor
- Check that a preset is selected
- Try increasing the density parameter
- Verify that the selected area has sufficient space

### Performance issues
- Reduce the detail level
- Enable anti-clutter mode
- Process smaller selections at a time
- Close other applications to free resources

## Development

### Adding New Presets

1. Edit `src/presets/AIPresetManager.cpp`
2. Add a new preset creation method following the existing pattern
3. Call the method in `loadBuiltInPresets()`
4. Rebuild the mod

### Modifying Decoration Logic

- Core engine: `src/engine/AIDecorationEngine.cpp`
- Geometry analysis: `src/engine/GeometryAnalyzer.cpp`
- Pattern detection: `src/engine/PatternDetector.cpp`

### UI Customization

- UI layer: `src/ui/AIDecorateLayer.cpp`
- Modify button positions, slider ranges, and toggle options

## License

This mod is provided as-is for educational and personal use.

## Credits

- Developed for Geometry Dash 2.2081 (PC)
- Built with Geode SDK
- AI decoration engine with adaptive procedural rules
