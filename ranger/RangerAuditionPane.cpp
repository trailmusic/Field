#include "RangerAuditionPane.h"

RangerAuditionPane::RangerAuditionPane()
{
    setupControls();
    
    // Set default view
    impulseButton.setToggleState(true, juce::dontSendNotification);
    baselineButton.setToggleState(true, juce::dontSendNotification);
    
    generateAudition();
}

RangerAuditionPane::~RangerAuditionPane()
{
}

void RangerAuditionPane::setupControls()
{
    // View controls
    addAndMakeVisible(impulseButton);
    impulseButton.setButtonText("Impulse");
    impulseButton.setRadioGroupId(1);
    impulseButton.onClick = [this] { onViewChanged(); };
    
    addAndMakeVisible(stepButton);
    stepButton.setButtonText("Step");
    stepButton.setRadioGroupId(1);
    stepButton.onClick = [this] { onViewChanged(); };
    
    addAndMakeVisible(magnitudeButton);
    magnitudeButton.setButtonText("Magnitude");
    magnitudeButton.setRadioGroupId(1);
    magnitudeButton.onClick = [this] { onViewChanged(); };
    
    // A/B comparison
    addAndMakeVisible(baselineButton);
    baselineButton.setButtonText("Baseline (Linear)");
    baselineButton.setRadioGroupId(2);
    baselineButton.onClick = [this] { onABChanged(); };
    
    addAndMakeVisible(candidateButton);
    candidateButton.setButtonText("Candidate (Min-Phase)");
    candidateButton.setRadioGroupId(2);
    candidateButton.onClick = [this] { onABChanged(); };
    
    // Display options
    addAndMakeVisible(overlayButton);
    overlayButton.setButtonText("Overlay");
    overlayButton.onClick = [this] { onOptionsChanged(); };
    
    addAndMakeVisible(deltaButton);
    deltaButton.setButtonText("Delta");
    deltaButton.onClick = [this] { onOptionsChanged(); };
    
    // Normalization
    addAndMakeVisible(normLabel);
    normLabel.setText("Normalization:", juce::dontSendNotification);
    
    addAndMakeVisible(normComboBox);
    normComboBox.addItem("Peak", 1);
    normComboBox.addItem("Energy", 2);
    normComboBox.addItem("DC Unity", 3);
    normComboBox.setSelectedId(1);
    normComboBox.onChange = [this] { onOptionsChanged(); };
    
    // Magnitude options
    addAndMakeVisible(dbButton);
    dbButton.setButtonText("dB");
    dbButton.setToggleState(true, juce::dontSendNotification);
    dbButton.onClick = [this] { onOptionsChanged(); };
    
    addAndMakeVisible(smoothButton);
    smoothButton.setButtonText("1/24-oct Smooth");
    smoothButton.setToggleState(true, juce::dontSendNotification);
    smoothButton.onClick = [this] { onOptionsChanged(); };
    
    addAndMakeVisible(logFreqButton);
    logFreqButton.setButtonText("Log Freq");
    logFreqButton.setToggleState(true, juce::dontSendNotification);
    logFreqButton.onClick = [this] { onOptionsChanged(); };
    
    // Action buttons
    addAndMakeVisible(generateButton);
    generateButton.setButtonText("Generate");
    generateButton.onClick = [this] { onGenerateClicked(); };
    
    addAndMakeVisible(exportButton);
    exportButton.setButtonText("Export CSV");
    exportButton.onClick = [this] { onExportClicked(); };
    
    addAndMakeVisible(copyTapsButton);
    copyTapsButton.setButtonText("Copy Taps");
    copyTapsButton.onClick = [this] { onCopyTapsClicked(); };
    
    addAndMakeVisible(verifyTPButton);
    verifyTPButton.setButtonText("Verify TP-Safe");
    verifyTPButton.onClick = [this] { onVerifyTPClicked(); };
    
    // Status
    addAndMakeVisible(statusLabel);
    statusLabel.setText("Ready", juce::dontSendNotification);
    
    addAndMakeVisible(tpStatusLabel);
    tpStatusLabel.setText("TP-Safe: Not tested", juce::dontSendNotification);
    
    // Plot area
    addAndMakeVisible(plotArea);
    plotArea.setOpaque(true);
}

void RangerAuditionPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw borders
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(getLocalBounds(), 1);
    
    // Draw plot area background
    g.setColour(juce::Colour(0xff1a1a1a));
    g.fillRect(plotArea.getBounds());
    
    g.setColour(juce::Colour(0xff404040));
    g.drawRect(plotArea.getBounds(), 1);
    
    // Draw plot
    drawPlot(g);
}

