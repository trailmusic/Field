#include "RangerPlotPane.h"

RangerPlotPane::RangerPlotPane()
{
    // Plot title
    plotTitleLabel.setText("Filter Analysis Plots", juce::dontSendNotification);
    plotTitleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    plotTitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(plotTitleLabel);
    
    // Plot type
    plotTypeLabel.setText("Plot Type:", juce::dontSendNotification);
    addAndMakeVisible(plotTypeLabel);
    
    plotTypeCombo.addItem("Frequency Response", 1);
    plotTypeCombo.addItem("Impulse Response", 2);
    plotTypeCombo.addItem("Phase Response", 3);
    plotTypeCombo.addItem("Group Delay", 4);
    plotTypeCombo.setSelectedId(1);
    plotTypeCombo.onChange = [this] { plotType = plotTypeCombo.getSelectedId() - 1; repaint(); };
    addAndMakeVisible(plotTypeCombo);
    
    // Buttons
    generatePlotButton.setButtonText("Generate Plot");
    generatePlotButton.onClick = [this] { generatePlot(); };
    addAndMakeVisible(generatePlotButton);
    
    exportPlotButton.setButtonText("Export Plot");
    exportPlotButton.onClick = [this] { exportPlot(); };
    addAndMakeVisible(exportPlotButton);
    
    // Plot area
    addAndMakeVisible(plotArea);
    
    // Set up the plot area
    plotArea.setComponentEffect(nullptr);
}

RangerPlotPane::~RangerPlotPane()
{
}

void RangerPlotPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    
    // Draw some visual elements
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawRect(getLocalBounds(), 2);
    
    // Draw the plot in the plot area
    auto plotBounds = plotArea.getBounds();
    if (plotBounds.getWidth() > 0 && plotBounds.getHeight() > 0)
    {
        auto bounds = plotBounds.reduced(20);
        
        switch (plotType)
        {
            case 0: drawFrequencyResponse(g, bounds); break;
            case 1: drawImpulseResponse(g, bounds); break;
            case 2: drawPhaseResponse(g, bounds); break;
            case 3: drawGroupDelay(g, bounds); break;
        }
    }
}

void RangerPlotPane::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Title
    plotTitleLabel.setBounds(bounds.removeFromTop(40));
    bounds.removeFromTop(20);
    
    // Controls
    auto controlRow = bounds.removeFromTop(40);
    plotTypeLabel.setBounds(controlRow.removeFromLeft(80));
    plotTypeCombo.setBounds(controlRow.removeFromLeft(150));
    controlRow.removeFromLeft(20);
    generatePlotButton.setBounds(controlRow.removeFromLeft(120));
    controlRow.removeFromLeft(10);
    exportPlotButton.setBounds(controlRow.removeFromLeft(120));
    
    bounds.removeFromTop(20);
    
    // Plot area
    plotArea.setBounds(bounds);
}

void RangerPlotPane::generatePlot()
{
    // Generate plot based on selected type
    plotArea.repaint();
}

void RangerPlotPane::exportPlot()
{
    auto chooser = std::make_unique<juce::FileChooser>("Save plot as...",
                                                       juce::File(),
                                                       "*.png;*.jpg;*.svg");
    
    auto chooserFlags = juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles;
    
    chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
    {
        auto file = fc.getResult();
        if (file != juce::File{})
        {
            // Export plot logic would go here
        }
    });
}

// Plot data methods
void RangerPlotPane::setFilterData(const juce::Array<float>& taps)
{
    filterTaps = taps;
    impulseResponse = taps; // For impulse response, taps are the impulse response
    
    // Calculate frequency response
    magnitudeResponse = calculateFrequencyResponse(taps);
    phaseResponse = calculatePhaseResponse(taps);
    
    repaint();
}

void RangerPlotPane::setFrequencyResponse(const juce::Array<float>& magnitude, const juce::Array<float>& phase)
{
    magnitudeResponse = magnitude;
    phaseResponse = phase;
    repaint();
}

