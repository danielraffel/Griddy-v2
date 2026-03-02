# Visage iOS Upstream PR Plan

Per Matt Tytel (VitalAudio/visage#65): keep PRs "small and incremental."

Our iOS work maps to ~5 focused upstream PRs. Each can be submitted
independently once verified.

## Proposed PRs (from our iOS work)

### PR 1: CMake iOS platform detection
- **Files**: `cmake/compile_flags.cmake`
- **Change**: Add `VISAGE_IOS=1` before the `APPLE` check so iOS is
  detected separately from macOS
- **Why**: Foundation for all iOS support — other PRs depend on this
- **Tests**: `windowing_tests.cpp` isMobileDevice test (iOS=true)

### PR 2: Shared Apple code guards
- **Files**: `visage_graphics/renderer.cpp`,
  `visage_graphics/windowless_context.h`,
  `visage_app/application_editor.cpp`,
  `visage_utils/file_system.cpp`,
  `visage_ui/shader_editor.cpp`
- **Change**: `VISAGE_MAC` → `VISAGE_MAC || VISAGE_IOS` where Apple
  frameworks (Metal, CoreGraphics, Foundation) are shared
- **Why**: iOS uses the same Apple APIs as macOS for rendering and
  file system access

### PR 3: iOS CMake file routing
- **Files**: `visage_windowing/CMakeLists.txt`,
  `visage_graphics/CMakeLists.txt`,
  `visage_ui/CMakeLists.txt`
- **Change**: Route to `ios/` source directories, reuse
  `emoji_macos.cpp` (shared Apple emoji API), skip `menu_bar.mm`
  (no menu bar on iOS)
- **Why**: Proper build-system integration for iOS targets

### PR 4: iOS windowing implementation
- **Files**: `visage_windowing/ios/windowing_ios.h`,
  `visage_windowing/ios/windowing_ios.mm`,
  `visage_graphics/ios/windowless_context.mm`
- **Change**: Full `WindowIos` class with:
  - MTKView + CAMetalLayer rendering
  - Touch-to-mouse event mapping (single primary touch)
  - Factory functions (`createWindow`, `createPluginWindow`)
  - Global utilities (`isMobileDevice`, `defaultDpiScale`,
    `computeWindowBounds`, clipboard, message box)
- **Why**: Core runtime support for Visage on iOS/iPadOS

### PR 5: iOS windowing tests
- **Files**: `visage_windowing/tests/windowing_tests.cpp`
- **Change**: Catch2 test cases:
  - `isMobileDevice` platform detection
  - Window creation and max dimensions
  - Plugin window creation
  - Show/hide/close lifecycle
  - DPI scale from constructor
- **Why**: Validates iOS windowing layer matches platform contract

## Already submitted PRs (from fork)

These are independent bug fixes applicable to all platforms:

| Branch | Description | Status |
|--------|-------------|--------|
| `fix/instance-counter-no-crash-on-exit` | VISAGE_ASSERT → VISAGE_LOG in InstanceCounter destructor | Submitted |
| `fix/macos-mtkview-60fps-cap` | Cap MTKView to 60fps via preferredFramesPerSecond | Submitted |
| `fix/macos-plugin-keyboard-handling` | Add performKeyEquivalent: for plugin keyboard events | Submitted |
| `fix/popup-menu-overflow-position` | Fix popup menu positioning at screen edges | Submitted |
| `fix/show-window-always-on-top-guard` | Only call setAlwaysOnTop when explicitly enabled | Submitted |
| `fix/single-line-text-editor-arrows` | Up/Down → Home/End in single-line text fields | Submitted |
| `macOS-pinch-to-zoom-gestures` | Trackpad pinch-to-zoom gesture support | Submitted |

## Cherry-picked into griddy-v2 embedded Visage

These 3 fixes were cherry-picked from the fork into `external/visage/`:

1. **instance-counter-no-crash-on-exit** — Prevents host crash during
   shutdown when static destructors run before plugin cleanup
2. **show-window-always-on-top-guard** — Prevents plugin window from
   being demoted behind DAW when always_on_top_ is false
3. **single-line-text-editor-arrows** — Up/Down navigate to start/end
   in single-line fields (matches native behavior)

The other 3 fixes (60fps, keyboard, popup) were already present in the
embedded copy. `macOS-pinch-to-zoom-gestures` was skipped (macOS trackpad only).

## Files added/modified in Visage for iOS support

All changes are in `external/visage/` within griddy-v2. These map
directly to the 5 upstream PRs above:

### New files
- `visage_windowing/ios/windowing_ios.h`
- `visage_windowing/ios/windowing_ios.mm`
- `visage_graphics/ios/windowless_context.mm`

### Modified files
- `cmake/compile_flags.cmake` — iOS platform detection
- `visage_windowing/CMakeLists.txt` — iOS source routing
- `visage_graphics/CMakeLists.txt` — iOS source routing
- `visage_ui/CMakeLists.txt` — Skip menu_bar.mm on iOS
- `visage_graphics/renderer.cpp` — Shared Apple guards
- `visage_graphics/windowless_context.h` — Shared Apple guards
- `visage_app/application_editor.cpp` — Shared Apple guards
- `visage_utils/file_system.cpp` — Shared Apple guards
- `visage_windowing/tests/windowing_tests.cpp` — iOS test cases
