# JUCE+Visage Integration Notes — JUCE Plugin Starter

<!-- This file is used by the juce-visage Claude skill. -->
<!-- It stores project-specific Visage integration details. -->
<!-- Fill in sections as you build out your Visage integration. -->

## Bridge Layer Files

| File | Purpose |
|------|---------|
| `Source/Visage/JuceVisageBridge.h/cpp` | Primary bridge: window, events, focus |
| `Source/PluginEditor.h/cpp` | Plugin editor with Visage UI creation |

## Visage Patches Applied

- [ ] `performKeyEquivalent:` (windowing_macos.mm) — Cmd+A/C/V/X/Z for TextEditor in plugins
- [ ] Cmd+Q propagation (windowing_macos.mm) — `[[self nextResponder] keyDown:event]`
- [ ] MTKView 60 FPS cap (windowing_macos.mm) — Prevent excessive GPU on ProMotion displays
- [ ] Popup menu overflow positioning (popup_menu.cpp) — Above-position fix
- [ ] Single-line Up/Down arrows (text_editor.cpp) — Up→start, Down→end
- [x] setAlwaysOnTop guard (application_window.cpp) — Only call when always_on_top_ is true
- [x] setDpiScale native_bounds recalculation (frame.h) — Fix child frames rendering at wrong size when DPI changes after setBounds

## Destruction Sequence

1. Stop timers (`stopTimer()`)
2. `bridge_->shutdownRendering()`
3. `rootFrame_->removeAllChildren()`
4. Null child pointers (`xyPad_ = nullptr`, `ledMatrix_ = nullptr`)
5. Disconnect bridge from frame tree (`bridge_->setRootFrame(nullptr)`)
6. Destroy root frame (`rootFrame_.reset()`)
7. Destroy bridge (`bridge_.reset()`)

## PopupMenu Instances

<!-- List all visage::PopupMenu usage locations -->

## Dropdown Instances

<!-- List all VisageDropdownComboBox usage locations -->

## Modal Dialog Instances

<!-- List all VisageModalDialog::show() and VisageOverlayBase usage locations -->

## JUCE AlertWindow Exceptions

<!-- List any places where JUCE native UI is used instead of Visage, with justification -->

## Known Technical Debt

<!-- Track Visage-related technical debt items -->

## Project-Specific Learnings

- **DPI scaling bug in `Frame::setDpiScale()`**: Visage's `Frame::setDpiScale()` did NOT recalculate `native_bounds_` when DPI changes. If child frames get `setBounds()` called before DPI is propagated (e.g., in `createVisageUI()` before the bridge creates the embedded window), they render at incorrect size. Fix: (1) Patch `setDpiScale` in `frame.h` to recalculate `native_bounds_` from `bounds_ * dpi_scale_` when the scale changes, (2) Always set child bounds from `resized()` / `layoutChildren()` not just once in `createVisageUI()`.
