#pragma once

#include <JuceHeader.h>

class GriddyMidiOut
{
public:
    GriddyMidiOut() = default;
    ~GriddyMidiOut() = default;

    void openVirtualPort();
    void close();

    void setEnabled(bool enabled) { enabled_.store(enabled); }

    void sendNoteOn(int channel, int noteNumber, int velocity);
    void sendNoteOff(int channel, int noteNumber);
    void sendCC(int channel, int ccNumber, int value);

private:
    std::unique_ptr<juce::MidiOutput> output_;
    juce::SpinLock lock_;
    std::atomic<bool> enabled_ { true };

    void sendMessage(const juce::MidiMessage& msg);
};
