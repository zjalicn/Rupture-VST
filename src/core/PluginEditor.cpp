#include "PluginEditor.h"

RuptureAudioProcessorEditor::RuptureAudioProcessorEditor(RuptureAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      layoutView(p.getReverbProcessor())
{
    addAndMakeVisible(layoutView);

    // Initialize the layout view with the output buffer for oscilloscope
    layoutView.updateBuffer(p.getOutputBuffer());

    // Start the timer for meter updates
    startTimerHz(30);

    // Set initial size
    setSize(CANVAS_WIDTH, CANVAS_HEIGHT);
}

RuptureAudioProcessorEditor::~RuptureAudioProcessorEditor()
{
    stopTimer();
}

void RuptureAudioProcessorEditor::paint(juce::Graphics &g)
{
    // No need to paint anything here since the layout view does that
}

void RuptureAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Set the layout view to cover the entire window
    layoutView.setBounds(bounds);
}

void RuptureAudioProcessorEditor::timerCallback()
{
    // Regular meter updates
    float leftLevel = audioProcessor.getLeftLevel();
    float rightLevel = audioProcessor.getRightLevel();
    float outLeftLevel = audioProcessor.getOutputLeftLevel();
    float outRightLevel = audioProcessor.getOutputRightLevel();

    // Scale for display - convert RMS values to percentage heights
    leftLevel = std::pow(leftLevel * 100.0f, 0.5f) * 10.0f;
    rightLevel = std::pow(rightLevel * 100.0f, 0.5f) * 10.0f;
    outLeftLevel = std::pow(outLeftLevel * 100.0f, 0.5f) * 10.0f;
    outRightLevel = std::pow(outRightLevel * 100.0f, 0.5f) * 10.0f;

    // Ensure values are in range
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    outLeftLevel = juce::jlimit(0.0f, 100.0f, outLeftLevel);
    outRightLevel = juce::jlimit(0.0f, 100.0f, outRightLevel);

    // Allow small values to show as completely empty
    if (leftLevel < 0.1f)
        leftLevel = 0.0f;
    if (rightLevel < 0.1f)
        rightLevel = 0.0f;
    if (outLeftLevel < 0.1f)
        outLeftLevel = 0.0f;
    if (outRightLevel < 0.1f)
        outRightLevel = 0.0f;

    // Update the levels in the layout view
    layoutView.updateLevels(leftLevel, rightLevel, outLeftLevel, outRightLevel);

    // Update the oscilloscope with latest audio buffer
    layoutView.updateBuffer(audioProcessor.getOutputBuffer());
}