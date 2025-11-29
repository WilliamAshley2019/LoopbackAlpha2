#include "LoopbackSendEditor.h"

LoopbackSendAudioProcessorEditor::LoopbackSendAudioProcessorEditor(LoopbackSendAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setSize(320, 200);
    startTimerHz(30);
}

LoopbackSendAudioProcessorEditor::~LoopbackSendAudioProcessorEditor()
{
    stopTimer();
}

void LoopbackSendAudioProcessorEditor::timerCallback()
{
    int64_t newWriteCount = LoopbackShared::getInstance().getTotalWritten();
    isActive = (newWriteCount != writeCount);
    writeCount = newWriteCount;
    repaint();
}

void LoopbackSendAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));

    // Title
    g.setColour(juce::Colours::white);
    g.setFont(22.0f);
    g.drawText("Loopback Send", 0, 10, getWidth(), 30, juce::Justification::centred);

    // Status indicator
    g.setFont(16.0f);
    g.setColour(isActive ? juce::Colours::lime : juce::Colours::grey);
    juce::String status = isActive ? "● SENDING" : "● IDLE";
    g.drawText(status, 0, 45, getWidth(), 25, juce::Justification::centred);

    // Instructions
    g.setColour(juce::Colours::lightgrey);
    g.setFont(13.0f);
    g.drawFittedText(
        "Place on any mixer track to\ncapture and route audio\nto Loopback Return plugin",
        20, 90, getWidth() - 40, 60,
        juce::Justification::centred, 3);

    // Statistics
    g.setFont(11.0f);
    g.setColour(juce::Colours::grey);
    g.drawText("Total sent: " + juce::String(writeCount) + " samples",
        0, getHeight() - 30, getWidth(), 20, juce::Justification::centred);
}

void LoopbackSendAudioProcessorEditor::resized()
{
}