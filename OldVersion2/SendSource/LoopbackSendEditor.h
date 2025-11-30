#pragma once
#include <JuceHeader.h>
#include "LoopbackSendProcessor.h"

class LoopbackSendAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer
{
public:
    explicit LoopbackSendAudioProcessorEditor(LoopbackSendAudioProcessor&);
    ~LoopbackSendAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    LoopbackSendAudioProcessor& audioProcessor;

    int64_t writeCount = 0;
    bool isActive = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopbackSendAudioProcessorEditor)
};