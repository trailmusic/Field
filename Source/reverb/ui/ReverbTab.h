#pragma once

#include <JuceHeader.h>
#include "ReverbPanel.h"
#include "ReverbControlsPane2x16.h"
#include "../../ui/ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Reverb tab: canvas/DynEQ pane + placeholder 2x16 grid (hidden initially)
class ReverbTab : public juce::Component
{
public:
    explicit ReverbTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        // Visuals-only Reverb pane (existing component)
        reverbPanel = std::make_unique<ReverbPanel>(p.apvts,
            [&p]{ return p.getReverbErRms(); },
            [&p]{ return p.getReverbTailRms(); },
            [&p]{ return p.getReverbDuckGrDb(); },
            [&p]{ return p.getReverbWidthNow(); });
        reverbPanel->setDynEqGrProvider ([&p]{ return p.getReverbDynEqGrDb(); });
        addAndMakeVisible (*reverbPanel);

        // Controls (2x16 grid)
        controls = std::make_unique<ReverbControlsPane2x16>(p.apvts);
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
        if (reverbPanel) reverbPanel->setBounds (r);
        if (controls && controls->isVisible()) controls->setBounds (controlsArea);
    }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<ReverbPanel> reverbPanel;
    std::unique_ptr<ReverbControlsPane2x16> controls;
};


