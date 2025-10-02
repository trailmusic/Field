// ─────────────────────────────────────────────────────────────────────────────
// ReverbVisuals.cpp — Primary renderer for the Reverb visualization panel
// ----------------------------------------------------------------------------
// DEV NOTES
// - This component paints the *content* (Rays / Waterfall / Spectral).
//   Chrome/shell is owned by the parent panel (ReverbGraphics).
// - Alpha and banner states are resolved via resolveViz(...) so disabled,
//   host-bypassed, frozen, and idle-preview cases stay readable.
// - All colours should come from FieldLNF (theme); avoid hardcoding.
// - If you later add FFT-driven spectral, this file is the right home for it.
//
// TODO (optional):
// [ ] Add keyboard shortcuts 1/2/3 for view switching, handled by parent.
// [ ] Add eased animation for bars/rays using a lightweight integrator.
// [ ] Gate heavy work on isShowing() if you add FFT.
// ─────────────────────────────────────────────────────────────────────────────

#include "ReverbVisuals.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/PluginProcessor.h"
#include "ReverbParamIDs.h"

// ─────────────────────────────────────────────────────────────────────────────
// File-local constants
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
    constexpr float kCornerRadius        = 8.0f;
    constexpr float kEdgeOutlineThick    = 2.0f;
    constexpr float kOuterShadowAlpha    = 0.60f;

    // Waterfall texture density
    constexpr int   kWaterfallHLines     = 8;
    constexpr float kWaterfallRadius     = 4.0f;

    // Spectral bars
    constexpr int   kSpectralBars        = 32;

    // Rays
    constexpr int   kRaysCount           = 32;

    // Idle/hold policy
    constexpr double kHoldMs             = 450.0;  // grace window post-signal
    constexpr float  kIdleER             = 0.40f;  // idle preview levels
    constexpr float  kIdleTail           = 0.30f;

    inline const FieldTheme& themeFrom (const juce::LookAndFeel& lnf)
    {
        if (auto* f = dynamic_cast<const FieldLNF*> (&lnf)) return f->theme;
        static FieldLNF fallback;                     // local static OK
        return fallback.theme;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

ReverbVisuals::ReverbVisuals (MyPluginAudioProcessor& p,
                              juce::AudioProcessorValueTreeState& s,
                              std::function<float()> getEr,
                              std::function<float()> getTail,
                              std::function<float()> getDuckDb,
                              std::function<float()> getWidthNow)
    : proc (p),
      state (s),
      getErRms (std::move (getEr)),
      getTailRms (std::move (getTail)),
      getDuckGrDb (std::move (getDuckDb)),
      getWidthNow (std::move (getWidthNow))
{
    setOpaque (true);
}

ReverbVisuals::~ReverbVisuals () = default;

// ─────────────────────────────────────────────────────────────────────────────
// Layout / LookAndFeel
// ─────────────────────────────────────────────────────────────────────────────

void ReverbVisuals::resized ()
{
    // Parent positions us; we paint full bounds.
}

void ReverbVisuals::lookAndFeelChanged ()
{
    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// Paint
// ─────────────────────────────────────────────────────────────────────────────

void ReverbVisuals::paint (juce::Graphics& g)
{
    const auto r  = getLocalBounds ().toFloat ();
    const auto& th = themeFrom (getLookAndFeel ());

    // Background + shell (kept subtle; main shell is parent's)
    g.setColour (th.meters.panelDark);
    g.fillRoundedRectangle (r, kCornerRadius);

    g.setColour (th.sh.withAlpha (kOuterShadowAlpha));
    g.drawRoundedRectangle (r.reduced (0.5f), kCornerRadius - 0.5f, kEdgeOutlineThick);

    // Current signal levels
    const float er   = getErRms   ? getErRms ()   : 0.0f;
    const float tail = getTailRms ? getTailRms () : 0.0f;

    // Params / bypass / freeze
    const auto* enabledParam = state.getRawParameterValue (ReverbParamIDs::enabled);
    const auto* freezeParam  = state.getRawParameterValue (ReverbParamIDs::freeze);

    if (! enabledParam || ! freezeParam)
    {
       #if JUCE_DEBUG
        DBG ("[ReverbVisuals] Missing APVTS params: enabled or freeze");
       #endif
        return;
    }

    bool hostBypassed = false;
    if (auto* bp = proc.getBypassParameter ())
        hostBypassed = (bp->getValue () > 0.5f);

    const double nowMs = juce::Time::getMillisecondCounterHiRes ();
    const auto vr = resolveViz (er,
                                tail,
                                (*enabledParam > 0.5f),
                                hostBypassed,
                                (*freezeParam  > 0.5f),
                                allowIdlePreview,
                                nowMs);

    // Dim/brighten content based on resolved state
    g.setOpacity (vr.alpha);

    const auto inner = r.reduced (10.0f);
    switch (currentViewMode)
    {
        case ViewMode::Rays:      paintRaysInBounds      (g, inner, vr.er,   vr.tail); break;
        case ViewMode::Waterfall: paintWaterfallInBounds (g, inner, vr.er,   vr.tail); break;
        case ViewMode::Spectral:  paintSpectralInBounds  (g, inner, vr.er,   vr.tail); break;
    }

    // Optional state banner (debug/UX hint)
    if (vr.banner)
    {
        g.setColour (juce::Colours::white.withAlpha (0.60f));
        g.setFont (14.0f);
        g.drawFittedText (vr.banner, r.toNearestInt (), juce::Justification::topRight, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// View mode
// ─────────────────────────────────────────────────────────────────────────────

void ReverbVisuals::setViewMode (ViewMode mode)
{
    if (currentViewMode == mode) return;
    currentViewMode = mode;
    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// State resolution
// ─────────────────────────────────────────────────────────────────────────────

ReverbVisuals::VizResolve ReverbVisuals::resolveViz (float erLevel,
                                                     float tailLevel,
                                                     bool enabled,
                                                     bool hostBypassed,
                                                     bool frozen,
                                                     bool allowPreview,
                                                     double nowMs)
{
    auto nearZero = [] (float v) { return std::abs (v) < 1.0e-5f; };

    // 0) Disabled / host-bypassed → dim & label
    if (!enabled || hostBypassed)
        return { VizState::Disabled, 0.f, 0.f, 0.40f, "Bypassed" };

    // 1) Frozen → show tail, no ER
    if (frozen)
    {
        float t = tailLevel; if (nearZero (t)) t = kIdleTail;
        return { VizState::Frozen, 0.f, t, 0.90f, "Frozen" };
    }

    // 2) Active vs idle, with grace hold
    static double s_lastActiveMs = 0.0;

    const bool hasSignal = (!nearZero (erLevel) || !nearZero (tailLevel));
    if (hasSignal)
    {
        s_lastActiveMs = nowMs; // 🔧 FIX: remember last active time
        return { VizState::ActiveSignal, erLevel, tailLevel, 1.00f, nullptr };
    }

    const bool withinHold = (nowMs - s_lastActiveMs) < kHoldMs;
    if (withinHold)
        return { VizState::ActiveSignal, erLevel, tailLevel, 1.00f, nullptr };

    if (allowPreview)
        return { VizState::IdlePreview, kIdleER, kIdleTail, 0.75f, nullptr };

    // 3) True idle (no preview)
    return { VizState::Disabled, 0.f, 0.f, 0.35f, nullptr };
}

// ─────────────────────────────────────────────────────────────────────────────
// Paint helpers
// ─────────────────────────────────────────────────────────────────────────────

void ReverbVisuals::paintRaysInBounds (juce::Graphics& g,
                                       juce::Rectangle<float> bounds,
                                       float er,
                                       float tail)
{
    const auto& th = themeFrom (getLookAndFeel ());
    const auto c   = th.meters.panelLight; // subtle, inherits theme

    const auto cx = bounds.getCentreX ();
    const auto cy = bounds.getCentreY ();
    const auto radius = juce::jmin (bounds.getWidth (), bounds.getHeight ()) * 0.48f;

    // Visibility floor so idle is still readable but soft
    const float strength = juce::jlimit (0.08f, 0.25f, 0.12f + 0.40f * juce::jmax (er, tail));
    g.setColour (c.withAlpha (strength));

    for (int i = 0; i < kRaysCount; ++i)
    {
        const float ang = (float) i / (float) kRaysCount * juce::MathConstants<float>::twoPi;
        g.drawLine (cx, cy,
                    cx + radius * std::cos (ang),
                    cy + radius * std::sin (ang),
                    1.0f);
    }
}

void ReverbVisuals::paintWaterfallInBounds (juce::Graphics& g,
                                            juce::Rectangle<float> bounds,
                                            float er,
                                            float tail)
{
    const auto& th = themeFrom (getLookAndFeel ());
    const float intensity = juce::jmax (er, tail);

    juce::ColourGradient grad;
    grad.point1 = bounds.getTopLeft ();
    grad.point2 = bounds.getBottomLeft ();

    // Theme greys modulated by intensity (slightly brighter with signal)
    grad.addColour (0.00, th.meters.panelDark  .withAlpha (0.70f + 0.20f * intensity));
    grad.addColour (0.33, th.meters.panelMedium.withAlpha (0.65f + 0.25f * intensity));
    grad.addColour (0.66, th.meters.panelLight .withAlpha (0.55f + 0.30f * intensity));
    grad.addColour (1.00, th.meters.panelDark  .withAlpha (0.70f + 0.20f * intensity));

    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, kWaterfallRadius);

    // Texture lines scale with intensity
    g.setColour (th.textMuted.withAlpha (0.20f + 0.30f * intensity));
    for (int i = 0; i < kWaterfallHLines; ++i)
    {
        const float y = bounds.getY () + (bounds.getHeight () * (float) i / (float) kWaterfallHLines);
        g.drawLine (bounds.getX (), y, bounds.getRight (), y, 0.5f);
    }
}

void ReverbVisuals::paintSpectralInBounds (juce::Graphics& g,
                                           juce::Rectangle<float> bounds,
                                           float er,
                                           float tail)
{
    const auto& th = themeFrom (getLookAndFeel ());
    const float barW = bounds.getWidth () / (float) kSpectralBars;
    const float sig  = juce::jlimit (0.0f, 1.0f, 0.5f * (er + tail)); // quick level proxy

    for (int i = 0; i < kSpectralBars; ++i)
    {
        const float phase     = (float) i * 0.20f;
        const float heightN   = 0.10f + 0.90f * sig * (0.5f + 0.5f * std::sin (phase));
        const float barH      = bounds.getHeight () * heightN;
        const float x         = bounds.getX () + i * barW;
        const float y         = bounds.getBottom () - barH;

        g.setColour (th.meters.panelLight.withAlpha (0.60f + 0.30f * sig));
        g.fillRect (x, y, barW - 1.0f, barH);
    }
}