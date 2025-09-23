#pragma once

#include <JuceHeader.h>
#include "DelayUiBridge.h"
#include "DelayVisuals.h"
#include "DelayControlsPane.h"
#include "../ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Delay tab: top visuals + placeholder 2x16 grid (hidden initially)
class DelayTab : public juce::Component
{
public:
    explicit DelayTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        // Visuals
        auto* dv = new DelayVisuals (proc.getDelayUiBridge(), &proc);
        dv->setScopes (
            [&p] (juce::AudioBuffer<float>& out, int maxSamples) { return p.visPre.pull (out, maxSamples); },
            [&p] (juce::AudioBuffer<float>& out, int maxSamples) { return p.visPost.pull (out, maxSamples); }
        );
        dv->setParamSetter ([&p](const juce::String& id, float value){ if (auto* par = p.apvts.getParameter(id)) par->setValueNotifyingHost (value); });
        visuals.reset (dv);
        addAndMakeVisible (*visuals);

        // Controls 2x16 grid
        controls = std::make_unique<DelayControlsPane>(p.apvts);
        controls->setVisible (true);
        addAndMakeVisible (*controls);
    }

    void setControlsVisible (bool on)
    {
        if (controls)
            controls->setVisible (on);
        resized();
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
        if (visuals) visuals->setBounds (r);
        if (controls && controls->isVisible()) controls->setBounds (controlsArea);
    }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<DelayVisuals> visuals;
    std::unique_ptr<DelayControlsPane> controls;
};


