#pragma once

#include <JuceHeader.h>
#include "MotionPanel.h"
#include "MotionControlsPane.h"
#include "../ui/ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Motion tab: Motion visuals + 2x16 controls grid
class MotionTab : public juce::Component
{
public:
    explicit MotionTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        visuals = std::make_unique<motion::MotionPanel>(p.apvts, nullptr);
        addAndMakeVisible (*visuals);

        controls = std::make_unique<MotionControlsPane>(p.apvts);
        addAndMakeVisible (*controls);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto m = ControlGridMetrics::compute (r.getWidth(), r.getHeight());
        if (controls) { controls->setCellMetrics (m.knobPx, m.valuePx, m.labelGapPx, m.colW); controls->setRowHeightPx (m.rowH); }
        auto controlsArea = r.removeFromBottom (m.controlsH);
        if (visuals) visuals->setBounds (r);
        if (controls) controls->setBounds (controlsArea);
    }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<motion::MotionPanel> visuals;
    std::unique_ptr<MotionControlsPane> controls;
};


