#pragma once

#include <JuceHeader.h>

class RangerDesigner : public juce::Component
{
public:
    RangerDesigner();
    ~RangerDesigner() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // UI Components
    juce::TextButton loadButton;
    juce::TextButton generateButton;
    juce::TextButton exportButton;
    juce::TextButton clearButton;
    
    juce::Label titleLabel;
    juce::Label statusLabel;
    
    // File info
    juce::Label fileInfoLabel;
    juce::TextEditor filePathEditor;
    
    // Filter parameters
    juce::Slider orderSlider;
    juce::Label orderLabel;
    
    juce::Slider cutoffSlider;
    juce::Label cutoffLabel;
    
    juce::ComboBox filterTypeCombo;
    juce::Label filterTypeLabel;
    
    // Results
    juce::TextEditor resultsEditor;
    juce::Label resultsLabel;
    
    // Methods
    void loadFile();
    void generateFilter();
    void exportResults();
    void clearAll();
    
    void updateStatus(const juce::String& message);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerDesigner)
};