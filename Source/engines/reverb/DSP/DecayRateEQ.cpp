/*
====================================================================================================
 DecayRateEQ.cpp — Implementation Overview
 ---------------------------------------------------------------------------------------------------
 Rendering Pipeline
    paint()
      ├─ drawUnits(g)                       : grid, Hz ticks, multiplier ticks; 1.0× line emphasized
      ├─ rebuildEqPath()                    : (on size changes / edits) samples response across log-Hz
      │    • Combined path: product of per-band multipliers (starts at 1.0× everywhere)
      │    • Per-band paths: thin colored overlays for visual attribution
      └─ Points & selection/hover           : band handles, selection ring, hover guides, ghost preview

 Path Sampling Details
    • N = max(128, analyzer width in px); log-spaced Hz from 20..20k.
    • Combined value = Π bandMultAtForPaint(bi, hz).
    • Bell uses Gaussian (log-domain distance); Tilts use logistic to bias low or high region.
    • Rebuild is triggered in resized() and after param/point changes.

 Mouse / Overlay / Badge Lifecycle
    • mouseDown:
         - Hit point → select (toggle overlay if clicked again)
         - Empty → add band (limit 3). The type heuristic:
             <~50 Hz  → TiltLo
             >~10 kHz → TiltHi
             else     → Bell
         - New band writes APVTS: db_active=1, db_freqHz, db_q, db_decayMult.
    • mouseDrag:
         - Moves selected point → writes db_freqHz and db_decayMult.
    • mouseWheel:
         - Changes Q multiplicatively (Shift for finer steps) → writes db_q.
    • mouseDoubleClick:
         - Deletes band → sets db_active=0 in that slot, removes local point, updates paths.
    • Hover:
         - Shows vertical guides and Hz badges; after ~220 ms idle, shows a "ghost" preview curve at
           the cursor (Bell/Tilt by region; sign based on cursor above/below 1.0× line).

 APVTS Bridging
    • bandId(base, idx)    : "base_idx" naming (e.g., "db_q_1").
    • allocateBandSlot()   : scans 0..kMaxBands-1; first with db_active<0.5 is free.
    • setBandParam(...)    : convertTo0to1 + setValueNotifyingHost — host automation safe.
    • getBandParamFloat(...) provided for future "syncFromParameters()".

 Theme Reactivity
    • FieldLNF ChangeBroadcaster is observed; lookAndFeelChanged() → parentHierarchyChanged() re-wires
      listener; repaint() on theme change. Color IDs used:
        - eqBorderColourId, eqLabelTextColourId, eqZeroLineColourId, eqGridLineColourId,
          eqBandHandleColourId, eqBandHandleActiveId, eqAnalyzerTraceColourId

 Coordinate Maps
    • mapHzToX / mapXToHz           : log scale map (20..20k).
    • mapMultToY / mapYToMult       : 2.0× (top) → 0.5× (bottom), 1.0× emphasized by zeroCol.

 Anti-aliasing Note
    • To avoid rounded-corner fringe artifacts, the component fills the full rect, then draws the
      rounded rectangle and the border.

 Performance & Safety
    • A 30 Hz timer drives hover/ghost UX and lightweight repaints (no heavy allocations in timer).
    • Path sampling preallocates and uses stack functors; cost scales ~O(width).
    • UI thread only; APVTS setValueNotifyingHost is used for automation correctness.

 Gaps / TODOs (intentional)
    • "Type" persistence:
         The UI's type (Bell/TiltLo/TiltHi) is currently *not* persisted. If required, either:
         (1) add ReverbEQParams::DecayBand::type to the param set, or
         (2) explicitly store in db_dynAmt and treat it as an enum (document this contract).
         Right now onTypeChanged() writes to db_dynAmt only if you choose that path.
    • State hydration on open:
         Provide a `syncFromParameters()` to read existing db_* slots (0..2) and rebuild `points`.
         Call it from your owning editor after APVTS is fully initialized.

 Consistency with DSP
    • Ensure the DSP uses the same band shapes (Bell/Tilt logistic), Q semantics, and multiplier
      interpretation (product at frequency) to match visuals. If DSP differs, consider sharing a
      small shape helper to keep math identical.

 Limits
    • kMaxBands = 3 (UI enforced).
====================================================================================================
*/

#include "DecayRateEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

using namespace juce;

