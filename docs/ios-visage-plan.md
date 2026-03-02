# Plan: iOS Visage Skill/Plugin + Griddy-v2 Verification

## Context

Phases 1-3 of Visage iOS support are committed (`8f796ab`, `2d712f5`, `987ab56`). Visage compiles and links for iOS with Metal rendering and touch events. Phase 4 CMake linking is done — Visage is linked to `GriddyApp` but the app still uses plain JUCE UI.

**Goal**: Update the `juce-visage` skill and `juce-dev` plugin to support iOS/iPadOS, then use them to upgrade griddy-v2's iOS app to **100% Visage UI** (no JUCE UI components) as verification. Cherry-pick missing fork PRs, track all changes for upstream, add Visage-aligned tests.

**Matt Tytel's guidance** (VitalAudio/visage#65): "small and incremental" PRs. Our iOS work maps to ~5 focused upstream PRs.

**Branch coordination**: Use branch name `ios/visage-support` across all 3 repos:
- `~/Code/griddy-v2` (this project)
- `~/Code/visage` (Visage fork)
- `~/Code/generous-corp-marketplace` (skill/plugin source)

## Repos & File Locations

### Skill file (3 copies — must stay identical)
1. **Source (authoritative)**: `~/Code/generous-corp-marketplace/plugins/juce-dev/skills/juce-visage/SKILL.md`
2. **Source mirror**: `~/Code/generous-corp-marketplace/skills/juce-visage/SKILL.md`
3. **Installed**: `~/.claude/skills/juce-visage/SKILL.md`

### Plugin command (2 copies — must stay identical)
1. **Source**: `~/Code/generous-corp-marketplace/plugins/juce-dev/commands/setup-ios.md`
2. **Installed**: `~/.claude/plugins/cache/generous-corp-marketplace/juce-dev/1.0.0/commands/setup-ios.md`

## Phase A: Create Branches + Cherry-Pick Fork PRs

### A1: Create `ios/visage-support` branches

- `~/Code/griddy-v2` — create branch from current HEAD
- `~/Code/visage` — create branch from main
- `~/Code/generous-corp-marketplace` — create branch from main

### A2: Cherry-pick 3 missing PRs into griddy-v2 embedded Visage

| PR | File in `external/visage/` | Change |
|----|---------------------------|--------|
| `fix/instance-counter-no-crash-on-exit` | `visage_utils/defines.h` | VISAGE_ASSERT → VISAGE_LOG in InstanceCounter destructor |
| `fix/show-window-always-on-top-guard` | `visage_app/application_window.cpp` | Only call `setAlwaysOnTop()` when `always_on_top_` is true |
| `fix/single-line-text-editor-arrows` | `visage_widgets/text_editor.cpp` | Up/Down → Home/End in single-line fields |

**Skip**: `macOS-pinch-to-zoom-gestures` (macOS trackpad only)

Each as a separate commit with same message as the fork PR.

**Verify**: macOS build still passes. iOS build still passes.

## Phase B: Create New Visage UI Frames for iOS Transport

The iOS app currently has JUCE Play/Stop button, Record button, MIDI-only toggle, Tempo slider+editor. **All must become Visage frames — no JUCE UI components.**

### New frames to create in `Source/UI/`:

**`TransportButtonFrame.h`** — Toggle button with icon + label
- Pattern: Follows `ResetButtonFrame.h` (circle button with press state, glow)
- Properties: `label_` (text), `color_` (tint), `active_` (toggle state), `iconType_` (play/record/stop)
- Callbacks: `std::function<void()> onPress`
- Draw: Circle or rounded-rect button with play triangle / record circle / stop square icon
- Mouse: `mouseDown` sets pressed, `mouseUp` fires callback if still inside

**`TempoFrame.h`** — Horizontal tempo slider with numeric BPM display
- Pattern: Follows `DensitySliderFrame.h` (track + fill + thumb) but horizontal
- Properties: `value_` (float, mapped to BPM range), `minBpm_`, `maxBpm_`
- Display: Draw "120 BPM" text centered, with horizontal drag to change
- Callbacks: `std::function<void(float)> onValueChange`
- Mouse: Horizontal drag changes tempo, shows current value
- Note: No text editing needed — drag-only for mobile simplicity

**`ToggleFrame.h`** — Simple on/off toggle (for MIDI-only)
- Pattern: Follows `ResetButtonFrame.h` but with on/off state visuals
- Properties: `label_`, `active_`, `color_`
- Callbacks: `std::function<void(bool)> onToggle`
- Draw: Pill-shaped toggle switch with label

