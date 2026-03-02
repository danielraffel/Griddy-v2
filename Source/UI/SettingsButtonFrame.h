#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <functional>

class SettingsButtonFrame : public visage::Frame {
public:
    SettingsButtonFrame() : Frame("SettingsButton") {}

    std::function<void()> onPress;

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float dotR = 2.5f;
        float spacing = 5.0f;
        float cx = w / 2.0f;
        float cy = h / 2.0f;

        // Three dots (vertical)
        unsigned int color = hovering_ ? 0xffff8833 : 0xff888888;
        canvas.setColor(color);
        canvas.circle(cx - dotR, cy - spacing - dotR, dotR * 2);
        canvas.circle(cx - dotR, cy - dotR, dotR * 2);
        canvas.circle(cx - dotR, cy + spacing - dotR, dotR * 2);
    }

    void mouseEnter(const visage::MouseEvent&) override {
        hovering_ = true;
        redraw();
    }

    void mouseExit(const visage::MouseEvent&) override {
        hovering_ = false;
        redraw();
    }

    void mouseDown(const visage::MouseEvent&) override {
        if (onPress)
            onPress();
    }

private:
    bool hovering_ = false;
};
