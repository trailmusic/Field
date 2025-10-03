#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// ReverbGraphics.h — Visualization + EQ wrapper for the Reverb section
// ----------------------------------------------------------------------------
// DEV NOTES
// - Keep this header light: forward-declare heavy UI/DSP classes; include in .cpp.
// - Public API exposes: view-mode control, EQ accessors, ducking accessor,
//   analyzer hooks (setSampleRate/pause/resume/pushBlock/Pre).
// - Nested components (VisualizationControlPanel, BandIndicator) are small and
//   final to enable inlining and avoid vtable surprises.
// - All colour decisions live in LookAndFeel (FieldLNF). No hardcoded label colours.
// ----------------------------------------------------------------------------

#include <JuceHeader.h>
#include "DuckingFloat.h"
#include "../DSP/ReverbEQ.h"
#include "../DSP/DecayRateEQ.h"
#include "../BandIdFinder.h"
#include "../BandCounter.h"
#include "ReverbVisuals.h"
#include "shared/ui/Utilities/SafetySentinels.h"

class MyPluginAudioProcessor;   // fwd

class ReverbGraphics final : public juce::Component,
                             public juce::Timer
{
    JUCE_LEAK_DETECTOR (ReverbGraphics)

public:
    // ─────────────────────────────────────────────────────────────────────────
    // Types
    // ─────────────────────────────────────────────────────────────────────────
    enum class ViewMode { Rays, Waterfall, Spectral };

    using FloatCB = std::function<float()>; // audio-meter callback

    // ─────────────────────────────────────────────────────────────────────────
    // Lifecycle
    // ─────────────────────────────────────────────────────────────────────────
    ReverbGraphics (MyPluginAudioProcessor& processor,
                    juce::AudioProcessorValueTreeState& state,
                    FloatCB getEr,
                    FloatCB getTail,
                    FloatCB getDuckDb,
                    FloatCB getWidthNow);

    ~ReverbGraphics () override;

    // ─────────────────────────────────────────────────────────────────────────
    // juce::Component overrides
    // ─────────────────────────────────────────────────────────────────────────
    void resized () override;
    void paint (juce::Graphics& g) override;
    void visibilityChanged () override;
    void lookAndFeelChanged () override;

    // ─────────────────────────────────────────────────────────────────────────
    // View mode
    // ─────────────────────────────────────────────────────────────────────────
    void setViewMode (ViewMode mode);
    [[nodiscard]] ViewMode getViewMode () const noexcept { return currentViewMode; }

    // ─────────────────────────────────────────────────────────────────────────
    // Theme
    // ─────────────────────────────────────────────────────────────────────────
    void updateLabelColors ();

    // ─────────────────────────────────────────────────────────────────────────
    // Accessors
    // ─────────────────────────────────────────────────────────────────────────
    [[nodiscard]] DuckingFloat*  getDuckingFloat ()  noexcept { return duckingFloat.get (); }
    [[nodiscard]] ReverbToneEQ*  getReverbEQ   ()    noexcept { return reverbEQ.get (); }
    [[nodiscard]] DecayRateEQ*   getDecayRateEQ ()   noexcept { return decayRateEQ.get (); }

    // ─────────────────────────────────────────────────────────────────────────
    // Analyzer / DSP hooks passthrough
    // ─────────────────────────────────────────────────────────────────────────
    void setSampleRate (double sr);
    void pause ();
    void resume ();
    void pushBlock    (const float* L, const float* R, int n);
    void pushBlockPre (const float* L, const float* R, int n);

    // ─────────────────────────────────────────────────────────────────────────
    // Timer (animation / meters)
    // ─────────────────────────────────────────────────────────────────────────
    void timerCallback () override;

    // ─────────────────────────────────────────────────────────────────────────
    // UI helpers
    // ─────────────────────────────────────────────────────────────────────────
    void updateDuckingModuleVisibility ();
    void updateBandIndicatorsManually ();
    void setupVisualizationControlPanel ();
    void setupEQLabels ();

private:
    // ─────────────────────────────────────────────────────────────────────────
    // Parent-level visual paints (decorative; child owns main visuals)
    // ─────────────────────────────────────────────────────────────────────────
    void paintRays            (juce::Graphics& g);
    void paintWaterfall       (juce::Graphics& g);
    void paintSpectral        (juce::Graphics& g);
    void paintGrOverlay       (juce::Graphics& g);

    // With bounds (used by parent decoration path)
    void paintRaysInBounds      (juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintWaterfallInBounds (juce::Graphics& g, juce::Rectangle<float> bounds);
    void paintSpectralInBounds  (juce::Graphics& g, juce::Rectangle<float> bounds);

    // ─────────────────────────────────────────────────────────────────────────
    // Nested UI: Visualization container shell
    // ─────────────────────────────────────────────────────────────────────────
    class VisualizationControlPanel final : public juce::Component
    {
    public:
        void paint (juce::Graphics& g) override;
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Nested UI: Minimal band-indicator row (●●○○ etc.)
    // ─────────────────────────────────────────────────────────────────────────
    class BandIndicator final : public juce::Component
    {
    public:
        explicit BandIndicator (int maxBands);
        void paint (juce::Graphics& g) override;
        void setActiveBands (int count);
        void setMaxBands (int max);

    private:
        int   maxBands    = 0;
        int   activeBands = 0;

        static constexpr float kCircleSize    = 8.0f;
        static constexpr float kCircleSpacing = 12.0f;
    };

    // ─────────────────────────────────────────────────────────────────────────
    // State
    // ─────────────────────────────────────────────────────────────────────────
    MyPluginAudioProcessor&                proc;
    juce::AudioProcessorValueTreeState&    state;

    // Audio callbacks (provided by owner)
    FloatCB getErRms, getTailRms, getDuckGrDb, getWidthNow;

    // View-mode UI
    juce::TextButton  raysButton, waterfallButton, spectralButton;
    ViewMode          currentViewMode { ViewMode::Rays };

    // Main visualization child (primary renderer)
    std::unique_ptr<ReverbVisuals> reverbVisuals;
    VisualizationControlPanel      visualizationControlPanel;

    // EQ panels
    std::unique_ptr<ReverbToneEQ>  reverbEQ;
    std::unique_ptr<DecayRateEQ>   decayRateEQ;

    // Labels
    juce::Label toneEqLabel, decayRateEqLabel, duckingLabel, visualizationLabel;

    // Band indicators + counters
    BandIndicator                         toneEqIndicator { 4 };
    BandIndicator                         decayRateEqIndicator { 3 };
    std::unique_ptr<BandCounter>          toneCounter, decayCounter;
    juce::StringArray                     toneEnabledIds, decayEnabledIds;

    // Ducking module
    std::unique_ptr<DuckingFloat>         duckingFloat;

    // Animation
    float                     animationTime    = 0.0f;
    static constexpr float    kAnimationSpeed  = 0.02f; // radians per tick
};