DecayRateEQ::DecayRateEQ (MyPluginAudioProcessor& p)
    : proc (p)
{
    setOpaque (true);
    setWantsKeyboardFocus (true);

    // Zoom for decay multiplier view; we map explicitly (2.0 .. 0.5) but
    // keep a zoom state for potential future gestures.
    zoomState.prepare (1.5f);

    // Analyzer (visual only)
    addAndMakeVisible (analyzer);
    analyzer.setInterceptsMouseClicks (false, false);
    analyzer.setAutoHeadroomEnabled (true);
    analyzer.setHeadroomTargetFill (0.70f);
    SpectrumAnalyzer::Params prm;
    prm.fps = 30;
    analyzer.setParams (prm);
    analyzer.setDrawGridHorizontal (false); // draw our own units

    // Editors
    addAndMakeVisible (overlay);
    overlay.setVisible (false);

    addAndMakeVisible (badge);
    badge.setVisible (false);

    // Overlay callbacks
    overlay.onMultChanged = [this] (float m)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.mult = jlimit (0.5f, 2.0f, m);
            if (pt.bandIdx >= 0)
                setBandParam (pt.bandIdx, "db_decayMult", pt.mult);
            rebuildEqPath(); repaint();
        }
    };
    overlay.onQChanged = [this] (float qv)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.q = jlimit (0.1f, 36.0f, qv);
            if (pt.bandIdx >= 0)
                setBandParam (pt.bandIdx, "db_q", pt.q);
            rebuildEqPath(); repaint();
        }
    };
    overlay.onFreqChanged = [this] (float f)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.hz = jlimit (20.f, 20000.f, f);
            if (pt.bandIdx >= 0)
                setBandParam (pt.bandIdx, "db_freqHz", pt.hz);
            rebuildEqPath(); repaint();
        }
    };
    overlay.onTypeChanged = [this] (int t)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.type = jlimit (0, 2, t);
            // type is visual classification only; no direct param for type here.
            rebuildEqPath(); repaint(); positionBadgeFor (selected);
        }
    };

    // Badge callbacks
    badge.onDelete = [this]
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            const int bandIdx = points[(size_t) selected].bandIdx;
            if (bandIdx >= 0)
                setBandParam (bandIdx, "db_active", 0.0f);

            points.erase (points.begin() + selected);
            selected = -1;
            rebuildEqPath(); repaint();
            overlay.setVisible (false);
            badge.setVisible (false);
        }
    };
    badge.onBypass = [this] (bool off)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            const int bandIdx = points[(size_t) selected].bandIdx;
            if (bandIdx >= 0)
                setBandParam (bandIdx, "db_active", off ? 0.0f : 1.0f);
        }
    };
    badge.onSetType = [this] (int tp)
    {
        const int idx = (badgeFor >= 0 ? badgeFor : selected);
        if (idx >= 0 && idx < (int) points.size())
        {
            auto& pnt = points[(size_t) idx];
            pnt.type = jlimit (0, 2, tp);
            rebuildEqPath(); repaint(); positionBadgeFor (idx);
        }
    };

    // Start animation after UI has a chance to show
    startTimerHz (30);
}

DecayRateEQ::~DecayRateEQ()
{
    stopTimer();
    if (auto* old = listeningTo)
        old->removeChangeListener (this);
}

void DecayRateEQ::visibilityChanged()
{
    if (isVisible()) startTimerHz (30);
    else             stopTimer();

    if (! isShowing())
    {
        overlay.setVisible (false);
        badge.setVisible (false);
        selected = -1;
    }
}

void DecayRateEQ::timerCallback()
{
    // Drive hover reveals and subtle ghosting
    if (isShowing())
        repaint();
}

void DecayRateEQ::lookAndFeelChanged()
{
    parentHierarchyChanged();
    repaint();
}

void DecayRateEQ::parentHierarchyChanged()
{
    if (auto* old = listeningTo)
        old->removeChangeListener (this);

    listeningTo = dynamic_cast<FieldLNF*> (&getLookAndFeel());
    if (listeningTo)
        listeningTo->addChangeListener (this);
}

void DecayRateEQ::changeListenerCallback (juce::ChangeBroadcaster* src)
{
    if (src == dynamic_cast<FieldLNF*> (&getLookAndFeel()))
        repaint();
}