### iOS vs iPadOS layout considerations
- **iPhone**: Portrait-only, compact layout, 44pt+ touch targets
- **iPad**: Portrait + landscape support (iPadOS can do Split View), more screen space, can show more controls side-by-side
- **Both**: Safe area insets, no hover states (except Apple Pencil hover on iPad), DPI 2x/3x

### Add to CMakeLists.txt
New frames go in `target_sources` for both macOS plugin AND iOS app targets (they're platform-agnostic Visage code).

**Verify**: macOS plugin builds with new frames. New frames render correctly.

## Phase C: Fix JuceVisageBridge for iOS

**File**: `Source/Visage/JuceVisageBridge.h` and `.cpp`

1. **Modifier mapping** (line 287): `#if JUCE_MAC` → `#if JUCE_MAC || JUCE_IOS`

2. **Mouse event overrides**: Wrap with `#if !JUCE_IOS`:
   - `mouseDown`, `mouseUp`, `mouseDrag`, `mouseMove`, `mouseWheelMove`
   - Reason: VisageMetalView handles touch events natively on iOS. The Metal view sits atop the JUCE UIView, so UIKit routes touches to it first.

3. **Cursor style** lambda: Wrap body with `#if !JUCE_IOS` (no-op on iOS)

4. **Header**: Match `#if !JUCE_IOS` guards on mouse override declarations

**Verify**: Both macOS and iOS targets compile.

## Phase D: Replace GriddyApp JUCE UI with Full Visage

### D1: Update CMakeLists.txt iOS target sources

**Add** to GriddyApp `target_sources`:
```
Source/Visage/JuceVisageBridge.cpp
Source/Visage/JuceVisageBridge.h
Source/UI/XYPadFrame.h
Source/UI/LEDMatrixFrame.h
Source/UI/DensitySliderFrame.h
Source/UI/RotaryKnobFrame.h
Source/UI/ResetButtonFrame.h
Source/UI/TransportButtonFrame.h
Source/UI/TempoFrame.h
Source/UI/ToggleFrame.h
```

**Remove**: `App/XYPad.cpp`, `App/XYPad.h` (replaced by XYPadFrame)

### D2: Rewrite GriddyAppMainComponent.h

Replace all JUCE UI member declarations:

```cpp
#include "Visage/JuceVisageBridge.h"
#include "UI/XYPadFrame.h"
#include "UI/LEDMatrixFrame.h"
#include "UI/DensitySliderFrame.h"
#include "UI/RotaryKnobFrame.h"
#include "UI/ResetButtonFrame.h"
#include "UI/TransportButtonFrame.h"
#include "UI/TempoFrame.h"
#include "UI/ToggleFrame.h"

// Members:
std::unique_ptr<JuceVisageBridge> bridge_;
std::unique_ptr<visage::Frame> rootFrame_;

// Frame pointers (owned by rootFrame_ via addChild)
XYPadFrame* xyPad_ = nullptr;
LEDMatrixFrame* ledMatrix_ = nullptr;
DensitySliderFrame* bdDensity_ = nullptr;
DensitySliderFrame* sdDensity_ = nullptr;
DensitySliderFrame* hhDensity_ = nullptr;
RotaryKnobFrame* chaosKnob_ = nullptr;
RotaryKnobFrame* swingKnob_ = nullptr;
RotaryKnobFrame* bdVelKnob_ = nullptr;
RotaryKnobFrame* sdVelKnob_ = nullptr;
RotaryKnobFrame* hhVelKnob_ = nullptr;
ResetButtonFrame* resetButton_ = nullptr;
TransportButtonFrame* playButton_ = nullptr;
TransportButtonFrame* recordButton_ = nullptr;
TempoFrame* tempoControl_ = nullptr;
ToggleFrame* midiOnlyToggle_ = nullptr;

GriddyAppEngine engine_;
bool uiCreated_ = false;
```

Remove: all juce::Slider, juce::TextEditor, juce::TextButton, juce::ToggleButton, juce::Label, XYPad, LED arrays.

### D3: Rewrite GriddyAppMainComponent.cpp

Follow macOS `PluginEditor.cpp` pattern adapted for iOS:

**Constructor**: Minimal setup, start timer, defer audio setup
**`createVisageUI()`**:
  - Create rootFrame with dark background onDraw
  - Create all Visage frames with engine-based callbacks:
    ```cpp
    xyPad_->onValueChange = [this](float x, float y) {
        engine_.setX(x); engine_.setY(y);
    };
    playButton_->onPress = [this]() {
        bool play = !engine_.isPlaying();
        engine_.setPlaying(play);
        playButton_->setActive(play);
    };
    tempoControl_->onValueChange = [this](float bpm) {
        engine_.setTempo(bpm);
    };
    ```
  - Add all frames to rootFrame
  - Create bridge, add as visible child, set root frame, start timer

**`layoutChildren()`**: Responsive portrait layout:
  - Safe area insets applied first
  - XY pad: top ~35% of screen (large touch target)
  - Transport row: Play + Record + Tempo (44pt+ height)
  - Density sliders (3) + Velocity knobs (3): middle section
  - Chaos + Swing + Reset: row
  - LED matrix: bottom ~15%
  - iPad: wider layout, controls can go side-by-side

**`timerCallback()`**: Deferred creation + engine polling + frame updates
**`updateFromEngine()`**: Read engine atomics, update all frame values
**Destructor**: Proper cleanup order (stop timer → shutdown → remove children → reset)

**Verify**: iOS build passes. App launches. Full Visage rendering. Touch works.

## Phase E: Update juce-visage Skill

**Update all 3 copies** (source → mirror → installed):

### Skill changes:

1. **Scope** (line 10): "macOS and iOS/iPadOS" + shared bridge architecture note

2. **"When to Use"** triggers (lines 14-28): Add iOS entries

3. **New section: "iOS/iPadOS Integration"** (after ~line 525):
   - Architecture: `JUCE AudioAppComponent → JuceVisageBridge → ApplicationWindow → WindowIos → VisageMetalView`
   - Key: VisageMetalView handles touches natively — bridge skips mouse forwarding on iOS
   - Build system: `VISAGE_IOS=1` auto-defined, same `visage` link target
   - Bridge iOS simplification: `#if !JUCE_IOS` mouse guards
   - Modifier: `#if JUCE_MAC || JUCE_IOS` for Cmd key
   - DPI: `Desktop::getDisplays().getMainDisplay().scale`
   - Safe area: Query in `resized()`, apply to root frame
   - Touch: No hover, no right-click, 44pt+ targets
   - iPhone vs iPad: Compact vs regular layouts, Split View on iPad
   - No title bar, no cursor, no `performKeyEquivalent:` on iOS

4. **Update modifier section** (~line 576): Add `JUCE_IOS` guard documentation

5. **Update "Plugin-Specific Fixes"** (~line 702): Note macOS-only vs cross-platform

6. **Update "Common Mistakes"** (~line 1246): iOS mistakes (double events, safe area, hover)

7. **Update per-project template**: iOS-specific patch entries

8. **Update "Build System"** (~line 2284): iOS CMake pattern

### Sync all 3 locations:
```bash
# Source of truth
edit ~/Code/generous-corp-marketplace/plugins/juce-dev/skills/juce-visage/SKILL.md
# Copy to mirror
cp ~/Code/generous-corp-marketplace/plugins/juce-dev/skills/juce-visage/SKILL.md \
   ~/Code/generous-corp-marketplace/skills/juce-visage/SKILL.md
# Copy to installed
cp ~/Code/generous-corp-marketplace/plugins/juce-dev/skills/juce-visage/SKILL.md \
   ~/.claude/skills/juce-visage/SKILL.md
```

## Phase F: Add `setup-ios` Command to juce-dev Plugin

**Create in both locations**:
1. `~/Code/generous-corp-marketplace/plugins/juce-dev/commands/setup-ios.md`
2. `~/.claude/plugins/cache/generous-corp-marketplace/juce-dev/1.0.0/commands/setup-ios.md`

### Command structure (mirrors `setup-visage.md`):

```
Step 1: Verify Project
  - CMakeLists.txt, .env exist
  - Check if iOS target already exists in CMakeLists.txt

Step 2: Detect Visage Status
  - If external/visage/ exists AND USE_VISAGE_UI=TRUE → Visage mode
  - Otherwise → Plain JUCE mode

Step 3: Confirm with User
  - Show: "Add iOS app target to this project?"
  - Note: Will use Visage GPU UI / plain JUCE UI based on detection

Step 4: Create iOS App Files
  - Visage mode: App/ with JuceVisageBridge + Visage frame-based main component
  - Plain JUCE mode: App/ with AudioAppComponent + JUCE slider/button UI
  - Add iOS target to CMakeLists.txt (juce_add_gui_app with iOS config)
  - Add sample resources if Resources/Samples/ exists

Step 5: Summary
  - Build: cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS
  - Reference juce-visage skill iOS section (if Visage mode)
  - Note safe areas, touch targets, iOS/iPadOS differences
```

## Phase G: Visage-Aligned Tests

### Already created (windowing_tests.cpp):
- Window base class, DPI scale, isMobileDevice, computeWindowBounds, createWindow, coordinate conversion, clipboard, iOS max dimensions, createPluginWindow

### New tests to add:
- `TEST_CASE("iOS createWindow with scale parameter")` [ios] — verify scale from WindowIos constructor
- `TEST_CASE("iOS window show/hide cycle")` [ios] — show(), isShowing(), hide(), isShowing()
- `TEST_CASE("iOS window close removes view")` [ios] — close(), isShowing() false
- `TEST_CASE("iOS windowContentsResized updates frame")` [ios] — resize + verify

Follow Catch2 patterns from `visage_utils/tests/`, `visage_ui/tests/`.

## Phase H: Create Upstream PR Tracking Document

**File**: `docs/visage-ios-upstream-prs.md` (in griddy-v2)

Contents:
```markdown
# Visage iOS Upstream PR Plan

Per Matt Tytel (VitalAudio/visage#65): keep PRs small and incremental.

## Proposed PRs (from our iOS work)

### PR 1: CMake iOS platform detection
- Files: cmake/compile_flags.cmake
- Adds VISAGE_IOS=1 before APPLE check
- Tests: windowing_tests.cpp isMobileDevice test

### PR 2: Shared Apple code guards
- Files: renderer.cpp, windowless_context.h, application_editor.cpp,
         file_system.cpp, shader_editor.cpp
- Changes VISAGE_MAC → VISAGE_MAC || VISAGE_IOS where Apple APIs are shared

### PR 3: iOS CMake file routing
- Files: visage_windowing/CMakeLists.txt, visage_graphics/CMakeLists.txt,
         visage_ui/CMakeLists.txt
- Routes to ios/ sources, reuses emoji_macos.cpp, skips menu_bar.mm

### PR 4: iOS windowing implementation
- Files: visage_windowing/ios/windowing_ios.h, windowing_ios.mm,
         visage_graphics/ios/windowless_context.mm
- Full WindowIos with Metal rendering + touch-to-mouse mapping

### PR 5: iOS windowing tests
- Files: visage_windowing/tests/windowing_tests.cpp
- Catch2 tests for iOS window, DPI, dimensions, lifecycle

## Already submitted PRs (from fork)
- fix/instance-counter-no-crash-on-exit
- fix/macos-mtkview-60fps-cap
- fix/macos-plugin-keyboard-handling
- fix/popup-menu-overflow-position
- fix/show-window-always-on-top-guard
- fix/single-line-text-editor-arrows
- macOS-pinch-to-zoom-gestures
```

## Phase I: Migrate iOS Changes to Fork

After verification, copy to `~/Code/visage` on `ios/visage-support` branch:

1. Cherry-pick CMake platform detection changes
2. Cherry-pick shared Apple guard changes
3. Cherry-pick CMake file routing changes
4. Copy windowing_ios.h/mm + windowless_context.mm
5. Copy windowing_tests.cpp additions

Each can later become a separate PR branch when ready to submit upstream.

## Execution Order

```
A1: Create ios/visage-support branches in all 3 repos
A2: Cherry-pick 3 fork PRs (3 commits)
B:  Create TransportButtonFrame + TempoFrame + ToggleFrame (1 commit)
C:  Fix JuceVisageBridge for iOS (1 commit)
D:  Replace GriddyApp UI with full Visage (1-2 commits)
G:  Add windowing tests (1 commit)
    ↓ VERIFY: iOS simulator build + launch + touch + audio
E:  Update juce-visage skill (all 3 copies) (1 commit to marketplace)
F:  Add setup-ios command (both copies) (1 commit to marketplace)
H:  Create upstream PR tracking doc (1 commit to griddy-v2)
I:  Migrate iOS changes to fork (1 commit to visage fork)
```

## Verification Checklist

- [ ] macOS plugin builds and runs (`./scripts/build.sh standalone`)
- [ ] iOS cmake generates: `cmake -B build-ios -G Xcode -DCMAKE_SYSTEM_NAME=iOS`
- [ ] iOS build: `xcodebuild -scheme GriddyApp -sdk iphonesimulator`
- [ ] iOS app launches — Metal renders (not JUCE paint)
- [ ] XY pad: touch drag updates position
- [ ] Density sliders: vertical drag works
- [ ] Rotary knobs: drag works
- [ ] Transport: Play/Stop toggles, Record toggles
- [ ] Tempo: drag changes BPM
- [ ] LED matrix: patterns visible, step indicator advances
- [ ] Safe area: content respects notch/home indicator
- [ ] Cherry-picked fixes don't break any build
- [ ] All 3 skill copies are identical
- [ ] Both command copies are identical
- [ ] docs/visage-ios-upstream-prs.md lists all 5 potential PRs
- [ ] Windowing tests compile
- [ ] Fork branch has iOS changes
- [ ] Branches named `ios/visage-support` in all repos
