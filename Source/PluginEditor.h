#pragma once

#include "PluginProcessor.h"
#include "Visage/JuceVisageBridge.h"
#include "UI/XYPadFrame.h"
#include "UI/LEDMatrixFrame.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <visage_ui/frame.h>

class GriddyAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    public juce::Timer {
public:
    explicit GriddyAudioProcessorEditor(GriddyAudioProcessor&);
    ~GriddyAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    void createVisageUI();
    void layoutChildren();
    void updateUIFromProcessor();

    GriddyAudioProcessor& processorRef;
    std::unique_ptr<JuceVisageBridge> bridge_;
    std::unique_ptr<visage::Frame> rootFrame_;

    // UI components (owned by rootFrame_ via addChild, but we keep pointers)
    XYPadFrame* xyPad_ = nullptr;
    LEDMatrixFrame* ledMatrix_ = nullptr;

    bool uiCreated_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GriddyAudioProcessorEditor)
};
