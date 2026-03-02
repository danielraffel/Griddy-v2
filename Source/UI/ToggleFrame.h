#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <functional>
#include <string>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

class ToggleFrame : public visage::Frame {
public:
    ToggleFrame(const std::string& label, unsigned int color = 0xff58a8d0)
        : Frame("Toggle"), label_(label), color_(color) {}

    bool isActive() const { return active_; }

    void setActive(bool active) {
        if (active_ != active) {
            active_ = active;
            redraw();
        }
    }

    std::function<void(bool)> onToggle;

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float labelH = 14.0f;

        // Label above
        visage::Font font(10.0f, visage::fonts::Lato_Regular_ttf);
        canvas.setColor(active_ ? color_ : 0xff999999);
        canvas.text(label_.c_str(), font, visage::Font::kCenter, 0, 0, w, labelH);

        // Toggle track (pill shape)
        float trackW = std::min(w - 4.0f, 44.0f);
        float trackH = std::min(h - labelH - 4.0f, 24.0f);
        float trackX = (w - trackW) / 2.0f;
        float trackY = labelH + 2.0f;
        float radius = trackH / 2.0f;

        // Track background
        canvas.setColor(active_ ? color_ : 0xff2a2a2a);
        canvas.roundedRectangle(trackX, trackY, trackW, trackH, radius);

        // Track border
        canvas.setColor(active_ ? color_ : 0xff444444);
        canvas.roundedRectangleBorder(trackX, trackY, trackW, trackH, radius, 1.0f);

        // Thumb circle
        float thumbSize = trackH - 4.0f;
        float thumbY = trackY + 2.0f;
        float thumbX = active_
            ? trackX + trackW - thumbSize - 2.0f
            : trackX + 2.0f;

        canvas.setColor(pressed_ ? 0xffdddddd : 0xffffffff);
        canvas.circle(thumbX, thumbY, thumbSize);
    }

    void mouseDown(const visage::MouseEvent&) override {
        pressed_ = true;
        redraw();
    }

    void mouseUp(const visage::MouseEvent&) override {
        if (pressed_) {
            pressed_ = false;
            active_ = !active_;
            redraw();
            if (onToggle)
                onToggle(active_);
        }
    }

private:
    std::string label_;
    unsigned int color_;
    bool active_ = false;
    bool pressed_ = false;
};
