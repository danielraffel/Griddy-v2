#include "GriddyAppMainComponent.h"
#include "Settings/SettingsManager.h"

namespace
{
constexpr int kSectionLabelHeight = 14;
}

GriddyAppMainComponent::GriddyAppMainComponent()
{
    addAndMakeVisible(xyPad_);
    xyPad_.onValueChange = [this](float x, float y) {
        engine_.setX(x);
        engine_.setY(y);
    };
    engine_.setX(0.5f);
    engine_.setY(0.5f);

    tempoSlider_.setSliderStyle(juce::Slider::LinearHorizontal);
    tempoSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    tempoSlider_.setRange(GriddyAppEngine::kMinTempoBpm, GriddyAppEngine::kMaxTempoBpm, 1.0);
    tempoSlider_.setValue(engine_.getTempo());
    engine_.setTempo((float)tempoSlider_.getValue());
    tempoSlider_.onValueChange = [this] {
        engine_.setTempo((float)tempoSlider_.getValue());
        if (!tempoEditor_.hasKeyboardFocus(true))
            tempoEditor_.setText(juce::String((int) tempoSlider_.getValue()), juce::dontSendNotification);
        updateLabels();
    };
    addAndMakeVisible(tempoSlider_);

    tempoEditor_.setMultiLine(false);
    tempoEditor_.setReturnKeyStartsNewLine(false);
    tempoEditor_.setInputRestrictions(3, "0123456789");
    tempoEditor_.setKeyboardType(juce::TextInputTarget::phoneNumberKeyboard);
    tempoEditor_.setSelectAllWhenFocused(true);
    tempoEditor_.setJustification(juce::Justification::centred);
    tempoEditor_.setText(juce::String((int) tempoSlider_.getValue()), juce::dontSendNotification);
    tempoEditor_.applyFontToAllText(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    tempoEditor_.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff0f1114));
    tempoEditor_.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    tempoEditor_.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff7c8793));
    tempoEditor_.setColour(juce::TextEditor::focusedOutlineColourId, juce::Colour(0xff58a8d0));
    tempoEditor_.setColour(juce::CaretComponent::caretColourId, juce::Colours::white);
    tempoEditor_.onTextChange = [this]
    {
        const auto text = tempoEditor_.getText().trim();
        if (text.isEmpty())
            return;

        const int typedTempo = text.getIntValue();
        const int clampedTempo = juce::jlimit((int) GriddyAppEngine::kMinTempoBpm,
                                              (int) GriddyAppEngine::kMaxTempoBpm,
                                              typedTempo);
        tempoSlider_.setValue(clampedTempo, juce::sendNotificationSync);
    };
    tempoEditor_.onReturnKey = [this]
    {
        const auto text = tempoEditor_.getText().trim();
        auto newTempo = text.isNotEmpty()
            ? juce::jlimit((int) GriddyAppEngine::kMinTempoBpm,
                           (int) GriddyAppEngine::kMaxTempoBpm,
                           text.getIntValue())
            : (int) tempoSlider_.getValue();
        tempoSlider_.setValue(newTempo, juce::sendNotificationSync);
        tempoEditor_.setText(juce::String(newTempo), juce::dontSendNotification);
        tempoEditor_.giveAwayKeyboardFocus();
    };
    tempoEditor_.onFocusLost = [this]
    {
        const auto text = tempoEditor_.getText().trim();
        auto newTempo = text.isNotEmpty()
            ? juce::jlimit((int) GriddyAppEngine::kMinTempoBpm,
                           (int) GriddyAppEngine::kMaxTempoBpm,
                           text.getIntValue())
            : (int) tempoSlider_.getValue();
        tempoSlider_.setValue(newTempo, juce::sendNotificationSync);
        tempoEditor_.setText(juce::String(newTempo), juce::dontSendNotification);
    };
    addAndMakeVisible(tempoEditor_);

    tempoUnitLabel_.setText("BPM", juce::dontSendNotification);
    tempoUnitLabel_.setJustificationType(juce::Justification::centredLeft);
    tempoUnitLabel_.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    tempoUnitLabel_.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.72f));
    addAndMakeVisible(tempoUnitLabel_);

    auto configureDensity = [this](juce::Slider& slider, const juce::String& name, std::function<void(float)> setter) {
        slider.setSliderStyle(juce::Slider::LinearVertical);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(0.0, 1.0, 0.01);
        slider.onValueChange = [setter, &slider] { setter((float)slider.getValue()); };
        slider.setName(name);
        addAndMakeVisible(slider);
    };

    configureDensity(bdDensity_, "BD", [this](float v){ engine_.setBDDensity(v); });
    configureDensity(sdDensity_, "SD", [this](float v){ engine_.setSDDensity(v); });
    configureDensity(hhDensity_, "HH", [this](float v){ engine_.setHHDensity(v); });

    bdDensity_.setValue(0.5, juce::dontSendNotification);
    sdDensity_.setValue(0.5, juce::dontSendNotification);
    hhDensity_.setValue(0.5, juce::dontSendNotification);
    engine_.setBDDensity(0.5f);
    engine_.setSDDensity(0.5f);
    engine_.setHHDensity(0.5f);

    auto configureVelocity = [this](juce::Slider& slider, const juce::String& name, std::function<void(float)> setter) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
        slider.setRange(0.0, 1.0, 0.01);
        slider.onValueChange = [setter, &slider] { setter((float)slider.getValue()); };
        slider.setName(name);
        addAndMakeVisible(slider);
    };

    configureVelocity(bdVelocity_, "BD Vel", [this](float v){ engine_.setBDVelocityRange(v); });
    configureVelocity(sdVelocity_, "SD Vel", [this](float v){ engine_.setSDVelocityRange(v); });
    configureVelocity(hhVelocity_, "HH Vel", [this](float v){ engine_.setHHVelocityRange(v); });

    bdVelocity_.setValue(0.5, juce::dontSendNotification);
    sdVelocity_.setValue(0.5, juce::dontSendNotification);
    hhVelocity_.setValue(0.5, juce::dontSendNotification);
    engine_.setBDVelocityRange(0.5f);
    engine_.setSDVelocityRange(0.5f);
    engine_.setHHVelocityRange(0.5f);

    chaosSlider_.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    chaosSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    chaosSlider_.setRange(0.0, 1.0, 0.01);
    chaosSlider_.setValue(0.0, juce::dontSendNotification);
    engine_.setChaos(0.0f);
    chaosSlider_.onValueChange = [this] { engine_.setChaos((float)chaosSlider_.getValue()); };
    addAndMakeVisible(chaosSlider_);

    swingSlider_.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    swingSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    swingSlider_.setRange(0.0, 1.0, 0.01);
    swingSlider_.setValue(0.5);
    engine_.setSwing(0.5f);
    swingSlider_.onValueChange = [this] { engine_.setSwing((float)swingSlider_.getValue()); };
    addAndMakeVisible(swingSlider_);

    playButton_.onClick = [this] {
        bool nowPlaying = !engine_.isPlaying();
        engine_.setPlaying(nowPlaying);
        playButton_.setButtonText(nowPlaying ? "Stop" : "Play");
    };
    addAndMakeVisible(playButton_);

    recordButton_.onClick = [this] {
        bool nowRecording = !engine_.isRecording();
        engine_.setRecording(nowRecording);
        recordButton_.setButtonText(nowRecording ? "Recording" : "Record");
    };
    addAndMakeVisible(recordButton_);

    midiOnlyToggle_.onClick = [this] {
        engine_.setMidiOnly(midiOnlyToggle_.getToggleState());
    };
    addAndMakeVisible(midiOnlyToggle_);

    configureSectionLabel(densityGroupLabel_, "DENSITY");
    configureSectionLabel(velocityGroupLabel_, "VELOCITY");
    configureSectionLabel(chaosGroupLabel_, "CHAOS");
    configureSectionLabel(swingGroupLabel_, "SWING");

    statusLabel_.setJustificationType(juce::Justification::centred);
    statusLabel_.setColour(juce::Label::textColourId, juce::Colours::white);
    statusLabel_.setColour(juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    addAndMakeVisible(statusLabel_);

    updateLabels();

    juce::Component::SafePointer<GriddyAppMainComponent> safeThis(this);
    juce::MessageManager::callAsync([safeThis]
    {
        if (safeThis != nullptr)
            safeThis->setAudioChannels(0, 2);
    });

    addMouseListener(this, true);
    startTimerHz(15);
    setSize(375, 780);
}

