#include "GriddyAppEngine.h"

GriddyAppEngine::GriddyAppEngine()
{
    midiOut_.openVirtualPort();

    for (int i = 0; i < 8; ++i)
        synth_.addVoice(new juce::SamplerVoice());

    if (samples_.loadBundledSamples())
    {
        synth_.addSound(samples_.getBassDrumSound());
        synth_.addSound(samples_.getSnareDrumSound());
        synth_.addSound(samples_.getHiHatSound());
    }

    updateSamplesPerStep();
}

void GriddyAppEngine::prepare(double sampleRate, int /*samplesPerBlock*/, int /*outputChannels*/)
{
    sampleRate_ = sampleRate;
    synth_.setCurrentPlaybackSampleRate(sampleRate_);
    updateSamplesPerStep();
    samplesUntilNextStep_ = samplesPerStep_;
}

void GriddyAppEngine::releaseResources()
{
}

void GriddyAppEngine::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* buffer = bufferToFill.buffer;
    if (!buffer)
        return;

    buffer->clear(bufferToFill.startSample, bufferToFill.numSamples);

    if (!playing_.load())
        return;

    updateSamplesPerStep();

    int samplesRemaining = bufferToFill.numSamples;
    int renderStart = bufferToFill.startSample;

    while (samplesRemaining > 0)
    {
        int block = juce::jmin(samplesUntilNextStep_, samplesRemaining);

        if (!midiOnly_.load())
            synth_.renderNextBlock(*buffer, juce::MidiBuffer(), renderStart, block);

        samplesRemaining -= block;
        renderStart += block;
        samplesUntilNextStep_ -= block;

        if (samplesUntilNextStep_ <= 0)
        {
            advanceStep();
            samplesUntilNextStep_ += samplesPerStep_;
        }
    }
}

void GriddyAppEngine::setPlaying(bool isPlaying)
{
    if (isPlaying && !playing_.load())
    {
        currentStep_ = 0;
        samplesUntilNextStep_ = samplesPerStep_;
        gridsEngine_.reset();
        prevBD_ = prevSD_ = prevHH_ = false;
    }

    playing_.store(isPlaying);
}

void GriddyAppEngine::setRecording(bool isRecording)
{
    if (isRecording) {
        // Pre-fill all 32 steps with current live values so that
        // unrecorded steps don't default to zero (which silences patterns).
        float x = x_.load(), y = y_.load();
        float bd = bdDensity_.load(), sd = sdDensity_.load(), hh = hhDensity_.load();
        float bv = bdVelocityRange_.load(), sv = sdVelocityRange_.load(), hv = hhVelocityRange_.load();
        float ch = chaos_.load(), sw = swing_.load(), tm = tempo_.load();
        for (int i = 0; i < 32; ++i) {
            recordedX_[i] = x;           recordedY_[i] = y;
            recordedBDDensity_[i] = bd;  recordedSDDensity_[i] = sd;  recordedHHDensity_[i] = hh;
            recordedBDVelocity_[i] = bv; recordedSDVelocity_[i] = sv; recordedHHVelocity_[i] = hv;
            recordedChaos_[i] = ch;      recordedSwing_[i] = sw;      recordedTempo_[i] = tm;
        }
        hasRecording_.store(true);
    }
    recording_.store(isRecording);
}

void GriddyAppEngine::setMidiOnly(bool midiOnly)
{
    midiOnly_.store(midiOnly);
}

void GriddyAppEngine::setTempo(float bpm)
{
    tempo_.store(juce::jlimit(kMinTempoBpm, kMaxTempoBpm, bpm));
    midiOut_.sendCC(midiChannel_, ccTempo_, tempoToCCValue(tempo_.load()));
}

void GriddyAppEngine::setX(float value)
{
    x_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccX_, (int)(x_.load() * 127.0f));
}

void GriddyAppEngine::setY(float value)
{
    y_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccY_, (int)(y_.load() * 127.0f));
}

void GriddyAppEngine::setBDDensity(float value)
{
    bdDensity_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccBDDensity_, (int)(bdDensity_.load() * 127.0f));
}

void GriddyAppEngine::setSDDensity(float value)
{
    sdDensity_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccSDDensity_, (int)(sdDensity_.load() * 127.0f));
}

