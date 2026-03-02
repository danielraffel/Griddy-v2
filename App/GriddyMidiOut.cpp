#include "GriddyMidiOut.h"
#include <TargetConditionals.h>

void GriddyMidiOut::openVirtualPort()
{
    const juce::SpinLock::ScopedLockType lock(lock_);
    if (output_)
        return;

#if JUCE_IOS && TARGET_IPHONE_SIMULATOR
    // CoreMIDI virtual endpoints are not available on the iOS simulator.
    enabled_.store(false);
    return;
#endif

    output_ = juce::MidiOutput::createNewDevice("Griddy MIDI");
}

void GriddyMidiOut::close()
{
    const juce::SpinLock::ScopedLockType lock(lock_);
    output_.reset();
}

void GriddyMidiOut::sendNoteOn(int channel, int noteNumber, int velocity)
{
    if (!enabled_.load())
        return;

    sendMessage(juce::MidiMessage::noteOn(channel, noteNumber, (juce::uint8)juce::jlimit(0, 127, velocity)));
}

void GriddyMidiOut::sendNoteOff(int channel, int noteNumber)
{
    if (!enabled_.load())
        return;

    sendMessage(juce::MidiMessage::noteOff(channel, noteNumber));
}

void GriddyMidiOut::sendCC(int channel, int ccNumber, int value)
{
    if (!enabled_.load())
        return;

    sendMessage(juce::MidiMessage::controllerEvent(channel, ccNumber, juce::jlimit(0, 127, value)));
}

void GriddyMidiOut::sendMessage(const juce::MidiMessage& msg)
{
    const juce::SpinLock::ScopedLockType lock(lock_);
    if (output_)
        output_->sendMessageNow(msg);
}
