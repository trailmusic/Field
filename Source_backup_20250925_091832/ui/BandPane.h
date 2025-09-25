#pragma once
#include <JuceHeader.h>
#include "ImagerPane.h"
#include "ControlGridMetrics.h"
#include "BandControlsPane.h"

// BandPane: A focused view that reuses Imager's Width mode (including Designer overlay)
class BandPane : public juce::Component
{
public:
    BandPane (MyPluginAudioProcessor& p)
    {
        imager = std::make_unique<ImagerPane>();
        addAndMakeVisible (*imager);
        ImagerPane::Options o;
        o.showPre = false;     // hide pre overlay in Band
        o.toolingEnabled = false; // remove all Imager tooling in Band
        o.autoGain = true;
        o.fps = 30;
        o.enableWidthView = true; // allow Width rendering
        o.mode = ImagerPane::Options::Mode::Width; // force Width view
        imager->setOptions (o);
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
        if (imager) imager->setBounds (r);
        if (controls) controls->setBounds (controlsArea);
    }

    // Runtime hooks forwarded to the underlying Imager
    void setSampleRate (double fs)                  { if (imager) imager->setSampleRate (fs); }
    void pushBlock (const float* L, const float* R, int n, bool isPre)
    {
        if (imager) imager->pushBlock (L, R, n, isPre);
    }
    void setCrossovers (float loHz, float hiHz)     { if (imager) imager->setCrossovers (loHz, hiHz); }
    void setWidths (float lo, float mid, float hi)  { if (imager) imager->setWidths (lo, mid, hi); }

    // Allow PaneManager to route parameter edits to the processor
    void setParamEditCallback (std::function<void(const juce::String&, float)> cb)
    {
        if (imager)
            imager->onParamEdit = std::move (cb);
    }

private:
    std::unique_ptr<ImagerPane> imager;
    std::unique_ptr<BandControlsPane> controls;
};


