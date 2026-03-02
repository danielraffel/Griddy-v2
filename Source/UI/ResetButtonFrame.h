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
        float size = std::min(w, h);
        float cx = w / 2.0f;
        float cy = h / 2.0f;

        // Shadow
        canvas.setColor(0x40000000);
        canvas.circle(cx - size / 2.0f + 1, cy - size / 2.0f + 2, size);

        // Button body
        unsigned int bodyColor = glowing_ ? 0xffff8833 : 0xff3a3a3a;
        canvas.setColor(bodyColor);
        canvas.circle(cx - size / 2.0f, cy - size / 2.0f, size);

        // Border
        unsigned int borderColor = glowing_ ? 0xffffaa55 : 0xff505050;
        canvas.setColor(borderColor);
        canvas.ring(cx - size / 2.0f, cy - size / 2.0f, size, 1.5f);

        // Reset arrow icon (simplified: circular arc + triangle)
        float iconSize = size * 0.35f;
        float iconThick = 2.0f;

        // Arc (partial circle)
        canvas.setColor(glowing_ ? 0xff1e1e1e : 0xffcccccc);
        canvas.flatArc(cx - iconSize, cy - iconSize, iconSize * 2, iconThick,
                       0.0f, 4.5f);

        // Small arrowhead
        float arrowSize = iconSize * 0.4f;
        float ax = cx + iconSize * 0.9f;
        float ay = cy - iconSize * 0.1f;
        canvas.triangleDown(ax - arrowSize / 2.0f, ay - arrowSize / 2.0f, arrowSize);
    }

    void mouseDown(const visage::MouseEvent&) override {
        glowing_ = true;
        redraw();
        if (onPress)
            onPress();
    }

    void mouseUp(const visage::MouseEvent&) override {
        glowing_ = false;
        redraw();
    }

private:
    bool glowing_ = false;
};
