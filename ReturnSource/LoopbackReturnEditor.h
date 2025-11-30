#pragma once
#include <JuceHeader.h>
#include "LoopbackReturnProcessor.h"

class LoopbackReturnAudioProcessorEditor : public juce::AudioProcessorEditor,
    private juce::Timer,
    private juce::Slider::Listener,
    private juce::Button::Listener
{
public:
    explicit LoopbackReturnAudioProcessorEditor(LoopbackReturnAudioProcessor&);
    ~LoopbackReturnAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

private:
    LoopbackReturnAudioProcessor& audioProcessor;

    juce::Slider bufferLengthSlider;
    juce::Label bufferLengthLabel;

    juce::Slider feedbackSlider;
    juce::Label feedbackLabel;

    juce::ToggleButton antiFeedbackButton;
    juce::TextButton stopButton;
    juce::TextButton clearButton;      // ✅ NEW
    juce::TextButton fadeOutButton;    // ✅ NEW

    int bufferLevel = 0;
    int64_t writeCount = 0;
    int64_t readCount = 0;
    bool isPlaying = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopbackReturnAudioProcessorEditor)
};