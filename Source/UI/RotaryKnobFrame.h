#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <functional>
#include <string>
#include <cmath>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

class RotaryKnobFrame : public visage::Frame {
public:
    RotaryKnobFrame(const std::string& label, unsigned int color = 0xffff8833)
        : Frame("RotaryKnob"), label_(label), color_(color) {}

    float getValue() const { return value_; }

    void setValue(float v) {
        v = std::max(0.0f, std::min(1.0f, v));
        if (std::abs(value_ - v) > 0.001f) {
            value_ = v;
            redraw();
        }
    }

    std::function<void(float)> onValueChange;

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float labelH = 16.0f;

        // Text label
        visage::Font font(11.0f, visage::fonts::Lato_Regular_ttf);
        canvas.setColor(color_);
        canvas.text(label_.c_str(), font, visage::Font::kCenter, 0, 0, w, labelH);

        // Knob area
        float knobTop = labelH + 2.0f;
        float knobSize = std::min(w, h - knobTop) - 4.0f;
        float knobX = (w - knobSize) / 2.0f;
        float knobY = knobTop;
        float cx = knobX + knobSize / 2.0f;
        float cy = knobY + knobSize / 2.0f;

        // Arc angles (radians): sweep from ~225 deg to ~-45 deg (270 deg range)
        float startAngle = 2.356f;  // 135 deg from top (225 from right)
        float sweepRange = 4.712f;  // 270 deg total sweep
        float thickness = 3.0f;

        // Background arc
        canvas.setColor(0xff2a2a2a);
        canvas.roundedArc(knobX, knobY, knobSize, thickness,
                          startAngle + sweepRange / 2.0f, sweepRange);

        // Value arc
        float valueSweep = sweepRange * value_;
        if (valueSweep > 0.01f) {
            canvas.setColor(color_);
            canvas.roundedArc(knobX, knobY, knobSize, thickness,
                              startAngle + valueSweep / 2.0f, valueSweep);
        }

        // Center circle
        float innerSize = knobSize * 0.55f;
        float innerX = cx - innerSize / 2.0f;
        float innerY = cy - innerSize / 2.0f;
        canvas.setColor(0xff2a2a2a);
        canvas.circle(innerX, innerY, innerSize);

        // Indicator dot on the arc
        float indicatorAngle = startAngle + sweepRange * value_ - 3.14159f / 2.0f;
        float indicatorRadius = knobSize / 2.0f - thickness;
        float dotX = cx + indicatorRadius * std::cos(indicatorAngle);
        float dotY = cy + indicatorRadius * std::sin(indicatorAngle);
        float indSize = 4.0f;
        canvas.setColor(hovering_ ? 0xffffffff : 0xffdddddd);
        canvas.circle(dotX - indSize / 2.0f, dotY - indSize / 2.0f, indSize);
    }

    void mouseEnter(const visage::MouseEvent&) override { hovering_ = true; redraw(); }
    void mouseExit(const visage::MouseEvent&) override { hovering_ = false; redraw(); }

    void mouseDown(const visage::MouseEvent& e) override {
        dragStartY_ = e.position.y;
        dragStartValue_ = value_;
    }

    void mouseDrag(const visage::MouseEvent& e) override {
        float delta = (dragStartY_ - e.position.y) / 150.0f;
        float newVal = dragStartValue_ + delta;
        newVal = std::max(0.0f, std::min(1.0f, newVal));

        value_ = newVal;
        redraw();

        if (onValueChange)
            onValueChange(value_);
    }

private:
    std::string label_;
    unsigned int color_;
    float value_ = 0.5f;
    float dragStartY_ = 0.0f;
    float dragStartValue_ = 0.0f;
    bool hovering_ = false;
};
