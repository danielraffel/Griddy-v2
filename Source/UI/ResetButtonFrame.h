#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <functional>

class ResetButtonFrame : public visage::Frame {
public:
    ResetButtonFrame() : Frame("ResetButton") {}

    std::function<void()> onPress;

    void setGlow(bool glow) {
        if (glowing_ != glow) {
            glowing_ = glow;
            redraw();
        }
    }

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float size = std::min(w, h) - 4.0f;
        float x = (w - size) / 2.0f;
        float y = (h - size) / 2.0f;

        // Drop shadow
        canvas.setColor(0x40000000);
        canvas.circle(x + 1, y + 2, size);

        // Button face — flat filled circle
        if (pressed_) {
            canvas.setColor(0xff555555);
        } else if (glowing_) {
            canvas.setColor(0xffff8833);
        } else {
            canvas.setColor(0xff3a3a3a);
        }
        canvas.circle(x, y, size);

        // Top highlight for raised look (not when pressed)
        if (!pressed_) {
            canvas.setColor(0x15ffffff);
            float hlSize = size * 0.7f;
            canvas.circle(x + (size - hlSize) / 2.0f, y + 1, hlSize);
        }

        // Border ring
        unsigned int borderColor = pressed_ ? 0xffff8833 : (glowing_ ? 0xffffaa55 : 0xff555555);
        canvas.setColor(borderColor);
        float inset = 0.5f;
        canvas.roundedRectangleBorder(x + inset, y + inset,
                                      size - inset * 2, size - inset * 2,
                                      size / 2, 1.5f);
    }

    void mouseDown(const visage::MouseEvent&) override {
        pressed_ = true;
        glowing_ = true;
        redraw();
        if (onPress)
            onPress();
    }

    void mouseUp(const visage::MouseEvent&) override {
        pressed_ = false;
        glowing_ = false;
        redraw();
    }

private:
    bool pressed_ = false;
    bool glowing_ = false;
};
