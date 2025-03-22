#include "LayoutView.h"
#include "BinaryData.h"

LayoutView::LayoutMessageHandler::LayoutMessageHandler(LayoutView &owner)
    : ownerView(owner)
{
}

bool LayoutView::LayoutMessageHandler::pageAboutToLoad(const juce::String &url)
{
    // Handle rupture: protocol for control messages
    if (url.startsWith("rupture:"))
    {
        juce::String params = url.fromFirstOccurrenceOf("rupture:", false, true);

        // Handle reverb parameters
        if (params.startsWith("reverb:"))
        {
            params = params.fromFirstOccurrenceOf("reverb:", false, true);

            if (params.startsWith("roomSize="))
            {
                float value = params.fromFirstOccurrenceOf("roomSize=", false, true).getFloatValue();
                ownerView.reverbProcessor.setRoomSize(value);
                return false;
            }
            else if (params.startsWith("damping="))
            {
                float value = params.fromFirstOccurrenceOf("damping=", false, true).getFloatValue();
                ownerView.reverbProcessor.setDamping(value);
                return false;
            }
            else if (params.startsWith("wetLevel="))
            {
                float value = params.fromFirstOccurrenceOf("wetLevel=", false, true).getFloatValue();
                ownerView.reverbProcessor.setWetLevel(value);
                return false;
            }
            else if (params.startsWith("dryLevel="))
            {
                float value = params.fromFirstOccurrenceOf("dryLevel=", false, true).getFloatValue();
                ownerView.reverbProcessor.setDryLevel(value);
                return false;
            }
            else if (params.startsWith("width="))
            {
                float value = params.fromFirstOccurrenceOf("width=", false, true).getFloatValue();
                ownerView.reverbProcessor.setWidth(value);
                return false;
            }
            else if (params.startsWith("freezeMode="))
            {
                float value = params.fromFirstOccurrenceOf("freezeMode=", false, true).getFloatValue();
                ownerView.reverbProcessor.setFreezeMode(value);
                return false;
            }
        }

        return false; // We handled this URL
    }
    // Handle custom font loading
    else if (url.startsWith("BinaryData::"))
    {
        // This is our own resource URL format
        juce::String resourceName = url.substring(12);
        int size = 0;
        const char *data = nullptr;

        // Get the binary data (not used in this simplified version but kept for compatibility)
        if (resourceName == "old_english_hearts_ttf")
        {
            // If you want to add a font later, add it to the binary resources and handle it here
            data = BinaryData::old_english_hearts_ttf;
            size = BinaryData::old_english_hearts_ttfSize;

            if (data != nullptr && size > 0)
            {
                // Convert binary data to base64
                juce::MemoryBlock mb(data, size);
                juce::String base64 = mb.toBase64Encoding();

                // Create a data URL for the font
                juce::String dataUrl = "data:font/ttf;base64," + base64;

                // Now navigate to this data URL
                this->goToURL(dataUrl);
                return false; // We've handled this URL
            }
        }
    }

    return true; // We didn't handle this URL
}

// Main LayoutView implementation
LayoutView::LayoutView(ReverbProcessor &revProc)
    : reverbProcessor(revProc),
      pageLoaded(false),
      lastLeftLevel(0.0f),
      lastRightLevel(0.0f),
      lastRoomSize(revProc.getRoomSize()),
      lastDamping(revProc.getDamping()),
      lastWetLevel(revProc.getWetLevel()),
      lastDryLevel(revProc.getDryLevel()),
      lastWidth(revProc.getWidth()),
      lastFreezeMode(revProc.getFreezeMode())
{
    auto browser = new LayoutMessageHandler(*this);
    webView.reset(browser);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    juce::String htmlContent = juce::String(BinaryData::layout_html, BinaryData::layout_htmlSize);
    juce::String cssContent = juce::String(BinaryData::layout_css, BinaryData::layout_cssSize);

    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./layout.css\" />",
        "<style>\n" + cssContent + "\n    </style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);

    // Start timer for updates
    startTimerHz(30);
}

