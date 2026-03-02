#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <array>
#include <cstdint>

class LEDMatrixFrame : public visage::Frame {
public:
    LEDMatrixFrame() : Frame("LEDMatrix") {}

    void setPatterns(const std::array<uint8_t, 32>& bd,
                     const std::array<uint8_t, 32>& sd,
                     const std::array<uint8_t, 32>& hh) {
        bdPattern_ = bd;
        sdPattern_ = sd;
        hhPattern_ = hh;
        redraw();
    }

    void setDensities(float bd, float sd, float hh) {
        bdDensity_ = bd;
        sdDensity_ = sd;
        hhDensity_ = hh;
        redraw();
    }

    void setCurrentStep(int step) {
        if (currentStep_ != step) {
            currentStep_ = step;
            redraw();
        }
    }

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();

        // Background
        canvas.setColor(0xff0a0a0a);
        canvas.roundedRectangle(0, 0, w, h, 6.0f);

        // Border
        canvas.setColor(0xff202020);
        canvas.roundedRectangleBorder(0, 0, w, h, 6.0f, 1.0f);

        // Layout constants
        float labelWidth = 25.0f;
        float startX = labelWidth + 10.0f;
        float ledSize = 10.0f;
        float spacing = (w - startX - 10.0f) / 32.0f;
        float rowSpacing = h / 4.0f;

        // Density thresholds
        uint8_t bdThresh = static_cast<uint8_t>(255 * (1.0f - bdDensity_));
        uint8_t sdThresh = static_cast<uint8_t>(255 * (1.0f - sdDensity_));
        uint8_t hhThresh = static_cast<uint8_t>(255 * (1.0f - hhDensity_));

        // Row indicator dots (small colored dots instead of text labels)
        float dotSize = 6.0f;
        float dotX = 10.0f;
        canvas.setColor(0xffcc2222); // BD red
        canvas.circle(dotX, rowSpacing * 1 - dotSize / 2, dotSize);
        canvas.setColor(0xff22cc22); // SD green
        canvas.circle(dotX, rowSpacing * 2 - dotSize / 2, dotSize);
        canvas.setColor(0xffcccc22); // HH yellow
        canvas.circle(dotX, rowSpacing * 3 - dotSize / 2, dotSize);

        // Group separators
        for (int g = 1; g < 4; ++g) {
            float gx = startX + g * 8 * spacing;
            canvas.setColor(0x30ffffff);
            canvas.segment(gx - spacing * 0.3f, rowSpacing * 0.7f,
                           gx - spacing * 0.3f, rowSpacing * 3 + ledSize,
                           0.75f, false);
        }

        // Draw LEDs
        for (int step = 0; step < 32; ++step) {
            float lx = startX + step * spacing + (spacing - ledSize) / 2;

            bool bdOn = bdPattern_[step] > bdThresh;
            bool sdOn = sdPattern_[step] > sdThresh;
            bool hhOn = hhPattern_[step] > hhThresh;

            bool bdAccent = bdPattern_[step] > 200 && bdOn;
            bool sdAccent = sdPattern_[step] > 200 && sdOn;
            bool hhAccent = hhPattern_[step] > 200 && hhOn;

            bool isCurrent = (step == currentStep_);

            // BD LED
            drawLED(canvas, lx, rowSpacing * 1 - ledSize / 2, ledSize,
                    bdOn, bdAccent, isCurrent,
                    0xff331111, 0xffcc2222, 0xffff4444);

            // SD LED
            drawLED(canvas, lx, rowSpacing * 2 - ledSize / 2, ledSize,
                    sdOn, sdAccent, isCurrent,
                    0xff113311, 0xff22cc22, 0xff44ff44);

            // HH LED
            drawLED(canvas, lx, rowSpacing * 3 - ledSize / 2, ledSize,
                    hhOn, hhAccent, isCurrent,
                    0xff333311, 0xffcccc22, 0xffffff44);
        }
    }

private:
    void drawLED(visage::Canvas& canvas, float x, float y, float size,
                 bool on, bool accent, bool current,
                 unsigned int offColor, unsigned int onColor, unsigned int accentColor) {
        // Shadow for active LEDs
        if (on) {
            canvas.setColor(0x30000000);
            canvas.circle(x + 1, y + 1, size);
        }

        // Main LED
        if (accent)
            canvas.setColor(accentColor);
        else if (on)
            canvas.setColor(onColor);
        else
            canvas.setColor(offColor);
        canvas.circle(x, y, size);

        // Highlight for active LEDs
        if (on) {
            canvas.setColor(0x40ffffff);
            canvas.circle(x + 2, y + 1, size * 0.5f);
        }

        // Current step indicator (white ring)
        if (current) {
            canvas.setColor(0x80ffffff);
            float inset = -1.5f;
            canvas.roundedRectangleBorder(x + inset, y + inset,
                                          size - inset * 2, size - inset * 2,
                                          size / 2, 1.5f);
        }
    }

    std::array<uint8_t, 32> bdPattern_{};
    std::array<uint8_t, 32> sdPattern_{};
    std::array<uint8_t, 32> hhPattern_{};
    float bdDensity_ = 0.5f;
    float sdDensity_ = 0.5f;
    float hhDensity_ = 0.5f;
    int currentStep_ = -1;
};
