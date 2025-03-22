#include "PluginProcessor.h"
#include "PluginEditor.h"

RuptureAudioProcessor::RuptureAudioProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

RuptureAudioProcessor::~RuptureAudioProcessor()
{
}

const juce::String RuptureAudioProcessor::getName() const
{
    return "Rupture";
}

bool RuptureAudioProcessor::acceptsMidi() const
{
    return false;
}

bool RuptureAudioProcessor::producesMidi() const
{
    return false;
}

bool RuptureAudioProcessor::isMidiEffect() const
{
    return false;
}

double RuptureAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RuptureAudioProcessor::getNumPrograms()
{
    return 1;
}

int RuptureAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RuptureAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RuptureAudioProcessor::getProgramName(int index)
{
    return {};
}

void RuptureAudioProcessor::changeProgramName(int index, const juce::String &newName)
{
}

void RuptureAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    levelLeft.reset(sampleRate, 0.1); // Smooth over 100ms
    levelRight.reset(sampleRate, 0.1);
    outputLevelLeft.reset(sampleRate, 0.1);
    outputLevelRight.reset(sampleRate, 0.1);

    // Prepare DSP components
    reverbProcessor.prepare(sampleRate, samplesPerBlock);
}

void RuptureAudioProcessor::releaseResources()
{
    // When playback stops, release all resources
    reverbProcessor.reset();
}

bool RuptureAudioProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const
{
    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    // We only support stereo and mono
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RuptureAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels that didn't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Calculate input levels for the meters
    float newLevelLeft = 0.0f;
    float newLevelRight = 0.0f;
    if (totalNumInputChannels > 0)
        newLevelLeft = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    if (totalNumInputChannels > 1)
        newLevelRight = buffer.getRMSLevel(1, 0, buffer.getNumSamples());
    levelLeft.setTargetValue(newLevelLeft);
    levelRight.setTargetValue(newLevelRight);
    levelLeft.skip(buffer.getNumSamples());
    levelRight.skip(buffer.getNumSamples());

    // Process audio through reverb
    reverbProcessor.processBlock(buffer);

    // Calculate output levels after all processing
    float newOutputLevelLeft = 0.0f;
    float newOutputLevelRight = 0.0f;
    if (totalNumOutputChannels > 0)
        newOutputLevelLeft = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    if (totalNumOutputChannels > 1)
        newOutputLevelRight = buffer.getRMSLevel(1, 0, buffer.getNumSamples());
    outputLevelLeft.setTargetValue(newOutputLevelLeft);
    outputLevelRight.setTargetValue(newOutputLevelRight);
    outputLevelLeft.skip(buffer.getNumSamples());
    outputLevelRight.skip(buffer.getNumSamples());

    // Store post-processed buffer for oscilloscope
    {
        juce::ScopedLock lock(outputBufferLock);
        outputBuffer.makeCopyOf(buffer);
    }
}

bool RuptureAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *RuptureAudioProcessor::createEditor()
{
    return new RuptureAudioProcessorEditor(*this);
}

void RuptureAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    // Store plugin state (parameters)
    juce::MemoryOutputStream stream(destData, true);

    // Store reverb parameters
    stream.writeFloat(reverbProcessor.getRoomSize());
    stream.writeFloat(reverbProcessor.getDamping());
    stream.writeFloat(reverbProcessor.getWetLevel());
    stream.writeFloat(reverbProcessor.getDryLevel());
    stream.writeFloat(reverbProcessor.getWidth());
    stream.writeFloat(reverbProcessor.getFreezeMode());
}

void RuptureAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    // Restore plugin state (parameters)
    juce::MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);

    // Check how much data we have available
    const int bytesAvailable = stream.getNumBytesRemaining();

    if (bytesAvailable >= sizeof(float) * 6)
    {
        // Full state with all reverb parameters
        float roomSize = stream.readFloat();
        float damping = stream.readFloat();
        float wetLevel = stream.readFloat();
        float dryLevel = stream.readFloat();
        float width = stream.readFloat();
        float freezeMode = stream.readFloat();

        // Apply reverb parameters
        reverbProcessor.setRoomSize(roomSize);
        reverbProcessor.setDamping(damping);
        reverbProcessor.setWetLevel(wetLevel);
        reverbProcessor.setDryLevel(dryLevel);
        reverbProcessor.setWidth(width);
        reverbProcessor.setFreezeMode(freezeMode);
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new RuptureAudioProcessor();
}