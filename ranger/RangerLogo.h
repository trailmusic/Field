#pragma once

#include <JuceHeader.h>
#include "SimpleFieldTheme.h"

class RangerLogo : public juce::Component
{
public:
    RangerLogo();
    ~RangerLogo() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Theme support
    void setTheme(const SimpleFieldTheme& theme);

private:
    SimpleFieldTheme currentTheme;
    juce::String logoText = "FIELD RANGER";
    juce::String tagline = "Patrol quality, phase, and oversampling.";
    
    void drawFieldStylePanel(juce::Graphics& g, juce::Rectangle<float> bounds, float radius);
    void drawFieldStyleLogo(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawMetallicEffect(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawTagline(juce::Graphics& g, juce::Rectangle<float> bounds);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerLogo)
};
