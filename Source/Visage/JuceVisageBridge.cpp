#include "JuceVisageBridge.h"
#include <visage_utils/dimension.h>

JuceVisageBridge::JuceVisageBridge() {
    setOpaque(true);
    setWantsKeyboardFocus(false);
    setInterceptsMouseClicks(true, true);
    setMouseClickGrabsKeyboardFocus(false);

    eventHandler_.request_keyboard_focus = [this](visage::Frame* child) {
        setFocusedChild(child);
    };
    eventHandler_.read_clipboard_text = []() -> std::string {
        return juce::SystemClipboard::getTextFromClipboard().toStdString();
    };
    eventHandler_.set_clipboard_text = [](const std::string& text) {
        juce::SystemClipboard::copyTextToClipboard(juce::String(text));
    };
    eventHandler_.set_cursor_style = [this](visage::MouseCursor cursor) {
        switch (cursor) {
            case visage::MouseCursor::Arrow:
                setMouseCursor(juce::MouseCursor::NormalCursor); break;
            case visage::MouseCursor::IBeam:
                setMouseCursor(juce::MouseCursor::IBeamCursor); break;
            case visage::MouseCursor::Pointing:
                setMouseCursor(juce::MouseCursor::PointingHandCursor); break;
            case visage::MouseCursor::Crosshair:
                setMouseCursor(juce::MouseCursor::CrosshairCursor); break;
            case visage::MouseCursor::Dragging:
                setMouseCursor(juce::MouseCursor::DraggingHandCursor); break;
            case visage::MouseCursor::HorizontalResize:
                setMouseCursor(juce::MouseCursor::LeftRightResizeCursor); break;
            case visage::MouseCursor::VerticalResize:
                setMouseCursor(juce::MouseCursor::UpDownResizeCursor); break;
            default:
                setMouseCursor(juce::MouseCursor::NormalCursor); break;
        }
    };
    eventHandler_.request_redraw = [this](visage::Frame*) {
        repaint();
    };
}

JuceVisageBridge::~JuceVisageBridge() {
    stopTimer();
    shutdownRendering();
}

void JuceVisageBridge::setRootFrame(visage::Frame* frame) {
    rootFrame_ = frame;
    if (rootFrame_)
        rootFrame_->setEventHandler(&eventHandler_);
}

void JuceVisageBridge::createEmbeddedWindow() {
    if (windowCreated_ || !isShowing() || !getPeer())
        return;

    auto* peer = getPeer();
    void* parentHandle = peer->getNativeHandle();
    auto bounds = getLocalBounds();
    if (bounds.getWidth() <= 0 || bounds.getHeight() <= 0)
        return;

    visageWindow_ = std::make_unique<visage::ApplicationWindow>();

    // Set DPI scale from JUCE display info before showing
    float scale = juce::Desktop::getInstance().getDisplays()
                      .getDisplayForPoint(getScreenPosition())->scale;
    visageWindow_->setDpiScale(scale);

    int w = bounds.getWidth();
    int h = bounds.getHeight();
    visageWindow_->show(
        visage::Dimension::logicalPixels(static_cast<float>(w)),
        visage::Dimension::logicalPixels(static_cast<float>(h)),
        parentHandle
    );
    visageWindow_->setBounds(0, 0, static_cast<float>(w), static_cast<float>(h));

    if (rootFrame_) {
        rootFrame_->init();
        visageWindow_->addChild(rootFrame_);
        rootFrame_->setBounds(0, 0, static_cast<float>(w), static_cast<float>(h));
    }

    visageWindow_->drawWindow();
    windowCreated_ = true;
}

void JuceVisageBridge::shutdownRendering() {
    if (rootFrame_ && visageWindow_) {
        visageWindow_->removeChild(rootFrame_);
    }
    visageWindow_.reset();
    windowCreated_ = false;
}

