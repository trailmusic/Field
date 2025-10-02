#pragma once

#include <JuceHeader.h>

class RangerInstructionsPane : public juce::Component
{
public:
    RangerInstructionsPane();
    ~RangerInstructionsPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::Label titleLabel;
    juce::TextEditor instructionsEditor;
    
    void setupInstructions();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerInstructionsPane)
};
