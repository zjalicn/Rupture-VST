#pragma once

#include <JuceHeader.h>

class ReverbProcessor
{
public:
    ReverbProcessor();
    ~ReverbProcessor() = default;

    void prepare(double sampleRate, int maxBlockSize);
    void processBlock(juce::AudioBuffer<float> &buffer);
    void reset();

    void updateReverbSettings();

    // Parameter setters
    void setRoomSize(float newRoomSize);     // 0.0 - 1.0
    void setDamping(float newDamping);       // 0.0 - 1.0
    void setWetLevel(float newWetLevel);     // 0.0 - 1.0
    void setDryLevel(float newDryLevel);     // 0.0 - 1.0
    void setWidth(float newWidth);           // 0.0 - 1.0
    void setFreezeMode(float newFreezeMode); // 0.0 - 1.0

    // Parameter getters
    float getRoomSize() const;
    float getDamping() const;
    float getWetLevel() const;
    float getDryLevel() const;
    float getWidth() const;
    float getFreezeMode() const;

private:
    // Reverb parameters
    float roomSize;
    float damping;
    float wetLevel;
    float dryLevel;
    float width;
    float freezeMode;

    // Internal state
    double currentSampleRate;
    int bufferSize;

    // JUCE Reverb processor
    juce::Reverb reverb;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ReverbProcessor)
};