void DecayRateEQ::paint (juce::Graphics& g)
{
    auto r  = getLocalBounds().toFloat();
    auto& lf = getLookAndFeel();

    // Theme colours
    const auto border   = lf.findColour (FieldLNF::eqBorderColourId);
    const auto panel    = lf.findColour (ResizableWindow::backgroundColourId);
    const auto accent   = lf.findColour (FieldLNF::eqLabelTextColourId);
    const auto zeroLine = lf.findColour (FieldLNF::eqZeroLineColourId);
    const auto gridLine = lf.findColour (FieldLNF::eqGridLineColourId);

    // Fill full rect first (anti-aliased rounded corners without bright pixels)
    const float cr = 8.0f;
    g.setColour (panel);
    g.fillRect (r);
    g.fillRoundedRectangle (r, cr);

    // Border
    g.setColour (border);
    g.drawRoundedRectangle (r.reduced (1.0f), cr - 1.0f, 1.5f);

    // Units & grid
    drawUnits (g);

    // Curves
    auto rA = analyzer.getBounds().toFloat();
    if (rA.isEmpty())
        return;

    // Combined curve
    g.setColour (accent.withAlpha (0.95f));
    g.strokePath (eqPath, PathStrokeType (3.0f));

    // Per-band curves
    for (size_t i = 0; i < bandPaths.size(); ++i)
    {
        g.setColour (bandColourFor ((int) i).withAlpha (0.90f));
        g.strokePath (bandPaths[i], PathStrokeType (1.2f));
    }

    // Points
    g.setColour (accent.withAlpha (0.95f));
    for (const auto& pt : points)
    {
        const float x = mapHzToX (pt.hz);
        const float y = mapMultToY (pt.mult);
        g.fillEllipse (x - 8, y - 8, 16, 16);
    }

    // Selection ring
    if (selected >= 0 && selected < (int) points.size())
    {
        const auto& pt = points[(size_t) selected];
        const float x = mapHzToX (pt.hz);
        const float y = mapMultToY (pt.mult);
        g.setColour (border.withAlpha (0.6f));
        g.drawEllipse (x - 12, y - 12, 24, 24, 1.6f);
    }

    // Hover guides + predictive ghost
    if (hoverInPane)
    {
        const auto nowMs   = (int64) Time::getMillisecondCounterHiRes();
        const bool ghostOn = (nowMs - lastMouseMoveMs) >= (int64) ghostDelayMs;

        const float x = (float) hoverPos.x;
        const float aGhost = 0.34f;
        const float aMove  = jmap ((float) jlimit<int64> (0, ghostDelayMs, nowMs - lastMouseMoveMs),
                                   0.0f, (float) ghostDelayMs, 0.26f, 0.18f);

        // Guides
        g.setColour (accent.withAlpha (ghostOn ? aGhost : aMove));
        g.drawLine (x, rA.getY(), x, rA.getBottom(), ghostOn ? 1.4f : 1.0f);
        g.setColour (accent.withAlpha ((ghostOn ? 0.12f : 0.06f)));
        g.drawLine (x - 12.0f, rA.getY(), x - 12.0f, rA.getBottom(), ghostOn ? 1.0f : 0.8f);
        g.drawLine (x + 12.0f, rA.getY(), x + 12.0f, rA.getBottom(), ghostOn ? 1.0f : 0.8f);

        // Hz badges
        g.setColour (accent.withAlpha (0.60f));
        String hzText;
        if (hoverHz >= 1000.0f && hoverHz < 10000.0f) hzText = String (hoverHz / 1000.0f, 1) + "k";
        else if (hoverHz >= 10000.0f)                 hzText = String ((int) std::round (hoverHz / 1000.0f)) + "k";
        else                                          hzText = String ((int) hoverHz);
        auto lbl = hzText + " Hz";

        auto tb = Rectangle<float> (x - 32.0f, rA.getBottom() - 20.0f, 64.0f, 14.0f);
        g.setColour (border.withAlpha (0.45f));
        g.fillRoundedRectangle (tb, 4.0f);
        g.setColour (accent.withAlpha (0.80f));
        g.drawFittedText (lbl, tb.toNearestInt(), Justification::centred, 1);

        auto tt = Rectangle<float> (x - 28.0f, rA.getY() + 6.0f, 56.0f, 14.0f);
        g.setColour (border.withAlpha (0.40f));
        g.fillRoundedRectangle (tt, 4.0f);
        g.setColour (accent.withAlpha (0.85f));
        g.drawFittedText (lbl, tt.toNearestInt(), Justification::centred, 1);

        // Ghost
        if (ghostOn)
        {
            // Avoid draw if too close to existing point
            const float suppressRadiusPx = 24.0f;
            bool nearPoint = false;
            for (const auto& pt : points)
                if (Point<float> (mapHzToX (pt.hz), mapMultToY (pt.mult))
                        .getDistanceFrom (hoverPos.toFloat()) <= suppressRadiusPx)
                { nearPoint = true; break; }

            if (! nearPoint)
            {
                Path ghost;
                const bool mouseAbove1 = mapYToMult (hoverPos.y) > 1.0f;

                auto makeGhost = [&] (int type, float amtMult)
                {
                    DecayBandPoint b; b.type = type; b.hz = hoverHz; b.mult = amtMult; b.q = 0.9f;
                    const int N = jmax (64, (int) rA.getWidth());
                    for (int i = 0; i < N; ++i)
                    {
                        const double minHz = 20.0, maxHz = 20000.0;
                        const double t = (double) i / (double) (N - 1);
                        const double a = std::log10 (minHz), bL = std::log10 (maxHz);
                        const double logF = jmap (t, 0.0, 1.0, a, bL);
                        const double hz = std::pow (10.0, logF);
                        const float xx = rA.getX() + (float) i / (float) (N - 1) * rA.getWidth();
                        const float yy = mapMultToY (bandMultAtForPaint (b, (float) hz));
                        if (i == 0) ghost.startNewSubPath (xx, yy);
                        else        ghost.lineTo (xx, yy);
                    }
                };

                if (hoverHz <= 50.0f)        makeGhost (1 /*TiltLo*/, mouseAbove1 ? 1.3f : 0.7f);
                else if (hoverHz >= 10000.0f) makeGhost (2 /*TiltHi*/, mouseAbove1 ? 1.3f : 0.7f);
                else                           makeGhost (0 /*Bell*/,   mouseAbove1 ? 1.3f : 0.7f);

                Path clipped;
                clipped.addEllipse (x - rA.getWidth() * 0.05f,
                                    (float) hoverPos.y - rA.getHeight() * 0.15f,
                                    rA.getWidth() * 0.10f,
                                    rA.getHeight() * 0.30f);
                Graphics::ScopedSaveState ss (g);
                g.reduceClipRegion (clipped);

                g.setColour (accent.withAlpha (0.16f));
                g.strokePath (ghost, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
            }
        }
    }
}

void DecayRateEQ::resized()
{
    auto r = getLocalBounds().reduced (6);
    analyzer.setBounds (r);
    rebuildEqPath();
}

void DecayRateEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();

    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    const int N = jmax (128, (int) r.getWidth());

    auto totalMultAt = [this] (double hz)
    {
        float s = 1.0f;
        for (const auto& b : points)
            s *= bandMultAtForPaint (b, (float) hz);
        return s;
    };

    auto p0y = mapMultToY (totalMultAt (20.0));
    eqPath.startNewSubPath (r.getX(), p0y);

    for (int i = 1; i < N; ++i)
    {
        const double minHz = 20.0, maxHz = 20000.0;
        const double t = (double) i / (double) (N - 1);
        const double a = std::log10 (minHz), b = std::log10 (maxHz);
        const double logF = jmap (t, 0.0, 1.0, a, b);
        const double hz = std::pow (10.0, logF);
        const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
        const float y = mapMultToY (totalMultAt (hz));
        eqPath.lineTo (x, y);
    }

    // Per-band paths
    bandPaths.resize (points.size());
    for (size_t bi = 0; bi < points.size(); ++bi)
    {
        auto& bp = bandPaths[bi];
        const float y0 = mapMultToY (bandMultAtForPaint (points[bi], 20.0f));
        bp.startNewSubPath (r.getX(), y0);
        for (int i = 1; i < N; ++i)
        {
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double) i / (double) (N - 1);
            const double a = std::log10 (minHz), b = std::log10 (maxHz);
            const double logF = jmap (t, 0.0, 1.0, a, b);
            const double hz = std::pow (10.0, logF);
            const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
            const float y = mapMultToY (bandMultAtForPaint (points[bi], (float) hz));
            bp.lineTo (x, y);
        }
    }
}

