#pragma once

#include <string>

// Sets the Cmd+, key equivalent on the "Settings..." menu item
// (after it has been added by JUCE's setMacMainMenu).
// Call from the PluginEditor after setting up the MenuBarModel.

namespace SettingsMenuHelper {
    void setSettingsKeyEquivalent();
}