LayoutView::~LayoutView()
{
    stopTimer();
    webView = nullptr;
}

void LayoutView::paint(juce::Graphics &g)
{
    // Nothing to paint - WebView handles rendering
}

void LayoutView::resized()
{
    // Make the webView take up all available space
    webView->setBounds(getLocalBounds());
}

void LayoutView::timerCallback()
{
    // First few cycles, just wait for page to load
    static int pageLoadCounter = 0;
    if (!pageLoaded)
    {
        pageLoadCounter++;
        if (pageLoadCounter >= 10) // About 333ms with 30Hz timer
        {
            pageLoaded = true;

            // Initialize with default values
            juce::String script = "window.setAudioState(0, 0, 0, 0)";
            webView->evaluateJavascript(script);
        }
        return;
    }

    // Check for parameter changes in reverb processor
    float roomSize = reverbProcessor.getRoomSize();
    float damping = reverbProcessor.getDamping();
    float wetLevel = reverbProcessor.getWetLevel();
    float dryLevel = reverbProcessor.getDryLevel();
    float width = reverbProcessor.getWidth();
    float freezeMode = reverbProcessor.getFreezeMode();

    bool reverbChanged = std::abs(roomSize - lastRoomSize) > 0.001f ||
                         std::abs(damping - lastDamping) > 0.001f ||
                         std::abs(wetLevel - lastWetLevel) > 0.001f ||
                         std::abs(dryLevel - lastDryLevel) > 0.001f ||
                         std::abs(width - lastWidth) > 0.001f ||
                         std::abs(freezeMode - lastFreezeMode) > 0.001f;

    if (reverbChanged)
    {
        // Update reverb UI with current values
        juce::String script = "window.setReverbValues(" +
                              juce::String(roomSize) + ", " +
                              juce::String(damping) + ", " +
                              juce::String(wetLevel) + ", " +
                              juce::String(dryLevel) + ", " +
                              juce::String(width) + ", " +
                              juce::String(freezeMode) + ")";
        webView->evaluateJavascript(script);

        lastRoomSize = roomSize;
        lastDamping = damping;
        lastWetLevel = wetLevel;
        lastDryLevel = dryLevel;
        lastWidth = width;
        lastFreezeMode = freezeMode;
    }

    // Update oscilloscope if there's new audio data
    {
        juce::ScopedLock lock(bufferLock);
        if (latestBuffer.getNumChannels() > 0 && latestBuffer.getNumSamples() > 0)
        {
            juce::String dataJson = prepareWaveformData();
            juce::String script = "window.updateOscilloscopeData(" + dataJson + ")";
            webView->evaluateJavascript(script);
        }
    }
}

void LayoutView::updateBuffer(const juce::AudioBuffer<float> &buffer)
{
    juce::ScopedLock lock(bufferLock);

    // Update our buffer copy with the latest audio data
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return;

    latestBuffer.makeCopyOf(buffer);
}

void LayoutView::updateLevels(float leftLevel, float rightLevel, float outLeftLevel, float outRightLevel)
{
    if (!pageLoaded)
        return;

    // Store the last known levels
    lastLeftLevel = leftLevel;
    lastRightLevel = rightLevel;

    // If signal is very close to zero, explicitly set it to zero
    if (leftLevel < 0.01f)
        leftLevel = 0.0f;
    if (rightLevel < 0.01f)
        rightLevel = 0.0f;
    if (outLeftLevel < 0.01f)
        outLeftLevel = 0.0f;
    if (outRightLevel < 0.01f)
        outRightLevel = 0.0f;

    try
    {
        // Ensure values are valid by using String conversion with proper formatting
        juce::String script = "window.setAudioState(" +
                              juce::String(leftLevel, 1) + ", " +
                              juce::String(rightLevel, 1) + ", " +
                              juce::String(outLeftLevel, 1) + ", " +
                              juce::String(outRightLevel, 1) + ")";

        webView->evaluateJavascript(script);
    }
    catch (const std::exception &e)
    {
        // Log any errors for debugging
        juce::Logger::writeToLog("JavaScript error in meters: " + juce::String(e.what()));
    }
}