void DecayRateEQ::drawUnits (juce::Graphics& g)
{
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    auto& lf = getLookAndFeel();
    auto gridCol = lf.findColour (FieldLNF::eqGridLineColourId);
    auto textCol = lf.findColour (FieldLNF::eqLabelTextColourId).withAlpha (0.45f);
    auto zeroCol = lf.findColour (FieldLNF::eqZeroLineColourId);

    g.setFont (12.0f);

    // Multiplier ticks
    const float multVals[] = { 2.0f, 1.5f, 1.0f, 0.75f, 0.5f };
    for (float mult : multVals)
    {
        const float y = mapMultToY (mult);

        if (mult == 1.0f) { g.setColour (zeroCol); }
        else              { g.setColour (gridCol); }

        g.drawLine (r.getX(), y, r.getRight(), y, mult == 1.0f ? 1.2f : 0.6f);

        g.setColour (textCol);
        g.drawFittedText (String (mult, 1) + "×",
                          Rectangle<int> ((int) r.getX() + 4, (int) y - 8, 44, 16),
                          Justification::centredLeft, 1);
    }

    // Hz ticks
    const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000 };
    for (double hz : hzTicks)
    {
        const float x = mapHzToX ((float) hz);
        g.setColour (gridCol);
        g.drawLine (x, r.getBottom() - 16.0f, x, r.getBottom(), 0.8f);

        g.setColour (textCol);
        String lbl;
        if (hz >= 1000.0 && hz < 10000.0) lbl = String (hz / 1000.0, 1) + "k";
        else if (hz >= 10000.0)           lbl = String ((int) std::round (hz / 1000.0)) + "k";
        else                              lbl = String ((int) hz);

        g.drawFittedText (lbl,
                          Rectangle<int> ((int) x - 18, (int) r.getBottom() - 30, 36, 14),
                          Justification::centred, 1);
    }
}

// =========================== Interaction ===========================

