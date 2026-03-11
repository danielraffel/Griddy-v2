#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <functional>
#include <string>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

class DensitySliderFrame : public visage::Frame {
public:
    DensitySliderFrame(const std::string& label, unsigned int color)
        : Frame("DensitySlider"), label_(label), color_(color) {}

    float getValue() const { return value_; }

    void setValue(float v) {
        v = std::max(0.0f, std::min(1.0f, v));
        if (std::abs(value_ - v) > 0.001f) {
            value_ = v;
            redraw();
        }
    }

    std::function<void(float)> onValueChange;
    std::function<void()> onGestureStart;
    std::function<void()> onGestureEnd;

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        float trackW = 8.0f;
        float trackX = (w - trackW) / 2.0f;
        float labelH = 16.0f;
        float trackTop = labelH + 4.0f;
        float trackH = h - trackTop - 8.0f;

        // Text label
        visage::Font font(11.0f, visage::fonts::Lato_Regular_ttf);
        canvas.setColor(color_);
        canvas.text(label_.c_str(), font, visage::Font::kCenter, 0, 0, w, labelH);

        // Track background
        canvas.setColor(0xff1a1a1a);
        canvas.roundedRectangle(trackX, trackTop, trackW, trackH, 4.0f);

        // Track border
        canvas.setColor(0xff333333);
        canvas.roundedRectangleBorder(trackX, trackTop, trackW, trackH, 4.0f, 0.5f);

        // Fill level (bottom-up)
        float fillH = trackH * value_;
        float fillY = trackTop + trackH - fillH;
        if (fillH > 2.0f) {
            canvas.setColor(color_);
            canvas.roundedRectangle(trackX, fillY, trackW, fillH, 4.0f);
        }

        // Thumb
        float thumbH = 4.0f;
        float thumbW = w * 0.6f;
        float thumbX = (w - thumbW) / 2.0f;
        float thumbY = fillY - thumbH / 2.0f;
        thumbY = std::max(trackTop, std::min(trackTop + trackH - thumbH, thumbY));

        canvas.setColor(hovering_ ? 0xffffffff : 0xffcccccc);
        canvas.roundedRectangle(thumbX, thumbY, thumbW, thumbH, 2.0f);
    }

    void mouseEnter(const visage::MouseEvent&) override { hovering_ = true; redraw(); }
    void mouseExit(const visage::MouseEvent&) override { hovering_ = false; redraw(); }

    void mouseDown(const visage::MouseEvent& e) override {
        dragging_ = true;
        if (onGestureStart)
            onGestureStart();
        updateFromMouse(e);
    }

    void mouseDrag(const visage::MouseEvent& e) override {
        updateFromMouse(e);
    }

    void mouseUp(const visage::MouseEvent&) override {
        if (!dragging_)
            return;

        dragging_ = false;
        if (onGestureEnd)
            onGestureEnd();
    }

private:
    void updateFromMouse(const visage::MouseEvent& e) {
        float labelH = 14.0f;
        float trackTop = labelH + 4.0f;
        float trackH = height() - trackTop - 8.0f;

        float newVal = 1.0f - (e.position.y - trackTop) / trackH;
        newVal = std::max(0.0f, std::min(1.0f, newVal));

        value_ = newVal;
        redraw();

        if (onValueChange)
            onValueChange(value_);
    }

    std::string label_;
    unsigned int color_;
    float value_ = 0.5f;
    bool hovering_ = false;
    bool dragging_ = false;
};
