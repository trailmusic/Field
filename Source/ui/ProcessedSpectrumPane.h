#pragma once
#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"

// Minimal pane wrapper around SpectrumAnalyzer
class ProcessedSpectrumPane : public juce::Component
{
public:
    ProcessedSpectrumPane (juce::LookAndFeel* lnf = nullptr)
    {
        addAndMakeVisible (spec);
        if (lnf) spec.setLookAndFeel (lnf);
    }

    void setSampleRate (double fs) { spec.setSampleRate (fs); }
    void onAudioBlock (const float* L, const float* R, int n) { spec.pushBlock (L, R, n); }
    void onAudioBlockPre (const float* L, const float* R, int n) { spec.pushBlockPre (L, R, n); }
    void resized() override { spec.setBounds (getLocalBounds()); }

    SpectrumAnalyzer& analyzer() noexcept { return spec; }

private:
    SpectrumAnalyzer spec;
};


