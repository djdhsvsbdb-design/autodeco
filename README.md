# Alt3rDECO

A procedural decoration assistant for Geometry Dash 2.2081 (Windows PC) using the Geode modding framework. Version 2.0 is under active development and is not a stable release.

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

## Version 2.0 status\n\nThe 2.0 branch is development-only. The object allow-list and built-in preset IDs have been curated against a Geometry Dash 2.2081 object catalogue. Final red/black/gray tinting, visual preview, undo/redo behavior, compilation, and runtime QA are not yet complete. Do not treat the branch as a finished release. See [ROADMAP-2.0.md](ROADMAP-2.0.md).\n\n## Build Instructions

### Requirements
- Geode SDK 5.11.2
- Geode CLI
- Geometry Dash 2.2081 (Windows target currently declared in `mod.json`)
- CMake 3.21+ and a supported C++23 toolchain

### Build
1. Install and configure the Geode SDK using the official Geode setup guide.
2. Set the `GEODE_SDK` environment variable to the installed SDK directory.
3. From the repository root, run:

   ```bash
   geode build
   ```

4. Check the build output for the packaged `.geode` artifact and test it in a clean Geometry Dash + Geode installation.

Do not distribute a build as a release until compilation, loading, editor selection, undo/redo, and preset persistence have been tested.

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
