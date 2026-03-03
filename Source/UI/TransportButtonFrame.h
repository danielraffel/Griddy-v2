#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <functional>
#include <string>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

class TransportButtonFrame : public visage::Frame {
public:
    enum IconType { Play, Stop, Record };

    TransportButtonFrame(const std::string& label, IconType icon,
                         unsigned int color = 0xffff8833)
        : Frame("TransportButton"), label_(label), icon_(icon), color_(color) {}

    std::function<void()> onPress;
    std::function<void()> onLongPress;  // Fires during hold after 1.5s

    bool isActive() const { return active_; }

    void setActive(bool active) {
        if (active_ != active) {
            active_ = active;
            redraw();
        }
    }

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float labelH = 14.0f;
        float btnTop = 0.0f;
        float btnH = h - labelH - 2.0f;
        float btnSize = std::min(w - 4.0f, btnH);
        float btnX = (w - btnSize) / 2.0f;

        // Button background — rounded rectangle for larger touch target
        unsigned int bgColor = pressed_ ? 0xff555555
                             : active_  ? dimColor(color_, 0.3f)
                                        : 0xff2a2a2a;
        canvas.setColor(bgColor);
        canvas.roundedRectangle(btnX, btnTop, btnSize, btnSize, 8.0f);

        // Border
        unsigned int borderCol = active_ ? color_ : 0xff444444;
        canvas.setColor(borderCol);
        canvas.roundedRectangleBorder(btnX, btnTop, btnSize, btnSize, 8.0f, 1.5f);

        // Icon
        float iconSize = btnSize * 0.35f;
        float cx = btnX + btnSize / 2.0f;
        float cy = btnTop + btnSize / 2.0f;

        unsigned int iconColor = active_ ? color_ : 0xffcccccc;
        canvas.setColor(iconColor);

        switch (icon_) {
            case Play:
                if (active_) {
                    // Stop square when playing
                    float sq = iconSize * 0.8f;
                    canvas.roundedRectangle(cx - sq / 2, cy - sq / 2, sq, sq, 2.0f);
                } else {
                    // Play triangle
                    drawPlayTriangle(canvas, cx, cy, iconSize);
                }
                break;
            case Record:
                // Filled circle
                canvas.circle(cx - iconSize / 2, cy - iconSize / 2, iconSize);
                break;
            case Stop:
                // Square
                canvas.roundedRectangle(cx - iconSize / 2, cy - iconSize / 2,
                                        iconSize, iconSize, 2.0f);
                break;
        }

        // Label below button
        visage::Font font(10.0f, visage::fonts::Lato_Regular_ttf);
        canvas.setColor(active_ ? color_ : 0xff999999);
        canvas.text(label_.c_str(), font, visage::Font::kCenter,
                    0, btnSize + 2.0f, w, labelH);
    }

    void mouseDown(const visage::MouseEvent&) override {
        pressed_ = true;
        longPressFired_ = false;
        longPressGeneration_++;
        redraw();

        // Start a background timer to detect long-press (1.5s hold)
        if (onLongPress) {
            int gen = longPressGeneration_.load();
            std::thread([this, gen]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(1500));
                if (pressed_ && longPressGeneration_.load() == gen) {
                    longPressFired_ = true;
                    if (onLongPress)
                        onLongPress();
                }
            }).detach();
        }
    }

    void mouseUp(const visage::MouseEvent&) override {
        if (pressed_) {
            pressed_ = false;
            redraw();
            // Only fire normal press if long-press didn't fire
            if (!longPressFired_ && onPress)
                onPress();
        }
    }

private:
    void drawPlayTriangle(visage::Canvas& canvas, float cx, float cy, float size) {
        // Right-pointing play triangle using Canvas triangle API
        // Slight rightward offset (standard play icon is visually centered that way)
        float halfH = size * 0.55f;
        float halfW = size * 0.5f;
        float left = cx - halfW * 0.6f;
        float right = cx + halfW * 0.8f;
        canvas.triangle(left, cy - halfH,   // top-left
                         left, cy + halfH,   // bottom-left
                         right, cy);          // right tip
    }

    static unsigned int dimColor(unsigned int color, float amount) {
        int r = (color >> 16) & 0xff;
        int g = (color >> 8) & 0xff;
        int b = color & 0xff;
        r = (int)(r * amount);
        g = (int)(g * amount);
        b = (int)(b * amount);
        return 0xff000000 | (r << 16) | (g << 8) | b;
    }

    std::string label_;
    IconType icon_;
    unsigned int color_;
    bool active_ = false;
    bool pressed_ = false;
    bool longPressFired_ = false;
    std::atomic<int> longPressGeneration_ { 0 };
};
