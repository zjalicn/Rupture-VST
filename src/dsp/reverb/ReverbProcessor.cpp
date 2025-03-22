#include "ReverbProcessor.h"

ReverbProcessor::ReverbProcessor()
    : roomSize(0.5f),
      damping(0.5f),
      wetLevel(0.33f),
      dryLevel(0.4f),
      width(1.0f),
      freezeMode(0.0f),
      currentSampleRate(44100.0),
      bufferSize(0)
{
    updateReverbSettings();
}

void ReverbProcessor::prepare(double sampleRate, int maxBlockSize)
{
    currentSampleRate = sampleRate;
    bufferSize = maxBlockSize;

    // Initialize reverb with the current sample rate
    reverb.setSampleRate(sampleRate);
    updateReverbSettings();
}

void ReverbProcessor::processBlock(juce::AudioBuffer<float> &buffer)
{
    const int numChannels = buffer.getNumChannels();
    const int numSamples = buffer.getNumSamples();

    // Early return if we haven't been prepared yet
    if (bufferSize == 0)
        return;

    // Create a buffer for the dry signal
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);

    // Process stereo (or mono if that's all we have)
    if (numChannels == 2)
    {
        reverb.processStereo(buffer.getWritePointer(0),
                             buffer.getWritePointer(1),
                             numSamples);
    }
    else if (numChannels == 1)
    {
        // Create a temporary buffer for the second channel in mono case
        juce::AudioBuffer<float> temp(2, numSamples);
        temp.copyFrom(0, 0, buffer, 0, 0, numSamples);
        temp.copyFrom(1, 0, buffer, 0, 0, numSamples);

        // Process as stereo
        reverb.processStereo(temp.getWritePointer(0),
                             temp.getWritePointer(1),
                             numSamples);

        // Copy back the first channel to our buffer
        buffer.copyFrom(0, 0, temp, 0, 0, numSamples);
    }
}

void ReverbProcessor::reset()
{
    reverb.reset();
}

void ReverbProcessor::updateReverbSettings()
{
    juce::Reverb::Parameters params;
    params.roomSize = roomSize;
    params.damping = damping;
    params.wetLevel = wetLevel;
    params.dryLevel = dryLevel;
    params.width = width;
    params.freezeMode = freezeMode;

    reverb.setParameters(params);
}

// Parameter setters
void ReverbProcessor::setRoomSize(float newRoomSize)
{
    roomSize = juce::jlimit(0.0f, 1.0f, newRoomSize);
    updateReverbSettings();
}

void ReverbProcessor::setDamping(float newDamping)
{
    damping = juce::jlimit(0.0f, 1.0f, newDamping);
    updateReverbSettings();
}

void ReverbProcessor::setWetLevel(float newWetLevel)
{
    wetLevel = juce::jlimit(0.0f, 1.0f, newWetLevel);
    updateReverbSettings();
}

void ReverbProcessor::setDryLevel(float newDryLevel)
{
    dryLevel = juce::jlimit(0.0f, 1.0f, newDryLevel);
    updateReverbSettings();
}

void ReverbProcessor::setWidth(float newWidth)
{
    width = juce::jlimit(0.0f, 1.0f, newWidth);
    updateReverbSettings();
}

void ReverbProcessor::setFreezeMode(float newFreezeMode)
{
    freezeMode = juce::jlimit(0.0f, 1.0f, newFreezeMode);
    updateReverbSettings();
}

// Parameter getters
float ReverbProcessor::getRoomSize() const
{
    return roomSize;
}

float ReverbProcessor::getDamping() const
{
    return damping;
}

float ReverbProcessor::getWetLevel() const
{
    return wetLevel;
}

float ReverbProcessor::getDryLevel() const
{
    return dryLevel;
}

float ReverbProcessor::getWidth() const
{
    return width;
}

float ReverbProcessor::getFreezeMode() const
{
    return freezeMode;
}