#pragma once

#include <visage_ui/frame.h>
#include <visage_graphics/canvas.h>
#include <visage_graphics/font.h>
#include <array>
#include <cstdint>
#include <cmath>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

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

    void triggerResetAnimation(bool retrigger) {
        resetAnimProgress_ = 1.0f;
        resetIsRetrigger_ = retrigger;
        redraw();
    }

    void draw(visage::Canvas& canvas) override {
        float w = width();
        float h = height();
        visage::Font font(9.0f, visage::fonts::Lato_Regular_ttf);

        // Background
        canvas.setColor(0xff0a0a0a);
        canvas.roundedRectangle(0, 0, w, h, 6.0f);

        // Border
        canvas.setColor(0xff202020);
        canvas.roundedRectangleBorder(0, 0, w, h, 6.0f, 1.0f);

        // Layout constants
        float labelWidth = 25.0f;
        float startX = labelWidth + 10.0f;
        float spacing = (w - startX - 10.0f) / 32.0f;
        float ledSize = std::min(spacing * 0.7f, 10.0f); // Scale LEDs to fit spacing
        float rowSpacing = h / 4.0f;

        // Density thresholds
        uint8_t bdThresh = static_cast<uint8_t>(255 * (1.0f - bdDensity_));
        uint8_t sdThresh = static_cast<uint8_t>(255 * (1.0f - sdDensity_));
        uint8_t hhThresh = static_cast<uint8_t>(255 * (1.0f - hhDensity_));

        // Row text labels
        canvas.setColor(0xffcc2222);
        canvas.text("BD", font, visage::Font::kLeft, 4, rowSpacing * 1 - 6, labelWidth, 12);
        canvas.setColor(0xff22cc22);
        canvas.text("SD", font, visage::Font::kLeft, 4, rowSpacing * 2 - 6, labelWidth, 12);
        canvas.setColor(0xffcccc22);
        canvas.text("HH", font, visage::Font::kLeft, 4, rowSpacing * 3 - 6, labelWidth, 12);

        // Group separators and bar numbers
        visage::Font barFont(8.0f, visage::fonts::Lato_Regular_ttf);
        for (int g = 0; g < 4; ++g) {
            // Bar number at top of each group
            float barX = startX + g * 8 * spacing;
            canvas.setColor(0xff555555);
            const char* barNums[] = { "1", "2", "3", "4" };
            canvas.text(barNums[g], barFont, visage::Font::kCenter,
                        barX, 2, 8 * spacing, 10);

            // Separator line (between groups, not before first)
            if (g > 0) {
                float sepX = barX - spacing * 0.5f; // Midpoint between groups
                canvas.setColor(0x20ffffff);
                canvas.segment(sepX, rowSpacing * 0.7f,
                               sepX, rowSpacing * 3 + ledSize,
                               0.5f, false);
            }
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

            // Reset animation glow
            float resetGlow = 0.0f;
            if (resetAnimProgress_ > 0.0f) {
                if (resetIsRetrigger_) {
                    // Sweep animation
                    float sweepPos = (1.0f - resetAnimProgress_) * 32.0f;
                    float dist = std::abs(static_cast<float>(step) - sweepPos);
                    if (dist < 4.0f)
                        resetGlow = (1.0f - dist / 4.0f) * resetAnimProgress_;
                } else {
                    // Flash animation
                    resetGlow = resetAnimProgress_ * 0.5f;
                }
            }

            // BD LED
            drawLED(canvas, lx, rowSpacing * 1 - ledSize / 2, ledSize,
                    bdOn, bdAccent, isCurrent, resetGlow,
                    0xff331111, 0xffcc2222, 0xffff4444);

            // SD LED
            drawLED(canvas, lx, rowSpacing * 2 - ledSize / 2, ledSize,
                    sdOn, sdAccent, isCurrent, resetGlow,
                    0xff113311, 0xff22cc22, 0xff44ff44);

            // HH LED
            drawLED(canvas, lx, rowSpacing * 3 - ledSize / 2, ledSize,
                    hhOn, hhAccent, isCurrent, resetGlow,
                    0xff333311, 0xffcccc22, 0xffffff44);
        }

        // Decay reset animation
        if (resetAnimProgress_ > 0.0f) {
            resetAnimProgress_ -= 0.08f;
            if (resetAnimProgress_ < 0.0f)
                resetAnimProgress_ = 0.0f;
        }
    }

private:
    void drawLED(visage::Canvas& canvas, float x, float y, float size,
                 bool on, bool accent, bool current, float resetGlow,
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

        // Reset glow overlay
        if (resetGlow > 0.0f) {
            unsigned int alpha = static_cast<unsigned int>(resetGlow * 120);
            canvas.setColor((alpha << 24) | 0x00ffffff);
            canvas.circle(x - 1, y - 1, size + 2);
        }

        // Current step indicator (glow + white ring)
        if (current) {
            // Outer glow for current step (GPU-rendered)
            if (on) {
                unsigned int glowColor = (accentColor & 0x00ffffff) | 0x30000000;
                canvas.setColor(glowColor);
                canvas.circle(x - 2, y - 2, size + 4);
            }
            canvas.setColor(0xb0ffffff);
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
    float resetAnimProgress_ = 0.0f;
    bool resetIsRetrigger_ = false;
};