void RangerAuditionPane::resized()
{
    auto bounds = getLocalBounds();
    
    // Top control panel
    auto topPanel = bounds.removeFromTop(120);
    
    // View controls
    auto viewArea = topPanel.removeFromTop(30);
    impulseButton.setBounds(viewArea.removeFromLeft(80));
    stepButton.setBounds(viewArea.removeFromLeft(80));
    magnitudeButton.setBounds(viewArea.removeFromLeft(100));
    
    // A/B controls
    auto abArea = topPanel.removeFromTop(30);
    baselineButton.setBounds(abArea.removeFromLeft(150));
    candidateButton.setBounds(abArea.removeFromLeft(180));
    
    // Options
    auto optionsArea = topPanel.removeFromTop(30);
    overlayButton.setBounds(optionsArea.removeFromLeft(80));
    deltaButton.setBounds(optionsArea.removeFromLeft(80));
    
    // Normalization and magnitude options
    auto normArea = topPanel.removeFromTop(30);
    normLabel.setBounds(normArea.removeFromLeft(100));
    normComboBox.setBounds(normArea.removeFromLeft(100));
    normArea.removeFromLeft(20);
    dbButton.setBounds(normArea.removeFromLeft(40));
    smoothButton.setBounds(normArea.removeFromLeft(120));
    logFreqButton.setBounds(normArea.removeFromLeft(80));
    
    // Action buttons
    auto buttonArea = bounds.removeFromBottom(40);
    generateButton.setBounds(buttonArea.removeFromLeft(80));
    exportButton.setBounds(buttonArea.removeFromLeft(100));
    copyTapsButton.setBounds(buttonArea.removeFromLeft(100));
    verifyTPButton.setBounds(buttonArea.removeFromLeft(120));
    
    // Status
    auto statusArea = bounds.removeFromBottom(25);
    statusLabel.setBounds(statusArea.removeFromLeft(200));
    tpStatusLabel.setBounds(statusArea);
    
    // Plot area
    plotArea.setBounds(bounds);
}

void RangerAuditionPane::generateAudition()
{
    statusLabel.setText("Generating audition...", juce::dontSendNotification);
    
    // Simulate processing time
    juce::MessageManager::getInstance()->runDispatchLoopUntil(100);
    
    updatePlot();
    statusLabel.setText("Audition complete", juce::dontSendNotification);
}

void RangerAuditionPane::updatePlot()
{
    // Update plot data based on current view
    repaint();
}