void GriddyAppEngine::setHHDensity(float value)
{
    hhDensity_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccHHDensity_, (int)(hhDensity_.load() * 127.0f));
}

void GriddyAppEngine::setBDVelocityRange(float value)
{
    bdVelocityRange_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccBDVelocity_, (int)(bdVelocityRange_.load() * 127.0f));
}

void GriddyAppEngine::setSDVelocityRange(float value)
{
    sdVelocityRange_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccSDVelocity_, (int)(sdVelocityRange_.load() * 127.0f));
}

void GriddyAppEngine::setHHVelocityRange(float value)
{
    hhVelocityRange_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccHHVelocity_, (int)(hhVelocityRange_.load() * 127.0f));
}

void GriddyAppEngine::setChaos(float value)
{
    chaos_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccChaos_, (int)(chaos_.load() * 127.0f));
}

void GriddyAppEngine::setSwing(float value)
{
    swing_.store(juce::jlimit(0.0f, 1.0f, value));
    midiOut_.sendCC(midiChannel_, ccSwing_, (int)(swing_.load() * 127.0f));
}

void GriddyAppEngine::updateSamplesPerStep()
{
    double bpm = tempo_.load();
    double samplesPerBeat = (sampleRate_ * 60.0) / juce::jmax(1.0, bpm);
    samplesPerStep_ = juce::jmax(1, (int)(samplesPerBeat / 4.0));
}

void GriddyAppEngine::advanceStep()
{
    // Note off for previous triggers
    if (prevBD_) midiOut_.sendNoteOff(midiChannel_, bdNote_);
    if (prevSD_) midiOut_.sendNoteOff(midiChannel_, sdNote_);
    if (prevHH_) midiOut_.sendNoteOff(midiChannel_, hhNote_);

    // Apply XY for current step (record or playback)
    applyCurrentStepParams();

    gridsEngine_.setCurrentStep(currentStep_);
    gridsEngine_.evaluateDrums();

    bool bdTrig = gridsEngine_.getBDTrigger();
    bool sdTrig = gridsEngine_.getSDTrigger();
    bool hhTrig = gridsEngine_.getHHTrigger();

    if (bdTrig)
    {
        int vel = calculateVelocity(true, gridsEngine_.getBDAccent(), bdVelocityRange_.load());
        if (!midiOnly_.load())
            synth_.noteOn(midiChannel_, bdNote_, vel / 127.0f);
        midiOut_.sendNoteOn(midiChannel_, bdNote_, vel);
    }
    if (sdTrig)
    {
        int vel = calculateVelocity(false, gridsEngine_.getSDAccent(), sdVelocityRange_.load());
        if (!midiOnly_.load())
            synth_.noteOn(midiChannel_, sdNote_, vel / 127.0f);
        midiOut_.sendNoteOn(midiChannel_, sdNote_, vel);
    }
    if (hhTrig)
    {
        int vel = calculateVelocity(false, gridsEngine_.getHHAccent(), hhVelocityRange_.load());
        if (!midiOnly_.load())
            synth_.noteOn(midiChannel_, hhNote_, vel / 127.0f);
        midiOut_.sendNoteOn(midiChannel_, hhNote_, vel);
    }

    prevBD_ = bdTrig;
    prevSD_ = sdTrig;
    prevHH_ = hhTrig;

    currentStep_ = (currentStep_ + 1) % 32;
}

