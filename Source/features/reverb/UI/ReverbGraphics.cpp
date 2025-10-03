// ─────────────────────────────────────────────────────────────────────────────
// ReverbGraphics.cpp  —  Visualization + EQ wrapper for the Reverb section
// ----------------------------------------------------------------------------
// DEV NOTES
// - Responsibilities:
//   * Owns Tone EQ (4-band) + Decay-Rate EQ (3-band) subpanels and their labels.
//   * Hosts the Visualization Control Panel and a visualization mode toggle row.
//   * Manages a "ducking" float module displayed as a right vertical strip.
//   * Renders optional in-panel visual backgrounds and a ducking GR overlay.
//
// - Layout:
//   * Left 50%: Tone EQ (top) and Decay-Rate EQ (bottom), each with label + band indicator.
//   * Middle 40%: Visualization panel with Rays / Waterfall / Spectral buttons.
//   * Right 10%: Ducking module column (always visible; greyed when DUCK==off).
//
// - Band Indicators:
//   * Auto-update via BandCounter when tb_active_*/db_active_* params are present.
//   * Fallback manual scan runs in timer if params aren't discoverable.
//
// - Debugging & Logs:
//   * DBG statements compiled only in JUCE_DEBUG.
//   * Waterfall's debug border is yellow only in JUCE_DEBUG.
//   * Consider removing parent-drawn visualization (paintRays/Waterfall/Spectral) once
//     ReverbVisuals fully covers the view. See kPaintParentVisualization.
//
// - Styling:
//   * All colors come from FieldLNF theme. No hardcoded text colors.
//   * Rounded panel shells with subtle shadow/outline consistent with app.
//
// - TODO (optional):
//   [ ] Decide single source of truth for visuals: parent vs child (ReverbVisuals).
//   [ ] Add keyboard shortcuts (1/2/3) for view mode toggle.
//   [ ] Persist last selected view mode in state.
//
// ----------------------------------------------------------------------------

#include "ReverbGraphics.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/PluginProcessor.h"
#include "../DSP/ReverbParamIDs.h"
#include "shared/ui/Utilities/SafetySentinels.h"

// ─────────────────────────────────────────────────────────────────────────────
// File-local constants
// ─────────────────────────────────────────────────────────────────────────────
namespace
{
    // Percent widths of the two primary columns (50/50 split).
    constexpr float kLeftPct   = 0.50f;
    constexpr float kRightPct  = 0.50f;

    // Panel chrome
    constexpr float kCornerRadius     = 8.0f;
    constexpr float kEdgeOutlineThick = 2.0f;
    constexpr float kOuterShadowAlpha = 0.60f;

    // Spacing
    constexpr int   kOuterPad       = 10;
    constexpr int   kInterGapLarge  = 10;
    constexpr int   kInterGapSmall  = 5;
    constexpr int   kLabelHeight    = 25;
    constexpr int   kButtonsHeight  = 30;
    constexpr int   kButtonsWidth   = 80;
    constexpr int   kButtonsSpacing = 8;

    // Animation / timer
    constexpr int   kFps = 30;
    constexpr float kTwoPi = juce::MathConstants<float>::twoPi;

