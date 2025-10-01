#pragma once

#include <JuceHeader.h>
#include "MotionGraphics.h"
#include "MotionControlsPane.h"
#include "shared/ui/Controls/ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Motion tab: Motion visuals + 2x16 controls grid
class MotionTab : public juce::Component
{
public:
    explicit MotionTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        visuals = std::make_unique<motion::MotionGraphics>(p.apvts, nullptr);
        addAndMakeVisible (*visuals);

        controls = std::make_unique<MotionControlsPane>(p.apvts);
        addAndMakeVisible (*controls);
    }
    
    ~MotionTab() override
    {
        // CRITICAL: Ensure proper destruction order to prevent crashes
        // Clear controls first (which clears parameter attachments)
        controls.reset();
        // Then clear visuals
        visuals.reset();
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
    
    // Public getter for graphics container
    motion::MotionGraphics* getMotionGraphics() const { return visuals.get(); }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<motion::MotionGraphics> visuals;
    std::unique_ptr<MotionControlsPane> controls;
};


