#include "SimpleRangerAuditionPane.h"

SimpleRangerAuditionPane::SimpleRangerAuditionPane()
{
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(descriptionLabel);
    addAndMakeVisible(generateButton);
    addAndMakeVisible(exportButton);
    addAndMakeVisible(viewCombo);
    addAndMakeVisible(normalizationCombo);
    
    setupControls();
}

void SimpleRangerAuditionPane::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    descriptionLabel.setBounds(bounds.removeFromTop(60));
    bounds.removeFromTop(20);
    
    auto controlArea = bounds.removeFromTop(100);
    viewCombo.setBounds(controlArea.removeFromTop(25));
    controlArea.removeFromTop(5);
    normalizationCombo.setBounds(controlArea.removeFromTop(25));
    controlArea.removeFromTop(10);
    
    auto buttonArea = bounds.removeFromTop(40);
    generateButton.setBounds(buttonArea.removeFromLeft(120));
    buttonArea.removeFromLeft(10);
    exportButton.setBounds(buttonArea.removeFromLeft(120));
}

void SimpleRangerAuditionPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a));
    
    // Draw a placeholder plot area
    auto plotArea = getLocalBounds().reduced(20);
    plotArea.removeFromTop(200);
    
    g.setColour(juce::Colour(0xff404040));
    g.fillRect(plotArea);
    
    g.setColour(juce::Colour(0xff808080));
    g.drawRect(plotArea, 1);
    
    g.setColour(juce::Colour(0xffaaaaaa));
    g.setFont(14.0f);
    g.drawText("Audition Plot Area", plotArea, juce::Justification::centred);
}

void SimpleRangerAuditionPane::setupControls()
{
    titleLabel.setText("Filter Audition", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colour(0xffaaaaaa));
    
    descriptionLabel.setText("Test your filters with impulse, step, and magnitude responses. Compare linear vs minimum-phase characteristics.", juce::dontSendNotification);
    descriptionLabel.setFont(juce::Font(12.0f));
    descriptionLabel.setColour(juce::Label::textColourId, juce::Colour(0xff888888));
    descriptionLabel.setJustificationType(juce::Justification::topLeft);
    
    viewCombo.addItem("Impulse Response", 1);
    viewCombo.addItem("Step Response", 2);
    viewCombo.addItem("Magnitude Response", 3);
    viewCombo.setSelectedId(1);
    
    normalizationCombo.addItem("Peak Normalized", 1);
    normalizationCombo.addItem("Energy Normalized", 2);
    normalizationCombo.addItem("DC Unity", 3);
    normalizationCombo.setSelectedId(1);
    
    generateButton.setButtonText("Generate");
    generateButton.onClick = [this] { generateAudition(); };
    
    exportButton.setButtonText("Export CSV");
    exportButton.onClick = [this] { exportResults(); };
}

void SimpleRangerAuditionPane::generateAudition()
{
    // Placeholder for audition generation
    juce::AlertWindow::showMessageBox(juce::AlertWindow::InfoIcon, 
                                     "Audition", 
                                     "Audition generation will be implemented with real filter testing.");
}

void SimpleRangerAuditionPane::exportResults()
{
    // Placeholder for export functionality
    juce::AlertWindow::showMessageBox(juce::AlertWindow::InfoIcon, 
                                     "Export", 
                                     "Export functionality will be implemented for CSV output.");
}
