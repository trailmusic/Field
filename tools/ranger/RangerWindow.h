#pragma once

#include <JuceHeader.h>
#include "RangerDesigner.h"

class RangerWindow : public juce::DocumentWindow
{
public:
    RangerWindow(const juce::String& name);
    ~RangerWindow() override;

    void closeButtonPressed() override;

private:
    std::unique_ptr<RangerDesigner> designer;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerWindow)
};
