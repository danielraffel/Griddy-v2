#include "GriddySampleLibrary.h"
#include <BinaryData.h>

GriddySampleLibrary::GriddySampleLibrary()
{
    formatManager_.registerBasicFormats();
}

bool GriddySampleLibrary::loadBundledSamples()
{
    auto samplesDir = findSamplesDirectory();
    if (samplesDir.exists())
    {
        bdSound_ = loadSample(samplesDir.getChildFile("kick.wav"), "BD", 36);
        sdSound_ = loadSample(samplesDir.getChildFile("snare.wav"), "SD", 38);
        hhSound_ = loadSample(samplesDir.getChildFile("hihat.wav"), "HH", 42);
    }

    if (bdSound_ == nullptr)
        bdSound_ = loadSampleFromBinary(BinaryData::kick_wav, BinaryData::kick_wavSize, "BD", 36);
    if (sdSound_ == nullptr)
        sdSound_ = loadSampleFromBinary(BinaryData::snare_wav, BinaryData::snare_wavSize, "SD", 38);
    if (hhSound_ == nullptr)
        hhSound_ = loadSampleFromBinary(BinaryData::hihat_wav, BinaryData::hihat_wavSize, "HH", 42);

    return bdSound_ && sdSound_ && hhSound_;
}

juce::File GriddySampleLibrary::findSamplesDirectory() const
{
    auto appFile = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
    auto appDir = appFile.getParentDirectory();

    auto samplesDir = appDir.getChildFile("Samples");
    if (samplesDir.exists())
        return samplesDir;

    auto resourcesDir = appDir.getChildFile("Resources").getChildFile("Samples");
    if (resourcesDir.exists())
        return resourcesDir;

    return {};
}

juce::SynthesiserSound::Ptr GriddySampleLibrary::loadSample(const juce::File& file,
                                                            const juce::String& name,
                                                            int midiNote)
{
    if (!file.existsAsFile())
        return {};

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager_.createReaderFor(file));
    if (!reader)
        return {};

    juce::BigInteger noteRange;
    noteRange.setBit(midiNote);

    return new juce::SamplerSound(
        name,
        *reader,
        noteRange,
        midiNote,
        0.001,   // attack
        0.05,    // release
        10.0     // max length seconds
    );
}

juce::SynthesiserSound::Ptr GriddySampleLibrary::loadSampleFromBinary(const void* data,
                                                                      int dataSize,
                                                                      const juce::String& name,
                                                                      int midiNote)
{
    if (!data || dataSize <= 0)
        return {};

    auto stream = std::make_unique<juce::MemoryInputStream>(data, (size_t)dataSize, false);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager_.createReaderFor(std::move(stream)));
    if (!reader)
        return {};

    juce::BigInteger noteRange;
    noteRange.setBit(midiNote);

    return new juce::SamplerSound(
        name,
        *reader,
        noteRange,
        midiNote,
        0.001,
        0.05,
        10.0
    );
}
