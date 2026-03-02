#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <visage_graphics/font.h>

namespace visage::fonts { extern ::visage::EmbeddedFile Lato_Regular_ttf; }

GriddyAudioProcessorEditor::GriddyAudioProcessorEditor(GriddyAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
    setSize(580, 400);
    setResizable(false, false);
    startTimer(10);
}

GriddyAudioProcessorEditor::~GriddyAudioProcessorEditor() {
    stopTimer();
    if (bridge_)
        bridge_->shutdownRendering();
    if (rootFrame_)
        rootFrame_->removeAllChildren();
    xyPad_ = nullptr;
    ledMatrix_ = nullptr;
    bdDensity_ = nullptr;
    sdDensity_ = nullptr;
    hhDensity_ = nullptr;
    chaosKnob_ = nullptr;
    swingKnob_ = nullptr;
    resetButton_ = nullptr;
    settingsButton_ = nullptr;
    settingsPanel_ = nullptr;
    bdVelKnob_ = nullptr;
    sdVelKnob_ = nullptr;
    hhVelKnob_ = nullptr;
    if (bridge_)
        bridge_->setRootFrame(nullptr);
    rootFrame_.reset();
    bridge_.reset();
}

void GriddyAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void GriddyAudioProcessorEditor::resized() {
    if (bridge_)
        bridge_->setBounds(getLocalBounds());
    if (rootFrame_) {
        rootFrame_->setBounds(0, 0, static_cast<float>(getWidth()),
                              static_cast<float>(getHeight()));
        layoutChildren();
    }
}

void GriddyAudioProcessorEditor::layoutChildren() {
    // XY pad in left panel
    if (xyPad_)
        xyPad_->setBounds(14, 14, 236, 228);

    // Chaos and Swing knobs
    if (chaosKnob_)
        chaosKnob_->setBounds(280, 20, 70, 70);
    if (swingKnob_)
        swingKnob_->setBounds(360, 20, 70, 70);

    // Reset button below knobs
    if (resetButton_)
        resetButton_->setBounds(305, 100, 50, 50);

    // Density sliders (right side of controls panel)
    if (bdDensity_)
        bdDensity_->setBounds(450, 14, 35, 150);
    if (sdDensity_)
        sdDensity_->setBounds(490, 14, 35, 150);
    if (hhDensity_)
        hhDensity_->setBounds(530, 14, 35, 150);

    // Velocity knobs below density sliders
    if (bdVelKnob_)
        bdVelKnob_->setBounds(445, 186, 45, 55);
    if (sdVelKnob_)
        sdVelKnob_->setBounds(485, 186, 45, 55);
    if (hhVelKnob_)
        hhVelKnob_->setBounds(525, 186, 45, 55);

    // LED matrix at bottom
    if (ledMatrix_)
        ledMatrix_->setBounds(14, 262, 552, 88);

    // Settings button in top-right of controls panel
    if (settingsButton_)
        settingsButton_->setBounds(550, 12, 20, 20);

    // Settings overlay covers everything
    if (settingsPanel_)
        settingsPanel_->setBounds(0, 0, static_cast<float>(getWidth()), static_cast<float>(getHeight()));
}

void GriddyAudioProcessorEditor::timerCallback() {
    if (!uiCreated_ && getWidth() > 0 && isShowing()) {
        stopTimer();
        createVisageUI();
        startTimer(33); // 30fps update polling
        return;
    }

    if (uiCreated_)
        updateUIFromProcessor();
}