void JuceVisageBridge::paint(juce::Graphics& g) {
    // Dark background to prevent pink/magenta flash before Metal renders
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void JuceVisageBridge::resized() {
    if (windowCreated_ && visageWindow_ && rootFrame_) {
        auto bounds = getLocalBounds();
        if (bounds.isEmpty()) return;
        float w = static_cast<float>(bounds.getWidth());
        float h = static_cast<float>(bounds.getHeight());
        // macOS: pass logical pixels; Visage handles DPI scaling internally
        visageWindow_->setWindowDimensions(w, h);
        rootFrame_->setBounds(0, 0, w, h);
    }
}

void JuceVisageBridge::timerCallback() {
    if (!windowCreated_ && isShowing() && getPeer() && getWidth() > 0) {
        createEmbeddedWindow();
    }

    // Continuously trigger Visage re-rendering so dirty frames get flushed to Metal
    if (windowCreated_ && visageWindow_) {
        visageWindow_->drawWindow();
    }
}

// --- Mouse Events ---

visage::MouseEvent JuceVisageBridge::convertMouseEvent(const juce::MouseEvent& e) const {
    visage::MouseEvent ve;
    ve.position = { static_cast<float>(e.x), static_cast<float>(e.y) };
    ve.window_position = ve.position;
    ve.modifiers = convertModifiers(e.mods);

    if (e.mods.isLeftButtonDown())
        ve.button_state |= visage::kMouseButtonLeft;
    if (e.mods.isRightButtonDown())
        ve.button_state |= visage::kMouseButtonRight;
    if (e.mods.isMiddleButtonDown())
        ve.button_state |= visage::kMouseButtonMiddle;

    ve.button_id = visage::kMouseButtonLeft;
    ve.repeat_click_count = e.getNumberOfClicks();
    return ve;
}

void JuceVisageBridge::mouseDown(const juce::MouseEvent& e) {
    if (!rootFrame_) return;
    auto ve = convertMouseEvent(e);

    // Find the deepest child frame under the mouse cursor
    visage::Frame* target = rootFrame_->frameAtPoint({ ve.position.x, ve.position.y });
    if (!target) target = rootFrame_;

    mouseDownFrame_ = target;

    // Convert position to target's local coordinates
    visage::MouseEvent localEvent = ve;
    visage::Frame* f = target;
    while (f && f != rootFrame_) {
        localEvent.position.x -= f->x();
        localEvent.position.y -= f->y();
        f = f->parent();
    }

    target->mouseDown(localEvent);
}

void JuceVisageBridge::mouseUp(const juce::MouseEvent& e) {
    if (!mouseDownFrame_) return;
    auto ve = convertMouseEvent(e);

    // Convert to target's local coordinates
    visage::MouseEvent localEvent = ve;
    visage::Frame* f = mouseDownFrame_;
    while (f && f != rootFrame_) {
        localEvent.position.x -= f->x();
        localEvent.position.y -= f->y();
        f = f->parent();
    }

    mouseDownFrame_->mouseUp(localEvent);
    mouseDownFrame_ = nullptr;
}

void JuceVisageBridge::mouseDrag(const juce::MouseEvent& e) {
    if (!mouseDownFrame_) return;
    auto ve = convertMouseEvent(e);

    // Convert to target's local coordinates
    visage::MouseEvent localEvent = ve;
    visage::Frame* f = mouseDownFrame_;
    while (f && f != rootFrame_) {
        localEvent.position.x -= f->x();
        localEvent.position.y -= f->y();
        f = f->parent();
    }

    mouseDownFrame_->mouseDrag(localEvent);
}

void JuceVisageBridge::mouseMove(const juce::MouseEvent& e) {
    if (!rootFrame_) return;
    auto ve = convertMouseEvent(e);

    // Find the frame under cursor for enter/exit tracking
    visage::Frame* target = rootFrame_->frameAtPoint({ ve.position.x, ve.position.y });

    // Handle mouseEnter/mouseExit
    if (target != hoverFrame_) {
        if (hoverFrame_) {
            visage::MouseEvent exitEvent = ve;
            hoverFrame_->mouseExit(exitEvent);
        }
        hoverFrame_ = target;
        if (hoverFrame_ && hoverFrame_ != rootFrame_) {
            visage::MouseEvent enterEvent = ve;
            visage::Frame* f = hoverFrame_;
            while (f && f != rootFrame_) {
                enterEvent.position.x -= f->x();
                enterEvent.position.y -= f->y();
                f = f->parent();
            }
            hoverFrame_->mouseEnter(enterEvent);
        }
    }

    // Dispatch mouseMove to the frame under cursor
    if (target && target != rootFrame_) {
        visage::MouseEvent localEvent = ve;
        visage::Frame* f = target;
        while (f && f != rootFrame_) {
            localEvent.position.x -= f->x();
            localEvent.position.y -= f->y();
            f = f->parent();
        }
        target->mouseMove(localEvent);
    }
}

void JuceVisageBridge::mouseWheelMove(const juce::MouseEvent& e,
                                       const juce::MouseWheelDetails& wheel) {
    if (!rootFrame_) return;
    auto ve = convertMouseEvent(e);
    ve.wheel_delta_x = wheel.deltaX;
    ve.wheel_delta_y = wheel.deltaY;
    ve.precise_wheel_delta_x = wheel.deltaX;
    ve.precise_wheel_delta_y = wheel.deltaY;
    ve.wheel_momentum = wheel.isInertial;
    rootFrame_->mouseWheel(ve);
}

// --- Key Events ---

visage::KeyEvent JuceVisageBridge::convertKeyEvent(const juce::KeyPress& key) const {
    int keyCode = key.getKeyCode();
    int mods = convertModifiers(key.getModifiers());
    bool hasModifier = key.getModifiers().isCommandDown() ||
                       key.getModifiers().isCtrlDown();

    visage::KeyCode vk = visage::KeyCode::Unknown;

    if (hasModifier) {
        switch (keyCode) {
            case 'A': vk = visage::KeyCode::A; break;
            case 'C': vk = visage::KeyCode::C; break;
            case 'V': vk = visage::KeyCode::V; break;
            case 'X': vk = visage::KeyCode::X; break;
            case 'Z': vk = visage::KeyCode::Z; break;
            default: vk = static_cast<visage::KeyCode>(keyCode); break;
        }
    } else {
        auto ch = key.getTextCharacter();
        if (ch > 0 && ch < 127)
            vk = static_cast<visage::KeyCode>(ch);
        else
            vk = static_cast<visage::KeyCode>(keyCode);
    }

    return visage::KeyEvent(vk, mods, true);
}

int JuceVisageBridge::convertModifiers(const juce::ModifierKeys& mods) const {
    int result = 0;
    if (mods.isShiftDown())   result |= visage::kModifierShift;
    if (mods.isAltDown())     result |= visage::kModifierAlt;
#if JUCE_MAC
    if (mods.isCommandDown()) result |= visage::kModifierCmd;
    if (mods.isCtrlDown())    result |= visage::kModifierMacCtrl;
#else
    if (mods.isCtrlDown())    result |= visage::kModifierRegCtrl;
#endif
    return result;
}

bool JuceVisageBridge::keyPressed(const juce::KeyPress& key) {
    if (focusedChild_) {
        auto ve = convertKeyEvent(key);
        if (focusedChild_->keyPress(ve))
            return true;
    }
    if (rootFrame_) {
        auto ve = convertKeyEvent(key);
        return rootFrame_->keyPress(ve);
    }
    return false;
}

void JuceVisageBridge::setFocusedChild(visage::Frame* child) {
    if (child) {
        setWantsKeyboardFocus(true);
        grabKeyboardFocus();
    } else {
        setWantsKeyboardFocus(false);
        giveAwayKeyboardFocus();
    }
    focusedChild_ = child;
}
