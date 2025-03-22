#include "PresetManager.h"
#include "PluginProcessor.h"

PresetManager::PresetManager(AuraAudioProcessor &processor)
    : audioProcessor(processor)
{
    presetsDirectory = getPresetsDirectory();

    // Create the presets directory if it doesn't exist
    if (!presetsDirectory.exists())
    {
        presetsDirectory.createDirectory();
    }
}

PresetManager::~PresetManager()
{
}

void PresetManager::savePreset(const juce::String &name)
{
    // Create a memory block to store the state information
    juce::MemoryBlock memoryBlock;
    audioProcessor.getStateInformation(memoryBlock);

    // Create a file for the preset
    juce::File presetFile = presetsDirectory.getChildFile(name + ".preset");

    // Write the data to the file
    if (presetFile.existsAsFile())
    {
        presetFile.deleteFile();
    }

    juce::FileOutputStream outputStream(presetFile);
    if (outputStream.openedOk())
    {
        outputStream.write(memoryBlock.getData(), memoryBlock.getSize());
        outputStream.flush();
    }
}

bool PresetManager::loadPreset(const juce::String &name)
{
    // Find the preset file
    juce::File presetFile = presetsDirectory.getChildFile(name + ".preset");

    // Check if the file exists
    if (!presetFile.existsAsFile())
    {
        return false;
    }

    // Load the data from the file
    juce::MemoryBlock memoryBlock;
    juce::FileInputStream inputStream(presetFile);

    if (inputStream.openedOk())
    {
        inputStream.readIntoMemoryBlock(memoryBlock);

        // Set the state with the loaded data
        audioProcessor.setStateInformation(memoryBlock.getData(), static_cast<int>(memoryBlock.getSize()));
        return true;
    }

    return false;
}

juce::StringArray PresetManager::getPresetList()
{
    juce::StringArray presets;

    // Get all preset files from the directory
    juce::Array<juce::File> presetFiles = presetsDirectory.findChildFiles(
        juce::File::findFiles, false, "*.preset");

    // Extract the preset names
    for (auto &file : presetFiles)
    {
        presets.add(file.getFileNameWithoutExtension());
    }

    return presets;
}

juce::File PresetManager::getPresetsDirectory()
{
    // Get the user's application data directory
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);

    // Add our company and plugin name
    juce::File presetDir = appDataDir.getChildFile("YourCompany/Aura/Presets");

    return presetDir;
}