GriddyAppMainComponent::~GriddyAppMainComponent()
{
    removeMouseListener(this);
    shutdownAudio();
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
    g.fillAll(juce::Colour(0xff111111));

    // Draw LED matrix in its allocated bounds
    if (!ledMatrixBounds_.isEmpty())
        drawLEDMatrix(g, ledMatrixBounds_);
}

void GriddyAppMainComponent::drawLEDMatrix(juce::Graphics& g, juce::Rectangle<int> bounds)
{
    // Background panel
    g.setColour(juce::Colour(0xff0a0a0a));
    g.fillRoundedRectangle(bounds.toFloat(), 6.0f);
    g.setColour(juce::Colour(0xff202020));
    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f), 6.0f, 1.0f);

    float w = (float)bounds.getWidth();
    float h = (float)bounds.getHeight();
    float ox = (float)bounds.getX();
    float oy = (float)bounds.getY();

    float labelWidth = 22.0f;
    float startX = ox + labelWidth + 6.0f;
    float rightPad = 6.0f;
    float groupGap = 5.0f;
    float totalWidth = w - labelWidth - 6.0f - rightPad;
    float usableWidth = totalWidth - 3.0f * groupGap;
    float stepSpacing = usableWidth / 32.0f;
    float ledSize = std::min(stepSpacing * 0.8f, 7.0f);
    float rowSpacing = h / 4.0f;

    float bdDensity = (float)bdDensity_.getValue();
    float sdDensity = (float)sdDensity_.getValue();
    float hhDensity = (float)hhDensity_.getValue();
    uint8_t bdThresh = (uint8_t)(255 * (1.0f - bdDensity));
    uint8_t sdThresh = (uint8_t)(255 * (1.0f - sdDensity));
    uint8_t hhThresh = (uint8_t)(255 * (1.0f - hhDensity));

    // Row labels
    g.setFont(juce::Font(juce::FontOptions(8.0f)));
    g.setColour(juce::Colour(0xffcc2222));
    g.drawText("BD", (int)ox + 2, (int)(oy + rowSpacing * 1 - 5), (int)labelWidth, 10, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xff22cc22));
    g.drawText("SD", (int)ox + 2, (int)(oy + rowSpacing * 2 - 5), (int)labelWidth, 10, juce::Justification::centredLeft);
    g.setColour(juce::Colour(0xffcccc22));
    g.drawText("HH", (int)ox + 2, (int)(oy + rowSpacing * 3 - 5), (int)labelWidth, 10, juce::Justification::centredLeft);

    // Bar numbers and separators
    g.setFont(juce::Font(juce::FontOptions(7.0f)));
    const char* barNums[] = { "1", "2", "3", "4" };
    for (int gr = 0; gr < 4; ++gr) {
        float groupStartX = startX + gr * (8 * stepSpacing + groupGap);
        g.setColour(juce::Colour(0xff555555));
        g.drawText(barNums[gr], (int)groupStartX, (int)oy + 1, (int)(8 * stepSpacing), 8, juce::Justification::centred);

        if (gr > 0) {
            float sepX = groupStartX - groupGap / 2.0f;
            g.setColour(juce::Colour(0x25ffffff));
            g.drawVerticalLine((int)sepX, oy + rowSpacing * 0.6f, oy + rowSpacing * 3 + ledSize);
        }
    }

    // Draw LEDs
    for (int step = 0; step < 32; ++step) {
        int group = step / 8;
        int stepInGroup = step % 8;
        float lx = startX + group * (8 * stepSpacing + groupGap)
                  + stepInGroup * stepSpacing + (stepSpacing - ledSize) / 2;

        bool bdOn = bdPattern_[step] > bdThresh;
        bool sdOn = sdPattern_[step] > sdThresh;
        bool hhOn = hhPattern_[step] > hhThresh;

        bool bdAccent = bdPattern_[step] > 200 && bdOn;
        bool sdAccent = sdPattern_[step] > 200 && sdOn;
        bool hhAccent = hhPattern_[step] > 200 && hhOn;

        bool isCurrent = (step == currentStep_);

        auto drawLed = [&](float ly, bool on, bool accent, bool current,
                           juce::Colour offCol, juce::Colour onCol, juce::Colour accentCol) {
            if (on) {
                g.setColour(accent ? accentCol : onCol);
            } else {
                g.setColour(offCol);
            }
            g.fillEllipse(lx, ly, ledSize, ledSize);

            if (current) {
                g.setColour(juce::Colours::white.withAlpha(0.7f));
                g.drawEllipse(lx - 1, ly - 1, ledSize + 2, ledSize + 2, 1.5f);
            }
        };

        drawLed(oy + rowSpacing * 1 - ledSize / 2, bdOn, bdAccent, isCurrent,
                juce::Colour(0xff331111), juce::Colour(0xffcc2222), juce::Colour(0xffff4444));
        drawLed(oy + rowSpacing * 2 - ledSize / 2, sdOn, sdAccent, isCurrent,
                juce::Colour(0xff113311), juce::Colour(0xff22cc22), juce::Colour(0xff44ff44));
        drawLed(oy + rowSpacing * 3 - ledSize / 2, hhOn, hhAccent, isCurrent,
                juce::Colour(0xff333311), juce::Colour(0xffcccc22), juce::Colour(0xffffff44));
    }
}

