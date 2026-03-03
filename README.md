# Griddy v2

A topographic drum sequencer built with [JUCE](https://juce.com) and [Visage](https://github.com/VitalAudio/visage) GPU-accelerated UI.

## What is it?

Griddy generates drum patterns by sampling points on a topographic map. Drag an XY pad to explore the terrain — different positions produce different rhythmic patterns across multiple instrument channels. The patterns are generated using interpolation across a grid of pre-computed rhythmic maps (inspired by the [Mutable Instruments Grids](https://pichenettes.github.io/mutable-instruments-documentation/modules/grids/) algorithm).

## Platforms

- **macOS** — Standalone app and AU/VST3 audio plugin (JUCE + Visage UI)
- **iOS** — Standalone app with full Visage GPU UI and multi-touch support

## Building

Requires CMake, Xcode, and JUCE (auto-downloaded on first build).

```bash
# Generate Xcode project and build standalone app
./scripts/generate_and_open_xcode.sh
./scripts/build.sh standalone

# iOS
cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS
```

## Architecture

- **Engine**: `Source/Grids/` — 32-step pattern generator with bilinear interpolation
- **UI**: `Source/UI/` — Visage frames (XY pad, LED matrix, sliders, knobs)
- **Bridge**: `Source/Visage/` — Embeds Visage GPU rendering in JUCE plugin window
- **iOS App**: `App/` — Standalone iOS app with full Visage UI (no JUCE UI components)
- **Visage**: `external/visage/` — Fork with iOS support and multi-touch

## Features

- XY pad for topographic pattern exploration
- 3 instrument channels (BD, SD, HH) with density control
- Euclidean rhythm mode
- Pattern chaining (up to 4 patterns)
- Modulation matrix
- BPM control with tap tempo
- Multi-touch on iOS (independent finger tracking per control)

## Credits

Built with [JUCE](https://juce.com) and [Visage](https://github.com/VitalAudio/visage) by Matt Tytel. Created from the [JUCE-Plugin-Starter](https://github.com/danielraffel/JUCE-Plugin-Starter) template.
