# AI Decorator 3.0 for Geometry Dash

A Geode mod that converts a natural-language brief into a validated decoration plan and applies generated decorative objects inside the Geometry Dash editor.

## Version 3.0 implementation

- Replaces the old preset-driven Auto Deco workflow with a prompt-first AI interface.
- Reads the selected editor area and computes its bounds.
- Builds a runtime whitelist from the Geometry Dash ObjectToolbox. Only objects classified by the game as `GameObjectType::Decoration` are offered to the model.
- Supports three OpenAI-compatible providers (primary, backup, fallback). Rate limits, quota exhaustion, network errors, unavailable models, server errors, and malformed successful responses trigger failover.
- Requires structured JSON containing decoration layers and 1-120 object placements.
- Validates every object ID against the runtime whitelist; validates normalized coordinates, scale, rotation, color, array sizes, and response size.
- Shows a preview before any editor changes.
- Applies only after the user presses **Apply Decoration**. Generated objects are serialized and sent through Geometry Dash's native editor paste API.

## Provider setup

In the mod's Geode settings, configure endpoint, model, and API key for at least one provider. Configure backup/fallback providers to enable multi-step failover. Use HTTPS endpoints; HTTP is accepted only for localhost development.

API keys are stored in local Geode mod settings. Never commit keys or post them in public issues.

## Safety and current verification status

The plan is bounded to decorative object IDs discovered from the installed game's own object toolbox. It cannot request arbitrary triggers, portals, hazards, or gameplay objects. Placements are limited to the selected area's bounds and capped at 120 objects per generation. The user must review the plan and explicitly apply it.

The code uses Geometry Dash's native `EditorUI::pasteObjects` path so the editor can manage the generated batch like a paste action. **Undo behavior, placement offsets, color rendering, and actual in-game compatibility still require an SDK build and an in-game test.** This repository change has not yet been built against the user's local Geode SDK.

## Build

Requires Windows, CMake 3.21+, Visual Studio C++ tools, Geode SDK 2.0+, and Geometry Dash 2.2081. Set the `GEODE_SDK` environment variable to the Geode SDK directory.

```sh
cmake -S . -B build
cmake --build build --config Release
```