void GriddyAudioProcessorEditor::createVisageUI() {
    rootFrame_ = std::make_unique<visage::Frame>();

    // Draw dark background and panels (no title - it's in the OS title bar)
    rootFrame_->onDraw() += [](visage::Canvas& canvas) {
        float w = static_cast<float>(canvas.width());
        float h = static_cast<float>(canvas.height());
        canvas.setColor(0xff1e1e1e);
        canvas.fill(0, 0, w, h);

        visage::Font labelFont(10.0f, visage::fonts::Lato_Regular_ttf);

        // Background panel behind XY pad area
        canvas.setColor(0xff2a2a2a);
        canvas.roundedRectangle(8, 8, 248, 240, 8.0f);
        canvas.setColor(0xff333333);
        canvas.roundedRectangleBorder(8, 8, 248, 240, 8.0f, 1.0f);

        // Background panel behind controls area (knobs, sliders, velocity)
        canvas.setColor(0xff2a2a2a);
        canvas.roundedRectangle(264, 8, 308, 240, 8.0f);
        canvas.setColor(0xff333333);
        canvas.roundedRectangleBorder(264, 8, 308, 240, 8.0f, 1.0f);

        // "Velocity" label above velocity knobs
        canvas.setColor(0xffaaaaaa);
        canvas.text("Velocity", labelFont, visage::Font::kCenter, 440, 170, 130, 14);

        // "Reset" label below reset button
        canvas.setColor(0xffaaaaaa);
        canvas.text("Reset", labelFont, visage::Font::kCenter, 290, 155, 80, 14);

        // Background panel behind LED matrix
        canvas.setColor(0xff2a2a2a);
        canvas.roundedRectangle(8, 256, 564, 100, 8.0f);
        canvas.setColor(0xff333333);
        canvas.roundedRectangleBorder(8, 256, 564, 100, 8.0f, 1.0f);
    };

    // Create XY Pad
    auto xyPadOwned = std::make_unique<XYPadFrame>();
    xyPad_ = xyPadOwned.get();

    // Initialize XY pad from current parameter values
    xyPad_->setX(*processorRef.parameters.getRawParameterValue("x"));
    xyPad_->setY(*processorRef.parameters.getRawParameterValue("y"));

    // Wire XY pad drag to processor parameters
    xyPad_->onValueChange = [this](float x, float y) {
        if (auto* paramX = processorRef.parameters.getParameter("x"))
            paramX->setValueNotifyingHost(paramX->getNormalisableRange().convertTo0to1(x));
        if (auto* paramY = processorRef.parameters.getParameter("y"))
            paramY->setValueNotifyingHost(paramY->getNormalisableRange().convertTo0to1(y));
    };

    // Create Chaos knob
    auto chaosOwned = std::make_unique<RotaryKnobFrame>("Chaos", 0xffff8833);
    chaosKnob_ = chaosOwned.get();
    chaosKnob_->setValue(*processorRef.parameters.getRawParameterValue("chaos"));
    chaosKnob_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("chaos"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    // Create Swing knob
    auto swingOwned = std::make_unique<RotaryKnobFrame>("Swing", 0xff33aaff);
    swingKnob_ = swingOwned.get();
    swingKnob_->setValue(*processorRef.parameters.getRawParameterValue("swing"));
    swingKnob_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("swing"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    // Create Reset button
    auto resetOwned = std::make_unique<ResetButtonFrame>();
    resetButton_ = resetOwned.get();
    resetButton_->onPress = [this]() {
        if (auto* param = processorRef.parameters.getParameter("reset"))
            param->setValueNotifyingHost(1.0f);
    };

    // Create Density sliders (BD=red, SD=green, HH=yellow)
    auto bdOwned = std::make_unique<DensitySliderFrame>("BD", 0xffff4444);
    bdDensity_ = bdOwned.get();
    bdDensity_->setValue(*processorRef.parameters.getRawParameterValue("density_1_bd"));
    bdDensity_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("density_1_bd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    auto sdOwned = std::make_unique<DensitySliderFrame>("SD", 0xff44ff44);
    sdDensity_ = sdOwned.get();
    sdDensity_->setValue(*processorRef.parameters.getRawParameterValue("density_2_sd"));
    sdDensity_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("density_2_sd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    auto hhOwned = std::make_unique<DensitySliderFrame>("HH", 0xffffff44);
    hhDensity_ = hhOwned.get();
    hhDensity_->setValue(*processorRef.parameters.getRawParameterValue("density_3_hh"));
    hhDensity_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("density_3_hh"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    // Create Velocity knobs (below density sliders)
    auto bdVelOwned = std::make_unique<RotaryKnobFrame>("BD", 0xffff4444);
    bdVelKnob_ = bdVelOwned.get();
    bdVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_1_bd"));
    bdVelKnob_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("velocity_1_bd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    auto sdVelOwned = std::make_unique<RotaryKnobFrame>("SD", 0xff44ff44);
    sdVelKnob_ = sdVelOwned.get();
    sdVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_2_sd"));
    sdVelKnob_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("velocity_2_sd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    auto hhVelOwned = std::make_unique<RotaryKnobFrame>("HH", 0xffffff44);
    hhVelKnob_ = hhVelOwned.get();
    hhVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_3_hh"));
    hhVelKnob_->onValueChange = [this](float v) {
        if (auto* param = processorRef.parameters.getParameter("velocity_3_hh"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(v));
    };

    // Create Settings button
    auto settingsOwned = std::make_unique<SettingsButtonFrame>();
    settingsButton_ = settingsOwned.get();

    // Create Settings panel overlay
    auto settingsPanelOwned = std::make_unique<SettingsPanelFrame>();
    settingsPanel_ = settingsPanelOwned.get();
    settingsPanel_->setVisible(false); // Start hidden so it doesn't intercept mouse events

    // Initialize settings panel from current parameter values
    settingsPanel_->setMidiChannel(static_cast<int>(*processorRef.parameters.getRawParameterValue("midi_channel")));
    settingsPanel_->setBDNote(static_cast<int>(*processorRef.parameters.getRawParameterValue("note_1_bd")));
    settingsPanel_->setSDNote(static_cast<int>(*processorRef.parameters.getRawParameterValue("note_2_sd")));
    settingsPanel_->setHHNote(static_cast<int>(*processorRef.parameters.getRawParameterValue("note_3_hh")));
    settingsPanel_->setMidiThru(*processorRef.parameters.getRawParameterValue("midi_thru") > 0.5f);
    settingsPanel_->setLiveMode(*processorRef.parameters.getRawParameterValue("live_mode") > 0.5f);
    settingsPanel_->setResetMode(static_cast<int>(*processorRef.parameters.getRawParameterValue("reset_mode")));

    // Wire settings panel callbacks to processor parameters
    settingsPanel_->onMidiChannelChange = [this](int ch) {
        if (auto* param = processorRef.parameters.getParameter("midi_channel"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(ch)));
    };
    settingsPanel_->onBDNoteChange = [this](int note) {
        if (auto* param = processorRef.parameters.getParameter("note_1_bd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(note)));
    };
    settingsPanel_->onSDNoteChange = [this](int note) {
        if (auto* param = processorRef.parameters.getParameter("note_2_sd"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(note)));
    };
    settingsPanel_->onHHNoteChange = [this](int note) {
        if (auto* param = processorRef.parameters.getParameter("note_3_hh"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(note)));
    };
    settingsPanel_->onMidiThruChange = [this](bool v) {
        if (auto* param = processorRef.parameters.getParameter("midi_thru"))
            param->setValueNotifyingHost(v ? 1.0f : 0.0f);
    };
    settingsPanel_->onLiveModeChange = [this](bool v) {
        if (auto* param = processorRef.parameters.getParameter("live_mode"))
            param->setValueNotifyingHost(v ? 1.0f : 0.0f);
    };
    settingsPanel_->onResetModeChange = [this](int mode) {
        if (auto* param = processorRef.parameters.getParameter("reset_mode"))
            param->setValueNotifyingHost(param->getNormalisableRange().convertTo0to1(static_cast<float>(mode)));
    };
    settingsPanel_->onMidiLearnStart = [this]() {
        processorRef.startMidiLearnForReset();
    };
    settingsPanel_->onMidiLearnStop = [this]() {
        processorRef.stopMidiLearn();
    };

    // Wire settings button to toggle the panel
    settingsButton_->onPress = [this]() {
        if (settingsPanel_)
            settingsPanel_->toggleVisible();
    };

    // Create LED Matrix
    auto ledMatrixOwned = std::make_unique<LEDMatrixFrame>();
    ledMatrix_ = ledMatrixOwned.get();

    // Add children to root frame (settings panel last so it draws on top)
    rootFrame_->addChild(xyPadOwned.release());
    rootFrame_->addChild(chaosOwned.release());
    rootFrame_->addChild(swingOwned.release());
    rootFrame_->addChild(resetOwned.release());
    rootFrame_->addChild(bdOwned.release());
    rootFrame_->addChild(sdOwned.release());
    rootFrame_->addChild(hhOwned.release());
    rootFrame_->addChild(bdVelOwned.release());
    rootFrame_->addChild(sdVelOwned.release());
    rootFrame_->addChild(hhVelOwned.release());
    rootFrame_->addChild(settingsOwned.release());
    rootFrame_->addChild(ledMatrixOwned.release());
    rootFrame_->addChild(settingsPanelOwned.release());

    // Native macOS title bar for standalone mode
    if (auto* window = findParentComponentOfClass<juce::DocumentWindow>())
        window->setUsingNativeTitleBar(true);

    // Set up bridge
    bridge_ = std::make_unique<JuceVisageBridge>();
    addAndMakeVisible(*bridge_);
    bridge_->setBounds(getLocalBounds());
    bridge_->setRootFrame(rootFrame_.get());
    bridge_->startTimer(10);

    uiCreated_ = true;

    // Initial update
    updateUIFromProcessor();
}

void GriddyAudioProcessorEditor::updateUIFromProcessor() {
    auto& engine = processorRef.getGridsEngine();

    // Read current parameter values
    float paramX = *processorRef.parameters.getRawParameterValue("x");
    float paramY = *processorRef.parameters.getRawParameterValue("y");
    float bdDensity = *processorRef.parameters.getRawParameterValue("density_1_bd");
    float sdDensity = *processorRef.parameters.getRawParameterValue("density_2_sd");
    float hhDensity = *processorRef.parameters.getRawParameterValue("density_3_hh");

    // Ensure engine has latest values for pattern generation (UI thread sync)
    engine.setX(paramX);
    engine.setY(paramY);
    engine.setBDDensity(bdDensity);
    engine.setSDDensity(sdDensity);
    engine.setHHDensity(hhDensity);

    // Update XY pad
    if (xyPad_) {
        xyPad_->setX(paramX);
        xyPad_->setY(paramY);
    }

    if (bdDensity_) bdDensity_->setValue(bdDensity);
    if (sdDensity_) sdDensity_->setValue(sdDensity);
    if (hhDensity_) hhDensity_->setValue(hhDensity);

    // Update knobs
    if (chaosKnob_)
        chaosKnob_->setValue(*processorRef.parameters.getRawParameterValue("chaos"));
    if (swingKnob_)
        swingKnob_->setValue(*processorRef.parameters.getRawParameterValue("swing"));

    // Update velocity knobs
    if (bdVelKnob_)
        bdVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_1_bd"));
    if (sdVelKnob_)
        sdVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_2_sd"));
    if (hhVelKnob_)
        hhVelKnob_->setValue(*processorRef.parameters.getRawParameterValue("velocity_3_hh"));

    // Update settings panel MIDI learn state
    if (settingsPanel_) {
        settingsPanel_->setMidiLearnActive(processorRef.isMidiLearning());
        settingsPanel_->setResetMidiCC(processorRef.getResetMidiCC());
    }

    // Update reset button glow and LED animation
    if (processorRef.hasResetOccurred()) {
        bool retrigger = processorRef.wasResetRetrigger();
        if (resetButton_) resetButton_->setGlow(true);
        if (ledMatrix_) ledMatrix_->triggerResetAnimation(retrigger);
    }

    // Update LED matrix
    if (ledMatrix_) {
        ledMatrix_->setDensities(bdDensity, sdDensity, hhDensity);
        ledMatrix_->setPatterns(engine.getBDPattern(), engine.getSDPattern(), engine.getHHPattern());
        ledMatrix_->setCurrentStep(engine.getCurrentStep());
    }
}
