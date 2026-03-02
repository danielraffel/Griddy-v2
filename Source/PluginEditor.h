#pragma once

#include "PluginProcessor.h"
#include <juce_audio_processors/juce_audio_processors.h>

//==============================================================================
/**
*/
class GriddyAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit GriddyAudioProcessorEditor (GriddyAudioProcessor&);
    ~GriddyAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    GriddyAudioProcessor& processorRef;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GriddyAudioProcessorEditor)
};