void DecayRateEQ::mouseDown (const MouseEvent& e)
{
    const int h = hitTestPoint (e.getPosition());
    if (h >= 0)
    {
        if (h == selected && overlay.isVisible())
        {
            // toggle off
            overlay.setVisible (false);
            badge.setVisible   (false);
            selected = -1;
        }
        else
        {
            selected = h;
            auto& pt = points[(size_t) selected];
            overlay.setValues (pt.mult, pt.q, pt.hz, pt.type);
            overlay.setVisible (true);
            positionOverlay();
            positionBadgeFor (selected);
        }
        return;
    }

    if (e.mods.isPopupMenu())
    {
        overlay.setVisible (false);
        badge.setVisible   (false);
        return;
    }

    // Add new point (limit 3)
    if (points.size() >= (size_t) kMaxBands)
        return;

    DecayBandPoint bp;
    bp.hz   = jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x));
    bp.mult = jlimit (0.5f, 2.0f,   mapYToMult (e.getPosition().y));
    if (bp.hz <= 50.0f)        bp.type = 1; // TiltLo
    else if (bp.hz >= 10000.0f) bp.type = 2; // TiltHi
    else                        bp.type = 0; // Bell

    const int slot = allocateBandSlot();
    if (slot >= 0)
    {
        bp.bandIdx = slot;
        setBandParam (slot, "db_active",    1.0f);
        setBandParam (slot, "db_freqHz",    bp.hz);
        setBandParam (slot, "db_decayMult", bp.mult);
        setBandParam (slot, "db_q",         bp.q);
    }

    points.push_back (bp);
    selected = (int) points.size() - 1;

    overlay.setValues (bp.mult, bp.q, bp.hz, bp.type);
    overlay.setVisible (true);
    positionOverlay();
    positionBadgeFor (selected);

    rebuildEqPath();
    repaint();
}

void DecayRateEQ::mouseDrag (const MouseEvent& e)
{
    if (selected < 0 || selected >= (int) points.size())
        return;

    auto& pt = points[(size_t) selected];
    pt.hz   = jlimit (20.f, 20000.f, mapXToHz   (e.getPosition().x));
    pt.mult = jlimit (0.5f, 2.0f,    mapYToMult (e.getPosition().y));

    if (pt.bandIdx >= 0)
    {
        setBandParam (pt.bandIdx, "db_freqHz",    pt.hz);
        setBandParam (pt.bandIdx, "db_decayMult", pt.mult);
    }

    rebuildEqPath();
    overlay.setValues (pt.mult, pt.q, pt.hz, pt.type);
    positionOverlay();
    positionBadgeFor (selected);
    repaint();
}

void DecayRateEQ::mouseUp (const MouseEvent&)
{
    dragging = false;
}

void DecayRateEQ::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (selected < 0 || selected >= (int) points.size())
        return;

    auto& pt = points[(size_t) selected];
    const double factor = e.mods.isShiftDown() ? 1.0 : 0.2;
    const float delta   = (float) (wheel.deltaY * factor);

    // Wheel adjusts Q multiplicatively (feels natural for wide range)
    pt.q = jlimit (0.1f, 36.0f, pt.q * (1.0f + delta));
    if (pt.bandIdx >= 0)
        setBandParam (pt.bandIdx, "db_q", pt.q);

    rebuildEqPath();
    overlay.setValues (pt.mult, pt.q, pt.hz, pt.type);
    positionOverlay();
    positionBadgeFor (selected);
    repaint();
}

void DecayRateEQ::mouseDoubleClick (const MouseEvent& e)
{
    const int idx = hitTestPoint (e.getPosition());
    if (idx < 0 || idx >= (int) points.size())
        return;

    const int bandIdx = points[(size_t) idx].bandIdx;
    if (bandIdx >= 0)
        setBandParam (bandIdx, "db_active", 0.0f);

    points.erase (points.begin() + idx);
    if (selected == idx)      selected = -1;
    else if (selected > idx)  --selected;

    rebuildEqPath();
    repaint();

    if (selected < 0)
    {
        overlay.setVisible (false);
        badge.setVisible   (false);
    }
    else
    {
        const auto& pt2 = points[(size_t) selected];
        overlay.setValues (pt2.mult, pt2.q, pt2.hz, pt2.type);
        overlay.setVisible (true);
        positionOverlay();
        positionBadgeFor (selected);
    }
}

void DecayRateEQ::mouseMove (const MouseEvent& e)
{
    const int h = hitTestPoint (e.getPosition());
    if (h != hover)
    {
        hover = h;
        if (selected < 0)
        {
            if (hover >= 0) positionBadgeFor (hover);
            else            badge.setVisible (false);
        }
        else
        {
            if (hover >= 0) positionBadgeFor (hover);
            else            positionBadgeFor (selected);
        }
    }

    hoverPos   = e.getPosition();
    auto rPane = analyzer.getBounds();
    hoverInPane = rPane.contains (hoverPos);

    if (hoverInPane)
        hoverHz = jlimit (20.0f, 20000.0f, mapXToHz (hoverPos.x));

    lastMouseMoveMs = (int64) Time::getMillisecondCounterHiRes();
    repaint();
}

