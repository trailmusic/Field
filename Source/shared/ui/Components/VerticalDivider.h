#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

class VerticalDivider : public juce::Component
{
public:
    VerticalDivider(FieldLNF& l) : lnf(l) {}
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::transparentBlack);
        auto b = getLocalBounds().toFloat();
        // Thicker divider with subtle insets for spacing
        const float w = juce::jmax(2.0f, b.getWidth());
        const float x = b.getX();
        // Outer soft lines
        g.setColour(lnf.theme.sh.withAlpha(0.25f));
        g.fillRect(juce::Rectangle<float>(x, b.getY(), w, b.getHeight()));
        // Inner highlight
        g.setColour(lnf.theme.hl.withAlpha(0.65f));
        g.fillRect(juce::Rectangle<float>(x + w * 0.5f - 0.5f, b.getY(), 1.0f, b.getHeight()));
    }
    
private:
    FieldLNF& lnf;
};
