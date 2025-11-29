#pragma once
#include <JuceHeader.h>
#include "LoopbackShared.h"

class LoopbackReturnAudioProcessor : public juce::AudioProcessor
{
public:
    LoopbackReturnAudioProcessor();
    ~LoopbackReturnAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Loopback Return"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return std::numeric_limits<double>::infinity(); }
    bool silenceInProducesSilenceOut() const override { return false; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    // Parameter access
    juce::AudioParameterFloat* bufferLengthParam;
    juce::AudioParameterFloat* feedbackParam;
    juce::AudioParameterBool* feedbackReductionParam;

    // Manual stop control
    void setManualStop(bool shouldStop) { manualStop = shouldStop; }
    bool isManualStop() const { return manualStop; }

private:
    std::atomic<bool> manualStop{ false };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoopbackReturnAudioProcessor)
};