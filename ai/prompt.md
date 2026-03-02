# Griddy v2 — Port Griddy to Visage UI via JUCE-Plugin-Starter

## What This Is

Port [Griddy](https://github.com/danielraffel/Griddy) (a topographic drum sequencer MIDI effect plugin based on Mutable Instruments Grids) to use the Visage GPU-accelerated UI framework. The new project should be created from the [JUCE-Plugin-Starter](https://github.com/danielraffel/JUCE-Plugin-Starter) template with Visage enabled. This will serve as both Griddy v2 and a reference implementation for JUCE+Visage plugin development.

## Available Resources

### Project Template
- **JUCE-Plugin-Starter** (`/Users/danielraffel/Code/JUCE-Plugin-Starter`): macOS JUCE plugin template with build automation, code signing, auto-versioning, and optional Visage integration. Run `./scripts/init_plugin_project.sh` and select Visage during setup. Or use the Claude Code command `/juce-dev:create "Griddy" --visage`.

### Visage Integration Guidance
- **`juce-visage` Claude Code skill** ([SKILL.md](https://github.com/danielraffel/generous-corp-marketplace/blob/master/skills/juce-visage/SKILL.md)): Production-tested patterns for Metal embedding, event bridging, DAW keyboard handling, destruction ordering, popup/modal/dropdown systems. Claude Code specific.
- **`visage_prompt.md`** ([visage-ui-cookbook](https://github.com/danielraffel/visage-ui-cookbook)): AI-agnostic Visage API reference with verified method signatures and JUCE-to-Visage migration tables. Works with any AI assistant.
- **`danielraffel/visage` fork** ([repo](https://github.com/danielraffel/visage)): Visage with macOS patches for plugin hosting. The starter template's `setup_visage.sh` clones this fork and applies patches automatically.

### Original Griddy Source
- **Griddy v1** (`/Users/danielraffel/Code/Griddy`): Full source for the existing plugin. Key directories:
  - `Source/Grids/` — Pattern engine (GridsEngine, EuclideanEngine, pattern data)
  - `Source/PluginProcessor.h/cpp` — Audio processor, MIDI generation, APVTS parameters
  - `Source/Visage/` — Current JUCE UI (XYPad, LEDMatrix, SettingsPanel, VisageStyle)
  - `App/` — Standalone app with audio synthesis, recording, onboarding
  - `eurorack/` — Original Mutable Instruments Grids algorithm data

## What to Port

### Core Engine (copy directly, minimal changes)
The pattern engine is UI-independent and should transfer as-is:
- `GridsEngine.h/cpp` — Pattern generation, interpolation, density, chaos, accents
- `GridsPatternData.h` — 25-node pre-computed patterns
- `EuclideanEngine.h/cpp` + `EuclideanTables.h` — Alternative Euclidean mode
- `PluginProcessor.h/cpp` — APVTS parameters, MIDI output, transport sync, reset quantization, MIDI learn

### UI (rewrite for Visage)
The current UI in `Source/Visage/` uses JUCE components styled to look like Visage. Griddy v2 should use actual Visage Frames:

| Component | Current (JUCE) | Griddy v2 (Visage) |
|-----------|----------------|---------------------|
| **XY Pad** | Custom JUCE Component with mouse drag | Visage Frame with `mouseDown`/`mouseDrag`, `draw(Canvas&)` for grid + crosshair |
| **LED Matrix** | Custom Component painting 32-step grid | Visage Frame drawing filled rectangles per step with accent colors |
| **Density Sliders** | JUCE Slider (vertical) | Visage Frame with drag handling, `canvas.fill()` for fill level |
| **Knobs** | JUCE Slider (rotary) | Visage Frame with `canvas.arc()` for rotary display |
| **Settings Panel** | JUCE overlay with tabs | Visage Frame overlay with tab switching |
| **Main Editor** | JUCE AudioProcessorEditor | Visage Frame tree embedded via Metal in JUCE NSView |

### Visual Style
- Dark background (0xff1e1e1e)
- Accent: Orange (0xfff6a000)
- Use `VISAGE_THEME_COLOR` macros for all colors
- LED indicators via `canvas.roundedRectangle()` with color states
- Subtle borders via `canvas.roundedRectangleBorder()`

### Feature Flags (from Griddy v1 .env)
- `ENABLE_VELOCITY_SYSTEM` — Per-voice velocity ranges
- `ENABLE_EUCLIDEAN_MODE` — Alternative rhythm algorithm
- `ENABLE_PATTERN_CHAIN` — Pattern chaining
- `ENABLE_MODULATION_MATRIX` — LFO routing

## Phased Plan

### Phase 1: Scaffold Project
1. Create new project from JUCE-Plugin-Starter with Visage enabled
2. Copy pattern engine files (`Source/Grids/`, `eurorack/`) unchanged
3. Copy `PluginProcessor.h/cpp` with APVTS parameters
4. Get a minimal Visage Frame rendering (solid background) embedded in the JUCE editor
5. Build and verify AU/VST3/Standalone load without crashes

### Phase 2: Core UI
1. XY Pad — 2D drag control with grid overlay and position indicator
2. LED Matrix — 32-step pattern display with per-voice rows and accent highlights
3. Wire UI to processor parameters (X, Y positions update patterns in real-time)
4. Verify MIDI output works in a DAW (Logic Pro)

### Phase 3: Controls
1. Density sliders (BD, SD, HH) — vertical drag controls
2. Rotary knobs (Chaos, Swing) — arc-based display
3. Reset button with quantization options
4. Settings panel overlay with tabs

### Phase 4: Polish
1. Theme system with `VISAGE_THEME_COLOR`/`VISAGE_THEME_VALUE` macros
2. Hover animations and visual feedback
3. MIDI learn overlay
4. Test in Logic Pro, Ableton, standalone

### Phase 5: Distribution
1. Code signing and notarization via `build.sh publish`
2. GitHub release
3. Landing page via GitHub Pages

### Phase 6 (Future): iOS
Visage doesn't currently support iOS — would require:
- Touch/gesture handling in a new `windowing_ios.mm` (or extending `windowing_macos.mm`)
- UIKit Metal view embedding instead of NSView
- AUv3 plugin format
- This is a separate effort and should come after the macOS plugin is stable

## Reference Implementation Goal

Once complete, Griddy v2 should serve as a clean, real-world example of a JUCE plugin using Visage for its UI. The pattern engine is complex enough to be non-trivial, while the UI is straightforward enough (XY pad, LED grid, sliders, knobs) to demonstrate core Visage patterns without overwhelming complexity.
