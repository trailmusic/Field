#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbVisuals.h — Primary renderer for the Reverb visualization panel
// ----------------------------------------------------------------------------
// DEV NOTES
// - Keep header lean: forward declare processor, include JUCE only.
// - Public API: view-mode control, idle-preview toggle.
// - Visual state resolution handled internally via resolveViz(...).
// - All color comes from FieldLNF (accessed in .cpp).
// ─────────────────────────────────────────────────────────────────────────────

#include <JuceHeader.h>
#include <functional>

class MyPluginAudioProcessor; // fwd

class ReverbVisuals final : public juce::Component
{
    JUCE_LEAK_DETECTOR (ReverbVisuals)

public:
    // Visualization modes
    enum class ViewMode { Rays, Waterfall, Spectral };

    using FloatCB = std::function<float()>; // audio-level callback

    ReverbVisuals (MyPluginAudioProcessor& p,
                   juce::AudioProcessorValueTreeState& s,
                   FloatCB getEr,
                   FloatCB getTail,
                   FloatCB getDuckDb,
                   FloatCB getWidthNow);

    ~ReverbVisuals () override;

    // juce::Component
    void resized () override;
    void paint (juce::Graphics& g) override;
    void lookAndFeelChanged () override;

    // View mode
    void setViewMode (ViewMode mode);
    [[nodiscard]] ViewMode getViewMode () const noexcept { return currentViewMode; }

    // Idle preview policy (when no signal is detected)
    void setAllowIdlePreview (bool b) noexcept { allowIdlePreview = b; }
    [[nodiscard]] bool getAllowIdlePreview () const noexcept { return allowIdlePreview; }

private:
    // Processor and state
    MyPluginAudioProcessor&               proc;
    juce::AudioProcessorValueTreeState&   state;

    // Level getters (provided by owner)
    FloatCB getErRms;
    FloatCB getTailRms;
    FloatCB getDuckGrDb;
    FloatCB getWidthNow;

    // Current mode
    ViewMode currentViewMode { ViewMode::Rays };

    // Visualization state machine
    enum class VizState { Disabled, ActiveSignal, IdlePreview, Frozen };

    struct VizResolve
    {
        VizState     state;
        float        er;       // painter input: early reflections level
        float        tail;     // painter input: tail level
        float        alpha;    // overall opacity for dimming/brightening
        const char*  banner;   // optional tag ("Bypassed", "Frozen", ...)
    };

    // Resolve visual state + presentation parameters
    VizResolve resolveViz (float erLevel,
                           float tailLevel,
                           bool enabledParam,
                           bool hostBypassed,
                           bool freezeParam,
                           bool allowPreview,
                           double nowMs);

    // Policy
    bool allowIdlePreview = true;

    // Painters (render inside provided bounds)
    void paintRaysInBounds       (juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
    void paintWaterfallInBounds  (juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
    void paintSpectralInBounds   (juce::Graphics& g, juce::Rectangle<float> bounds, float er, float tail);
};