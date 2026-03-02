#pragma once

#include <JuceHeader.h>
#include "GriddyAppEngine.h"
#include "XYPad.h"

class OnboardingOverlay;

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
    void mouseDown(const juce::MouseEvent& event) override;

private:
    void timerCallback() override;
    void updateLabels();
    void configureSectionLabel(juce::Label& label, const juce::String& text);
    void drawLEDMatrix(juce::Graphics& g, juce::Rectangle<int> bounds);

    GriddyAppEngine engine_;

    XYPad xyPad_;

    juce::Slider tempoSlider_;
    juce::TextEditor tempoEditor_;
    juce::Label tempoUnitLabel_;

    juce::Slider bdDensity_;
    juce::Slider sdDensity_;
    juce::Slider hhDensity_;

    juce::Slider bdVelocity_;
    juce::Slider sdVelocity_;
    juce::Slider hhVelocity_;

    juce::Slider chaosSlider_;
    juce::Slider swingSlider_;

    juce::TextButton playButton_ { "Play" };
    juce::TextButton recordButton_ { "Record" };
    juce::ToggleButton midiOnlyToggle_ { "MIDI-only" };

    juce::Label densityGroupLabel_;
    juce::Label velocityGroupLabel_;
    juce::Label chaosGroupLabel_;
    juce::Label swingGroupLabel_;
    juce::Label statusLabel_;

    // LED matrix data (updated from engine on timer)
    std::array<uint8_t, 32> bdPattern_{};
    std::array<uint8_t, 32> sdPattern_{};
    std::array<uint8_t, 32> hhPattern_{};
    int currentStep_ = -1;
    juce::Rectangle<int> ledMatrixBounds_;

    bool tempoEditorWasFocused_ = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GriddyAppMainComponent)
};
