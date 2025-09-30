#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

class HelpFAQComponent : public juce::Component
{
public:
    explicit HelpFAQComponent(FieldLNF& lnf);
    
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::TextEditor text;
    FieldLNF& lnf;
};
