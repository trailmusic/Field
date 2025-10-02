#pragma once

#include <JuceHeader.h>
#include "RangerFilePane.h"
#include "RangerPlotPane.h"
#include "RangerSettingsPane.h"

class RangerDesigner : public juce::Component
{
public:
    RangerDesigner();
    ~RangerDesigner() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Main panels
    std::unique_ptr<RangerFilePane> filePane;
    std::unique_ptr<RangerPlotPane> plotPane;
    std::unique_ptr<RangerSettingsPane> settingsPane;
    
    // Layout
    juce::FlexBox mainLayout;
    juce::FlexBox centerLayout;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerDesigner)
};
