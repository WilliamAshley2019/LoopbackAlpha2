#include "LoopbackReturnEditor.h"

LoopbackReturnAudioProcessorEditor::LoopbackReturnAudioProcessorEditor(LoopbackReturnAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(500, 380); // ✅ Slightly taller for new buttons

    // Buffer Length Slider
    bufferLengthSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    bufferLengthSlider.setRange(50.0, 3000.0, 1.0);
    bufferLengthSlider.setValue(audioProcessor.bufferLengthParam->get());
    bufferLengthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    bufferLengthSlider.setTextValueSuffix(" ms");
    bufferLengthSlider.addListener(this);
    addAndMakeVisible(bufferLengthSlider);

    bufferLengthLabel.setText("Delay Time", juce::dontSendNotification);
    bufferLengthLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(bufferLengthLabel);

    // Feedback Slider
    feedbackSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    feedbackSlider.setRange(0.0, 95.0, 0.1);
    feedbackSlider.setValue(audioProcessor.feedbackParam->get());
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    feedbackSlider.setTextValueSuffix(" %");
    feedbackSlider.addListener(this);
    addAndMakeVisible(feedbackSlider);

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(feedbackLabel);

    // Anti-Feedback Toggle
    antiFeedbackButton.setButtonText("Anti-Feedback");
    antiFeedbackButton.setToggleState(audioProcessor.feedbackReductionParam->get(), juce::dontSendNotification);
    antiFeedbackButton.addListener(this);
    addAndMakeVisible(antiFeedbackButton);

    // Stop Button
    stopButton.setButtonText("STOP");
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.addListener(this);
    addAndMakeVisible(stopButton);

    // ✅ NEW: Clear Buffer Button
    clearButton.setButtonText("CLEAR");
    clearButton.setColour(juce::TextButton::buttonColourId, juce::Colours::orange);
    clearButton.addListener(this);
    addAndMakeVisible(clearButton);

    // ✅ NEW: Fade Out Button
    fadeOutButton.setButtonText("FADE OUT");
    fadeOutButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    fadeOutButton.addListener(this);
    addAndMakeVisible(fadeOutButton);

    startTimerHz(30);
}

LoopbackReturnAudioProcessorEditor::~LoopbackReturnAudioProcessorEditor()
{
    stopTimer();
}

void LoopbackReturnAudioProcessorEditor::timerCallback()
{
    bufferLevel = LoopbackShared::getInstance().getNumAvailable();
    writeCount = LoopbackShared::getInstance().getTotalWritten();
    readCount = LoopbackShared::getInstance().getTotalRead();

    isPlaying = true;
    if (auto* ph = audioProcessor.getPlayHead())
    {
        if (auto pos = ph->getPosition())
            isPlaying = pos->getIsPlaying() && !audioProcessor.isManualStop();
    }

    repaint();
}

void LoopbackReturnAudioProcessorEditor::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &bufferLengthSlider)
    {
        audioProcessor.bufferLengthParam->setValueNotifyingHost(
            audioProcessor.bufferLengthParam->getNormalisableRange().convertTo0to1(
                static_cast<float>(slider->getValue())));
    }
    else if (slider == &feedbackSlider)
    {
        audioProcessor.feedbackParam->setValueNotifyingHost(
            audioProcessor.feedbackParam->getNormalisableRange().convertTo0to1(
                static_cast<float>(slider->getValue())));
    }
}

void LoopbackReturnAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &antiFeedbackButton)
    {
        audioProcessor.feedbackReductionParam->setValueNotifyingHost(
            antiFeedbackButton.getToggleState() ? 1.0f : 0.0f);
    }
    else if (button == &stopButton)
    {
        bool newState = !audioProcessor.isManualStop();
        audioProcessor.setManualStop(newState);
        stopButton.setButtonText(newState ? "RESUME" : "STOP");
        stopButton.setColour(juce::TextButton::buttonColourId,
            newState ? juce::Colours::green : juce::Colours::red);
    }
    // ✅ NEW: Clear buffer instantly
    else if (button == &clearButton)
    {
        LoopbackShared::getInstance().clearBuffer();
    }
    // ✅ NEW: Fade out over 500ms
    else if (button == &fadeOutButton)
    {
        LoopbackShared::getInstance().fadeOutBuffer(500.0f, audioProcessor.getSampleRate());
    }
}

void LoopbackReturnAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(24.0f);
    g.drawText("Loopback Return", 0, 10, getWidth(), 30, juce::Justification::centred);

    // Status indicator
    g.setFont(16.0f);
    g.setColour(isPlaying && bufferLevel > 0 ? juce::Colours::lime : juce::Colours::red);
    juce::String status = isPlaying ? "● ACTIVE" : "● STOPPED";
    g.drawText(status, 0, 40, getWidth(), 20, juce::Justification::centred);

    // Buffer level visualization
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(20, 280, getWidth() - 40, 30); // ✅ Moved down

    float fillRatio = juce::jmin(1.0f, bufferLevel / 88200.0f);
    g.setColour(juce::Colours::lime.withAlpha(0.8f));
    g.fillRect(20, 280, static_cast<int>((getWidth() - 40) * fillRatio), 30);

    g.setColour(juce::Colours::white);
    g.setFont(12.0f);
    g.drawText("Buffer: " + juce::String(bufferLevel) + " samples",
        20, 315, getWidth() - 40, 20, juce::Justification::centred);

    // Statistics
    g.setFont(11.0f);
    g.drawText("Written: " + juce::String(writeCount),
        20, 340, (getWidth() - 40) / 2, 20, juce::Justification::left);
    g.drawText("Read: " + juce::String(readCount),
        getWidth() / 2, 340, (getWidth() - 40) / 2, 20, juce::Justification::right);
}

void LoopbackReturnAudioProcessorEditor::resized()
{
    int knobSize = 100;
    int knobY = 80;

    bufferLengthSlider.setBounds(50, knobY, knobSize, knobSize);
    bufferLengthLabel.setBounds(50, knobY + knobSize + 5, knobSize, 20);

    feedbackSlider.setBounds(getWidth() - 150, knobY, knobSize, knobSize);
    feedbackLabel.setBounds(getWidth() - 150, knobY + knobSize + 5, knobSize, 20);

    // Center column of buttons
    int buttonX = 190;
    int buttonWidth = 120;

    antiFeedbackButton.setBounds(buttonX, 90, buttonWidth, 30);
    stopButton.setBounds(buttonX, 130, buttonWidth, 35);
    clearButton.setBounds(buttonX, 175, buttonWidth, 35);      // ✅ NEW
    fadeOutButton.setBounds(buttonX, 220, buttonWidth, 35);   // ✅ NEW
}