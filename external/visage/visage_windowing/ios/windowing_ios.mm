/* Copyright Vital Audio, LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#import <UIKit/UIKit.h>
#import <MetalKit/MetalKit.h>

#include "windowing_ios.h"

// =============================================================================
// Phase 1 stub: provides linkable symbols so the iOS target builds.
// Phase 2 will add real Metal rendering, Phase 3 adds touch events.
// =============================================================================

namespace visage {

  // ---------------------------------------------------------------------------
  // WindowIos — stub implementation
  // ---------------------------------------------------------------------------

  WindowIos::WindowIos(int width, int height, float scale)
      : Window(width, height) {
    setDpiScale(scale);
  }

  WindowIos::WindowIos(int width, int height, float scale, void* parent_handle)
      : Window(width, height) {
    setDpiScale(scale);
  }

  WindowIos::~WindowIos() = default;

  void WindowIos::runEventLoop() { }
  void* WindowIos::nativeHandle() const { return metal_view_; }
  void* WindowIos::initWindow() const { return metal_view_; }
  void WindowIos::windowContentsResized(int width, int height) { }
  void WindowIos::show() { }
  void WindowIos::showMaximized() { }
  void WindowIos::hide() { }
  void WindowIos::close() { }
  bool WindowIos::isShowing() const { return metal_view_ != nullptr; }
  void WindowIos::setWindowTitle(const std::string& title) { }

  IPoint WindowIos::maxWindowDimensions() const {
    CGRect screen = [[UIScreen mainScreen] bounds];
    float scale = [[UIScreen mainScreen] nativeScale];
    return { static_cast<int>(screen.size.width * scale),
             static_cast<int>(screen.size.height * scale) };
  }

  // ---------------------------------------------------------------------------
  // Factory functions
  // ---------------------------------------------------------------------------

  std::unique_ptr<Window> createWindow(const Dimension& x, const Dimension& y,
                                       const Dimension& width, const Dimension& height,
                                       Window::Decoration decoration_style) {
    float scale = defaultDpiScale();
    IBounds bounds = computeWindowBounds(x, y, width, height);
    return std::make_unique<WindowIos>(bounds.width(), bounds.height(), scale);
  }

  std::unique_ptr<Window> createPluginWindow(const Dimension& width, const Dimension& height,
                                             void* parent_handle) {
    float scale = defaultDpiScale();
    int w = width.resolve(0, scale);
    int h = height.resolve(0, scale);
    return std::make_unique<WindowIos>(w, h, scale, parent_handle);
  }

  // ---------------------------------------------------------------------------
  // Global utility functions
  // ---------------------------------------------------------------------------

  bool isMobileDevice() { return true; }

  float defaultDpiScale() {
    return static_cast<float>([[UIScreen mainScreen] nativeScale]);
  }

  IBounds computeWindowBounds(const Dimension& x, const Dimension& y,
                              const Dimension& width, const Dimension& height) {
    float scale = defaultDpiScale();
    CGRect screen = [[UIScreen mainScreen] bounds];
    int screen_width = static_cast<int>(screen.size.width * scale);
    int screen_height = static_cast<int>(screen.size.height * scale);

    int w = width.resolve(screen_width, scale);
    int h = height.resolve(screen_height, scale);
    int px = x.resolve(screen_width, scale);
    int py = y.resolve(screen_height, scale);
    return { px, py, px + w, py + h };
  }

  void setCursorStyle(MouseCursor style) { }
  void setCursorVisible(bool visible) { }

  Point cursorPosition() { return { 0.0f, 0.0f }; }
  void setCursorPosition(Point window_position) { }
  void setCursorScreenPosition(Point screen_position) { }

  void showMessageBox(std::string title, std::string message) { }

  std::string readClipboardText() {
    UIPasteboard* pb = [UIPasteboard generalPasteboard];
    return pb.string ? std::string([pb.string UTF8String]) : std::string();
  }

  void setClipboardText(const std::string& text) {
    [UIPasteboard generalPasteboard].string =
        [NSString stringWithUTF8String:text.c_str()];
  }

  void closeApplication() { }
}