void GriddyAppEngine::applyCurrentStepParams()
{
    float xValue = x_.load();
    float yValue = y_.load();
    float bdDensity = bdDensity_.load();
    float sdDensity = sdDensity_.load();
    float hhDensity = hhDensity_.load();
    float bdVelocity = bdVelocityRange_.load();
    float sdVelocity = sdVelocityRange_.load();
    float hhVelocity = hhVelocityRange_.load();
    float chaos = chaos_.load();
    float swing = swing_.load();
    float tempo = tempo_.load();

    if (recording_.load())
    {
        recordedX_[currentStep_] = xValue;
        recordedY_[currentStep_] = yValue;
        recordedBDDensity_[currentStep_] = bdDensity;
        recordedSDDensity_[currentStep_] = sdDensity;
        recordedHHDensity_[currentStep_] = hhDensity;
        recordedBDVelocity_[currentStep_] = bdVelocity;
        recordedSDVelocity_[currentStep_] = sdVelocity;
        recordedHHVelocity_[currentStep_] = hhVelocity;
        recordedChaos_[currentStep_] = chaos;
        recordedSwing_[currentStep_] = swing;
        recordedTempo_[currentStep_] = tempo;
        hasRecording_.store(true);
    }
    else if (hasRecording_.load())
    {
        xValue = recordedX_[currentStep_];
        yValue = recordedY_[currentStep_];
        bdDensity = recordedBDDensity_[currentStep_];
        sdDensity = recordedSDDensity_[currentStep_];
        hhDensity = recordedHHDensity_[currentStep_];
        bdVelocity = recordedBDVelocity_[currentStep_];
        sdVelocity = recordedSDVelocity_[currentStep_];
        hhVelocity = recordedHHVelocity_[currentStep_];
        chaos = recordedChaos_[currentStep_];
        swing = recordedSwing_[currentStep_];
        tempo = recordedTempo_[currentStep_];

        x_.store(xValue);
        y_.store(yValue);
        bdDensity_.store(bdDensity);
        sdDensity_.store(sdDensity);
        hhDensity_.store(hhDensity);
        bdVelocityRange_.store(bdVelocity);
        sdVelocityRange_.store(sdVelocity);
        hhVelocityRange_.store(hhVelocity);
        chaos_.store(chaos);
        swing_.store(swing);
        tempo_.store(tempo);
        updateSamplesPerStep();
        sendAllParamCCs();
    }

    gridsEngine_.setX(xValue);
    gridsEngine_.setY(yValue);
    gridsEngine_.setBDDensity(bdDensity);
    gridsEngine_.setSDDensity(sdDensity);
    gridsEngine_.setHHDensity(hhDensity);
    gridsEngine_.setChaos(chaos);
    gridsEngine_.setSwing(swing);
}

void GriddyAppEngine::sendControlCCs(float x, float y)
{
    midiOut_.sendCC(midiChannel_, ccX_, (int)(x * 127.0f));
    midiOut_.sendCC(midiChannel_, ccY_, (int)(y * 127.0f));
}

void GriddyAppEngine::sendAllParamCCs()
{
    midiOut_.sendCC(midiChannel_, ccTempo_, tempoToCCValue(tempo_.load()));
    midiOut_.sendCC(midiChannel_, ccX_, (int)(x_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccY_, (int)(y_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccBDDensity_, (int)(bdDensity_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccSDDensity_, (int)(sdDensity_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccHHDensity_, (int)(hhDensity_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccBDVelocity_, (int)(bdVelocityRange_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccSDVelocity_, (int)(sdVelocityRange_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccHHVelocity_, (int)(hhVelocityRange_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccChaos_, (int)(chaos_.load() * 127.0f));
    midiOut_.sendCC(midiChannel_, ccSwing_, (int)(swing_.load() * 127.0f));
}

int GriddyAppEngine::tempoToCCValue(float bpm) const
{
    const float clampedTempo = juce::jlimit(kMinTempoBpm, kMaxTempoBpm, bpm);
    const float normalizedTempo = (clampedTempo - kMinTempoBpm) / (kMaxTempoBpm - kMinTempoBpm);
    return juce::jlimit(0, 127, juce::roundToInt(normalizedTempo * 127.0f));
}

int GriddyAppEngine::calculateVelocity(bool isBD, bool isAccent, float velocityRange) const
{
    // Range goes from narrow (80-100) to wide (40-127)
    int minVel = static_cast<int>(80 - (velocityRange * 40));
    int maxVel = static_cast<int>(100 + (velocityRange * 27));
    int normalVel = (minVel + maxVel) / 2;

    if (isAccent)
        return maxVel;

    float chaos = chaos_.load();
    if (chaos > 0.0f)
    {
        int range = maxVel - minVel;
        int variation = static_cast<int>((juce::Random::getSystemRandom().nextFloat() - 0.5f)
                                         * range * chaos * 0.3f);
        normalVel = juce::jlimit(minVel, maxVel, normalVel + variation);
    }

    if (isBD)
        normalVel = juce::jmin(127, normalVel + 10);

    return normalVel;
}
