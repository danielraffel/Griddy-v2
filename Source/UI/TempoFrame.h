#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <functional>
#include <string>
#include <cmath>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

class TempoFrame : public visage::Frame {
public:
    static constexpr float kMinBpm = 20.0f;
    static constexpr float kMaxBpm = 999.0f;

    TempoFrame() : Frame("Tempo") {}

    float getBpm() const { return bpm_; }

    void setBpm(float bpm) {
        bpm = std::max(kMinBpm, std::min(kMaxBpm, bpm));
        if (std::abs(bpm_ - bpm) > 0.5f) {
            bpm_ = bpm;
            redraw();
        }
    }

    std::function<void(float)> onValueChange;

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();

        // Background pill
        canvas.setColor(0xff1a1a1a);
        canvas.roundedRectangle(0, 0, w, h, h / 2.0f);

        // Border
        canvas.setColor(dragging_ ? 0xff58a8d0 : 0xff333333);
        canvas.roundedRectangleBorder(0, 0, w, h, h / 2.0f, 1.0f);

        // Fill indicator (proportional to tempo range)
        float normalized = (bpm_ - kMinBpm) / (kMaxBpm - kMinBpm);
        float fillW = (w - 8.0f) * normalized;
        if (fillW > 2.0f) {
            canvas.setColor(0xff2a4a5a);
            canvas.roundedRectangle(4, 4, fillW, h - 8, (h - 8) / 2.0f);
        }

        // BPM text centered
        char bpmText[16];
        snprintf(bpmText, sizeof(bpmText), "%d BPM", (int)std::round(bpm_));

        visage::Font font(16.0f, visage::fonts::Lato_Regular_ttf);
        canvas.setColor(0xffffffff);
        canvas.text(bpmText, font, visage::Font::kCenter, 0, 0, w, h);
    }

    void mouseDown(const visage::MouseEvent& e) override {
        dragStartX_ = e.position.x;
        dragStartBpm_ = bpm_;
        dragging_ = true;
        redraw();
    }

    void mouseDrag(const visage::MouseEvent& e) override {
        // Horizontal drag: full width of control = full BPM range
        float delta = (e.position.x - dragStartX_) / width() * (kMaxBpm - kMinBpm) * 0.5f;
        float newBpm = std::round(dragStartBpm_ + delta);
        newBpm = std::max(kMinBpm, std::min(kMaxBpm, newBpm));

        if (std::abs(bpm_ - newBpm) >= 1.0f) {
            bpm_ = newBpm;
            redraw();
            if (onValueChange)
                onValueChange(bpm_);
        }
    }

    void mouseUp(const visage::MouseEvent&) override {
        dragging_ = false;
        redraw();
    }

private:
    float bpm_ = 120.0f;
    float dragStartX_ = 0.0f;
    float dragStartBpm_ = 120.0f;
    bool dragging_ = false;
};
