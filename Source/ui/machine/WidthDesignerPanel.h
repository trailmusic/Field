#pragma once
#include <JuceHeader.h>

class WidthDesignerPanel : public juce::Component
{
public:
    WidthDesignerPanel() { setAlwaysOnTop (true); }
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        juce::ColourGradient grad (
            juce::Colours::black.withAlpha (0.40f), r.getX(), r.getY(),
            juce::Colours::black.withAlpha (0.25f), r.getX(), r.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawRoundedRectangle (r.reduced (1.0f), 7.0f, 1.2f);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.drawText ("Width Designer (stub)", getLocalBounds(), juce::Justification::centred);
    }
};


