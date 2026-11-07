# Alt3rDECO 2.0 — Release Plan

Target: Windows, Geometry Dash 2.2081, Geode 5.11.2. Android is out of scope.

## 15 release workstreams

1. **Verified decoration catalogue** — use documented 2.2081 object IDs and keep the source list auditable.
2. **Gameplay-object exclusion** — strict allow-list; no orbs, portals, pads, triggers, or hazards can enter a generated pool.
3. **Modern Redline preset** — clean geometric style, no glow objects; final red/black/gray tint application still requires verified color API testing.
4. **Preview before commit** — prepare a placement plan without modifying the level, then apply only after confirmation.
5. **Undo/redo correctness** — verify editor undo integration and add rollback on partial application failures.
6. **Selection isolation** — generated objects must stay inside the selected geometry's bounds and never edit source objects.
7. **Contour-aware placement** — improve edge and corner placement using measured geometry.
8. **Spacing and overlap controls** — configurable minimum distance and scale-aware collision spacing.
9. **Independent density controls** — separate edge, fill, accent, and micro-detail density.
10. **Symmetry improvements** — stable horizontal/vertical mirroring with duplicate suppression.
11. **Variation seeds** — reproducible generation and multiple alternate layouts.
12. **Preset management** — safe import/export, validation, and versioned preset format.
13. **Performance safeguards** — object-count limits, bounded loops, and large-selection protection.
14. **Input validation** — clamp non-finite values, reject malformed IDs/weights, and safely handle empty pools.
15. **Diagnostics and release QA** — generation report, build checks, in-game test matrix, and packaged-file malware scanning.

## Current implementation status

- Implemented in the v2 branch: version metadata bump, curated 2.2081 decorative ID allow-list, replacement of legacy placeholder/gameplay IDs in built-in preset pools, and invalid weighted-pool fallback removal.
- Modern Redline is the first preset in the list and excludes glow entries.
- Not yet verified: compilation, actual rendering/tint colors, in-game preview UX, undo/redo, selection behavior under complex editor states, and packaged artifact scanning.
- Do not publish a stable 2.0 release until all checks above pass.
