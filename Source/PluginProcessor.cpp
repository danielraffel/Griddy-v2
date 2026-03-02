#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
GriddyAudioProcessor::GriddyAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

GriddyAudioProcessor::~GriddyAudioProcessor()
{
}

const juce::String GriddyAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool GriddyAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool GriddyAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool GriddyAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double GriddyAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int GriddyAudioProcessor::getNumPrograms()
{
    return 1;
}

int GriddyAudioProcessor::getCurrentProgram()
{
    return 0;
}

void GriddyAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String GriddyAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void GriddyAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

void GriddyAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void GriddyAudioProcessor::releaseResources()
{
}

bool GriddyAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void GriddyAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                              juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);

    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is where you'd add your audio processing code
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);
        juce::ignoreUnused (channelData);
    }
}

bool GriddyAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* GriddyAudioProcessor::createEditor()
{
    return new GriddyAudioProcessorEditor (*this);
}

void GriddyAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void GriddyAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// Path Helper Functions - Use macOS Application Support (no permission prompts)
//
// These functions provide standard paths for plugin data following Apple's
// Human Interface Guidelines. Using Application Support prevents permission
// dialogs during installation.
//
// Example usage:
//   auto samplesDir = GriddyAudioProcessor::getSamplesPath();
//   if (!samplesDir.exists())
//       samplesDir.createDirectory();
//

juce::File GriddyAudioProcessor::getApplicationSupportPath()
{
    auto appSupport = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory
    );

    auto projectFolder = appSupport.getChildFile(JucePlugin_Name);

    // Create if doesn't exist
    if (!projectFolder.exists())
        projectFolder.createDirectory();

    return projectFolder;
}

juce::File GriddyAudioProcessor::getSamplesPath()
{
    auto samplesDir = getApplicationSupportPath().getChildFile("Samples");

    if (!samplesDir.exists())
        samplesDir.createDirectory();

    return samplesDir;
}

juce::File GriddyAudioProcessor::getPresetsPath()
{
    auto presetsDir = getApplicationSupportPath().getChildFile("Presets");

    if (!presetsDir.exists())
        presetsDir.createDirectory();

    return presetsDir;
}

juce::File GriddyAudioProcessor::getUserDataPath()
{
    auto userDataDir = getApplicationSupportPath().getChildFile("UserData");

    if (!userDataDir.exists())
        userDataDir.createDirectory();

    return userDataDir;
}

juce::File GriddyAudioProcessor::getLogsPath()
{
    // Logs go to ~/Library/Logs/PluginName (standard macOS location)
    auto home = juce::File::getSpecialLocation(juce::File::userHomeDirectory);
    auto logsDir = home.getChildFile("Library").getChildFile("Logs").getChildFile(JucePlugin_Name);

    if (!logsDir.exists())
        logsDir.createDirectory();

    return logsDir;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new GriddyAudioProcessor();
}
