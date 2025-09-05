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
        // Smoothing preset toggle (always visible in pane)
        addAndMakeVisible (presetButton);
        presetButton.setButtonText ("Silky");
        presetButton.onClick = [this]
        {
            auto cur = spec.getSmoothingPreset();
            auto next = (cur == SpectrumAnalyzer::SmoothingPreset::Silky)
                          ? SpectrumAnalyzer::SmoothingPreset::Clean
                          : SpectrumAnalyzer::SmoothingPreset::Silky;
            spec.setSmoothingPreset (next);
            presetButton.setButtonText (next == SpectrumAnalyzer::SmoothingPreset::Silky ? "Silky" : "Clean");
        };
    }

    void setSampleRate (double fs) { spec.setSampleRate (fs); }
    void onAudioBlock (const float* L, const float* R, int n) { spec.pushBlock (L, R, n); }
    void onAudioBlockPre (const float* L, const float* R, int n) { spec.pushBlockPre (L, R, n); }
    void resized() override
    {
        auto r = getLocalBounds();
        const int h = 22; const int w = 70; const int pad = 6;
        spec.setBounds (r);
        presetButton.setBounds (r.removeFromBottom (h + pad).removeFromLeft (w + pad).withTrimmedLeft (pad).withTrimmedTop (pad).withSizeKeepingCentre (w, h));
    }

    SpectrumAnalyzer& analyzer() noexcept { return spec; }

private:
    SpectrumAnalyzer spec;
    juce::TextButton presetButton;
};


