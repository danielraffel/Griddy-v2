#pragma once

#include <JuceHeader.h>
#include <atomic>

#include "GridsEngine.h"
#include "GriddyMidiOut.h"
#include "GriddySampleLibrary.h"

class GriddyAppEngine
{
public:
    GriddyAppEngine();
    static constexpr float kMinTempoBpm = 20.0f;
    static constexpr float kMaxTempoBpm = 999.0f;

    void prepare(double sampleRate, int samplesPerBlock, int outputChannels);
    void releaseResources();
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill);

    void setPlaying(bool isPlaying);
    bool isPlaying() const { return playing_.load(); }

    void setRecording(bool isRecording);
    bool isRecording() const { return recording_.load(); }

    void setMidiOnly(bool midiOnly);
    bool isMidiOnly() const { return midiOnly_.load(); }

    void setTempo(float bpm);
    float getTempo() const { return tempo_.load(); }

    void setX(float value);
    void setY(float value);
    float getX() const { return x_.load(); }
    float getY() const { return y_.load(); }

    void setBDDensity(float value);
    void setSDDensity(float value);
    void setHHDensity(float value);

    void setBDVelocityRange(float value);
    void setSDVelocityRange(float value);
    void setHHVelocityRange(float value);

    void setChaos(float value);
    void setSwing(float value);

    float getBDDensity() const { return bdDensity_.load(); }
    float getSDDensity() const { return sdDensity_.load(); }
    float getHHDensity() const { return hhDensity_.load(); }

    float getBDVelocityRange() const { return bdVelocityRange_.load(); }
    float getSDVelocityRange() const { return sdVelocityRange_.load(); }
    float getHHVelocityRange() const { return hhVelocityRange_.load(); }

    float getChaos() const { return chaos_.load(); }
    float getSwing() const { return swing_.load(); }

    bool hasRecording() const { return hasRecording_.load(); }

    // Access the underlying GridsEngine for pattern data
    GridsEngine& getGridsEngine() { return gridsEngine_; }

private:
    void updateSamplesPerStep();
    void advanceStep();
    void applyCurrentStepParams();
    void sendControlCCs(float x, float y);
    void sendAllParamCCs();
    int calculateVelocity(bool isBD, bool isAccent, float velocityRange) const;
    int tempoToCCValue(float bpm) const;

    GridsEngine gridsEngine_;
    GriddyMidiOut midiOut_;
    GriddySampleLibrary samples_;

    juce::Synthesiser synth_;

    std::atomic<bool> playing_ { false };
    std::atomic<bool> recording_ { false };
    std::atomic<bool> midiOnly_ { false };
    std::atomic<bool> hasRecording_ { false };

    std::atomic<float> tempo_ { 120.0f };
    std::atomic<float> x_ { 0.5f };
    std::atomic<float> y_ { 0.5f };

    std::atomic<float> bdDensity_ { 0.5f };
    std::atomic<float> sdDensity_ { 0.5f };
    std::atomic<float> hhDensity_ { 0.5f };

    std::atomic<float> bdVelocityRange_ { 0.5f };
    std::atomic<float> sdVelocityRange_ { 0.5f };
    std::atomic<float> hhVelocityRange_ { 0.5f };

    std::atomic<float> chaos_ { 0.0f };
    std::atomic<float> swing_ { 0.5f };

    std::array<float, 32> recordedX_ {};
    std::array<float, 32> recordedY_ {};
    std::array<float, 32> recordedBDDensity_ {};
    std::array<float, 32> recordedSDDensity_ {};
    std::array<float, 32> recordedHHDensity_ {};
    std::array<float, 32> recordedBDVelocity_ {};
    std::array<float, 32> recordedSDVelocity_ {};
    std::array<float, 32> recordedHHVelocity_ {};
    std::array<float, 32> recordedChaos_ {};
    std::array<float, 32> recordedSwing_ {};
    std::array<float, 32> recordedTempo_ {};

    double sampleRate_ = 44100.0;
    int samplesPerStep_ = 0;
    int samplesUntilNextStep_ = 0;
    int currentStep_ = 0;

    bool prevBD_ = false;
    bool prevSD_ = false;
    bool prevHH_ = false;

    const int midiChannel_ = 1;
    const int bdNote_ = 36;
    const int sdNote_ = 38;
    const int hhNote_ = 42;

    // CC mapping
    const int ccX_ = 16;
    const int ccY_ = 17;
    const int ccBDDensity_ = 20;
    const int ccSDDensity_ = 21;
    const int ccHHDensity_ = 22;
    const int ccBDVelocity_ = 23;
    const int ccSDVelocity_ = 24;
    const int ccHHVelocity_ = 25;
    const int ccChaos_ = 26;
    const int ccSwing_ = 27;
    const int ccTempo_ = 28;
};