void GriddyAppMainComponent::resized()
{
    auto bounds = getLocalBounds();

#if JUCE_IOS
    if (auto* display = juce::Desktop::getInstance().getDisplays().getDisplayForRect(getScreenBounds()))
        bounds = display->safeAreaInsets.subtractedFrom(bounds);
#endif

    const bool tempoEditorFocused = tempoEditor_.hasKeyboardFocus(true);

    bounds = bounds.reduced(16);
    bounds.removeFromTop(4);

    const float xyPadProportion = tempoEditorFocused ? 0.30f : 0.40f;
    const int minXYPadHeight = tempoEditorFocused ? 180 : 220;
    const int maxXYPadHeight = tempoEditorFocused ? 260 : 340;
    const int xyPadHeight = juce::jlimit(minXYPadHeight, maxXYPadHeight,
                                         (int) (bounds.getHeight() * xyPadProportion));
    auto top = bounds.removeFromTop(xyPadHeight);
    xyPad_.setBounds(top);
    bounds.removeFromTop(6);

    auto controls = bounds;

    auto tempoArea = controls.removeFromTop(58);
    auto tempoEditorArea = tempoArea.removeFromTop(30);
    constexpr int kTempoEditorWidth = 94;
    constexpr int kTempoLabelWidth = 36;
    auto tempoRow = tempoEditorArea.withSizeKeepingCentre(kTempoEditorWidth + 8 + kTempoLabelWidth, 26);
    tempoEditor_.setBounds(tempoRow.removeFromLeft(kTempoEditorWidth));
    tempoRow.removeFromLeft(8);
    tempoUnitLabel_.setBounds(tempoRow.removeFromLeft(kTempoLabelWidth));
    tempoArea.removeFromTop(6);
    tempoSlider_.setBounds(tempoArea.reduced(0, 2));

    controls.removeFromTop(tempoEditorFocused ? 6 : 8);

    auto sliderRow = controls.removeFromTop(juce::jmin(tempoEditorFocused ? 96 : 110, controls.getHeight()));
    auto sliderGroupLabels = sliderRow.removeFromTop(kSectionLabelHeight);
    densityGroupLabel_.setBounds(sliderGroupLabels.removeFromLeft(sliderGroupLabels.getWidth() / 2));
    velocityGroupLabel_.setBounds(sliderGroupLabels);
    sliderRow.removeFromTop(tempoEditorFocused ? 2 : 3);

    auto densityArea = sliderRow.removeFromLeft(sliderRow.getWidth() / 2);
    auto velocityArea = sliderRow;

    auto densityWidth = densityArea.getWidth() / 3;
    bdDensity_.setBounds(densityArea.removeFromLeft(densityWidth).reduced(4));
    sdDensity_.setBounds(densityArea.removeFromLeft(densityWidth).reduced(4));
    hhDensity_.setBounds(densityArea.removeFromLeft(densityWidth).reduced(4));

    auto velocityWidth = velocityArea.getWidth() / 3;
    bdVelocity_.setBounds(velocityArea.removeFromLeft(velocityWidth).reduced(4));
    sdVelocity_.setBounds(velocityArea.removeFromLeft(velocityWidth).reduced(4));
    hhVelocity_.setBounds(velocityArea.removeFromLeft(velocityWidth).reduced(4));

    auto modRow = controls.removeFromTop(juce::jmin(tempoEditorFocused ? 60 : 72, controls.getHeight()));
    auto modLabels = modRow.removeFromTop(kSectionLabelHeight);
    chaosGroupLabel_.setBounds(modLabels.removeFromLeft(modLabels.getWidth() / 2));
    swingGroupLabel_.setBounds(modLabels);
    modRow.removeFromTop(2);
    auto chaosArea = modRow.removeFromLeft(modRow.getWidth() / 2);
    chaosSlider_.setBounds(chaosArea.reduced(8));
    swingSlider_.setBounds(modRow.reduced(8));

    auto buttonRow = controls.removeFromTop(juce::jmin(tempoEditorFocused ? 60 : 70, controls.getHeight()));
    buttonRow.removeFromTop(2);

    auto leftButtonArea = buttonRow.removeFromLeft(buttonRow.getWidth() / 3);
    auto middleButtonArea = buttonRow.removeFromLeft(buttonRow.getWidth() / 2);
    auto rightButtonArea = buttonRow;

    const int buttonHeight = juce::jmax(36, buttonRow.getHeight() - 6);
    auto layoutTransportButton = [buttonHeight](juce::Rectangle<int> area)
    {
        return area.reduced(4, 0)
            .withHeight(buttonHeight)
            .withCentre(area.getCentre());
    };

    playButton_.setBounds(layoutTransportButton(leftButtonArea));
    recordButton_.setBounds(layoutTransportButton(middleButtonArea));
    midiOnlyToggle_.setBounds(rightButtonArea.reduced(6, juce::jmax(2, (rightButtonArea.getHeight() - buttonHeight) / 2)));

    controls.removeFromTop(4);

    // LED pattern grid — compact for mobile
    int ledHeight = juce::jmin(70, controls.getHeight());
    if (ledHeight > 30) {
        ledMatrixBounds_ = controls.removeFromTop(ledHeight);
    } else {
        ledMatrixBounds_ = {};
    }

    statusLabel_.setVisible(!tempoEditorFocused);
    if (tempoEditorFocused)
        statusLabel_.setBounds({});
    else
        statusLabel_.setBounds(controls.removeFromTop(juce::jmin(20, controls.getHeight())));
}

