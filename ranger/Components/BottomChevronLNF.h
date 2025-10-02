#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

struct BottomChevronLNF : public FieldLNF
{
    void drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                               bool isOver, bool isDown) override;
    void drawButtonText (juce::Graphics& g, juce::TextButton& button,
                         bool isOver, bool /*isDown*/) override;
};
