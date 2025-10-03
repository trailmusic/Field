#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbScopeComponent — Compact "status scope" for ER / Tail / Width / GR
// ----------------------------------------------------------------------------
// DEV NOTES
// - Lightweight, timer-driven mini-meter. Parent passes four callbacks:
//     getEr(), getTail(), getDuckDb(), getWidthNow()
// - Colours come from FieldLNF theme (resolved in .cpp).
// - Timer pauses when hidden to avoid unnecessary repaint.
// ─────────────────────────────────────────────────────────────────────────────

#include <JuceHeader.h>
#include <functional>

class ReverbScopeComponent final : public juce::Component,
                                   private juce::Timer
{
public:
    using FloatCB = std::function<float()>;

    explicit ReverbScopeComponent (FloatCB er,
                                   FloatCB tail,
                                   FloatCB duckDb,
                                   FloatCB widthNow);
    ~ReverbScopeComponent () override;

    // juce::Component
    void paint (juce::Graphics& g) override;
    void visibilityChanged () override;

    // Optional: adjust refresh rate (Hz). Default is 30.
    void setFps (int hz);

private:
    // Callbacks provided by owner
    FloatCB getEr;
    FloatCB getTail;
    FloatCB getDuck;
    FloatCB getWidth;

    // Timer
    void timerCallback () override;

    int currentFps { 30 };

    JUCE_LEAK_DETECTOR (ReverbScopeComponent)
};