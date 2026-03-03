#import <UIKit/UIKit.h>
#import <objc/runtime.h>
#include "KeyboardDoneHelper.h"

// ---------- tiny helper to walk the view tree ----------
static UIView* findFirstResponder(UIView* view) {
    if ([view isFirstResponder]) return view;
    for (UIView* sub in view.subviews) {
        UIView* hit = findFirstResponder(sub);
        if (hit) return hit;
    }
    return nil;
}

static UIWindow* activeKeyWindow() {
    for (UIScene* scene in [UIApplication sharedApplication].connectedScenes) {
        if ([scene isKindOfClass:[UIWindowScene class]]) {
            for (UIWindow* w in ((UIWindowScene*)scene).windows) {
                if (w.isKeyWindow) return w;
            }
        }
    }
    return nil;
}

// ---------- handler for Done button + gestures ----------
@interface _KBDoneHandler : NSObject
@property (nonatomic, copy) void (^onDone)(void);
- (void)doneTapped:(id)sender;
- (void)bgTapped:(UITapGestureRecognizer*)tap;
- (void)swipedDown:(UIPanGestureRecognizer*)pan;
@end

@implementation _KBDoneHandler
- (void)doneTapped:(id)sender { if (_onDone) _onDone(); }
- (void)bgTapped:(UITapGestureRecognizer*)tap { if (_onDone) _onDone(); }
- (void)swipedDown:(UIPanGestureRecognizer*)pan {
    if (pan.state == UIGestureRecognizerStateChanged) {
        CGPoint translation = [pan translationInView:pan.view];
        if (translation.y > 40) { // 40pt downward swipe threshold
            if (_onDone) _onDone();
            pan.enabled = NO;  // prevent firing again
            pan.enabled = YES;
        }
    }
}
@end

static _KBDoneHandler* sHandler = nil;
static UITapGestureRecognizer* sTapGesture = nil;
static UIPanGestureRecognizer* sSwipeGesture = nil;

// ---------- public API ----------

void installKeyboardDoneButton(std::function<void()> onDone) {
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), [onDone]{ installKeyboardDoneButton(onDone); });
        return;
    }

    removeKeyboardDoneButton();

    UIWindow* win = activeKeyWindow();
    if (!win) return;

    UIView* responder = findFirstResponder(win);
    if (!responder) return;

    sHandler = [[_KBDoneHandler alloc] init];
    sHandler.onDone = ^{ onDone(); };

    // ---- toolbar above keyboard ----
    // JUCE uses a custom JuceTextView (UIView <UITextInput>), not UITextField.
    // We use the inputAccessoryView property via KVC since any UIResponder supports it.
    UIToolbar* toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, 320, 44)];
    toolbar.barStyle = UIBarStyleDefault;

    UIBarButtonItem* flex = [[UIBarButtonItem alloc]
        initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
        target:nil action:nil];
    UIBarButtonItem* done = [[UIBarButtonItem alloc]
        initWithBarButtonSystemItem:UIBarButtonSystemItemDone
        target:sHandler action:@selector(doneTapped:)];
    toolbar.items = @[flex, done];

    // Set inputAccessoryView on whatever view is the first responder.
    // JUCE's JuceTextView conforms to UITextInput and supports this property.
    if ([responder respondsToSelector:@selector(setInputAccessoryView:)]) {
        [responder performSelector:@selector(setInputAccessoryView:) withObject:toolbar];
        [responder reloadInputViews];
    }

    // ---- tap gesture on main view to dismiss ----
    sTapGesture = [[UITapGestureRecognizer alloc]
        initWithTarget:sHandler action:@selector(bgTapped:)];
    sTapGesture.cancelsTouchesInView = NO;
    [win addGestureRecognizer:sTapGesture];

    // ---- swipe-down gesture to dismiss (like a sheet) ----
    sSwipeGesture = [[UIPanGestureRecognizer alloc]
        initWithTarget:sHandler action:@selector(swipedDown:)];
    sSwipeGesture.cancelsTouchesInView = NO;
    [win addGestureRecognizer:sSwipeGesture];
}

void removeKeyboardDoneButton() {
    if (sTapGesture) {
        [sTapGesture.view removeGestureRecognizer:sTapGesture];
        sTapGesture = nil;
    }
    if (sSwipeGesture) {
        [sSwipeGesture.view removeGestureRecognizer:sSwipeGesture];
        sSwipeGesture = nil;
    }
    sHandler = nil;
}
