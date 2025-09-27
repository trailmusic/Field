#pragma once
#include <JuceHeader.h>
#include "BandVisualPane.h"
#include "ControlGridMetrics.h"
#include "BandControlsPane.h"

// BandTab: A focused view with Band-specific Width mode visuals and controls
class BandTab : public juce::Component
{
public:
    BandTab (MyPluginAudioProcessor& p)
    {
        visuals = std::make_unique<BandVisualPane>();
        addAndMakeVisible (*visuals);
        // Band-specific controls pane (WIDTH + band widths)
        controls = std::make_unique<BandControlsPane>(p.apvts);
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

    // Runtime hooks forwarded to the underlying BandVisualPane
    void setSampleRate (double fs)                  { if (visuals) visuals->setSampleRate (fs); }
    void pushBlock (const float* L, const float* R, int n, bool isPre)
    {
        if (visuals) visuals->pushBlock (L, R, n, isPre);
    }
    void setCrossovers (float loHz, float hiHz)     { if (visuals) visuals->setCrossovers (loHz, hiHz); }
    void setWidths (float lo, float mid, float hi)  { if (visuals) visuals->setWidths (lo, mid, hi); }

    // Allow PaneManager to route parameter edits to the processor
    void setParamEditCallback (std::function<void(const juce::String&, float)> cb)
    {
        if (visuals)
            visuals->onParamEdit = std::move (cb);
    }

private:
    std::unique_ptr<BandVisualPane> visuals;
    std::unique_ptr<BandControlsPane> controls;
};


