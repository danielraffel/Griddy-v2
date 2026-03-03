#pragma once

#include <JuceHeader.h>
#include "GriddyAppEngine.h"
#include "Visage/JuceVisageBridge.h"
#include "UI/XYPadFrame.h"
#include "UI/LEDMatrixFrame.h"
#include "UI/DensitySliderFrame.h"
#include "UI/RotaryKnobFrame.h"
#include "UI/ResetButtonFrame.h"
#include "UI/TransportButtonFrame.h"
#include "UI/TempoFrame.h"
#include "UI/ToggleFrame.h"

class GriddyAppMainComponent : public juce::AudioAppComponent,
                               private juce::Timer
{
public:
    GriddyAppMainComponent();
    ~GriddyAppMainComponent() override;

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& e) override;

private:
    void timerCallback() override;
    void createVisageUI();
    void layoutChildren();
    void updateFromEngine();
    void showBpmEditor();
    void hideBpmEditor();

    GriddyAppEngine engine_;

    // Visage bridge and root frame
    std::unique_ptr<JuceVisageBridge> bridge_;
    std::unique_ptr<visage::Frame> rootFrame_;

    // Frame pointers (owned by rootFrame_ via addChild)
    XYPadFrame* xyPad_ = nullptr;
    LEDMatrixFrame* ledMatrix_ = nullptr;
    DensitySliderFrame* bdDensity_ = nullptr;
    DensitySliderFrame* sdDensity_ = nullptr;
    DensitySliderFrame* hhDensity_ = nullptr;
    RotaryKnobFrame* chaosKnob_ = nullptr;
    RotaryKnobFrame* swingKnob_ = nullptr;
    RotaryKnobFrame* bdVelKnob_ = nullptr;
    RotaryKnobFrame* sdVelKnob_ = nullptr;
    RotaryKnobFrame* hhVelKnob_ = nullptr;
    ResetButtonFrame* resetButton_ = nullptr;
    TransportButtonFrame* playButton_ = nullptr;
    TransportButtonFrame* recordButton_ = nullptr;
    TempoFrame* tempoControl_ = nullptr;
    ToggleFrame* midiOnlyToggle_ = nullptr;

    bool uiCreated_ = false;

    // BPM numeric editor (iOS double-tap)
    std::unique_ptr<juce::TextEditor> bpmEditor_;
    std::unique_ptr<juce::TextButton> bpmDoneBtn_;
    std::unique_ptr<juce::Component> bpmScrim_;  // Tap-outside-to-dismiss

    // Layout rects for drawing cell backgrounds in onDraw callback
    struct Rect { float x = 0, y = 0, w = 0, h = 0; };
    Rect cellXY_;
    Rect cellLED_;
    Rect cellTransport_;
    Rect cellControls_;
    Rect resetLabelRect_;
    Rect velLabelRect_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GriddyAppMainComponent)
};
