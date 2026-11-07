# Changelog

## 2.0.0 — development branch (not a stable release)

- Bumped project and mod metadata to 2.0.0.
- Added a curated Geometry Dash 2.2081 decorative object allow-list.
- Replaced legacy built-in preset IDs that included portals, placeholder IDs, or gameplay-like objects.
- Added the Modern Redline preset as the first built-in preset, with no glow entries.
- Removed invalid weighted-selection fallbacks that could silently insert unrelated objects.
- Added the Alt3rDECO 2.0 release plan covering 15 workstreams.

### Release blockers

- The Windows build has not yet been compiled in this environment.
- Final red/black/gray tint application still needs verified color API implementation and in-game checks.
- A visual preview/confirm workflow is still being completed.
- Undo/redo and partial-apply rollback require in-game verification.
- Large-selection performance and selection-isolation tests remain outstanding.
- No packaged Geode artifact exists yet; binary scanning is therefore pending.

## 1.1.0 — development

- Renamed the mod display name to Alt3rDECO.
- Updated the project to the Geode SDK CMake packaging flow.
- Updated metadata for Geode 5.11.2 and Geometry Dash 2.2081 (Windows).
- Fixed duplicate overlay-hide method definition.
- Made zero and low decoration density behave correctly.
- Added a per-apply placement limit.
- Removed the hard-coded fallback object ID.
- Improved preset file loading and restricted preset IDs to safe filename characters.
- Kept the existing mod ID to preserve mod identity and saved data.
