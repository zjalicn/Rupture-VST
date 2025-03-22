#pragma once

#include <JuceHeader.h>

class AuraAudioProcessor;

class PresetManager
{
public:
    PresetManager(AuraAudioProcessor &processor);
    ~PresetManager();

    void savePreset(const juce::String &name);
    bool loadPreset(const juce::String &name);
    juce::StringArray getPresetList();

private:
    AuraAudioProcessor &audioProcessor;
    juce::File presetsDirectory;

    juce::File getPresetsDirectory();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetManager)
};