    // Visualization: choose whether parent paints decorative visuals
    // (set false to let the child `ReverbVisuals` own everything visual).
    constexpr bool  kPaintParentVisualization = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / Destructor
// ─────────────────────────────────────────────────────────────────────────────

ReverbGraphics::ReverbGraphics (MyPluginAudioProcessor& p,
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
      getWidthNow (std::move (getWidthNow)),
      raysButton ("Rays"),
      waterfallButton ("Waterfall"),
      spectralButton ("Spectral"),
      toneEqIndicator (4),
      decayRateEqIndicator (3)
{
    // Ducking module
    duckingFloat = std::make_unique<DuckingFloat> (state);
    addAndMakeVisible (*duckingFloat);
    duckingFloat->setVisible (true);
    duckingFloat->setActive (false);
    duckingFloat->setGreyedOut (true);
    
    // Decay-rate module
    decayRateFloat = std::make_unique<DecayRateFloat> (state);
    addAndMakeVisible (*decayRateFloat);
    decayRateFloat->setVisible (true);
    decayRateFloat->setActive (true);

    // Band indicators (visible UI)
    addAndMakeVisible (toneEqIndicator);
    addAndMakeVisible (decayRateEqIndicator);

    // 1) Build ids deterministically (avoids scan timing issues)
    toneEnabledIds  = BandIdFinder::makeIndexedIds ("tb_active", 4);
    decayEnabledIds = BandIdFinder::makeIndexedIds ("db_active", 3);

    // 2) Show 0 immediately (visuals match default state)
    toneEqIndicator.setActiveBands (0);
    decayRateEqIndicator.setActiveBands (0);

   #if JUCE_DEBUG
    DBG ("--- Using Deterministic Tone EQ Parameters: " << toneEnabledIds.size () << " ---");
    for (auto& id : toneEnabledIds) DBG ("Tone: " << id);
    DBG ("--- Using Deterministic Decay-Rate EQ Parameters: " << decayEnabledIds.size () << " ---");
    for (auto& id : decayEnabledIds) DBG ("Decay: " << id);
   #endif

    // 3) Create counters (listeners attach here)
    toneCounter = std::make_unique<BandCounter> (state, toneEnabledIds,
        [this] (int n)
        {
            toneEqIndicator.setActiveBands (n);
            repaint ();
        });

    decayCounter = std::make_unique<BandCounter> (state, decayEnabledIds,
        [this] (int n)
        {
            decayRateEqIndicator.setActiveBands (n);
            repaint ();
        });

    // 4) Force an initial publish on the message thread (guaranteed "0")
    toneCounter->prime();
    decayCounter->prime();

    // EQ Panels
    reverbEQ    = std::make_unique<ReverbToneEQ> (proc);
    decayRateEQ = std::make_unique<DecayRateEQ> (proc);
    addAndMakeVisible (*reverbEQ);
    addAndMakeVisible (*decayRateEQ);

    // Visualization controls + buttons
    setupVisualizationControlPanel ();

    // Visualization child component (primary visual renderer)
    reverbVisuals = std::make_unique<ReverbVisuals> (proc, state, getErRms, getTailRms, getDuckGrDb, getWidthNow);
    addAndMakeVisible (*reverbVisuals);

    // Labels (Tone/Decay/Ducking/Visualization)
    setupEQLabels ();
    updateLabelColors (); // sync to theme

    startTimerHz (kFps);
}

ReverbGraphics::~ReverbGraphics ()
{
    stopTimer ();
}

// ─────────────────────────────────────────────────────────────────────────────
// Component lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::visibilityChanged ()
{
    if (! isVisible ())
        stopTimer ();
    else if (! isTimerRunning ())
        startTimerHz (kFps);
}

// ─────────────────────────────────────────────────────────────────────────────
// Paint
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::paint (juce::Graphics& g)
{
    const auto r  = getLocalBounds ().toFloat ();
    auto* lf      = dynamic_cast<FieldLNF*> (&getLookAndFeel ());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    // Panel base fill + rounded shell
    g.setColour (th.meters.panelDark);
    g.fillRect (r);
    g.fillRoundedRectangle (r, kCornerRadius);

    // Outer shadow + crisp edge
    g.setColour (th.sh.withAlpha (kOuterShadowAlpha));
    g.drawRoundedRectangle (r.reduced (0.5f), kCornerRadius - 0.5f, kEdgeOutlineThick);

    // Decorative parent-level visualization paint (optional)
    if constexpr (kPaintParentVisualization)
    {
        // Use the same inner area the child component occupies for a subtle background
        const auto vizPanel = visualizationControlPanel.getBounds ().toFloat ()
                                .reduced (15.0f).withTrimmedTop (60.0f);

       #if JUCE_DEBUG
        DBG ("🎨 Current view mode: " << (int) currentViewMode << " (0=Rays, 1=Waterfall, 2=Spectral)");
       #endif

        g.saveState ();
        g.reduceClipRegion (vizPanel.toNearestInt ());
        g.setOrigin (vizPanel.getX (), vizPanel.getY ());

        switch (currentViewMode)
        {
            case ViewMode::Rays:       paintRaysInBounds      (g, vizPanel.withPosition (0, 0)); break;
            case ViewMode::Waterfall:  paintWaterfallInBounds (g, vizPanel.withPosition (0, 0)); break;
            case ViewMode::Spectral:   paintSpectralInBounds  (g, vizPanel.withPosition (0, 0)); break;
        }
        g.restoreState ();
    }

    // Global ducking GR overlay (renders over everything)
    paintGrOverlay (g);
}

// ─────────────────────────────────────────────────────────────────────────────
// Layout
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::resized ()
{
    const auto total = getLocalBounds ();
    const int  W     = total.getWidth ();
    const int  H     = total.getHeight ();
    if (W <= 0 || H <= 0) return;

    // Column rectangles derived from total width (50/50 split)
    auto leftArea   = juce::Rectangle<int> (0, 0, (int) (W * kLeftPct),  H);
    auto rightArea  = juce::Rectangle<int> ((int) (W * kLeftPct), 0, (int) (W * kRightPct), H);

    // Add gutters
    leftArea.removeFromRight   (kInterGapLarge);
    rightArea.removeFromLeft   (kInterGapSmall);

    // Right column: Visualization (top) + DecayFloat/DuckingFloat (bottom)
    {
        // Visualization section (top half of right area)
        auto vizArea = rightArea.removeFromTop(rightArea.getHeight() / 2);
        auto vizLabel = vizArea.removeFromTop (kLabelHeight);
        visualizationLabel.setBounds (vizLabel);

        visualizationControlPanel.setBounds (vizArea);

        // Buttons row (centered)
        auto buttonsRow      = vizArea.removeFromTop (kButtonsHeight).reduced (kInterGapSmall, 0);
        const int totalBtnW  = (kButtonsWidth * 3) + (kButtonsSpacing * 2);
        const int startX     = buttonsRow.getX () + (buttonsRow.getWidth () - totalBtnW) / 2;

        raysButton.setBounds      (startX,                              buttonsRow.getY (), kButtonsWidth, kButtonsHeight);
        waterfallButton.setBounds (startX + kButtonsWidth + kButtonsSpacing,
                                   buttonsRow.getY (), kButtonsWidth, kButtonsHeight);
        spectralButton.setBounds  (startX + (kButtonsWidth + kButtonsSpacing) * 2,
                                   buttonsRow.getY (), kButtonsWidth, kButtonsHeight);

        // Child visuals below buttons
        if (reverbVisuals)
        {
            const int top = buttonsRow.getBottom () + 15;
            auto child = juce::Rectangle<int> (vizArea.getX () + kOuterPad,
                                               top,
                                               vizArea.getWidth () - 2 * kOuterPad,
                                               vizArea.getBottom () - top - kOuterPad);
            reverbVisuals->setBounds (child);
        }
        
        // Bottom section: DecayFloat and DuckingFloat side by side with labels
        auto bottomArea = rightArea; // remaining half
        
        // Decay section (left side)
        auto decaySection = bottomArea.removeFromLeft(bottomArea.getWidth() / 2);
        auto decayLabelArea = decaySection.removeFromTop(kLabelHeight);
        
        // Ducking section (right side)  
        auto duckingSection = bottomArea;
        auto duckingLabelArea = duckingSection.removeFromTop(kLabelHeight);
        
        // Add small gap between the two sections
        decaySection.removeFromRight(kInterGapSmall);
        duckingSection.removeFromLeft(kInterGapSmall);
        
        // Position labels
        decayLabel.setBounds(decayLabelArea);
        duckingLabel.setBounds(duckingLabelArea);
        
        // Position containers
        if (decayRateFloat)
        {
            decayRateFloat->setBounds(decaySection);
        }
        
        if (duckingFloat)
        {
            duckingFloat->setBounds(duckingSection);
        }
    }

    // Left column: Tone EQ (top) + Decay-Rate EQ (bottom), each with band indicator + label
    if (reverbEQ && decayRateEQ)
    {
        auto toneArea  = leftArea.removeFromTop (leftArea.getHeight () / 2);
        auto toneLabel = toneArea.removeFromTop (kLabelHeight);
        auto toneDots  = toneLabel.removeFromLeft (60).translated (12, 10);

        toneEqIndicator.setBounds (toneDots);
        toneEqLabel.setBounds     (toneLabel);
        reverbEQ->setBounds       (toneArea);

        auto decayArea  = leftArea; // remaining half
        auto decayLabel = decayArea.removeFromTop (kLabelHeight);
        auto decayDots  = decayLabel.removeFromLeft (45).translated (12, 10);

        decayRateEqIndicator.setBounds (decayDots);
        decayRateEqLabel.setBounds     (decayLabel);
        decayRateEQ->setBounds         (decayArea);
    }

}

// ─────────────────────────────────────────────────────────────────────────────
// Visualization panel + buttons
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::setupVisualizationControlPanel ()
{
    addAndMakeVisible (visualizationControlPanel);

    addAndMakeVisible (raysButton);
    addAndMakeVisible (waterfallButton);
    addAndMakeVisible (spectralButton);

    visualizationControlPanel.setOpaque (true);

    // Button labels
    raysButton.setButtonText ("Rays");
    waterfallButton.setButtonText ("Waterfall");
    spectralButton.setButtonText ("Spectral");

    // Toggle callbacks
    raysButton.onClick = [this]
    {
        setViewMode (ViewMode::Rays);
        if (reverbVisuals) reverbVisuals->setViewMode (ReverbVisuals::ViewMode::Rays);
       #if JUCE_DEBUG
        DBG ("✨ Rays visualization activated");
       #endif
    };

    waterfallButton.onClick = [this]
    {
        setViewMode (ViewMode::Waterfall);
        if (reverbVisuals) reverbVisuals->setViewMode (ReverbVisuals::ViewMode::Waterfall);
       #if JUCE_DEBUG
        DBG ("🌊 Waterfall visualization activated");
       #endif
    };

    spectralButton.onClick = [this]
    {
        setViewMode (ViewMode::Spectral);
        if (reverbVisuals) reverbVisuals->setViewMode (ReverbVisuals::ViewMode::Spectral);
       #if JUCE_DEBUG
        DBG ("📊 Spectral visualization activated");
       #endif
    };

    // Initial states
    raysButton.setToggleState (true,  juce::dontSendNotification);
    waterfallButton.setToggleState (false, juce::dontSendNotification);
    spectralButton.setToggleState (false, juce::dontSendNotification);

    // Minimal button theming (primary handled by LookAndFeel)
    auto style = [] (juce::TextButton& b)
    {
        b.setColour (juce::TextButton::buttonColourId,    juce::Colour (0xFF2D2D2D));
        b.setColour (juce::TextButton::buttonOnColourId,  juce::Colour (0xFF4A90E2));
        b.setColour (juce::TextButton::textColourOnId,    juce::Colour (0xFFFFFFFF));
        b.setColour (juce::TextButton::textColourOffId,   juce::Colour (0xFFCCCCCC));
    };
    style (raysButton);
    style (waterfallButton);
    style (spectralButton);
}

// ─────────────────────────────────────────────────────────────────────────────
// Labels
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::setupEQLabels ()
{
    addAndMakeVisible (toneEqLabel);
    addAndMakeVisible (decayRateEqLabel);
    addAndMakeVisible (duckingLabel);
    addAndMakeVisible (decayLabel);
    addAndMakeVisible (visualizationLabel);

    auto setLabel = [] (juce::Label& L, const juce::String& text)
    {
        L.setText (text, juce::dontSendNotification);
        L.setJustificationType (juce::Justification::centred);
        L.setFont (juce::Font (12.0f, juce::Font::bold));
        L.removeColour (juce::Label::textColourId); // Let LNF drive colours
    };

    setLabel (toneEqLabel,          "TONE EQ");
    setLabel (decayRateEqLabel,     "DECAY-RATE EQ");
    setLabel (duckingLabel,         "DUCKING");
    setLabel (decayLabel,           "DECAY");
    setLabel (visualizationLabel,   "VISUALIZATION");
}

void ReverbGraphics::updateLabelColors ()
{
    if (auto* lf = dynamic_cast<FieldLNF*> (&getLookAndFeel ()))
    {
        const auto c = lf->findColour (FieldLNF::eqLabelTextColourId);
        toneEqLabel.setColour        (juce::Label::textColourId, c);
        decayRateEqLabel.setColour   (juce::Label::textColourId, c);
        duckingLabel.setColour        (juce::Label::textColourId, c);
        decayLabel.setColour          (juce::Label::textColourId, c);
        visualizationLabel.setColour (juce::Label::textColourId, c);
    }
}

void ReverbGraphics::lookAndFeelChanged ()
{
    updateLabelColors ();

    if (reverbEQ)    { reverbEQ->lookAndFeelChanged ();    reverbEQ->repaint (); }
    if (decayRateEQ) { decayRateEQ->lookAndFeelChanged (); decayRateEQ->repaint (); }

    if (duckingFloat)
    {
        duckingFloat->setLookAndFeel (&getLookAndFeel ());
        duckingFloat->lookAndFeelChanged ();
        duckingFloat->repaint ();
    }

    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// View mode
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::setViewMode (ViewMode mode)
{
    currentViewMode = mode;

    raysButton.setToggleState      (mode == ViewMode::Rays,      juce::dontSendNotification);
    waterfallButton.setToggleState (mode == ViewMode::Waterfall, juce::dontSendNotification);
    spectralButton.setToggleState  (mode == ViewMode::Spectral,  juce::dontSendNotification);

    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// Parent-level visual paint helpers (decorative; child owns main visuals)
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::paintRays (juce::Graphics& g)              { paintRaysInBounds (g, getLocalBounds ().toFloat ()); }
void ReverbGraphics::paintWaterfall (juce::Graphics& g)         { paintWaterfallInBounds (g, getLocalBounds ().toFloat ()); }
void ReverbGraphics::paintSpectral (juce::Graphics& g)          { paintSpectralInBounds (g, getLocalBounds ().toFloat ()); }

void ReverbGraphics::paintRaysInBounds (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    const auto center = bounds.getCentre ();

    auto erLevel = getErRms   ? getErRms ()   : 0.0f;
    auto tail    = getTailRms ? getTailRms () : 0.0f;

    if (erLevel == 0.0f && tail == 0.0f) { erLevel = 0.4f; tail = 0.3f; }

    const int numRays = juce::jlimit (10, 50, 20 + (int) (erLevel * 30));
    const float rayLength   = bounds.getWidth () * 0.30f;
    const float baseThick   = 2.0f;
    const auto rayColour    = juce::Colour::fromHSV (0.60f, 0.80f, 0.30f + tail * 0.70f, 0.80f);

    g.setColour (rayColour);
    juce::Random rng;
    for (int i = 0; i < numRays; ++i)
    {
        float a = (float) i / (float) numRays * kTwoPi;
        a += rng.nextFloat () * 0.2f - 0.1f;

        const auto end = center + juce::Point<float> (rayLength * std::cos (a), rayLength * std::sin (a));
        const float t  = baseThick * (0.5f + erLevel * 0.5f);

        g.drawLine (center.x, center.y, end.x, end.y, t);
    }
}

void ReverbGraphics::paintWaterfallInBounds (juce::Graphics& g, juce::Rectangle<float> bounds)
{
   #if JUCE_DEBUG
    DBG ("🌊 paintWaterfallInBounds bounds=" << bounds.toString ());
   #endif

    auto erLevel = getErRms   ? getErRms ()   : 0.0f;
    auto tail    = getTailRms ? getTailRms () : 0.0f;

    auto* lf      = dynamic_cast<FieldLNF*> (&getLookAndFeel ());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    juce::ColourGradient grad;
    grad.point1 = bounds.getTopLeft ();
    grad.point2 = bounds.getBottomLeft ();

    auto base = th.meters.panelDark.withAlpha (0.80f);
    auto mid  = th.meters.panelMedium.withAlpha (0.70f);
    auto hi   = th.meters.panelLight.withAlpha (0.60f);

    if (erLevel > 0.0f || tail > 0.0f)
    {
        const float k = juce::jmax (erLevel, tail);
        base = th.meters.panelDark  .withAlpha (0.60f + 0.30f * k);
        mid  = th.meters.panelMedium.withAlpha (0.50f + 0.40f * k);
        hi   = th.meters.panelLight .withAlpha (0.40f + 0.50f * k);
    }

    grad.addColour (0.00f, base);
    grad.addColour (0.30f, mid);
    grad.addColour (0.70f, hi);
    grad.addColour (1.00f, base);

    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, kCornerRadius);

   #if JUCE_DEBUG
    // Yellow debug border to verify visibility (off in Release)
    g.setColour (juce::Colour (0xFFFFFF00));
    g.drawRoundedRectangle (bounds, kCornerRadius, 3.0f);
   #endif

    // Subtle texture
    g.setColour (th.textMuted.withAlpha (0.08f));
    for (int i = 0; i < 20; ++i)
    {
        const float y = bounds.getY () + (float) i / 20.0f * bounds.getHeight ();
        g.drawHorizontalLine ((int) y, bounds.getX (), bounds.getRight ());
    }

    g.setColour (th.textMuted.withAlpha (0.05f));
    for (int i = 0; i < 15; ++i)
    {
        const float x = bounds.getX () + (float) i / 15.0f * bounds.getWidth ();
        g.drawVerticalLine ((int) x, bounds.getY (), bounds.getBottom ());
    }
}

void ReverbGraphics::paintSpectralInBounds (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    auto erLevel = getErRms   ? getErRms ()   : 0.0f;
    auto tail    = getTailRms ? getTailRms () : 0.0f;
    if (erLevel == 0.0f && tail == 0.0f) { erLevel = 0.4f; tail = 0.3f; }

    juce::Path erPath, tailPath;

    // ER: gently falling with frequency
    erPath.startNewSubPath (bounds.getX (), bounds.getBottom ());
    for (int i = 0; i < bounds.getWidth (); i += 2)
    {
        const float x    = bounds.getX () + (float) i;
        const float f    = juce::jmap ((float) i, 0.0f, bounds.getWidth (), 20.0f, 20000.0f);
        const float resp = erLevel * (1.0f - (f - 1000.0f) / 19000.0f);
        const float y    = bounds.getBottom () - resp * bounds.getHeight () * 0.5f;
        erPath.lineTo (x, y);
    }
    erPath.lineTo (bounds.getRight (), bounds.getBottom ());
    erPath.closeSubPath ();

    // Tail: rises toward LF
    tailPath.startNewSubPath (bounds.getX (), bounds.getBottom ());
    for (int i = 0; i < bounds.getWidth (); i += 2)
    {
        const float x    = bounds.getX () + (float) i;
        const float f    = juce::jmap ((float) i, 0.0f, bounds.getWidth (), 20.0f, 20000.0f);
        const float resp = tail * (f / 1000.0f);
        const float y    = bounds.getBottom () - resp * bounds.getHeight () * 0.3f;
        tailPath.lineTo (x, y);
    }
    tailPath.lineTo (bounds.getRight (), bounds.getBottom ());
    tailPath.closeSubPath ();

    g.setColour (juce::Colour::fromHSV (0.60f, 0.70f, 0.40f, 0.60f)); g.fillPath (erPath);
    g.setColour (juce::Colour::fromHSV (0.10f, 0.70f, 0.40f, 0.60f)); g.fillPath (tailPath);

    g.setColour (juce::Colour::fromHSV (0.60f, 0.80f, 0.80f, 0.90f)); g.strokePath (erPath,   juce::PathStrokeType (2.0f));
    g.setColour (juce::Colour::fromHSV (0.10f, 0.80f, 0.80f, 0.90f)); g.strokePath (tailPath, juce::PathStrokeType (2.0f));
}

// ─────────────────────────────────────────────────────────────────────────────
// Ducking overlay, timer, and live updates
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::paintGrOverlay (juce::Graphics& g)
{
    const auto b = getLocalBounds ().toFloat ();
    const float grDb = getDuckGrDb ? getDuckGrDb () : 0.0f;
    if (grDb >= -0.1f) return;

    g.setColour (juce::Colours::red.withAlpha (0.30f));
    g.fillRoundedRectangle (b, kCornerRadius);

    g.setColour (juce::Colours::white);
    g.setFont (16.0f);
    g.drawText (juce::String (grDb, 1) + " dB GR", b, juce::Justification::centred);
}

void ReverbGraphics::timerCallback ()
{
    if (! isVisible () || ! isShowing () || getParentComponent () == nullptr)
        return;

    animationTime += kAnimationSpeed;
    if (animationTime > kTwoPi) animationTime -= kTwoPi;

    updateDuckingModuleVisibility ();

    if (toneEnabledIds.isEmpty () || decayEnabledIds.isEmpty ())
        updateBandIndicatorsManually ();

    if (duckingFloat && getDuckGrDb)
        duckingFloat->updateGrMeter (getDuckGrDb ());

    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// DSP hooks passthrough
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::setSampleRate (double sr)
{
    if (reverbEQ)    reverbEQ->setSampleRate (sr);
    if (decayRateEQ) decayRateEQ->setSampleRate (sr);
}

void ReverbGraphics::pause  () { if (reverbEQ) reverbEQ->pause  (); if (decayRateEQ) decayRateEQ->pause  (); }
void ReverbGraphics::resume () { if (reverbEQ) reverbEQ->resume (); if (decayRateEQ) decayRateEQ->resume (); }

void ReverbGraphics::pushBlock    (const float* L, const float* R, int n)
{
    if (reverbEQ)    reverbEQ->pushBlock    (L, R, n);
    if (decayRateEQ) decayRateEQ->pushBlock (L, R, n);
}
void ReverbGraphics::pushBlockPre (const float* L, const float* R, int n)
{
    if (reverbEQ)    reverbEQ->pushBlockPre    (L, R, n);
    if (decayRateEQ) decayRateEQ->pushBlockPre (L, R, n);
}

// ─────────────────────────────────────────────────────────────────────────────
// Ducking + Band Indicator helpers
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::updateDuckingModuleVisibility ()
{
    if (! duckingFloat) return;

    const auto* duckOnParam = state.getRawParameterValue (ReverbParamIDs::duckOn);
    const bool enabled = duckOnParam ? (duckOnParam->load () > 0.5f) : false;

    duckingFloat->setVisible (true);
    duckingFloat->setActive (enabled);
    duckingFloat->setGreyedOut (! enabled);
}

void ReverbGraphics::updateBandIndicatorsManually ()
{
    int activeTone  = 0;
    int activeDecay = 0;

    for (int i = 0; i < 4; ++i)
    {
        const auto id = "tb_active_" + juce::String (i);
        if (auto* p = state.getRawParameterValue (id))
            if (p->load () > 0.5f) ++activeTone;
    }
    for (int i = 0; i < 3; ++i)
    {
        const auto id = "db_active_" + juce::String (i);
        if (auto* p = state.getRawParameterValue (id))
            if (p->load () > 0.5f) ++activeDecay;
    }

    toneEqIndicator.setActiveBands (activeTone);
    decayRateEqIndicator.setActiveBands (activeDecay);
    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// BandIndicator (inline component)
// ─────────────────────────────────────────────────────────────────────────────

ReverbGraphics::BandIndicator::BandIndicator (int max)
    : maxBands (max), activeBands (0)
{
    setSize ((int) (maxBands * kCircleSpacing), (int) kCircleSize);
}

void ReverbGraphics::BandIndicator::paint (juce::Graphics& g)
{
    auto* lf      = dynamic_cast<FieldLNF*> (&getLookAndFeel ());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    g.setColour (th.accent);

    for (int i = 0; i < maxBands; ++i)
    {
        auto circle = juce::Rectangle<float> (i * kCircleSpacing, 0.0f, kCircleSize, kCircleSize);

        if (i < activeBands) g.fillEllipse (circle);
        else                 g.drawEllipse (circle, 1.5f);
    }
}

void ReverbGraphics::BandIndicator::setActiveBands (int count)
{
    activeBands = juce::jlimit (0, maxBands, count);
    repaint ();
}

void ReverbGraphics::BandIndicator::setMaxBands (int max)
{
    maxBands = max;
    setSize ((int) (maxBands * kCircleSpacing), (int) kCircleSize);
    repaint ();
}

// ─────────────────────────────────────────────────────────────────────────────
// VisualizationControlPanel chrome
// ─────────────────────────────────────────────────────────────────────────────

void ReverbGraphics::VisualizationControlPanel::paint (juce::Graphics& g)
{
    const auto b  = getLocalBounds ().toFloat ();
    auto* lf      = dynamic_cast<FieldLNF*> (&getLookAndFeel ());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    g.setColour (th.meters.panelDark);
    g.fillRect (b);
    g.fillRoundedRectangle (b, kCornerRadius);

    g.setColour (th.sh.withAlpha (kOuterShadowAlpha));
    g.drawRoundedRectangle (b.reduced (0.5f), kCornerRadius - 0.5f, kEdgeOutlineThick);

    g.setColour (th.accent.withAlpha (0.90f));
    g.drawRoundedRectangle (b.reduced (1.0f), kCornerRadius - 1.0f, 1.5f);
}
