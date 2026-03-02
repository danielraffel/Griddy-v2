#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    if (xyPad_)
        xyPad_->setBounds(20, 40, 220, 210);
    if (ledMatrix_)
        ledMatrix_->setBounds(20, 285, 540, 80);
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

    // Draw dark background
    rootFrame_->onDraw() += [](visage::Canvas& canvas) {
        float w = static_cast<float>(canvas.width());
        float h = static_cast<float>(canvas.height());
        canvas.setColor(0xff1e1e1e);
        canvas.fill(0, 0, w, h);
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

    // Create LED Matrix
    auto ledMatrixOwned = std::make_unique<LEDMatrixFrame>();
    ledMatrix_ = ledMatrixOwned.get();

    // Add children to root frame
    rootFrame_->addChild(xyPadOwned.release());
    rootFrame_->addChild(ledMatrixOwned.release());

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

    // Update XY pad from parameters (in case of automation)
    float paramX = *processorRef.parameters.getRawParameterValue("x");
    float paramY = *processorRef.parameters.getRawParameterValue("y");
    xyPad_->setX(paramX);
    xyPad_->setY(paramY);

    // Update LED matrix
    float bdDensity = *processorRef.parameters.getRawParameterValue("density_1_bd");
    float sdDensity = *processorRef.parameters.getRawParameterValue("density_2_sd");
    float hhDensity = *processorRef.parameters.getRawParameterValue("density_3_hh");

    ledMatrix_->setDensities(bdDensity, sdDensity, hhDensity);
    ledMatrix_->setPatterns(engine.getBDPattern(), engine.getSDPattern(), engine.getHHPattern());
    ledMatrix_->setCurrentStep(engine.getCurrentStep());
}
