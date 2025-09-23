#pragma once

#include <JuceHeader.h>
#include "XYControlsPane.h"
#include "ControlGridMetrics.h"

class MyPluginAudioProcessor;

// Composite XY tab: XY visuals (provided via PaneManager XYPaneAdapter) + 2x16 controls
class XYTab : public juce::Component
{
public:
    XYTab (MyPluginAudioProcessor& p, juce::Component& xyAdapter)
        : proc (p), xyVisual (&xyAdapter)
    {
        this->juce::Component::addAndMakeVisible (xyVisual);
        controls = std::make_unique<XYControlsPane>(p.apvts);
        this->juce::Component::addAndMakeVisible (controls.get());
    }

    void resized() override
    {
        auto r = getLocalBounds();
        auto m = ControlGridMetrics::compute (r.getWidth(), r.getHeight());
        if (controls) { controls->setCellMetrics (m.knobPx, m.valuePx, m.labelGapPx, m.colW); controls->setRowHeightPx (m.rowH); }
        auto controlsArea = r.removeFromBottom (m.controlsH);
        // reduce visuals height by 25%
        {
            const int newH = (r.getHeight() * 3) / 4;
            r.removeFromBottom (r.getHeight() - newH);
        }
        if (xyVisual) xyVisual->setBounds (r);
        if (controls) controls->juce::Component::setBounds (controlsArea);
    }

private:
    MyPluginAudioProcessor& proc;
    juce::Component* xyVisual { nullptr };
    std::unique_ptr<XYControlsPane> controls;
};


