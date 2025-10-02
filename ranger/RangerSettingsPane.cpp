#include "RangerSettingsPane.h"

RangerSettingsPane::RangerSettingsPane()
{
    setupComponents();
}

RangerSettingsPane::~RangerSettingsPane()
{
}

void RangerSettingsPane::setupComponents()
{
    // Title
    titleLabel.setText("Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);
    
    // Normalization
    normLabel.setText("Normalization:", juce::dontSendNotification);
    normLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(normLabel);
    
    normCombo.addItem("None", 1);
    normCombo.addItem("Unity", 2);
    normCombo.addItem("DC", 3);
    normCombo.setSelectedId(2);
    addAndMakeVisible(normCombo);
    
    // FFT settings
    fftLabel.setText("FFT Pad:", juce::dontSendNotification);
    fftLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(fftLabel);
    
    fftCombo.addItem("Auto", 1);
    fftCombo.addItem("1024", 2);
    fftCombo.addItem("2048", 3);
    fftCombo.addItem("4096", 4);
    fftCombo.setSelectedId(1);
    addAndMakeVisible(fftCombo);
    
    // Output settings
    outputLabel.setText("Output Prefix:", juce::dontSendNotification);
    outputLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(outputLabel);
    
    outputPrefix.setText("HB");
    outputPrefix.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff3a3a3a));
    outputPrefix.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    addAndMakeVisible(outputPrefix);
    
    emitCsvButton.setButtonText("Emit CSV");
    emitCsvButton.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(emitCsvButton);
    
    // Diff thresholds
    diffLabel.setText("Diff Thresholds:", juce::dontSendNotification);
    diffLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(diffLabel);
    
    sampleThreshold.setRange(1e-8, 1e-3, 1e-8);
    sampleThreshold.setValue(1e-6);
    sampleThreshold.setTextValueSuffix(" (sample)");
    addAndMakeVisible(sampleThreshold);
    
    magThreshold.setRange(0.01, 1.0, 0.01);
    magThreshold.setValue(0.1);
    magThreshold.setTextValueSuffix(" dB (mag)");
    addAndMakeVisible(magThreshold);
    
    // Action buttons
    convertButton.setButtonText("Convert → Min-Phase");
    convertButton.onClick = [this] { convertToMinPhase(); };
    addAndMakeVisible(convertButton);
    
    compareButton.setButtonText("Compare");
    compareButton.onClick = [this] { compareWithBaseline(); };
    addAndMakeVisible(compareButton);
    
    exportButton.setButtonText("Export Bank");
    exportButton.onClick = [this] { exportBank(); };
    addAndMakeVisible(exportButton);
    
    // Status
    statusLabel.setText("Ready", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
    addAndMakeVisible(statusLabel);
}

void RangerSettingsPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff2a2a2a)); // Dark panel background
    
    // Draw border
    g.setColour(juce::Colours::darkgrey);
    g.drawRect(getLocalBounds(), 1);
}

void RangerSettingsPane::resized()
{
    auto bounds = getLocalBounds().reduced(8);
    
    titleLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(8);
    
    // Normalization
    normLabel.setBounds(bounds.removeFromTop(20));
    normCombo.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(8);
    
    // FFT
    fftLabel.setBounds(bounds.removeFromTop(20));
    fftCombo.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(8);
    
    // Output
    outputLabel.setBounds(bounds.removeFromTop(20));
    outputPrefix.setBounds(bounds.removeFromTop(25));
    bounds.removeFromTop(4);
    emitCsvButton.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(8);
    
    // Diff thresholds
    diffLabel.setBounds(bounds.removeFromTop(20));
    sampleThreshold.setBounds(bounds.removeFromTop(30));
    magThreshold.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(8);
    
    // Buttons
    convertButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(4);
    compareButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(4);
    exportButton.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(8);
    
    // Status
    statusLabel.setBounds(bounds.removeFromTop(20));
}

void RangerSettingsPane::convertToMinPhase()
{
    statusLabel.setText("Converting...", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    repaint();
    
    // TODO: Implement conversion logic
    juce::MessageManager::getInstance()->callAsync([this] {
        statusLabel.setText("Conversion complete", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        repaint();
    });
}

void RangerSettingsPane::compareWithBaseline()
{
    statusLabel.setText("Comparing...", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    repaint();
    
    // TODO: Implement comparison logic
    juce::MessageManager::getInstance()->callAsync([this] {
        statusLabel.setText("Comparison complete", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        repaint();
    });
}

void RangerSettingsPane::exportBank()
{
    statusLabel.setText("Exporting...", juce::dontSendNotification);
    statusLabel.setColour(juce::Label::textColourId, juce::Colours::yellow);
    repaint();
    
    // TODO: Implement export logic
    juce::MessageManager::getInstance()->callAsync([this] {
        statusLabel.setText("Export complete", juce::dontSendNotification);
        statusLabel.setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        repaint();
    });
}
