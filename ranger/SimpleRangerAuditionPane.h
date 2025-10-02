#pragma once

#include <JuceHeader.h>

class SimpleRangerAuditionPane : public juce::Component
{
public:
    SimpleRangerAuditionPane();
    ~SimpleRangerAuditionPane() override = default;
    
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::Label titleLabel;
    juce::Label descriptionLabel;
    juce::TextButton generateButton;
    juce::TextButton exportButton;
    juce::ComboBox viewCombo;
    juce::ComboBox normalizationCombo;
    
    void setupControls();
    void generateAudition();
    void exportResults();
};
