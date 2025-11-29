#include "LoopbackSendProcessor.h"
#include "LoopbackSendEditor.h"

LoopbackSendAudioProcessor::LoopbackSendAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

LoopbackSendAudioProcessor::~LoopbackSendAudioProcessor() = default;

void LoopbackSendAudioProcessor::prepareToPlay(double, int) {}

void LoopbackSendAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    const int numSamples = buffer.getNumSamples();

    // Write to Windows shared memory
    LoopbackShared::getInstance().write(
        buffer.getReadPointer(0),
        buffer.getNumChannels() >= 2 ? buffer.getReadPointer(1) : nullptr,
        numSamples,
        getSampleRate()
    );

    // Audio passes through unchanged (in-place processing)
}

juce::AudioProcessorEditor* LoopbackSendAudioProcessor::createEditor()
{
    return new LoopbackSendAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoopbackSendAudioProcessor();
}