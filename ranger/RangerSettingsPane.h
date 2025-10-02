#pragma once

#include <JuceHeader.h>

class RangerSettingsPane : public juce::Component
{
public:
    RangerSettingsPane();
    ~RangerSettingsPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Settings sections
    juce::Label generalLabel;
    juce::Label advancedLabel;
    juce::Label aboutLabel;
    
    // General settings
    juce::ToggleButton autoSaveToggle;
    juce::Label autoSaveLabel;
    
    juce::ComboBox themeCombo;
    juce::Label themeLabel;
    
    // Advanced settings
    juce::Slider precisionSlider;
    juce::Label precisionLabel;
    
    juce::Slider maxOrderSlider;
    juce::Label maxOrderLabel;
    
    // About
    juce::Label versionLabel;
    juce::Label copyrightLabel;
    
    // Methods
    void updateSettings();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerSettingsPane)
};