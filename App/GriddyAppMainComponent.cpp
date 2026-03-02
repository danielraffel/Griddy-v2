#include "GriddyAppMainComponent.h"
#include "Settings/SettingsManager.h"
#include <visage_graphics/font.h>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

GriddyAppMainComponent::GriddyAppMainComponent()
{
    engine_.setX(0.5f);
    engine_.setY(0.5f);
    engine_.setBDDensity(0.5f);
    engine_.setSDDensity(0.5f);
    engine_.setHHDensity(0.5f);
    engine_.setBDVelocityRange(0.5f);
    engine_.setSDVelocityRange(0.5f);
    engine_.setHHVelocityRange(0.5f);
    engine_.setChaos(0.0f);
    engine_.setSwing(0.5f);

    // Defer audio setup
    juce::Component::SafePointer<GriddyAppMainComponent> safeThis(this);
    juce::MessageManager::callAsync([safeThis] {
        if (safeThis != nullptr)
            safeThis->setAudioChannels(0, 2);
    });

    startTimer(10);
    setSize(375, 780);
}

GriddyAppMainComponent::~GriddyAppMainComponent()
{
    stopTimer();
    shutdownAudio();

    if (bridge_)
        bridge_->shutdownRendering();
    if (rootFrame_)
        rootFrame_->removeAllChildren();

    // Clear frame pointers
    xyPad_ = nullptr;
    ledMatrix_ = nullptr;
    bdDensity_ = nullptr;
    sdDensity_ = nullptr;
    hhDensity_ = nullptr;
    chaosKnob_ = nullptr;
    swingKnob_ = nullptr;
    bdVelKnob_ = nullptr;
    sdVelKnob_ = nullptr;
    hhVelKnob_ = nullptr;
    resetButton_ = nullptr;
    playButton_ = nullptr;
    recordButton_ = nullptr;
    tempoControl_ = nullptr;
    midiOnlyToggle_ = nullptr;

    if (bridge_)
        bridge_->setRootFrame(nullptr);
    rootFrame_.reset();
    bridge_.reset();
}

void GriddyAppMainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    engine_.prepare(sampleRate, samplesPerBlockExpected, 2);
}

void GriddyAppMainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    engine_.getNextAudioBlock(bufferToFill);
}

void GriddyAppMainComponent::releaseResources()
{
    engine_.releaseResources();
}

void GriddyAppMainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void GriddyAppMainComponent::resized()
{
    if (bridge_)
        bridge_->setBounds(getLocalBounds());
    if (rootFrame_) {
        rootFrame_->setBounds(0, 0, static_cast<float>(getWidth()),
                              static_cast<float>(getHeight()));
        layoutChildren();
    }
}

void GriddyAppMainComponent::timerCallback()
{
    if (!uiCreated_ && getWidth() > 0 && isShowing()) {
        stopTimer();
        createVisageUI();
        startTimer(33); // 30fps update polling
        return;
    }

    if (uiCreated_)
        updateFromEngine();
}

