#pragma once

#include <functional>

// Native iOS helper: adds a "Done" toolbar above the keyboard
// and a tap gesture on the background to dismiss.
// No-ops on non-iOS platforms.

void installKeyboardDoneButton(std::function<void()> onDone);
void removeKeyboardDoneButton();