juce::String LayoutView::prepareWaveformData()
{
    juce::ScopedLock lock(bufferLock);

    // Number of data points to display
    const int numPoints = 128;

    if (latestBuffer.getNumChannels() == 0 || latestBuffer.getNumSamples() == 0)
        return "[]";

    // Create a JSON array of waveform data points
    juce::String jsonArray = "[";

    // Combine channels if stereo
    if (latestBuffer.getNumChannels() > 1)
    {
        const float *leftChannel = latestBuffer.getReadPointer(0);
        const float *rightChannel = latestBuffer.getReadPointer(1);

        // Sample the buffer at regular intervals
        const int samplesPerPoint = juce::jmax(1, latestBuffer.getNumSamples() / numPoints);

        for (int i = 0; i < numPoints && i * samplesPerPoint < latestBuffer.getNumSamples(); ++i)
        {
            // Average the samples in this segment
            float sum = 0.0f;
            int count = 0;

            for (int j = 0; j < samplesPerPoint && i * samplesPerPoint + j < latestBuffer.getNumSamples(); ++j)
            {
                int sampleIndex = i * samplesPerPoint + j;
                // Average of left and right channels
                sum += (leftChannel[sampleIndex] + rightChannel[sampleIndex]) * 0.5f;
                count++;
            }

            // Calculate average value for this point
            float value = count > 0 ? sum / count : 0.0f;

            // Add to JSON array
            jsonArray += juce::String(value);

            // Add comma if not the last item
            if (i < numPoints - 1 && (i + 1) * samplesPerPoint < latestBuffer.getNumSamples())
                jsonArray += ",";
        }
    }
    else // Mono
    {
        const float *channel = latestBuffer.getReadPointer(0);

        // Sample the buffer at regular intervals
        const int samplesPerPoint = juce::jmax(1, latestBuffer.getNumSamples() / numPoints);

        for (int i = 0; i < numPoints && i * samplesPerPoint < latestBuffer.getNumSamples(); ++i)
        {
            // Average the samples in this segment
            float sum = 0.0f;
            int count = 0;

            for (int j = 0; j < samplesPerPoint && i * samplesPerPoint + j < latestBuffer.getNumSamples(); ++j)
            {
                sum += channel[i * samplesPerPoint + j];
                count++;
            }

            // Calculate average value for this point
            float value = count > 0 ? sum / count : 0.0f;

            // Add to JSON array
            jsonArray += juce::String(value);

            // Add comma if not the last item
            if (i < numPoints - 1 && (i + 1) * samplesPerPoint < latestBuffer.getNumSamples())
                jsonArray += ",";
        }
    }

    jsonArray += "]";
    return jsonArray;
}

void LayoutView::refreshAllParameters()
{
    // Force an immediate refresh of all parameters

    // Reverb parameters
    {
        float roomSize = reverbProcessor.getRoomSize();
        float damping = reverbProcessor.getDamping();
        float wetLevel = reverbProcessor.getWetLevel();
        float dryLevel = reverbProcessor.getDryLevel();
        float width = reverbProcessor.getWidth();
        float freezeMode = reverbProcessor.getFreezeMode();

        juce::String script = "window.setReverbValues(" +
                              juce::String(roomSize) + ", " +
                              juce::String(damping) + ", " +
                              juce::String(wetLevel) + ", " +
                              juce::String(dryLevel) + ", " +
                              juce::String(width) + ", " +
                              juce::String(freezeMode) + ")";
        webView->evaluateJavascript(script);

        lastRoomSize = roomSize;
        lastDamping = damping;
        lastWetLevel = wetLevel;
        lastDryLevel = dryLevel;
        lastWidth = width;
        lastFreezeMode = freezeMode;
    }

    // Update levels
    updateLevels(lastLeftLevel, lastRightLevel, 0.0f, 0.0f);
}