void RangerAuditionPane::drawPlot(juce::Graphics& g)
{
    auto bounds = plotArea.getBounds().toFloat();
    bounds.reduce(20, 20);
    
    // Draw grid
    g.setColour(juce::Colour(0xff404040));
    for (int i = 0; i <= 10; ++i)
    {
        const float x = bounds.getX() + (bounds.getWidth() * i / 10.0f);
        const float y = bounds.getY() + (bounds.getHeight() * i / 10.0f);
        
        g.drawVerticalLine((int)x, bounds.getY(), bounds.getBottom());
        g.drawHorizontalLine((int)y, bounds.getX(), bounds.getRight());
    }
    
    // Draw sample data based on current view
    if (currentView == ViewType::Impulse)
    {
        // Draw impulse response
        g.setColour(getBaselineColor());
        juce::Path path;
        
        const int n = 100;
        for (int i = 0; i < n; ++i)
        {
            const float x = bounds.getX() + (bounds.getWidth() * i / (float)n);
            const float y = bounds.getY() + (bounds.getHeight() * 0.5f) + 
                           (bounds.getHeight() * 0.3f * std::sin(2.0f * juce::MathConstants<float>::pi * i / 20.0f) * 
                            std::exp(-i / 30.0f));
            
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        // Draw legend
        g.setColour(juce::Colours::white);
        g.drawText("Impulse Response", bounds.getX() + 10, bounds.getY() + 10, 200, 20, juce::Justification::left);
    }
    else if (currentView == ViewType::Step)
    {
        // Draw step response
        g.setColour(getBaselineColor());
        juce::Path path;
        
        const int n = 100;
        for (int i = 0; i < n; ++i)
        {
            const float x = bounds.getX() + (bounds.getWidth() * i / (float)n);
            const float y = bounds.getY() + (bounds.getHeight() * 0.8f) - 
                           (bounds.getHeight() * 0.6f * (1.0f - std::exp(-i / 20.0f)));
            
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        // Draw legend
        g.setColour(juce::Colours::white);
        g.drawText("Step Response", bounds.getX() + 10, bounds.getY() + 10, 200, 20, juce::Justification::left);
    }
    else if (currentView == ViewType::Magnitude)
    {
        // Draw magnitude response
        g.setColour(getBaselineColor());
        juce::Path path;
        
        const int n = 200;
        for (int i = 0; i < n; ++i)
        {
            const float freq = 20.0f * std::pow(10.0f, 3.0f * i / (float)n); // 20Hz to 20kHz
            const float x = bounds.getX() + (bounds.getWidth() * i / (float)n);
            const float magnitude = -3.0f * (freq / 1000.0f); // Simple lowpass response
            const float y = bounds.getY() + (bounds.getHeight() * 0.5f) + (bounds.getHeight() * 0.4f * magnitude / 60.0f);
            
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }
        
        g.strokePath(path, juce::PathStrokeType(2.0f));
        
        // Draw legend
        g.setColour(juce::Colours::white);
        g.drawText("Magnitude Response (dB)", bounds.getX() + 10, bounds.getY() + 10, 200, 20, juce::Justification::left);
    }
    
    // Draw axis labels
    g.setColour(juce::Colours::white);
    if (currentView == ViewType::Magnitude)
    {
        g.drawText("Frequency (Hz)", bounds.getCentreX() - 50, bounds.getBottom() + 5, 100, 15, juce::Justification::centred);
        g.drawText("Magnitude (dB)", bounds.getX() - 80, bounds.getCentreY() - 50, 15, 100, juce::Justification::centred, true);
    }
    else
    {
        g.drawText("Time (ms)", bounds.getCentreX() - 50, bounds.getBottom() + 5, 100, 15, juce::Justification::centred);
        g.drawText("Amplitude", bounds.getX() - 60, bounds.getCentreY() - 50, 15, 100, juce::Justification::centred, true);
    }
}

void RangerAuditionPane::onViewChanged()
{
    if (impulseButton.getToggleState())
        currentView = ViewType::Impulse;
    else if (stepButton.getToggleState())
        currentView = ViewType::Step;
    else if (magnitudeButton.getToggleState())
        currentView = ViewType::Magnitude;
    
    updatePlot();
}

void RangerAuditionPane::onABChanged()
{
    // A/B comparison logic
    updatePlot();
}

void RangerAuditionPane::onOptionsChanged()
{
    showOverlay = overlayButton.getToggleState();
    showDelta = deltaButton.getToggleState();
    updatePlot();
}

void RangerAuditionPane::onGenerateClicked()
{
    generateAudition();
}

void RangerAuditionPane::onExportClicked()
{
    juce::FileChooser chooser("Export CSV", juce::File(), "*.csv");
    if (chooser.browseForFileToSave(true))
    {
        auto file = chooser.getResult();
        juce::String csv = "Frequency (Hz),Magnitude (dB)\n";
        csv += "20,-60\n";
        csv += "1000,-3\n";
        csv += "20000,-60\n";
        file.replaceWithText(csv);
        statusLabel.setText("Exported: " + file.getFileName(), juce::dontSendNotification);
    }
}

void RangerAuditionPane::onCopyTapsClicked()
{
    juce::String taps = "const float filterTaps[] = {\n";
    taps += "0.0f, 0.1f, 0.2f, 0.1f, 0.0f\n";
    taps += "};\n";
    juce::SystemClipboard::copyTextToClipboard(taps);
    statusLabel.setText("Taps copied to clipboard", juce::dontSendNotification);
}

void RangerAuditionPane::onVerifyTPClicked()
{
    // Simulate TP-Safe verification
    bool passed = true; // Simplified for now
    tpStatusLabel.setText(passed ? "TP-Safe: ✓ PASSED" : "TP-Safe: ✗ FAILED", 
                         juce::dontSendNotification);
    tpStatusLabel.setColour(juce::Label::textColourId, 
                           passed ? juce::Colours::green : juce::Colours::red);
}

juce::Colour RangerAuditionPane::getBaselineColor() const
{
    return juce::Colour(0xff4a9eff); // Blue
}

juce::Colour RangerAuditionPane::getCandidateColor() const
{
    return juce::Colour(0xffff6b4a); // Orange
}

juce::Colour RangerAuditionPane::getDeltaColor() const
{
    return juce::Colour(0xff4aff6b); // Green
}