#pragma once
#include <JuceHeader.h>
#include "ImagerPane.h"

// BandPane: A focused view that reuses Imager's Width mode (including Designer overlay)
class BandPane : public juce::Component
{
public:
    BandPane()
    {
        imager = std::make_unique<ImagerPane>();
        addAndMakeVisible (*imager);
        ImagerPane::Options o;
        o.showPre = true;
        o.autoGain = true;
        o.fps = 30;
        o.mode = ImagerPane::Options::Mode::Width; // force Width view
        imager->setOptions (o);
    }

    void resized() override
    {
        if (imager)
            imager->setBounds (getLocalBounds());
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
};


