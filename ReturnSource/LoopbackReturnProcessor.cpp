#include "LoopbackReturnProcessor.h"
#include "LoopbackReturnEditor.h"

LoopbackReturnAudioProcessor::LoopbackReturnAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    // Add parameters with newer JUCE 8 syntax
    addParameter(bufferLengthParam = new juce::AudioParameterFloat(
        juce::ParameterID{ "bufferLength", 1 },
        "Buffer Length",
        juce::NormalisableRange<float>(50.0f, 3000.0f, 1.0f),
        500.0f));

    addParameter(feedbackParam = new juce::AudioParameterFloat(
        juce::ParameterID{ "feedback", 1 },
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 95.0f, 0.1f),
        0.0f));

    addParameter(feedbackReductionParam = new juce::AudioParameterBool(
        juce::ParameterID{ "feedbackReduction", 1 },
        "Anti-Feedback",
        false));

    setLatencySamples(0);
}

LoopbackReturnAudioProcessor::~LoopbackReturnAudioProcessor() = default;

void LoopbackReturnAudioProcessor::prepareToPlay(double, int) {}

void LoopbackReturnAudioProcessor::releaseResources() {}

void LoopbackReturnAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer&)
{
    buffer.clear();

    // Update shared config from parameters
    LoopbackShared::getInstance().setBufferLength(bufferLengthParam->get());
    LoopbackShared::getInstance().setFeedback(feedbackParam->get() / 100.0f);
    LoopbackShared::getInstance().setFeedbackReduction(feedbackReductionParam->get());

    // Check if transport is playing AND manual stop is not engaged
    bool isPlaying = true;

    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
            isPlaying = pos->getIsPlaying();
    }

    // Override with manual stop
    if (manualStop)
        isPlaying = false;

    const int numSamples = buffer.getNumSamples();

    // Read from shared buffer
    LoopbackShared::getInstance().read(
        buffer.getWritePointer(0),
        buffer.getNumChannels() >= 2 ? buffer.getWritePointer(1) : nullptr,
        numSamples,
        getSampleRate(),
        isPlaying
    );

    // Tiny DC offset to prevent Smart Disable
    if (numSamples > 0 && isPlaying)
    {
        buffer.setSample(0, 0, buffer.getSample(0, 0) + 0.0000001f);
        if (buffer.getNumChannels() >= 2)
            buffer.setSample(1, 0, buffer.getSample(1, 0) + 0.0000001f);
    }
}

bool LoopbackReturnAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    return out == juce::AudioChannelSet::mono()
        || out == juce::AudioChannelSet::stereo();
}

juce::AudioProcessorEditor* LoopbackReturnAudioProcessor::createEditor()
{
    return new LoopbackReturnAudioProcessorEditor(*this);
}

void LoopbackReturnAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream(destData, true);
    stream.writeFloat(bufferLengthParam->get());
    stream.writeFloat(feedbackParam->get());
    stream.writeBool(feedbackReductionParam->get());
}

void LoopbackReturnAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    bufferLengthParam->setValueNotifyingHost(
        bufferLengthParam->getNormalisableRange().convertTo0to1(stream.readFloat()));
    feedbackParam->setValueNotifyingHost(
        feedbackParam->getNormalisableRange().convertTo0to1(stream.readFloat()));
    feedbackReductionParam->setValueNotifyingHost(stream.readBool() ? 1.0f : 0.0f);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new LoopbackReturnAudioProcessor();
}