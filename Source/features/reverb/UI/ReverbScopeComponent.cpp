// ─────────────────────────────────────────────────────────────────────────────
// ReverbScopeComponent.cpp
// ----------------------------------------------------------------------------

#include "ReverbScopeComponent.h"
#include "shared/Core/FieldLookAndFeel.h" // adjust include path if needed
#include "shared/Core/FieldMetallic.h"    // kept for parity; not used directly

namespace
{
    // Layout + style
    constexpr float kCornerRadius     = 6.0f;
    constexpr float kShellAlpha       = 0.40f;
    constexpr float kRimAlpha         = 0.20f;
    constexpr float kRimThickness     = 1.2f;

    constexpr int   kInnerPad         = 6;     // outer reduce
    constexpr int   kMeterPadY        = 8;     // top/bottom breathing room

    // ER/Tail bar cluster
    constexpr float kBarsCenterFrac   = 0.25f; // x-position anchor as fraction
    constexpr float kBarWidth         = 20.0f;
    constexpr float kBarGap           = 20.0f; // between ER and Tail bars

    // Width meter
    constexpr float kWidthFracX       = 0.62f; // left edge as fraction of width
    constexpr float kWidthTrackFrac   = 0.35f; // portion of total width reserved
    constexpr float kWidthHeight      = 8.0f;
    constexpr float kWidthCorner      = 4.0f;

    // Text
    constexpr int   kLabelH           = 14;
    constexpr int   kGrTextW          = 90;
    constexpr int   kGrTextH          = 16;

    inline const FieldTheme& themeFrom (const juce::LookAndFeel& lnf)
    {
        if (auto* f = dynamic_cast<const FieldLNF*> (&lnf)) return f->theme;
        static FieldLNF fallback;
        return fallback.theme;
    }

    inline float safeDb (float lin) noexcept
    {
        // Use a small epsilon to avoid -inf and keep quiet values displayable
        return juce::Decibels::gainToDecibels (juce::jmax (lin, 1.0e-6f));
    }
}

ReverbScopeComponent::ReverbScopeComponent (FloatCB er,
                                            FloatCB tail,
                                            FloatCB duckDb,
                                            FloatCB widthNow)
    : getEr   (std::move (er))
    , getTail (std::move (tail))
    , getDuck (std::move (duckDb))
    , getWidth(std::move (widthNow))
{
    setOpaque (false);
    startTimerHz (currentFps);
}

ReverbScopeComponent::~ReverbScopeComponent ()
{
    stopTimer ();
}

void ReverbScopeComponent::setFps (int hz)
{
    currentFps = juce::jlimit (5, 120, hz);
    startTimerHz (currentFps);
}

void ReverbScopeComponent::visibilityChanged ()
{
    if (isVisible () && isShowing ()) startTimerHz (currentFps);
    else                               stopTimer ();
}

void ReverbScopeComponent::timerCallback ()
{
    if (isShowing ()) repaint ();
}

void ReverbScopeComponent::paint (juce::Graphics& g)
{
    const auto outer = getLocalBounds ().toFloat ().reduced (kInnerPad);
    if (outer.isEmpty ()) return;

    const auto& th = themeFrom (getLookAndFeel ());

    // Shell
    g.setColour (th.panel.withAlpha (kShellAlpha));
    g.fillRoundedRectangle (outer, kCornerRadius);

    g.setColour (th.text.withAlpha (kRimAlpha));
    g.drawRoundedRectangle (outer, kCornerRadius, kRimThickness);

    // Read meters
    const float er   = getEr    ? getEr ()    : 0.0f;
    const float tail = getTail  ? getTail ()  : 0.0f;
    const float grDb = getDuck  ? getDuck ()  : 0.0f;  // negative when reducing
    const float widthPct = getWidth ? getWidth () : 100.0f; // 0..100 expected

    // Y mapping: -60 dB .. 0 dB → bottom..top (with padding)
    auto mapY = [&outer] (float lin) -> float
    {
        const float dB = safeDb (lin);
        return juce::jmap (dB, -60.0f, 0.0f,
                           outer.getBottom () - kMeterPadY,
                           outer.getY () + kMeterPadY);
    };

    // ─────────────────────────────────────────────────────────────────────────
    // ER / Tail bars
    // ─────────────────────────────────────────────────────────────────────────
    const float xMid = outer.getX () + outer.getWidth () * kBarsCenterFrac;

    const auto erTop   = mapY (er);
    const auto tailTop = mapY (tail);
    const float baseY  = outer.getBottom () - kMeterPadY;

    g.setColour (th.textMuted.withAlpha (0.85f));
    g.fillRect (xMid - (kBarGap * 0.5f) - kBarWidth,
                erTop,
                kBarWidth,
                juce::jmax (0.0f, baseY - erTop));

    g.setColour (th.accent.withAlpha (0.95f));
    g.fillRect (xMid + (kBarGap * 0.5f),
                tailTop,
                kBarWidth,
                juce::jmax (0.0f, baseY - tailTop));

    // ─────────────────────────────────────────────────────────────────────────
    // Width meter (track + label + fill)
    // ─────────────────────────────────────────────────────────────────────────
    const float wx = outer.getX () + outer.getWidth () * kWidthFracX;

    g.setColour (th.text.withAlpha (0.60f));
    g.setFont   (juce::Font (12.0f));
    g.drawText ("Width",
                juce::Rectangle<int> ((int) wx, (int) outer.getY () + 6, 60, kLabelH),
                juce::Justification::left, false);

    const float trackW = outer.getWidth () * kWidthTrackFrac;
    const auto track   = juce::Rectangle<float> (wx,
                                                 outer.getBottom () - (kWidthHeight + 6.0f),
                                                 trackW,
                                                 kWidthHeight);

    // Track background (subtle)
    g.setColour (th.text.withAlpha (0.15f));
    g.fillRoundedRectangle (track, kWidthCorner);

    // Fill portion (0..100%)
    const float pct      = juce::jlimit (0.0f, 100.0f, widthPct) / 100.0f;
    const float fillW    = juce::jlimit (0.0f, trackW, pct * trackW);

    g.setColour (th.hl);
    g.fillRoundedRectangle (juce::Rectangle<float> (track.getX (), track.getY (), fillW, track.getHeight ()),
                            kWidthCorner);

    // ─────────────────────────────────────────────────────────────────────────
    // GR readout (positive display for readability)
    // ─────────────────────────────────────────────────────────────────────────
    g.setColour (th.text.withAlpha (0.90f));
    g.setFont   (juce::Font (12.0f));

    const float grDisplay = juce::jmax (0.0f, grDb); // show positive magnitude
    g.drawText (juce::String (grDisplay, 1) + " dB GR",
                juce::Rectangle<int> ((int) wx, (int) outer.getCentreY () - (kGrTextH / 2), kGrTextW, kGrTextH),
                juce::Justification::left, false);
}