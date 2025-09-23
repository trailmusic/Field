#pragma once

#include <JuceHeader.h>
#include "ImagerPane.h"
#include "ImagerControlsPane.h"
#include "ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Imager tab: Imager visuals + 2x16 controls grid
class ImagerTab : public juce::Component
{
public:
    explicit ImagerTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p)
    {
        visuals = std::make_unique<ImagerPane>();
        addAndMakeVisible (*visuals);
    }
    // Mirror legacy callbacks so PaneManager integrations continue to work
public:
    std::function<void(const juce::String&, const juce::var&)> onUiChange;
    std::function<void(const juce::String& paramID, float value)> onParamEdit;

    void setOptions (const ImagerPane::Options& o)
    {
        if (visuals) visuals->setOptions (o);
    }


    void resized() override
    {
        auto r = getLocalBounds();
        // Imager is visuals-only now (no 2x16 controls)
        if (visuals) visuals->setBounds (r);
    }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<ImagerPane> visuals;
};


