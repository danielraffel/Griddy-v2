#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <visage_app/application_window.h>
#include <visage_ui/frame.h>

class JuceVisageBridge : public juce::Component, public juce::Timer {
public:
    JuceVisageBridge();
    ~JuceVisageBridge() override;

    void setRootFrame(visage::Frame* frame);
    void createEmbeddedWindow();
    void shutdownRendering();

    // juce::Component
    void paint(juce::Graphics& g) override;
    void resized() override;
#if !JUCE_IOS
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseMove(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
#endif
    bool keyPressed(const juce::KeyPress& key) override;

    // juce::Timer
    void timerCallback() override;

private:
#if !JUCE_IOS
    visage::MouseEvent convertMouseEvent(const juce::MouseEvent& e) const;
#endif
    visage::KeyEvent convertKeyEvent(const juce::KeyPress& key) const;
    int convertModifiers(const juce::ModifierKeys& mods) const;
    void setFocusedChild(visage::Frame* child);

    std::unique_ptr<visage::ApplicationWindow> visageWindow_;
    visage::Frame* rootFrame_ = nullptr;
#if !JUCE_IOS
    visage::Frame* mouseDownFrame_ = nullptr;
    visage::Frame* hoverFrame_ = nullptr;
#endif
    visage::Frame* focusedChild_ = nullptr;
    visage::FrameEventHandler eventHandler_;
    bool windowCreated_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(JuceVisageBridge)
};