void GriddyAppMainComponent::createVisageUI()
{
    rootFrame_ = std::make_unique<visage::Frame>();

    // Dark background
    rootFrame_->onDraw() += [](visage::Canvas& canvas) {
        float w = static_cast<float>(canvas.width());
        float h = static_cast<float>(canvas.height());
        canvas.setColor(0xff1e1e1e);
        canvas.fill(0, 0, w, h);
    };

    // --- XY Pad ---
    auto xyPadOwned = std::make_unique<XYPadFrame>();
    xyPad_ = xyPadOwned.get();
    xyPad_->setX(engine_.getX());
    xyPad_->setY(engine_.getY());
    xyPad_->onValueChange = [this](float x, float y) {
        engine_.setX(x);
        engine_.setY(y);
    };

    // --- Transport ---
    auto playOwned = std::make_unique<TransportButtonFrame>("Play",
        TransportButtonFrame::Play, 0xff44cc44);
    playButton_ = playOwned.get();
    playButton_->onPress = [this]() {
        bool play = !engine_.isPlaying();
        engine_.setPlaying(play);
        playButton_->setActive(play);
    };

    auto recordOwned = std::make_unique<TransportButtonFrame>("Record",
        TransportButtonFrame::Record, 0xffff4444);
    recordButton_ = recordOwned.get();
    recordButton_->onPress = [this]() {
        bool rec = !engine_.isRecording();
        engine_.setRecording(rec);
        recordButton_->setActive(rec);
    };

    auto tempoOwned = std::make_unique<TempoFrame>();
    tempoControl_ = tempoOwned.get();
    tempoControl_->setBpm(engine_.getTempo());
    tempoControl_->onValueChange = [this](float bpm) {
        engine_.setTempo(bpm);
    };

    auto midiToggleOwned = std::make_unique<ToggleFrame>("MIDI Only", 0xff58a8d0);
    midiOnlyToggle_ = midiToggleOwned.get();
    midiOnlyToggle_->onToggle = [this](bool on) {
        engine_.setMidiOnly(on);
    };

    // --- Density sliders ---
    auto bdDensOwned = std::make_unique<DensitySliderFrame>("BD", 0xffff4444);
    bdDensity_ = bdDensOwned.get();
    bdDensity_->setValue(engine_.getBDDensity());
    bdDensity_->onValueChange = [this](float v) { engine_.setBDDensity(v); };

    auto sdDensOwned = std::make_unique<DensitySliderFrame>("SD", 0xff44ff44);
    sdDensity_ = sdDensOwned.get();
    sdDensity_->setValue(engine_.getSDDensity());
    sdDensity_->onValueChange = [this](float v) { engine_.setSDDensity(v); };

    auto hhDensOwned = std::make_unique<DensitySliderFrame>("HH", 0xffffff44);
    hhDensity_ = hhDensOwned.get();
    hhDensity_->setValue(engine_.getHHDensity());
    hhDensity_->onValueChange = [this](float v) { engine_.setHHDensity(v); };

    // --- Velocity knobs ---
    auto bdVelOwned = std::make_unique<RotaryKnobFrame>("BD Vel", 0xffff4444);
    bdVelKnob_ = bdVelOwned.get();
    bdVelKnob_->setValue(engine_.getBDVelocityRange());
    bdVelKnob_->onValueChange = [this](float v) { engine_.setBDVelocityRange(v); };

    auto sdVelOwned = std::make_unique<RotaryKnobFrame>("SD Vel", 0xff44ff44);
    sdVelKnob_ = sdVelOwned.get();
    sdVelKnob_->setValue(engine_.getSDVelocityRange());
    sdVelKnob_->onValueChange = [this](float v) { engine_.setSDVelocityRange(v); };

    auto hhVelOwned = std::make_unique<RotaryKnobFrame>("HH Vel", 0xffffff44);
    hhVelKnob_ = hhVelOwned.get();
    hhVelKnob_->setValue(engine_.getHHVelocityRange());
    hhVelKnob_->onValueChange = [this](float v) { engine_.setHHVelocityRange(v); };

    // --- Chaos, Swing, Reset ---
    auto chaosOwned = std::make_unique<RotaryKnobFrame>("Chaos", 0xffff8833);
    chaosKnob_ = chaosOwned.get();
    chaosKnob_->setValue(engine_.getChaos());
    chaosKnob_->onValueChange = [this](float v) { engine_.setChaos(v); };

    auto swingOwned = std::make_unique<RotaryKnobFrame>("Swing", 0xff33aaff);
    swingKnob_ = swingOwned.get();
    swingKnob_->setValue(engine_.getSwing());
    swingKnob_->onValueChange = [this](float v) { engine_.setSwing(v); };

    auto resetOwned = std::make_unique<ResetButtonFrame>();
    resetButton_ = resetOwned.get();
    resetButton_->onPress = [this]() {
        engine_.getGridsEngine().reset();
    };

    // --- LED Matrix ---
    auto ledOwned = std::make_unique<LEDMatrixFrame>();
    ledMatrix_ = ledOwned.get();

    // Add all children to root frame
    rootFrame_->addChild(xyPadOwned.release());
    rootFrame_->addChild(playOwned.release());
    rootFrame_->addChild(recordOwned.release());
    rootFrame_->addChild(tempoOwned.release());
    rootFrame_->addChild(midiToggleOwned.release());
    rootFrame_->addChild(bdDensOwned.release());
    rootFrame_->addChild(sdDensOwned.release());
    rootFrame_->addChild(hhDensOwned.release());
    rootFrame_->addChild(bdVelOwned.release());
    rootFrame_->addChild(sdVelOwned.release());
    rootFrame_->addChild(hhVelOwned.release());
    rootFrame_->addChild(chaosOwned.release());
    rootFrame_->addChild(swingOwned.release());
    rootFrame_->addChild(resetOwned.release());
    rootFrame_->addChild(ledOwned.release());

    // Set up bridge
    bridge_ = std::make_unique<JuceVisageBridge>();
    addAndMakeVisible(*bridge_);
    bridge_->setBounds(getLocalBounds());
    bridge_->setRootFrame(rootFrame_.get());
    bridge_->startTimer(10);

    uiCreated_ = true;

    // Layout and initial update
    layoutChildren();
    updateFromEngine();
}

