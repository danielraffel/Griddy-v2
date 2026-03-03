#include "SettingsMenuHelper.h"

#if JUCE_MAC
#import <Cocoa/Cocoa.h>

namespace SettingsMenuHelper {

void setSettingsKeyEquivalent() {
    // Find the "Settings..." item in the app menu and set its key equivalent to Cmd+,
    NSMenu* mainMenu = [NSApp mainMenu];
    if (!mainMenu || [mainMenu numberOfItems] == 0) return;

    NSMenu* appMenu = [[mainMenu itemAtIndex:0] submenu];
    if (!appMenu) return;

    for (NSInteger i = 0; i < [appMenu numberOfItems]; i++) {
        NSMenuItem* item = [appMenu itemAtIndex:i];
        if ([[item title] isEqualToString:@"Settings..."]) {
            [item setKeyEquivalent:@","];
            [item setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
            return;
        }
    }
}

} // namespace SettingsMenuHelper

#else
// No-op on non-Mac platforms
namespace SettingsMenuHelper {
    void setSettingsKeyEquivalent() {}
}
#endif
