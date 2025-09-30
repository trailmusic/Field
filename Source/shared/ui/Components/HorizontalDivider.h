#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

class HorizontalDivider : public juce::Component
{
public:
    HorizontalDivider(FieldLNF& l) : lnf(l) {}
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::transparentBlack);
        g.setColour(lnf.theme.sh.withAlpha(0.4f));
        auto b = getLocalBounds().toFloat();
        g.fillRect(juce::Rectangle<float>(b.getX(), b.getY(), b.getWidth(), 1.0f));
    }
    
private:
    FieldLNF& lnf;
};