void DecayRateEQ::mouseExit (const MouseEvent&)
{
    if (selected < 0)
        badge.setVisible (false);

    hover = -1;
    hoverInPane = false;
    repaint();
}

bool DecayRateEQ::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::escapeKey)
    {
        selected = -1;
        overlay.setVisible (false);
        badge.setVisible   (false);
        repaint();
        return true;
    }
    return false;
}

// =========================== Mapping / HitTest ===========================

float DecayRateEQ::mapHzToX (float hz) const
{
    auto r = analyzer.getBounds().toFloat();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = (float) (std::log10 (jlimit (minHz, maxHz, hz) / minHz)
                             / std::log10 (maxHz / minHz));
    return r.getX() + t * r.getWidth();
}

float DecayRateEQ::mapMultToY (float mult) const
{
    auto r = analyzer.getBounds().toFloat();
    const float top = r.getY() + 8.f, bottom = r.getBottom() - 8.f;
    return jmap (mult, 2.0f, 0.5f, top, bottom);
}

float DecayRateEQ::mapXToHz (int px) const
{
    auto r = analyzer.getBounds();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = jlimit (0.0f, 1.0f, (px - (float) r.getX()) / (float) r.getWidth());
    const float a = std::log10 (minHz), b = std::log10 (maxHz);
    return std::pow (10.0f, jmap (t, 0.0f, 1.0f, a, b));
}

float DecayRateEQ::mapYToMult (int py) const
{
    auto r = analyzer.getBounds();
    return jmap ((float) py, (float) r.getY(), (float) r.getBottom(), 2.0f, 0.5f);
}

int DecayRateEQ::hitTestPoint (juce::Point<int> p) const
{
    const float radius = 12.0f;
    for (int i = (int) points.size() - 1; i >= 0; --i)
    {
        const float x = mapHzToX   (points[(size_t) i].hz);
        const float y = mapMultToY (points[(size_t) i].mult);
        if (Point<float> (x, y).getDistanceFrom (p.toFloat()) <= radius)
            return i;
    }
    return -1;
}

// =========================== Bands / Params ===========================

int DecayRateEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto id = bandId ("db_active", i);
        if (auto* v = proc.apvts.getRawParameterValue (id))
            if (v->load() < 0.5f)
                return i;
    }
    return -1;
}

void DecayRateEQ::setBandParam (int bandIdx, const char* baseId, float value)
{
    auto id = bandId (baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter (id))
    {
        const float norm = p->convertTo0to1 (value);
        p->setValueNotifyingHost (norm);
    }
}

float DecayRateEQ::getBandParamFloat (int bandIdx, const char* baseId, float fallback) const
{
    auto id = bandId (baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue (id))
        return v->load();
    return fallback;
}

Colour DecayRateEQ::bandColourFor (int bandIdx) const
{
    auto& lf = getLookAndFeel();
    auto accent = lf.findColour (FieldLNF::eqLabelTextColourId);

    const float baseHue = accent.getHue();
    const float baseSat = jlimit (0.25f, 0.95f, accent.getSaturation());
    const float baseBrt = jlimit (0.35f, 0.95f, accent.getBrightness());

    const float golden = 0.61803398875f;
    float hue = std::fmod (baseHue + golden * (float) (bandIdx + 1), 1.0f);
    hue = jlimit (0.0f, 1.0f, 0.65f * hue + 0.35f * baseHue);
    float sat = jlimit (0.30f, 0.95f, baseSat * 0.9f + 0.1f);
    float brt = jlimit (0.40f, 0.95f, baseBrt * 0.9f + 0.1f);
    return Colour::fromHSV (hue, sat, brt, 1.0f);
}

