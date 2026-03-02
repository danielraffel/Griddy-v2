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

    void setVelocityRanges(float bd, float sd, float hh) {
        bdVelocity_ = bd;
        sdVelocity_ = sd;
        hhVelocity_ = hh;
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
        float rightPad = 10.0f;
        float groupGap = 8.0f; // Gap between groups of 8 for clear separation
        float totalWidth = w - startX - rightPad;
        float usableWidth = totalWidth - 3.0f * groupGap; // 3 gaps between 4 groups
        float stepSpacing = usableWidth / 32.0f;
        float ledSize = std::min(stepSpacing * 0.8f, 10.0f);
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
            // Calculate group start X (accounting for gaps)
            float groupStartX = startX + g * (8 * stepSpacing + groupGap);

            // Bar number at top of each group
            canvas.setColor(0xff555555);
            const char* barNums[] = { "1", "2", "3", "4" };
            canvas.text(barNums[g], barFont, visage::Font::kCenter,
                        groupStartX, 2, 8 * stepSpacing, 10);

            // Separator line in the gap (not through LEDs)
            if (g > 0) {
                float sepX = groupStartX - groupGap / 2.0f;
                canvas.setColor(0x30ffffff);
                canvas.segment(sepX, rowSpacing * 0.7f,
                               sepX, rowSpacing * 3 + ledSize,
                               0.5f, false);
            }
        }

        // Draw LEDs
        for (int step = 0; step < 32; ++step) {
            int group = step / 8;
            int stepInGroup = step % 8;
            float lx = startX + group * (8 * stepSpacing + groupGap)
                      + stepInGroup * stepSpacing + (stepSpacing - ledSize) / 2;

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
                    float sweepPos = (1.0f - resetAnimProgress_) * 32.0f;
                    float dist = std::abs(static_cast<float>(step) - sweepPos);
                    if (dist < 4.0f)
                        resetGlow = (1.0f - dist / 4.0f) * resetAnimProgress_;
                } else {
                    resetGlow = resetAnimProgress_ * 0.5f;
                }
            }

            // Velocity brightness (accurate: based on velocity range parameter)
            float bdVelBright = velocityBrightness(bdAccent, bdVelocity_);
            float sdVelBright = velocityBrightness(sdAccent, sdVelocity_);
            float hhVelBright = velocityBrightness(hhAccent, hhVelocity_);

            // BD LED
            drawLED(canvas, lx, rowSpacing * 1 - ledSize / 2, ledSize,
                    bdOn, bdAccent, isCurrent, resetGlow, bdVelBright,
                    0xff331111, 0xffcc2222, 0xffff4444);

            // SD LED
            drawLED(canvas, lx, rowSpacing * 2 - ledSize / 2, ledSize,
                    sdOn, sdAccent, isCurrent, resetGlow, sdVelBright,
                    0xff113311, 0xff22cc22, 0xff44ff44);

            // HH LED
            drawLED(canvas, lx, rowSpacing * 3 - ledSize / 2, ledSize,
                    hhOn, hhAccent, isCurrent, resetGlow, hhVelBright,
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
    // Calculate brightness multiplier based on velocity range
    // Returns 0.6..1.0 range — accents are always brighter than normals
    float velocityBrightness(bool isAccent, float velocityRange) {
        float minVel = 80.0f - (velocityRange * 40.0f);
        float maxVel = 100.0f + (velocityRange * 27.0f);
        float normalVel = (minVel + maxVel) / 2.0f;
        float vel = isAccent ? maxVel : normalVel;
        // Map velocity (40..127) to brightness (0.55..1.0)
        return 0.55f + 0.45f * (vel / 127.0f);
    }

    unsigned int scaleColor(unsigned int color, float brightness) {
        unsigned int r = static_cast<unsigned int>(((color >> 16) & 0xff) * brightness);
        unsigned int g = static_cast<unsigned int>(((color >> 8) & 0xff) * brightness);
        unsigned int b = static_cast<unsigned int>((color & 0xff) * brightness);
        r = std::min(r, 255u);
        g = std::min(g, 255u);
        b = std::min(b, 255u);
        return (0xff << 24) | (r << 16) | (g << 8) | b;
    }

    void drawLED(visage::Canvas& canvas, float x, float y, float size,
                 bool on, bool accent, bool current, float resetGlow,
                 float velBrightness,
                 unsigned int offColor, unsigned int onColor, unsigned int accentColor) {
        // Shadow for active LEDs
        if (on) {
            canvas.setColor(0x30000000);
            canvas.circle(x + 1, y + 1, size);
        }

        // Main LED — velocity brightness applied to active LEDs
        if (accent)
            canvas.setColor(scaleColor(accentColor, velBrightness));
        else if (on)
            canvas.setColor(scaleColor(onColor, velBrightness));
        else
            canvas.setColor(offColor);
        canvas.circle(x, y, size);

        // Highlight for active LEDs
        if (on) {
            unsigned int hlAlpha = static_cast<unsigned int>(0x40 * velBrightness);
            canvas.setColor((hlAlpha << 24) | 0x00ffffff);
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
    float bdVelocity_ = 0.5f;
    float sdVelocity_ = 0.5f;
    float hhVelocity_ = 0.5f;
    int currentStep_ = -1;
    float resetAnimProgress_ = 0.0f;
    bool resetIsRetrigger_ = false;
};