void GriddyAppMainComponent::mouseDown(const juce::MouseEvent& event)
{
    if (!tempoEditor_.hasKeyboardFocus(true))
        return;

    auto* clicked = event.eventComponent;
    if (clicked == nullptr || clicked == &tempoEditor_ || tempoEditor_.isParentOf(clicked))
        return;

    tempoEditor_.giveAwayKeyboardFocus();
}

void GriddyAppMainComponent::timerCallback()
{
    const bool tempoEditorFocused = tempoEditor_.hasKeyboardFocus(true);
    if (tempoEditorFocused != tempoEditorWasFocused_)
    {
        tempoEditorWasFocused_ = tempoEditorFocused;
        resized();
    }

    xyPad_.setValues(engine_.getX(), engine_.getY());
    if (engine_.hasRecording() && !engine_.isRecording() && engine_.isPlaying())
    {
        tempoSlider_.setValue(engine_.getTempo(), juce::dontSendNotification);
        tempoEditor_.setText(juce::String((int) engine_.getTempo()), juce::dontSendNotification);
        bdDensity_.setValue(engine_.getBDDensity(), juce::dontSendNotification);
        sdDensity_.setValue(engine_.getSDDensity(), juce::dontSendNotification);
        hhDensity_.setValue(engine_.getHHDensity(), juce::dontSendNotification);
        bdVelocity_.setValue(engine_.getBDVelocityRange(), juce::dontSendNotification);
        sdVelocity_.setValue(engine_.getSDVelocityRange(), juce::dontSendNotification);
        hhVelocity_.setValue(engine_.getHHVelocityRange(), juce::dontSendNotification);
        chaosSlider_.setValue(engine_.getChaos(), juce::dontSendNotification);
        swingSlider_.setValue(engine_.getSwing(), juce::dontSendNotification);
    }

    // Update LED matrix pattern data from engine
    auto& grids = engine_.getGridsEngine();
    grids.setX(engine_.getX());
    grids.setY(engine_.getY());
    grids.setBDDensity(engine_.getBDDensity());
    grids.setSDDensity(engine_.getSDDensity());
    grids.setHHDensity(engine_.getHHDensity());
    bdPattern_ = grids.getBDPattern();
    sdPattern_ = grids.getSDPattern();
    hhPattern_ = grids.getHHPattern();
    currentStep_ = grids.getCurrentStep();

    updateLabels();
    repaint();
}

void GriddyAppMainComponent::updateLabels()
{
    statusLabel_.setText(engine_.isRecording() ? "Recording" : (engine_.isPlaying() ? "Playing" : "Stopped"),
                         juce::dontSendNotification);
}

void GriddyAppMainComponent::configureSectionLabel(juce::Label& label, const juce::String& text)
{
    label.setText(text, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    label.setColour(juce::Label::textColourId, juce::Colours::white.withAlpha(0.72f));
    addAndMakeVisible(label);
}