// Plot drawing methods
void RangerPlotPane::drawFrequencyResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    if (magnitudeResponse.isEmpty()) return;
    
    g.setColour(juce::Colour(0xff00ff00));
    g.drawRect(bounds);
    
    // Draw axes
    g.setColour(juce::Colour(0xff666666));
    g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());
    
    // Draw frequency response
    g.setColour(juce::Colour(0xff00ff00));
    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());
    
    for (int i = 0; i < magnitudeResponse.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / magnitudeResponse.size();
        float y = bounds.getCentreY() - (magnitudeResponse[i] * bounds.getHeight() / 2);
        path.lineTo(x, y);
    }
    
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void RangerPlotPane::drawImpulseResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    if (impulseResponse.isEmpty()) return;
    
    g.setColour(juce::Colour(0xff00ff00));
    g.drawRect(bounds);
    
    // Draw axes
    g.setColour(juce::Colour(0xff666666));
    g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());
    
    // Draw impulse response
    g.setColour(juce::Colour(0xff00ff00));
    for (int i = 0; i < impulseResponse.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / impulseResponse.size();
        float y = bounds.getCentreY() - (impulseResponse[i] * bounds.getHeight() / 2);
        g.drawLine(x, y, x, bounds.getCentreY());
    }
}

void RangerPlotPane::drawPhaseResponse(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    if (phaseResponse.isEmpty()) return;
    
    g.setColour(juce::Colour(0xff00ff00));
    g.drawRect(bounds);
    
    // Draw axes
    g.setColour(juce::Colour(0xff666666));
    g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());
    
    // Draw phase response
    g.setColour(juce::Colour(0xff00ff00));
    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());
    
    for (int i = 0; i < phaseResponse.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / phaseResponse.size();
        float y = bounds.getCentreY() - (phaseResponse[i] * bounds.getHeight() / 2);
        path.lineTo(x, y);
    }
    
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void RangerPlotPane::drawGroupDelay(juce::Graphics& g, const juce::Rectangle<int>& bounds)
{
    if (phaseResponse.isEmpty()) return;
    
    g.setColour(juce::Colour(0xff00ff00));
    g.drawRect(bounds);
    
    // Draw axes
    g.setColour(juce::Colour(0xff666666));
    g.drawLine(bounds.getX(), bounds.getCentreY(), bounds.getRight(), bounds.getCentreY());
    g.drawLine(bounds.getX(), bounds.getY(), bounds.getX(), bounds.getBottom());
    
    // Calculate and draw group delay
    juce::Array<float> groupDelay = calculateGroupDelay(filterTaps);
    
    g.setColour(juce::Colour(0xff00ff00));
    juce::Path path;
    path.startNewSubPath(bounds.getX(), bounds.getCentreY());
    
    for (int i = 0; i < groupDelay.size(); ++i)
    {
        float x = bounds.getX() + (i * bounds.getWidth()) / groupDelay.size();
        float y = bounds.getCentreY() - (groupDelay[i] * bounds.getHeight() / 2);
        path.lineTo(x, y);
    }
    
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

// Utility functions
juce::Array<float> RangerPlotPane::calculateFrequencyResponse(const juce::Array<float>& taps)
{
    juce::Array<float> response;
    int numPoints = 1024;
    
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = (float)i / numPoints;
        float magnitude = 0.0f;
        
        for (int j = 0; j < taps.size(); ++j)
        {
            magnitude += taps[j] * std::cos(2.0f * juce::MathConstants<float>::pi * freq * j);
        }
        
        response.add(std::abs(magnitude));
    }
    
    return response;
}

juce::Array<float> RangerPlotPane::calculatePhaseResponse(const juce::Array<float>& taps)
{
    juce::Array<float> response;
    int numPoints = 1024;
    
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = (float)i / numPoints;
        float real = 0.0f, imag = 0.0f;
        
        for (int j = 0; j < taps.size(); ++j)
        {
            real += taps[j] * std::cos(2.0f * juce::MathConstants<float>::pi * freq * j);
            imag += taps[j] * std::sin(2.0f * juce::MathConstants<float>::pi * freq * j);
        }
        
        response.add(std::atan2(imag, real));
    }
    
    return response;
}

juce::Array<float> RangerPlotPane::calculateGroupDelay(const juce::Array<float>& taps)
{
    juce::Array<float> response;
    int numPoints = 1024;
    
    for (int i = 0; i < numPoints; ++i)
    {
        float freq = (float)i / numPoints;
        float real = 0.0f, imag = 0.0f;
        float dReal = 0.0f, dImag = 0.0f;
        
        for (int j = 0; j < taps.size(); ++j)
        {
            float angle = 2.0f * juce::MathConstants<float>::pi * freq * j;
            real += taps[j] * std::cos(angle);
            imag += taps[j] * std::sin(angle);
            dReal += taps[j] * (-j) * std::sin(angle);
            dImag += taps[j] * j * std::cos(angle);
        }
        
        float groupDelay = (real * dImag - imag * dReal) / (real * real + imag * imag);
        response.add(groupDelay);
    }
    
    return response;
}