void GriddyAppMainComponent::layoutChildren()
{
    float w = static_cast<float>(getWidth());
    float h = static_cast<float>(getHeight());

    // Apply safe area insets on iOS
    float safeTop = 0, safeBottom = 0, safeLeft = 0, safeRight = 0;
#if JUCE_IOS
    if (auto* display = juce::Desktop::getInstance().getDisplays()
            .getDisplayForRect(getScreenBounds())) {
        auto insets = display->safeAreaInsets;
        safeTop = static_cast<float>(insets.getTop());
        safeBottom = static_cast<float>(insets.getBottom());
        safeLeft = static_cast<float>(insets.getLeft());
        safeRight = static_cast<float>(insets.getRight());
    }
#endif

    float pad = 12.0f;
    float x0 = safeLeft + pad;
    float y0 = safeTop + pad;
    float contentW = w - safeLeft - safeRight - pad * 2;
    float contentH = h - safeTop - safeBottom - pad * 2;

    // Responsive layout for portrait mode
    // XY pad: ~35% of content height
    float xyH = contentH * 0.32f;
    float xyW = std::min(contentW, xyH); // Keep roughly square
    float xyX = x0 + (contentW - xyW) / 2.0f;
    if (xyPad_)
        xyPad_->setBounds(xyX, y0, xyW, xyH);

    float curY = y0 + xyH + 8.0f;

    // Transport row: Play, Record, Tempo, MIDI toggle
    float transportH = 56.0f;
    float btnW = 56.0f;
    float tempoW = contentW - btnW * 2 - 52.0f - pad * 3;
    tempoW = std::max(80.0f, tempoW);

    if (playButton_)
        playButton_->setBounds(x0, curY, btnW, transportH);
    if (recordButton_)
        recordButton_->setBounds(x0 + btnW + pad, curY, btnW, transportH);
    if (tempoControl_)
        tempoControl_->setBounds(x0 + btnW * 2 + pad * 2, curY + 12.0f, tempoW, 32.0f);
    if (midiOnlyToggle_)
        midiOnlyToggle_->setBounds(x0 + contentW - 48.0f, curY, 48.0f, transportH);

    curY += transportH + 8.0f;

    // Density sliders + Velocity knobs side by side
    float remainingH = (y0 + contentH) - curY;
    float ledH = std::min(80.0f, remainingH * 0.25f);
    float controlsH = remainingH - ledH - 8.0f;
    float sliderH = controlsH * 0.55f;
    float knobH = controlsH * 0.45f;

    // Density sliders (3 columns, left half)
    float colW = contentW / 6.0f;
    if (bdDensity_)
        bdDensity_->setBounds(x0, curY, colW, sliderH);
    if (sdDensity_)
        sdDensity_->setBounds(x0 + colW, curY, colW, sliderH);
    if (hhDensity_)
        hhDensity_->setBounds(x0 + colW * 2, curY, colW, sliderH);

    // Chaos, Swing, Reset (right half, upper)
    float rightX = x0 + colW * 3 + pad;
    float rightW = contentW - colW * 3 - pad;
    float modKnobW = rightW / 3.0f;
    if (chaosKnob_)
        chaosKnob_->setBounds(rightX, curY, modKnobW, sliderH * 0.6f);
    if (swingKnob_)
        swingKnob_->setBounds(rightX + modKnobW, curY, modKnobW, sliderH * 0.6f);
    if (resetButton_)
        resetButton_->setBounds(rightX + modKnobW * 2 + (modKnobW - 34.0f) / 2.0f,
                                curY + 8.0f, 34.0f, 34.0f);

    curY += sliderH;

    // Velocity knobs row
    float velKnobW = contentW / 6.0f;
    if (bdVelKnob_)
        bdVelKnob_->setBounds(x0, curY, velKnobW, knobH);
    if (sdVelKnob_)
        sdVelKnob_->setBounds(x0 + velKnobW, curY, velKnobW, knobH);
    if (hhVelKnob_)
        hhVelKnob_->setBounds(x0 + velKnobW * 2, curY, velKnobW, knobH);

    curY += knobH;

    // LED matrix at bottom
    if (ledMatrix_)
        ledMatrix_->setBounds(x0, curY, contentW, ledH);
}

