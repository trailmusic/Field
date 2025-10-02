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
    // Settings components
    juce::Label titleLabel;
    
    // Normalization
    juce::Label normLabel;
    juce::ComboBox normCombo;
    
    // FFT settings
    juce::Label fftLabel;
    juce::ComboBox fftCombo;
    
    // Output settings
    juce::Label outputLabel;
    juce::TextEditor outputPrefix;
    juce::ToggleButton emitCsvButton;
    
    // Diff thresholds
    juce::Label diffLabel;
    juce::Slider sampleThreshold;
    juce::Slider magThreshold;
    
    // Action buttons
    juce::TextButton convertButton;
    juce::TextButton compareButton;
    juce::TextButton exportButton;
    
    // Status
    juce::Label statusLabel;
    
    void setupComponents();
    void convertToMinPhase();
    void compareWithBaseline();
    void exportBank();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerSettingsPane)
};
