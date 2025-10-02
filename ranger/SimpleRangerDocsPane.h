#pragma once

#include <JuceHeader.h>

class SimpleRangerDocsPane : public juce::Component
{
public:
    SimpleRangerDocsPane();
    ~SimpleRangerDocsPane() override = default;
    
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::TextEditor docContent;
    void setupDocumentation();
};