float DecayRateEQ::bandMultAtForPaint (const DecayBandPoint& b, float hz) const
{
    const double logHz = std::log10 (jlimit (20.0f, 20000.0f, hz));
    const double logC  = std::log10 (jlimit (20.0f, 20000.0f, b.hz));
    const double q     = jlimit (0.1, 36.0, (double) b.q);
    const double width = jlimit (0.02, 0.50, 0.22 / q);
    const double d     = (logHz - logC) / width;

    switch (b.type)
    {
        case 0: // Bell
        {
            const float w = (float) std::exp (-0.5 * d * d);
            return jmap (w, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 1: // TiltLo (increasing to the right)
        {
            const double k = 8.0 * jlimit (0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
            return jmap ((float) s, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 2: // TiltHi (decreasing to the right)
        {
            const double k = 8.0 * jlimit (0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
            return jmap ((float) (1.0 - s), 0.0f, 1.0f, 1.0f, b.mult);
        }
        default: return 1.0f;
    }
}

// =========================== Overlay ===========================

DecayRateEQ::BandOverlay::BandOverlay()
{
    setInterceptsMouseClicks (true, true);

    mult.setSliderStyle (Slider::LinearHorizontal);
    mult.setTextBoxStyle (Slider::TextBoxRight, false, 48, 18);
    mult.setRange (0.5, 2.0, 0.01);
    mult.onValueChange = [this] { if (! updating && onMultChanged) onMultChanged ((float) mult.getValue()); };
    mult.onDragStart   = [this] { if (onDragAny) onDragAny (true); };
    mult.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (mult);

    q.setSliderStyle (Slider::LinearHorizontal);
    q.setTextBoxStyle (Slider::TextBoxRight, false, 48, 18);
    q.setRange (0.1, 36.0, 0.01);
    q.onValueChange = [this] { if (! updating && onQChanged) onQChanged ((float) q.getValue()); };
    q.onDragStart   = [this] { if (onDragAny) onDragAny (true); };
    q.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (q);

    freq.setSliderStyle (Slider::LinearHorizontal);
    freq.setTextBoxStyle (Slider::TextBoxRight, false, 64, 18);
    freq.setRange (20.0, 20000.0, 0.01);
    freq.setSkewFactorFromMidPoint (1000.0);
    freq.onValueChange = [this] { if (! updating && onFreqChanged) onFreqChanged ((float) freq.getValue()); };
    freq.onDragStart   = [this] { if (onDragAny) onDragAny (true); };
    freq.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (freq);

    multLabel.setText ("MULT", dontSendNotification);
    multLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (multLabel);

    qLabel.setText ("Q", dontSendNotification);
    qLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (qLabel);

    freqLabel.setText ("FREQ", dontSendNotification);
    freqLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (freqLabel);

    typeCb.addItemList (StringArray { "Bell", "TiltLo", "TiltHi" }, 1);
    typeCb.onChange = [this] { if (! updating && onTypeChanged) onTypeChanged (typeCb.getSelectedItemIndex()); };
    addAndMakeVisible (typeCb);

    typeLabel.setText ("TYPE", dontSendNotification);
    typeLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (typeLabel);
}

DecayRateEQ::BandOverlay::~BandOverlay() {}

void DecayRateEQ::BandOverlay::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    auto* lf = dynamic_cast<FieldLNF*> (&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;

    Colour bg = th.panel.darker (0.20f);
    g.setColour (bg.withAlpha (0.96f));
    g.fillRoundedRectangle (r, 8.0f);

    g.setColour (th.accent.withAlpha (0.8f));
    g.drawRoundedRectangle (r, 8.0f, 2.0f);

    Rectangle<float> strip = r.removeFromLeft (3.0f).reduced (0.5f, 2.0f);
    g.setColour (th.accent);
    g.fillRoundedRectangle (strip, 1.5f);
}

void DecayRateEQ::BandOverlay::resized()
{
    auto r = getLocalBounds().reduced (8);
    const int labelW = 40, sliderH = 20, gap = 4;

    multLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    mult.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    qLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    q.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    freqLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    freq.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    typeLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    typeCb.setBounds    (r.removeFromTop (sliderH));
}

void DecayRateEQ::BandOverlay::setValues (float multVal, float qVal, float freqVal, int type)
{
    updating = true;
    mult.setValue (multVal, dontSendNotification);
    q.setValue    (qVal,    dontSendNotification);
    freq.setValue (freqVal, dontSendNotification);
    typeCb.setSelectedItemIndex (type, dontSendNotification);
    updating = false;
}

// =========================== Badge ===========================

DecayRateEQ::BandBadge::BandBadge()
{
    setInterceptsMouseClicks (true, true);

    deleteBtn.setButtonText ("×");
    deleteBtn.onClick = [this] { if (onDelete) onDelete(); };
    addAndMakeVisible (deleteBtn);

    bypassBtn.setButtonText ("BYP");
    bypassBtn.onClick = [this] { if (onBypass) onBypass (bypassBtn.getToggleState()); };
    addAndMakeVisible (bypassBtn);

    typeBtn.setButtonText ("Bell");
    typeBtn.onClick = [this] { if (onSetType) onSetType (0); };
    addAndMakeVisible (typeBtn);

    freqLabel.setText ("1.0k", dontSendNotification);
    freqLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (freqLabel);

    multLabel.setText ("1.0x", dontSendNotification);
    multLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (multLabel);

    qLabel.setText ("0.7", dontSendNotification);
    qLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (qLabel);
}

DecayRateEQ::BandBadge::~BandBadge() {}

void DecayRateEQ::BandBadge::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    auto& lf = getLookAndFeel();
    auto accent = lf.findColour (FieldLNF::eqLabelTextColourId);
    auto panel  = lf.findColour (ResizableWindow::backgroundColourId);

    Colour bg = panel.darker (0.30f);
    g.setColour (bg.withAlpha (0.95f));
    g.fillRoundedRectangle (r, 6.0f);

    g.setColour (accent.withAlpha (0.9f));
    g.drawRoundedRectangle (r, 6.0f, 1.5f);

    Rectangle<float> strip = r.removeFromLeft (2.0f).reduced (0.5f, 1.0f);
    g.setColour (accent);
    g.fillRoundedRectangle (strip, 1.0f);
}

void DecayRateEQ::BandBadge::resized()
{
    auto r = getLocalBounds().reduced (4);
    const int btnW = 24, btnH = 16;

    deleteBtn.setBounds (r.removeFromTop (btnH).removeFromLeft (btnW));
    bypassBtn.setBounds (r.removeFromTop (btnH).removeFromLeft (btnW));
    typeBtn.setBounds   (r.removeFromTop (btnH).removeFromLeft (btnW));

    freqLabel.setBounds (r.removeFromTop (btnH));
    multLabel.setBounds (r.removeFromTop (btnH));
    qLabel.setBounds    (r.removeFromTop (btnH));
}

void DecayRateEQ::BandBadge::setValues (float mult, float freq, int type, bool bypass)
{
    currentMult   = mult;
    currentFreq   = freq;
    currentType   = type;
    currentBypass = bypass;

    freqLabel.setText (String (freq, freq >= 1000.0f ? 1 : 0) + (freq >= 1000.0f ? "k" : ""),
                       dontSendNotification);
    multLabel.setText (String (mult, 1) + "x", dontSendNotification);
    bypassBtn.setToggleState (bypass, dontSendNotification);

    static const char* typeNames[] = { "Bell", "TiltLo", "TiltHi" };
    typeBtn.setButtonText (typeNames[jlimit (0, 2, type)]);
}

void DecayRateEQ::BandBadge::setDetails (float q, float mult, bool /*dynOn*/, bool /*dynUp*/,
                                         float /*dynRange*/, bool /*specOn*/, const String& /*channel*/,
                                         int /*slopeDb*/, const String& /*tap*/)
{
    currentQ    = q;
    currentMult = mult;

    multLabel.setText (String (mult, 1) + "x", dontSendNotification);
    qLabel.setText    (String (q, 2),         dontSendNotification);
}

// =========================== Overlay/Badge placement ===========================

void DecayRateEQ::positionOverlay()
{
    if (selected < 0 || selected >= (int) points.size())
        return;

    auto r = getLocalBounds();
    const int w = 200, h = 120;

    const auto& pt = points[(size_t) selected];
    const float bandX = mapHzToX   (pt.hz);
    const float bandY = mapMultToY (pt.mult);

    int ox = (int) bandX - w / 2;
    int oy = (int) bandY - h / 2;

    const int bandRadius = 12;
    const int margin     = 20;

    const bool overlapsBand =
        (ox <= bandX + bandRadius + margin && ox + w >= bandX - bandRadius - margin &&
         oy <= bandY + bandRadius + margin && oy + h >= bandY - bandRadius - margin);

    if (overlapsBand)
    {
        // Right
        ox = (int) bandX + bandRadius + margin;
        oy = (int) bandY - h / 2;

        // Left
        if (ox + w > r.getRight())
            ox = (int) bandX - w - bandRadius - margin;

        // Above
        if (ox < r.getX() || ox + w > r.getRight())
        {
            ox = (int) bandX - w / 2;
            oy = (int) bandY - h - bandRadius - margin;
        }

        // Below
        if (oy < r.getY())
            oy = (int) bandY + bandRadius + margin;
    }

    ox = jlimit (r.getX() + 10, r.getRight() - w - 10, ox);
    oy = jlimit (r.getY() + 10, r.getBottom() - h - 10, oy);

    overlay.setBounds (ox, oy, w, h);
    overlay.setVisible (true);
}

void DecayRateEQ::positionBadgeFor (int idx)
{
    if (idx < 0 || idx >= (int) points.size())
        return;

    const auto& pt = points[(size_t) idx];
    const float x = mapHzToX (pt.hz);
    const float y = mapMultToY (pt.mult);

    const int w = 120, h = 60;
    auto pane = analyzer.getBounds();

    const int bandRadius = 12;
    const int margin     = 15;

    int ox = (int) x + bandRadius + margin;
    int oy = (int) y - h / 2;

    if (ox + w > pane.getRight())               ox = (int) x - w - bandRadius - margin;
    if (ox < pane.getX())                       { ox = (int) x - w / 2; oy = (int) y - h - bandRadius - margin; }
    if (oy < pane.getY())                       oy = (int) y + bandRadius + margin;

    ox = jlimit (pane.getX() + 5, pane.getRight() - w - 5, ox);
    oy = jlimit (pane.getY() + 5, pane.getBottom() - h - 5, oy);

    badge.setBounds (ox, oy, w, h);
    badge.setVisible (true);
    badge.setValues (pt.mult, pt.hz, pt.type, false);
    badge.setDetails (pt.q, pt.mult, false, false, 0.0f, false, "St", 0, "Pre");
    badge.toFront (true);
}
