#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GriddyAudioProcessorEditor::GriddyAudioProcessorEditor (GriddyAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (400, 300);
}

GriddyAudioProcessorEditor::~GriddyAudioProcessorEditor()
{
}

void GriddyAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void GriddyAudioProcessorEditor::resized()
{
    // This is where you'd lay out your UI components
}
