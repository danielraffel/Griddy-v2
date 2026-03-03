#include "GriddyAppMainComponent.h"
#include "Settings/SettingsManager.h"
#include <visage_graphics/font.h>
#if JUCE_IOS
#include "KeyboardDoneHelper.h"
#endif

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
    hideBpmEditor();
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

    // Dark background + cell panels (drawn before children)
    rootFrame_->onDraw() += [this](visage::Canvas& canvas) {
        float w = static_cast<float>(canvas.width());
        float h = static_cast<float>(canvas.height());
        canvas.setColor(0xff1e1e1e);
        canvas.fill(0, 0, w, h);

        // Cell panels are drawn based on stored layout rects
        visage::Font labelFont(10.0f, visage::fonts::Lato_Regular_ttf);

        // XY pad cell
        if (cellXY_.w > 0) {
            canvas.setColor(0xff2a2a2a);
            canvas.roundedRectangle(cellXY_.x, cellXY_.y, cellXY_.w, cellXY_.h, 8.0f);
            canvas.setColor(0xff333333);
            canvas.roundedRectangleBorder(cellXY_.x, cellXY_.y, cellXY_.w, cellXY_.h, 8.0f, 1.0f);
        }

        // LED matrix cell
        if (cellLED_.w > 0) {
            canvas.setColor(0xff2a2a2a);
            canvas.roundedRectangle(cellLED_.x, cellLED_.y, cellLED_.w, cellLED_.h, 8.0f);
            canvas.setColor(0xff333333);
            canvas.roundedRectangleBorder(cellLED_.x, cellLED_.y, cellLED_.w, cellLED_.h, 8.0f, 1.0f);
        }

        // Transport cell
        if (cellTransport_.w > 0) {
            canvas.setColor(0xff2a2a2a);
            canvas.roundedRectangle(cellTransport_.x, cellTransport_.y, cellTransport_.w, cellTransport_.h, 8.0f);
            canvas.setColor(0xff333333);
            canvas.roundedRectangleBorder(cellTransport_.x, cellTransport_.y, cellTransport_.w, cellTransport_.h, 8.0f, 1.0f);
        }

        // Controls cell (density + velocity + chaos/swing/reset)
        if (cellControls_.w > 0) {
            canvas.setColor(0xff2a2a2a);
            canvas.roundedRectangle(cellControls_.x, cellControls_.y, cellControls_.w, cellControls_.h, 8.0f);
            canvas.setColor(0xff333333);
            canvas.roundedRectangleBorder(cellControls_.x, cellControls_.y, cellControls_.w, cellControls_.h, 8.0f, 1.0f);

            // "Velocity" label above velocity knobs
            canvas.setColor(0xffaaaaaa);
            canvas.text("Velocity", labelFont, visage::Font::kCenter,
                        velLabelRect_.x, velLabelRect_.y, velLabelRect_.w, velLabelRect_.h);

            // "Reset" label below reset button
            canvas.setColor(0xffaaaaaa);
            canvas.text("Reset", labelFont, visage::Font::kCenter,
                        resetLabelRect_.x, resetLabelRect_.y, resetLabelRect_.w, resetLabelRect_.h);
        }
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
    recordButton_->onLongPress = [this]() {
        engine_.clearRecording();
        recordButton_->setActive(false);
    };

    auto tempoOwned = std::make_unique<TempoFrame>();
    tempoControl_ = tempoOwned.get();
    tempoControl_->setBpm(engine_.getTempo());
    tempoControl_->onValueChange = [this](float bpm) {
        engine_.setTempo(bpm);
    };
    tempoControl_->onEditRequest = [this]() {
        showBpmEditor();
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
        engine_.resetStep();
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

    float pad = 10.0f;       // Padding between cells
    float cellPad = 6.0f;    // Padding inside cells
    float x0 = safeLeft + pad;
    float y0 = safeTop + pad;
    float contentW = w - safeLeft - safeRight - pad * 2;
    float contentH = h - safeTop - safeBottom - pad * 2;

    float curY = y0;

    // ========================================
    // Cell 1: XY Pad (top, roughly square)
    // ========================================
    float xyOuterH = contentH * 0.30f;
    float xyOuterW = contentW;
    float xyInnerW = std::min(xyOuterW - cellPad * 2, xyOuterH - cellPad * 2);
    float xyInnerH = xyInnerW;
    xyOuterH = xyInnerH + cellPad * 2; // Tighten cell to content

    cellXY_ = { x0, curY, xyOuterW, xyOuterH };

    if (xyPad_)
        xyPad_->setBounds(x0 + (xyOuterW - xyInnerW) / 2.0f,
                          curY + cellPad,
                          xyInnerW, xyInnerH);

    curY += xyOuterH + pad;

    // ========================================
    // Cell 2: LED Matrix (below XY pad)
    // ========================================
    float ledOuterH = 80.0f;

    cellLED_ = { x0, curY, contentW, ledOuterH };

    if (ledMatrix_)
        ledMatrix_->setBounds(x0 + cellPad, curY + cellPad,
                              contentW - cellPad * 2, ledOuterH - cellPad * 2);

    curY += ledOuterH + pad;

    // ========================================
    // Cell 3: Transport (Play, Record, Tempo, MIDI)
    // ========================================
    float transportOuterH = 64.0f;

    cellTransport_ = { x0, curY, contentW, transportOuterH };

    float tInnerY = curY + cellPad;
    float tInnerH = transportOuterH - cellPad * 2;
    float tInnerX = x0 + cellPad;
    float tInnerW = contentW - cellPad * 2;

    float btnW = 52.0f;
    float tempoW = tInnerW - btnW * 2 - 52.0f - pad * 3;
    tempoW = std::max(80.0f, tempoW);

    if (playButton_)
        playButton_->setBounds(tInnerX, tInnerY, btnW, tInnerH);
    if (recordButton_)
        recordButton_->setBounds(tInnerX + btnW + pad, tInnerY, btnW, tInnerH);
    if (tempoControl_)
        tempoControl_->setBounds(tInnerX + btnW * 2 + pad * 2, tInnerY + 10.0f, tempoW, 32.0f);
    if (midiOnlyToggle_)
        midiOnlyToggle_->setBounds(tInnerX + tInnerW - 48.0f, tInnerY, 48.0f, tInnerH);

    curY += transportOuterH + pad;

    // ========================================
    // Cell 4: Controls (Density/Velocity columns + Chaos/Swing/Reset)
    // ========================================
    float controlsOuterH = (y0 + contentH) - curY;

    cellControls_ = { x0, curY, contentW, controlsOuterH };

    float cInnerX = x0 + cellPad;
    float cInnerY = curY + cellPad;
    float cInnerW = contentW - cellPad * 2;
    float cInnerH = controlsOuterH - cellPad * 2;

    // Left half: 3 columns — density slider, "Velocity" label, velocity knob
    float leftW = cInnerW * 0.5f;
    float densColW = leftW / 3.0f;
    float velLabelH = 14.0f;
    float densH = cInnerH * 0.55f;    // density slider takes upper portion
    float velY = cInnerY + densH + velLabelH;
    float velH = cInnerH - densH - velLabelH;  // velocity gets remainder

    if (bdDensity_)
        bdDensity_->setBounds(cInnerX, cInnerY, densColW, densH);
    if (sdDensity_)
        sdDensity_->setBounds(cInnerX + densColW, cInnerY, densColW, densH);
    if (hhDensity_)
        hhDensity_->setBounds(cInnerX + densColW * 2, cInnerY, densColW, densH);

    // "Velocity" label between density and velocity
    velLabelRect_ = { cInnerX, cInnerY + densH, leftW, velLabelH };

    if (bdVelKnob_)
        bdVelKnob_->setBounds(cInnerX, velY, densColW, velH);
    if (sdVelKnob_)
        sdVelKnob_->setBounds(cInnerX + densColW, velY, densColW, velH);
    if (hhVelKnob_)
        hhVelKnob_->setBounds(cInnerX + densColW * 2, velY, densColW, velH);

    // Right half: Chaos, Swing, Reset — tightly grouped
    float rightX = cInnerX + leftW + pad;
    float rightW = cInnerW - leftW - pad;
    float modKnobW = rightW / 2.0f;

    float resetSize = 34.0f;
    float resetLabelH = 12.0f;
    // Cap mod knob height — smaller than density to keep Reset close
    float modKnobH = std::min(densH * 0.75f, cInnerH * 0.42f);
    modKnobH = std::max(modKnobH, 60.0f);

    if (chaosKnob_)
        chaosKnob_->setBounds(rightX, cInnerY, modKnobW, modKnobH);
    if (swingKnob_)
        swingKnob_->setBounds(rightX + modKnobW, cInnerY, modKnobW, modKnobH);

    // Reset button centered right below chaos/swing with small gap
    float resetX = rightX + (rightW - resetSize) / 2.0f;
    float resetY = cInnerY + modKnobH + 4.0f;
    if (resetButton_)
        resetButton_->setBounds(resetX, resetY, resetSize, resetSize);

    // "Reset" label below button
    resetLabelRect_ = { rightX, resetY + resetSize + 1.0f, rightW, resetLabelH };

    // Request redraw to update cell backgrounds
    if (rootFrame_)
        rootFrame_->redraw();
}

void GriddyAppMainComponent::showBpmEditor()
{
    if (bpmEditor_) return; // Already showing

    // The JUCE TextEditor is invisible behind Visage's Metal UIView on iOS,
    // but it still receives keyboard input. We create it off-screen and use
    // native iOS APIs (inputAccessoryView) for the Done button + tap-to-dismiss.
    bpmEditor_ = std::make_unique<juce::TextEditor>();
    bpmEditor_->setInputRestrictions(3, "0123456789");
    bpmEditor_->setKeyboardType(juce::TextInputTarget::phoneNumberKeyboard);
    bpmEditor_->setText(juce::String((int)std::round(tempoControl_->getBpm())));
    bpmEditor_->selectAll();

    bpmEditor_->onTextChange = [this]() {
        if (!bpmEditor_) return;
        int val = bpmEditor_->getText().getIntValue();
        if (val >= (int)TempoFrame::kMinBpm && val <= (int)TempoFrame::kMaxBpm) {
            tempoControl_->setBpm(static_cast<float>(val));
            engine_.setTempo(static_cast<float>(val));
        }
    };

    bpmEditor_->onReturnKey = [this]() { hideBpmEditor(); };
    bpmEditor_->onEscapeKey = [this]() { hideBpmEditor(); };
    bpmEditor_->onFocusLost = [this]() {
        juce::MessageManager::callAsync([this]() { hideBpmEditor(); });
    };

    // Place off-screen (hidden behind Metal view anyway)
    bpmEditor_->setBounds(-200, -200, 100, 30);
    addAndMakeVisible(*bpmEditor_);
    bpmEditor_->grabKeyboardFocus();

#if JUCE_IOS
    // Install native "Done" toolbar above keyboard + tap-background-to-dismiss
    juce::MessageManager::callAsync([this]() {
        if (!bpmEditor_) return;
        installKeyboardDoneButton([this]() {
            juce::MessageManager::callAsync([this]() { hideBpmEditor(); });
        });
    });
#endif
}

void GriddyAppMainComponent::hideBpmEditor()
{
#if JUCE_IOS
    removeKeyboardDoneButton();
#endif
    if (bpmDoneBtn_) {
        removeChildComponent(bpmDoneBtn_.get());
        bpmDoneBtn_.reset();
    }
    if (bpmEditor_) {
        removeChildComponent(bpmEditor_.get());
        bpmEditor_.reset();
    }
    if (bpmScrim_) {
        bpmScrim_->removeMouseListener(this);
        removeChildComponent(bpmScrim_.get());
        bpmScrim_.reset();
    }
}

void GriddyAppMainComponent::mouseDown(const juce::MouseEvent& e)
{
    // If the BPM editor is showing and the click landed on the scrim, dismiss
    if (bpmScrim_ && e.eventComponent == bpmScrim_.get())
        hideBpmEditor();
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
