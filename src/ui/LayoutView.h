#pragma once

#include <JuceHeader.h>
#include "ReverbProcessor.h"

class LayoutView : public juce::Component,
                   private juce::Timer
{
public:
    LayoutView(ReverbProcessor &reverbProcessor);
    ~LayoutView() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

    // Update audio buffer for oscilloscope
    void updateBuffer(const juce::AudioBuffer<float> &buffer);

    // Update levels for meters
    void updateLevels(float leftLevel, float rightLevel, float outLeftLevel, float outRightLevel);

    // Force a refresh of all UI parameters
    void refreshAllParameters();

    // URL handler for callbacks from JS
    class LayoutMessageHandler : public juce::WebBrowserComponent
    {
    public:
        LayoutMessageHandler(LayoutView &owner);
        bool pageAboutToLoad(const juce::String &url) override;

    private:
        LayoutView &ownerView;
    };

private:
    ReverbProcessor &reverbProcessor;

    std::unique_ptr<juce::WebBrowserComponent> webView;

    // State tracking variables
    bool pageLoaded;
    juce::CriticalSection bufferLock;
    juce::AudioBuffer<float> latestBuffer;

    // Input/Output levels
    float lastLeftLevel;
    float lastRightLevel;

    // Reverb parameters
    float lastRoomSize;
    float lastDamping;
    float lastWetLevel;
    float lastDryLevel;
    float lastWidth;
    float lastFreezeMode;

    // Timer callback for UI updates
    void timerCallback() override;

    // Prepare waveform data for oscilloscope
    juce::String prepareWaveformData();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutView)
};