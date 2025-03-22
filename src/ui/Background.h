#pragma once

#include <JuceHeader.h>

class Background : public juce::Component
{
public:
    Background() {}

    void paint(juce::Graphics &g) override
    {
        // Fill the background with dark gradient
        juce::Colour darkBlue(22, 25, 33);
        juce::Colour darkPurple(28, 25, 36);
        g.setGradientFill(juce::ColourGradient(
            darkBlue,
            0.0f, 0.0f,
            darkPurple,
            static_cast<float>(getWidth()), static_cast<float>(getHeight()),
            false));
        g.fillAll();

        // Add subtle grid lines
        g.setColour(juce::Colour(40, 40, 50));
        float gridSize = 20.0f;
        for (float x = 0; x < getWidth(); x += gridSize)
        {
            g.drawLine(x, 0.0f, x, static_cast<float>(getHeight()), 0.5f);
        }
        for (float y = 0; y < getHeight(); y += gridSize)
        {
            g.drawLine(0.0f, y, static_cast<float>(getWidth()), y, 0.5f);
        }

        // Add a subtle glow in the center for ambient effect
        juce::ColourGradient glow(
            juce::Colour(100, 100, 180, 20),
            getWidth() * 0.5f, getHeight() * 0.5f,
            juce::Colour(100, 100, 180, 0),
            getWidth() * 0.5f, getHeight() * 0.8f,
            true);
        g.setGradientFill(glow);
        g.fillEllipse(getWidth() * 0.2f, getHeight() * 0.2f, getWidth() * 0.6f, getHeight() * 0.6f);
    }

    void resized() override
    {
        // Nothing to resize
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Background)
};