void GriddyAppMainComponent::updateFromEngine()
{
    auto& grids = engine_.getGridsEngine();
    grids.setX(engine_.getX());
    grids.setY(engine_.getY());
    grids.setBDDensity(engine_.getBDDensity());
    grids.setSDDensity(engine_.getSDDensity());
    grids.setHHDensity(engine_.getHHDensity());

    // Update XY pad
    if (xyPad_) {
        xyPad_->setX(engine_.getX());
        xyPad_->setY(engine_.getY());
    }

    // Update density sliders
    if (bdDensity_) bdDensity_->setValue(engine_.getBDDensity());
    if (sdDensity_) sdDensity_->setValue(engine_.getSDDensity());
    if (hhDensity_) hhDensity_->setValue(engine_.getHHDensity());

    // Update velocity knobs
    if (bdVelKnob_) bdVelKnob_->setValue(engine_.getBDVelocityRange());
    if (sdVelKnob_) sdVelKnob_->setValue(engine_.getSDVelocityRange());
    if (hhVelKnob_) hhVelKnob_->setValue(engine_.getHHVelocityRange());

    // Update modulation knobs
    if (chaosKnob_) chaosKnob_->setValue(engine_.getChaos());
    if (swingKnob_) swingKnob_->setValue(engine_.getSwing());

    // Update transport state
    if (playButton_) playButton_->setActive(engine_.isPlaying());
    if (recordButton_) recordButton_->setActive(engine_.isRecording());
    if (tempoControl_) tempoControl_->setBpm(engine_.getTempo());
    if (midiOnlyToggle_) midiOnlyToggle_->setActive(engine_.isMidiOnly());

    // Playback recording
    if (engine_.hasRecording() && !engine_.isRecording() && engine_.isPlaying()) {
        // Engine updates values during recording playback — UI reads them
    }

    // Update LED matrix
    if (ledMatrix_) {
        ledMatrix_->setDensities(engine_.getBDDensity(), engine_.getSDDensity(),
                                 engine_.getHHDensity());
        ledMatrix_->setVelocityRanges(engine_.getBDVelocityRange(),
                                      engine_.getSDVelocityRange(),
                                      engine_.getHHVelocityRange());
        ledMatrix_->setPatterns(grids.getBDPattern(), grids.getSDPattern(),
                                grids.getHHPattern());
        ledMatrix_->setCurrentStep(grids.getCurrentStep());
    }
}
