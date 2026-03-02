#pragma once

#include <JuceHeader.h>

class GriddySampleLibrary
{
public:
    GriddySampleLibrary();

    bool loadBundledSamples();

    juce::SynthesiserSound::Ptr getBassDrumSound() const { return bdSound_; }
    juce::SynthesiserSound::Ptr getSnareDrumSound() const { return sdSound_; }
    juce::SynthesiserSound::Ptr getHiHatSound() const { return hhSound_; }

private:
    juce::AudioFormatManager formatManager_;
    juce::SynthesiserSound::Ptr bdSound_;
    juce::SynthesiserSound::Ptr sdSound_;
    juce::SynthesiserSound::Ptr hhSound_;

    juce::File findSamplesDirectory() const;
    juce::SynthesiserSound::Ptr loadSample(const juce::File& file, const juce::String& name, int midiNote);
    juce::SynthesiserSound::Ptr loadSampleFromBinary(const void* data, int dataSize, const juce::String& name, int midiNote);
};
