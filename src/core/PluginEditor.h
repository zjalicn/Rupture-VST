#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "LayoutView.h"

class RuptureAudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    static constexpr int CANVAS_WIDTH = 720;
    static constexpr int CANVAS_HEIGHT = 320;

    RuptureAudioProcessorEditor(RuptureAudioProcessor &);
    ~RuptureAudioProcessorEditor() override;

    void paint(juce::Graphics &g) override;
    void resized() override;

private:
    RuptureAudioProcessor &audioProcessor;

    LayoutView layoutView;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RuptureAudioProcessorEditor)
};