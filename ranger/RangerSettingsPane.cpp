#include "RangerSettingsPane.h"

RangerSettingsPane::RangerSettingsPane()
{
    // General settings
    generalLabel.setText("General Settings", juce::dontSendNotification);
    generalLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(generalLabel);
    
    autoSaveLabel.setText("Auto-save results:", juce::dontSendNotification);
    addAndMakeVisible(autoSaveLabel);
    
    autoSaveToggle.setToggleState(true, juce::dontSendNotification);
    addAndMakeVisible(autoSaveToggle);
    
    themeLabel.setText("Theme:", juce::dontSendNotification);
    addAndMakeVisible(themeLabel);
    
    themeCombo.addItem("Dark", 1);
    themeCombo.addItem("Light", 2);
    themeCombo.addItem("Auto", 3);
    themeCombo.setSelectedId(1);
    addAndMakeVisible(themeCombo);
    
    // Advanced settings
    advancedLabel.setText("Advanced Settings", juce::dontSendNotification);
    advancedLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(advancedLabel);
    
    precisionLabel.setText("Calculation Precision:", juce::dontSendNotification);
    addAndMakeVisible(precisionLabel);
    
    precisionSlider.setRange(0.001, 0.000001, 0.000001);
    precisionSlider.setValue(0.0001);
    precisionSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
    addAndMakeVisible(precisionSlider);
    
    maxOrderLabel.setText("Maximum Filter Order:", juce::dontSendNotification);
    addAndMakeVisible(maxOrderLabel);
    
    maxOrderSlider.setRange(63, 511, 1);
    maxOrderSlider.setValue(255);
    maxOrderSlider.setTextBoxStyle(juce::Slider::TextBoxLeft, false, 80, 20);
    addAndMakeVisible(maxOrderSlider);
    
    // About
    aboutLabel.setText("About Field Ranger", juce::dontSendNotification);
    aboutLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    addAndMakeVisible(aboutLabel);
    
    versionLabel.setText("Version 1.0.0", juce::dontSendNotification);
    addAndMakeVisible(versionLabel);
    
    copyrightLabel.setText("© 2025 Trail Audio. All rights reserved.", juce::dontSendNotification);
    addAndMakeVisible(copyrightLabel);
}

RangerSettingsPane::~RangerSettingsPane()
{
}

void RangerSettingsPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
    
    // Draw some visual elements
    g.setColour(juce::Colour(0xff3d3d3d));
    g.drawRect(getLocalBounds(), 2);
}

void RangerSettingsPane::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // General settings
    generalLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    auto generalRow1 = bounds.removeFromTop(30);
    autoSaveLabel.setBounds(generalRow1.removeFromLeft(150));
    autoSaveToggle.setBounds(generalRow1.removeFromLeft(50));
    
    bounds.removeFromTop(10);
    
    auto generalRow2 = bounds.removeFromTop(30);
    themeLabel.setBounds(generalRow2.removeFromLeft(80));
    themeCombo.setBounds(generalRow2.removeFromLeft(100));
    
    bounds.removeFromTop(30);
    
    // Advanced settings
    advancedLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    auto advancedRow1 = bounds.removeFromTop(30);
    precisionLabel.setBounds(advancedRow1.removeFromLeft(180));
    precisionSlider.setBounds(advancedRow1.removeFromLeft(200));
    
    bounds.removeFromTop(10);
    
    auto advancedRow2 = bounds.removeFromTop(30);
    maxOrderLabel.setBounds(advancedRow2.removeFromLeft(180));
    maxOrderSlider.setBounds(advancedRow2.removeFromLeft(200));
    
    bounds.removeFromTop(30);
    
    // About
    aboutLabel.setBounds(bounds.removeFromTop(30));
    bounds.removeFromTop(10);
    
    versionLabel.setBounds(bounds.removeFromTop(25));
    copyrightLabel.setBounds(bounds.removeFromTop(25));
}

void RangerSettingsPane::updateSettings()
{
    // Update settings based on UI values
}