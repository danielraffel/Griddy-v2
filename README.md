# Griddy

A topographic drum sequencer built with [JUCE](https://juce.com) and a GPU-accelerated [Visage](https://github.com/danielraffel/visage) fork.

## What is it?

Griddy generates drum patterns by sampling points on a topographic map. Drag an XY pad to explore the terrain — different positions produce different rhythmic patterns across multiple instrument channels. The patterns are generated using interpolation across a grid of pre-computed rhythmic maps (inspired by the [Mutable Instruments Grids](https://pichenettes.github.io/mutable-instruments-documentation/modules/grids/) algorithm).

## Download

- [Download the latest macOS installer (PKG)](https://github.com/danielraffel/Griddy/releases/download/v1.0.64/Griddy_1.0.64.pkg)
- [View all releases](https://github.com/danielraffel/Griddy/releases)

## Platforms

- **macOS** — Standalone app and AU/VST3 audio plugin (JUCE + Visage UI)
- **iOS** — Standalone app with full Visage GPU UI and native multi-touch support

## Building

Requires CMake and Xcode. JUCE is auto-downloaded on first build.

```bash
# Create your local build config
cp .env.example .env

# Generate the Xcode project
./scripts/generate_and_open_xcode.sh

# Local builds
./scripts/build.sh standalone
./scripts/build.sh all local

# iOS
cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS
```

For signed, notarized, or published macOS releases, fill in the Apple Developer and GitHub fields in `.env` first.

## Architecture

- **Engine**: `Source/Grids/` — 32-step pattern generator with bilinear interpolation
- **UI**: `Source/UI/` — Visage frames (XY pad, LED matrix, sliders, knobs)
- **Bridge**: `Source/Visage/` — Embeds Visage GPU rendering in JUCE plugin window
- **iOS App**: `App/` — Standalone iOS app with full Visage UI (no JUCE UI components)
- **Visage**: `external/visage/` — Fork with iOS touch support and multi-touch handling

## Features

- XY pad for topographic pattern exploration
- 3 instrument channels (BD, SD, HH) with density control
- Euclidean rhythm mode
- Pattern chaining (up to 4 patterns)
- Modulation matrix
- BPM control with tap tempo
- Multi-touch on iOS (independent finger tracking per control)

## Credits

Built with [JUCE](https://juce.com) and a forked [Visage](https://github.com/danielraffel/visage) rendering stack, with an iOS app that uses the fork's iOS touch-event support. Created from the [JUCE-Plugin-Starter](https://github.com/danielraffel/JUCE-Plugin-Starter) template.
