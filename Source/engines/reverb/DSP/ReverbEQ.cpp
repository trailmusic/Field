#include "ReverbEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

using namespace juce;

// ─────────────────────────────────────────────────────────────────────────────
// ReverbToneEQ
// ─────────────────────────────────────────────────────────────────────────────

ReverbToneEQ::ReverbToneEQ (MyPluginAudioProcessor& p)
    : proc (p)
{
    setOpaque (true);

    // Analyzer setup
    zoomState.prepare (60.0); // ±dB half-range
    addAndMakeVisible (analyzer);
    analyzer.setInterceptsMouseClicks (false, false);
    analyzer.setAutoHeadroomEnabled   (true);
    analyzer.setHeadroomTargetFill    (0.70f);
    SpectrumAnalyzer::Params prm; prm.fps = 30;
    analyzer.setParams (prm);
    analyzer.setDrawGridHorizontal (false); // we draw our own dB grid

    // Overlay + badge (initially hidden)
    addAndMakeVisible (overlay); overlay.setVisible (false);
    addAndMakeVisible (badge);   badge.setVisible   (false);

    // Overlay callbacks
    overlay.onGainChanged = [this] (float g)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& bp = points[(size_t) selected];
            bp.db = jlimit (kMinGainDb, kMaxGainDb, g);
            if (bp.bandIdx >= 0)
                setBandParam (bp.bandIdx, "tb_gainDb", bp.db);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onQChanged = [this] (float qVal)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& bp = points[(size_t) selected];
            bp.q = jlimit (kMinQ, kMaxQ, qVal);
            if (bp.bandIdx >= 0)
                setBandParam (bp.bandIdx, "tb_q", bp.q);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onFreqChanged = [this] (float f)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& bp = points[(size_t) selected];
            bp.hz = jlimit (kMinHz, kMaxHz, f);
            if (bp.bandIdx >= 0)
                setBandParam (bp.bandIdx, "tb_freqHz", bp.hz);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onTypeChanged = [this] (int t)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& bp = points[(size_t) selected];
            bp.type = jlimit (0, 2, t);
            if (bp.bandIdx >= 0)
                setBandParam (bp.bandIdx, "tb_type", (float) bp.type);
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
                setBandParam (bandIdx, "tb_active", 0.0f);

            points.erase (points.begin() + selected);
            selected = -1;
            rebuildEqPath(); repaint();
            overlay.setVisible (false);
            badge.setVisible   (false);
        }
    };

    badge.onBypass = [this] (bool off)
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            const int bandIdx = points[(size_t) selected].bandIdx;
            if (bandIdx >= 0)
                setBandParam (bandIdx, "tb_active", off ? 0.0f : 1.0f);
        }
    };

    badge.onSetType = [this] (int tp)
    {
        const int idx = (badgeFor >= 0 ? badgeFor : selected);
        if (idx >= 0 && idx < (int) points.size())
        {
            auto& bp = points[(size_t) idx];
            bp.type = jlimit (0, 2, tp);
            if (bp.bandIdx >= 0)
                setBandParam (bp.bandIdx, "tb_type", (float) bp.type);
            rebuildEqPath(); repaint(); positionBadgeFor (idx);
        }
    };

    // Start the UI timer only when the component is visible
    if (isShowing()) startTimerHz (30);
}

ReverbToneEQ::~ReverbToneEQ()
{
    stopTimer();
    if (auto* old = listeningTo) old->removeChangeListener (this);
}

// ─────────────────────────────────────────────────────────────────────────────
// Lifecycle
// ─────────────────────────────────────────────────────────────────────────────

void ReverbToneEQ::visibilityChanged()
{
    if (isVisible()) startTimerHz (30);
    else             stopTimer();
}

void ReverbToneEQ::lookAndFeelChanged()
{
    parentHierarchyChanged();
    repaint();
}

void ReverbToneEQ::parentHierarchyChanged()
{
    // Re-wire listener whenever LNF or parent changes
    if (auto* old = listeningTo) old->removeChangeListener (this);
    listeningTo = dynamic_cast<FieldLNF*> (&getLookAndFeel());
    if (listeningTo) listeningTo->addChangeListener (this);
}

void ReverbToneEQ::changeListenerCallback (ChangeBroadcaster* src)
{
    if (src == dynamic_cast<FieldLNF*> (&getLookAndFeel()))
        repaint();
}

void ReverbToneEQ::timerCallback()
{
    // Drive hover HUD / ghost and spectrum redraws at ~30Hz
    if (isShowing()) repaint();
}

// ─────────────────────────────────────────────────────────────────────────────
// Interaction
// ─────────────────────────────────────────────────────────────────────────────

void ReverbToneEQ::mouseDown (const MouseEvent& e)
{
    const int h = hitTestPoint (e.getPosition());
    if (h >= 0)
    {
        // Toggle overlay if clicking the already selected point
        if (h == selected && overlay.isVisible())
        {
            overlay.setVisible (false);
            badge.setVisible   (false);
            selected = -1;
        }
        else
        {
            selected = h;
            auto& pt = points[(size_t) selected];
            overlay.setValues (pt.db, pt.q, pt.hz, pt.type);
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
        
    // Add new band (max 4)
    if (points.size() >= (size_t) kMaxBands)
        return;

        BandPoint bp; 
    bp.hz = jlimit (kMinHz, kMaxHz, mapXToHz (e.getPosition().x));
    bp.db = jlimit (kMinGainDb, kMaxGainDb, mapYToDb (e.getPosition().y));

    // Predict shelves in extremes
    if      (bp.hz <= 50.0f)    { bp.type = 1; bp.db = -12.0f; }   // LowShelf
    else if (bp.hz >= 10000.0f) { bp.type = 2; bp.db = -12.0f; }   // HighShelf
    else                        { bp.type = 0; }                   // Bell
        
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
        setBandParam (slot, "tb_active", 1.0f);
        setBandParam (slot, "tb_freqHz", bp.hz);
        setBandParam (slot, "tb_gainDb", bp.db);
        setBandParam (slot, "tb_q",      bp.q);
        setBandParam (slot, "tb_type",   (float) bp.type);
        setBandParam (slot, "tb_phase",  (float) bp.phase);
    }

    points.push_back (bp);
    selected = (int) points.size() - 1;

    overlay.setValues (bp.db, bp.q, bp.hz, bp.type);
    overlay.setVisible (true);
        positionOverlay();
    positionBadgeFor (selected);
        rebuildEqPath();
        repaint();
}

void ReverbToneEQ::mouseDrag (const MouseEvent& e)
{
    if (selected >= 0 && selected < (int) points.size())
    {
        auto& pt = points[(size_t) selected];
        pt.hz = jlimit (kMinHz, kMaxHz, mapXToHz (e.getPosition().x));
        pt.db = jlimit (kMinGainDb, kMaxGainDb, mapYToDb (e.getPosition().y));

        if (pt.bandIdx >= 0)
        {
            setBandParam (pt.bandIdx, "tb_freqHz", pt.hz);
            setBandParam (pt.bandIdx, "tb_gainDb", pt.db);
        }

        rebuildEqPath();
        repaint();
    }
}

void ReverbToneEQ::mouseUp (const MouseEvent&)
{
    if (selected >= 0 && selected < (int) points.size())
    {
        const auto& pt = points[(size_t) selected];
        overlay.setValues (pt.db, pt.q, pt.hz, pt.type);
        positionOverlay();
        positionBadgeFor (selected);
    }
}

void ReverbToneEQ::mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel)
{
    if (selected >= 0 && selected < (int) points.size())
    {
        auto& pt = points[(size_t) selected];
        pt.q = jlimit (kMinQ, kMaxQ, pt.q + wheel.deltaY * 0.1f);

        if (pt.bandIdx >= 0)
            setBandParam (pt.bandIdx, "tb_q", pt.q);

        rebuildEqPath();
        overlay.setValues (pt.db, pt.q, pt.hz, pt.type);
        positionOverlay();
        positionBadgeFor (selected);
        repaint();
    }
}

void ReverbToneEQ::mouseDoubleClick (const MouseEvent& e)
{
    const int idx = hitTestPoint (e.getPosition());
    if (idx >= 0 && idx < (int) points.size())
    {
        const int bandIdx = points[(size_t) idx].bandIdx;
        if (bandIdx >= 0)
            setBandParam (bandIdx, "tb_active", 0.0f);

        points.erase (points.begin() + idx);

        if      (selected == idx) selected = -1;
        else if (selected  > idx) --selected;

        rebuildEqPath();
        repaint();

        if (selected < 0) 
        {
            overlay.setVisible (false);
            badge.setVisible   (false);
            return;
        }

        const auto& pt2 = points[(size_t) selected];
        overlay.setValues (pt2.db, pt2.q, pt2.hz, pt2.type);
        overlay.setVisible (true);
            positionOverlay();
        positionBadgeFor (selected);
    }
}

void ReverbToneEQ::mouseMove (const MouseEvent& e)
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
    auto r     = analyzer.getBounds();
    hoverInPane= r.contains (hoverPos);
    if (hoverInPane)
        hoverHz = jlimit (kMinHz, kMaxHz, mapXToHz (hoverPos.x));

    lastMouseMoveMs = (int64) Time::getMillisecondCounterHiRes();
    repaint();
}

void ReverbToneEQ::mouseExit (const MouseEvent&)
{
    if (selected < 0)
        badge.setVisible (false);

    hover       = -1;
    hoverInPane = false;
        repaint();
}

// ─────────────────────────────────────────────────────────────────────────────
// Painting
// ─────────────────────────────────────────────────────────────────────────────

void ReverbToneEQ::paint (Graphics& g)
{
    auto r  = getLocalBounds().toFloat();
    auto& lf= getLookAndFeel();

    // Theme colours
    const auto border      = lf.findColour (FieldLNF::eqBorderColourId);
    const auto panel       = lf.findColour (ResizableWindow::backgroundColourId);
    const auto accent      = lf.findColour (FieldLNF::eqLabelTextColourId);
    const auto zeroLine    = lf.findColour (FieldLNF::eqZeroLineColourId);
    const auto gridLine    = lf.findColour (FieldLNF::eqGridLineColourId);
    const auto accentTrace = lf.findColour (FieldLNF::eqAnalyzerTraceColourId);

    const float cr = 8.0f;
    
    // Panel with rounded border
    g.setColour (panel);            g.fillRect (r);
    g.setColour (panel);            g.fillRoundedRectangle (r, cr);
    g.setColour (border);           g.drawRoundedRectangle (r.reduced (1.0f), cr - 1.0f, 1.5f);

    // Units/grid
    drawUnits (g);

    // Curves + points
    const auto rA = analyzer.getBounds().toFloat();
    if (rA.isEmpty())
        return;

    // Combined curve
    g.setColour (accent.withAlpha (0.95f));
    g.strokePath (eqPath, PathStrokeType (3.0f));
        
        // Per-band curves
        for (size_t i = 0; i < bandPaths.size(); ++i)
        {
        const Colour base = bandColourFor ((int) i);
        g.setColour (base.withAlpha (0.90f));
        g.strokePath (bandPaths[i], PathStrokeType (1.2f));
    }

    // Band handles
    g.setColour (accent.withAlpha (0.95f));
        for (const auto& pt : points)
        {
        const float x = mapHzToX (pt.hz);
        const float y = mapDbToY (pt.db);
        g.fillEllipse (x - 8.0f, y - 8.0f, 16.0f, 16.0f);
    }

    // Selected highlight
    if (selected >= 0 && selected < (int) points.size())
    {
        const auto& pt = points[(size_t) selected];
        const float x  = mapHzToX (pt.hz);
        const float y  = mapDbToY (pt.db);
        g.setColour (border.withAlpha (0.6f));
        g.drawEllipse (x - 12.0f, y - 12.0f, 24.0f, 24.0f, 1.6f);
    }

    // Hover guide + predictive ghost
        if (hoverInPane)
        {
        const int64 nowMs    = (int64) Time::getMillisecondCounterHiRes();
        const bool  ghostOn  = (nowMs - lastMouseMoveMs) >= (int64) ghostDelayMs;

        // Center guide with soft fades
            const float x = (float) hoverPos.x;
        {
            Graphics::ScopedSaveState ss (g);
            const float aMove  = jmap (float (jlimit<int64> (0, ghostDelayMs, nowMs - lastMouseMoveMs)),
                                       0.0f, float (ghostDelayMs), 0.26f, 0.18f);
            const float aGhost = 0.34f;
            const float alpha  = ghostOn ? aGhost : aMove;

            g.setColour (accent.withAlpha (alpha));
            g.drawLine (x, rA.getY(), x, rA.getBottom(), ghostOn ? 1.4f : 1.0f);

            g.setColour (accent.withAlpha ((ghostOn ? 0.12f : 0.06f)));
            g.drawLine (x - 12.0f, rA.getY(), x - 12.0f, rA.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.drawLine (x + 12.0f, rA.getY(), x + 12.0f, rA.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.setColour (accent.withAlpha ((ghostOn ? 0.072f : 0.036f)));
            g.drawLine (x - 24.0f, rA.getY(), x - 24.0f, rA.getBottom(), 0.8f);
            g.drawLine (x + 24.0f, rA.getY(), x + 24.0f, rA.getBottom(), 0.8f);
        }

        // Hz readouts (top/bottom)
        String hzText;
        if      (hoverHz >= 10000.0f) hzText = String ((int) std::round (hoverHz / 1000.0f)) + "k";
        else if (hoverHz >= 1000.0f)  hzText = String (hoverHz / 1000.0f, 1) + "k";
        else                          hzText = String ((int) hoverHz);

        const String lbl = hzText + " Hz";

        {
            auto tb = Rectangle<float> (x - 32.0f, rA.getBottom() - 20.0f, 64.0f, 14.0f);
            g.setColour (border.withAlpha (0.45f));
            g.fillRoundedRectangle (tb, 4.0f);
            g.setColour (accent.withAlpha (0.80f));
            g.drawFittedText (lbl, tb.toNearestInt(), Justification::centred, 1);
        }
        {
            auto tt = Rectangle<float> (x - 28.0f, rA.getY() + 6.0f, 56.0f, 14.0f);
            g.setColour (border.withAlpha (0.40f));
            g.fillRoundedRectangle (tt, 4.0f);
            g.setColour (accent.withAlpha (0.85f));
            g.drawFittedText (lbl, tt.toNearestInt(), Justification::centred, 1);
        }

        // Predictive ghost (shelving at extremes, bell otherwise)
        if (ghostOn)
            {
            // Avoid ghost when too close to an existing handle
                bool nearPoint = false;
                for (const auto& pt : points)
                {
                if (Point<float> (mapHzToX (pt.hz), mapDbToY (pt.db))
                        .getDistanceFrom (hoverPos.toFloat()) <= 24.0f)
                    { nearPoint = true; break; }
                }

                if (! nearPoint)
                {
                Path ghost;
                const bool mouseAbove0 = (mapYToDb (hoverPos.y) > 0.0f);

                auto addGhost = [&] (int type, float amtDb)
                {
                    BandPoint b; b.type = type; b.hz = hoverHz; b.db = amtDb; b.q = 0.9f;
                    const int N = jmax (64, (int) rA.getWidth());
                    for (int i = 0; i < N; ++i)
                    {
                        const double t    = (double) i / (double) (N - 1);
                        const double a    = std::log10 (kMinHz), bL = std::log10 (kMaxHz);
                        const double logF = jmap (t, 0.0, 1.0, a, bL);
                        const double hz   = std::pow (10.0, logF);
                        const float  xPos = rA.getX() + (float) i / (float) (N - 1) * rA.getWidth();
                        const float  yPos = mapDbToY (bandDbAtForPaint (b, (float) hz));
                        if (i == 0) ghost.startNewSubPath (xPos, yPos);
                        else        ghost.lineTo          (xPos, yPos);
                    }
                };

                if      (hoverHz <= 50.0f)    addGhost (1, mouseAbove0 ? +3.0f : -3.0f); // LS
                else if (hoverHz >= 10000.0f) addGhost (2, mouseAbove0 ? +3.0f : -3.0f); // HS
                else                          addGhost (0, mouseAbove0 ? +3.0f : -3.0f); // Bell

                // Reveal only near the cursor with a soft elliptical clip
                Graphics::ScopedSaveState ss (g);
                Path clip; clip.addEllipse (x - rA.getWidth() * 0.05f,
                                            (float) hoverPos.y - rA.getHeight() * 0.15f,
                                            rA.getWidth() * 0.10f,
                                            rA.getHeight()* 0.30f);
                g.reduceClipRegion (clip);
                g.setColour (accent.withAlpha (0.16f));
                g.strokePath (ghost, PathStrokeType (1.0f, PathStrokeType::curved, PathStrokeType::rounded));
            }
        }
    }
}

void ReverbToneEQ::resized()
{
    const auto r = getLocalBounds().reduced (6);
    analyzer.setBounds (r);
    rebuildEqPath();
}

// ─────────────────────────────────────────────────────────────────────────────
// Curves / grid
// ─────────────────────────────────────────────────────────────────────────────

void ReverbToneEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();

    const auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty())
        return;

    const int N = jmax (128, (int) r.getWidth());

    // Combined EQ curve
    auto totalDbAt = [this] (double hz)
    {
        float s = 0.0f;
        for (const auto& b : points) 
            s += bandDbAtForPaint (b, (float) hz);
        return s;
    };
    
    auto mapX = [&] (int i)
    {
        const double t    = (double) i / (double) (N - 1);
        const double a    = std::log10 (kMinHz), b = std::log10 (kMaxHz);
        const double logF = jmap (t, 0.0, 1.0, a, b);
        const double hz   = std::pow (10.0, logF);
        return std::pair<float, float> ((float) hz, mapDbToY (totalDbAt (hz)));
    };

    const auto p0 = mapX (0);
    eqPath.startNewSubPath (r.getX(), p0.second);

    for (int i = 1; i < N; ++i)
    {
        const auto p = mapX (i);
        const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
        eqPath.lineTo (x, p.second);
    }
    
    // Per-band paths
    bandPaths.resize (points.size());
    for (size_t bi = 0; bi < points.size(); ++bi)
    {
        auto& bp = bandPaths[bi];

        auto mapBand = [&] (int i)
        {
            const double t    = (double) i / (double) (N - 1);
            const double a    = std::log10 (kMinHz), bL = std::log10 (kMaxHz);
            const double logF = jmap (t, 0.0, 1.0, a, bL);
            const double hz   = std::pow (10.0, logF);
            return std::pair<float, float> ((float) hz, mapDbToY (bandDbAtForPaint (points[bi], (float) hz)));
        };

        const auto q0 = mapBand (0);
        bp.startNewSubPath (r.getX(), q0.second);

        for (int i = 1; i < N; ++i)
        {
            const auto q = mapBand (i);
            const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
            bp.lineTo (x, q.second);
        }
    }
}

void ReverbToneEQ::drawUnits (Graphics& g)
{
    const auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;
    
    auto& lf   = getLookAndFeel();
    auto grid  = lf.findColour (FieldLNF::eqGridLineColourId);
    auto text  = lf.findColour (FieldLNF::eqLabelTextColourId).withAlpha (0.45f);
    auto zero  = lf.findColour (FieldLNF::eqZeroLineColourId);

    g.setFont (12.0f);

    // dB ticks
    g.setColour (grid);
    const float halfRange = zoomState.getCurrent();
    const float dbVals[]  = { +18, +12, +6, 0, -6, -12, -18, -24, -30, -36 };

    for (float dbv : dbVals)
    {
        if (dbv > halfRange || dbv < -halfRange) continue;

        const float y = mapDbToY (dbv);

        if (dbv == 0.0f)
        {
            g.setColour (zero);
            g.drawLine  (r.getX(), y, r.getRight(), y, 1.2f);
        }
        else
        {
            g.setColour (grid);
            g.drawLine  (r.getX(), y, r.getRight(), y, 0.6f);
        }

        g.setColour (text);
        g.drawFittedText (String ((int) dbv) + " dB",
                          Rectangle<int> ((int) r.getX() + 4, (int) y - 8, 44, 16),
                          Justification::centredLeft, 1);
    }

    // Hz ticks
    g.setColour (grid);
    const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000,
                               5000, 7000, 8000, 10000, 20000 };

    for (double hz : hzTicks)
    {
        const float x = mapHzToX ((float) hz);
        g.drawLine (x, r.getBottom() - 16.0f, x, r.getBottom(), 0.8f);

        String lbl;
        if      (hz >= 10000.0) lbl = String ((int) std::round (hz / 1000.0)) + "k";
        else if (hz >= 1000.0)  lbl = String (hz / 1000.0, 1) + "k";
        else                    lbl = String ((int) hz);

        g.setColour (text);
        g.drawFittedText (lbl,
                          Rectangle<int> ((int) x - 18, (int) r.getBottom() - 30, 36, 14),
                          Justification::centred, 1);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Mapping & hit-testing
// ─────────────────────────────────────────────────────────────────────────────

float ReverbToneEQ::mapHzToX (float hz) const
{
    const auto r = analyzer.getBounds().toFloat();
    const float t = (float) (std::log10 (jlimit (kMinHz, kMaxHz, hz) / kMinHz)
                           / std::log10 (kMaxHz / kMinHz));
    return r.getX() + t * r.getWidth();
}

float ReverbToneEQ::mapDbToY (float dB) const
{
    const auto r = analyzer.getBounds().toFloat();
    const float top = r.getY() + 8.0f, bottom = r.getBottom() - 8.0f;
    const float halfRange = zoomState.getCurrent();
    return jmap (dB, +halfRange, -halfRange, top, bottom);
}

float ReverbToneEQ::mapXToHz (int px) const
{
    const auto r = analyzer.getBounds();
    const float t = jlimit (0.0f, 1.0f, (px - (float) r.getX()) / (float) r.getWidth());
    const float a = std::log10 (kMinHz), b = std::log10 (kMaxHz);
    return std::pow (10.0f, jmap (t, 0.0f, 1.0f, a, b));
}

float ReverbToneEQ::mapYToDb (int py) const
{
    const auto r = analyzer.getBounds();
    const float halfRange = zoomState.getCurrent();
    return jmap ((float) py, (float) r.getY(), (float) r.getBottom(), +halfRange, -halfRange);
}

int ReverbToneEQ::hitTestPoint (Point<int> p) const
{
    for (int i = (int) points.size() - 1; i >= 0; --i)
    {
        const float x = mapHzToX (points[(size_t) i].hz);
        const float y = mapDbToY (points[(size_t) i].db);
        if (Point<float> (x, y).getDistanceFrom (p.toFloat()) <= kHandleRadius)
            return i;
    }
    return -1;
}

// ─────────────────────────────────────────────────────────────────────────────
// Band management & paint helpers
// ─────────────────────────────────────────────────────────────────────────────

int ReverbToneEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        const auto id = bandId ("tb_active", i);
        if (auto* v = proc.apvts.getRawParameterValue (id))
            if (v->load() < 0.5f)
                return i;
    }
    return -1;
}

void ReverbToneEQ::setBandParam (int bandIdx, const char* baseId, float value)
{
    const auto id = bandId (baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (value));
}

float ReverbToneEQ::getBandParamFloat (int bandIdx, const char* baseId, float fallback) const
{
    const auto id = bandId (baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue (id))
        return v->load();
    return fallback;
}

Colour ReverbToneEQ::bandColourFor (int bandIdx) const
{
    auto& lf      = getLookAndFeel();
    const auto ac = lf.findColour (FieldLNF::eqLabelTextColourId);
    const float baseHue = ac.getHue();
    const float baseSat = jlimit (0.25f, 0.95f, ac.getSaturation());
    const float baseBrt = jlimit (0.35f, 0.95f, ac.getBrightness());
    const float golden  = 0.61803398875f;

    float hue = std::fmod (baseHue + golden * (float) (bandIdx + 1), 1.0f);
    hue       = jlimit (0.0f, 1.0f, 0.65f * hue + 0.35f * baseHue);
    float sat = jlimit (0.30f, 0.95f, baseSat * 0.9f + 0.1f);
    float brt = jlimit (0.40f, 0.95f, baseBrt * 0.9f + 0.1f);
    return Colour::fromHSV (hue, sat, brt, 1.0f);
}

float ReverbToneEQ::bandDbAtForPaint (const BandPoint& b, float hz) const
{
    const double logHz = std::log10 (jlimit (double (kMinHz), double (kMaxHz), (double) hz));
    const double logC  = std::log10 (jlimit (double (kMinHz), double (kMaxHz), (double) b.hz));
    const double q     = jlimit (0.1, 36.0, (double) b.q);
    const double width = jlimit (0.02, 0.50, 0.22 / q);
    const double d     = (logHz - logC) / width;
    
    switch (b.type)
    {
        case 0: // Bell
        {
            const float w = (float) std::exp (-0.5 * d * d);
            return b.db * w;
        }
        case 1: // Low Shelf
        {
            const double k = 8.0 * jlimit (0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
            return (float) (b.db * s);
        }
        case 2: // High Shelf
        {
            const double k = 8.0 * jlimit (0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
            return (float) (b.db * (1.0 - s));
        }
        default: return 0.0f;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Overlay
// ─────────────────────────────────────────────────────────────────────────────

ReverbToneEQ::BandOverlay::BandOverlay()
{
    setInterceptsMouseClicks (true, true);

    // Sliders
    gain.setSliderStyle (Slider::LinearHorizontal);
    gain.setTextBoxStyle (Slider::TextBoxRight, false, 56, 18);
    gain.setRange (kMinGainDb, kMaxGainDb, 0.1);
    gain.onValueChange = [this] { if (! updating && onGainChanged) onGainChanged ((float) gain.getValue()); };
    gain.onDragStart   = [this] { if (onDragAny) onDragAny (true);  };
    gain.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (gain);

    q.setSliderStyle (Slider::LinearHorizontal);
    q.setTextBoxStyle (Slider::TextBoxRight, false, 56, 18);
    q.setRange (kMinQ, kMaxQ, 0.01);
    q.onValueChange = [this] { if (! updating && onQChanged) onQChanged ((float) q.getValue()); };
    q.onDragStart   = [this] { if (onDragAny) onDragAny (true);  };
    q.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (q);

    freq.setSliderStyle (Slider::LinearHorizontal);
    freq.setTextBoxStyle (Slider::TextBoxRight, false, 72, 18);
    freq.setRange (kMinHz, kMaxHz, 0.01);
    freq.setSkewFactorFromMidPoint (1000.0);
    freq.onValueChange = [this] { if (! updating && onFreqChanged) onFreqChanged ((float) freq.getValue()); };
    freq.onDragStart   = [this] { if (onDragAny) onDragAny (true);  };
    freq.onDragEnd     = [this] { if (onDragAny) onDragAny (false); };
    addAndMakeVisible (freq);

    // Labels
    gainLabel.setText ("GAIN", dontSendNotification);
    gainLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (gainLabel);

    qLabel.setText ("Q", dontSendNotification);
    qLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (qLabel);

    freqLabel.setText ("FREQ", dontSendNotification);
    freqLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (freqLabel);

    // Type combo
    typeCb.addItemList (StringArray { "Bell", "LowShelf", "HighShelf" }, 1);
    typeCb.onChange = [this] { if (! updating && onTypeChanged) onTypeChanged (typeCb.getSelectedItemIndex()); };
    addAndMakeVisible (typeCb);

    typeLabel.setText ("TYPE", dontSendNotification);
    typeLabel.setJustificationType (Justification::centredLeft);
    addAndMakeVisible (typeLabel);
}

void ReverbToneEQ::BandOverlay::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto& lf = getLookAndFeel();
    const auto accent = lf.findColour (FieldLNF::eqLabelTextColourId);
    const auto panel  = lf.findColour (ResizableWindow::backgroundColourId);

    // Background + accent border
    g.setColour (panel.darker (0.20f).withAlpha (0.96f)); g.fillRoundedRectangle (r, 8.0f);
    g.setColour (accent.withAlpha (0.8f));                 g.drawRoundedRectangle (r, 8.0f, 2.0f);

    // Accent strip
    auto strip = r.removeFromLeft (3.0f).reduced (0.5f, 2.0f);
    g.setColour (accent); g.fillRoundedRectangle (strip, 1.5f);
}

void ReverbToneEQ::BandOverlay::resized()
{
    auto r = getLocalBounds().reduced (8);
    constexpr int labelW = 44, sliderH = 20, gap = 4;

    // Gain
    gainLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    gain.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    // Q
    qLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    q.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    // Freq
    freqLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    freq.setBounds      (r.removeFromTop (sliderH));
    r.removeFromTop (gap);

    // Type
    typeLabel.setBounds (r.removeFromTop (sliderH).removeFromLeft (labelW));
    typeCb.setBounds    (r.removeFromTop (sliderH));
}

void ReverbToneEQ::BandOverlay::setValues (float gainVal, float qVal, float freqVal, int type)
{
    updating = true;
    gain.setValue (gainVal, dontSendNotification);
    q.setValue    (qVal,    dontSendNotification);
    freq.setValue (freqVal, dontSendNotification);
    typeCb.setSelectedItemIndex (jlimit (0, 2, type), dontSendNotification);
    updating = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Badge
// ─────────────────────────────────────────────────────────────────────────────

ReverbToneEQ::BandBadge::BandBadge()
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

    gainLabel.setText ("0.0", dontSendNotification);
    gainLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (gainLabel);

    qLabel.setText ("0.7", dontSendNotification);
    qLabel.setJustificationType (Justification::centred);
    addAndMakeVisible (qLabel);
}

void ReverbToneEQ::BandBadge::paint (Graphics& g)
{
    auto r   = getLocalBounds().toFloat();
    auto& lf = getLookAndFeel();
    const auto accent = lf.findColour (FieldLNF::eqLabelTextColourId);
    const auto panel  = lf.findColour (ResizableWindow::backgroundColourId);

    g.setColour (panel.darker (0.30f).withAlpha (0.95f));
    g.fillRoundedRectangle (r, 6.0f);

    g.setColour (accent.withAlpha (0.9f));
    g.drawRoundedRectangle (r, 6.0f, 1.5f);

    auto strip = r.removeFromLeft (2.0f).reduced (0.5f, 1.0f);
    g.setColour (accent);
    g.fillRoundedRectangle (strip, 1.0f);
}

void ReverbToneEQ::BandBadge::resized()
{
    auto r = getLocalBounds().reduced (4);
    constexpr int btnW = 24, btnH = 16;

    // Top row
    deleteBtn.setBounds (r.removeFromTop (btnH).removeFromLeft (btnW));
    bypassBtn.setBounds (r.removeFromTop (btnH).removeFromLeft (btnW));
    typeBtn.setBounds   (r.removeFromTop (btnH).removeFromLeft (btnW));

    // Bottom row
    freqLabel.setBounds (r.removeFromTop (btnH));
    gainLabel.setBounds (r.removeFromTop (btnH));
    qLabel.setBounds    (r.removeFromTop (btnH));
}

void ReverbToneEQ::BandBadge::setValues (float gr, float freq, int type, bool bypass)
{
    currentGr     = gr;
    currentFreq   = freq;
    currentType   = type;
    currentBypass = bypass;

    if      (freq >= 10000.0f) freqLabel.setText (String ((int) std::round (freq / 1000.0f)) + "k", dontSendNotification);
    else if (freq >= 1000.0f)  freqLabel.setText (String (freq / 1000.0f, 1) + "k",                dontSendNotification);
    else                       freqLabel.setText (String ((int) freq),                             dontSendNotification);

    bypassBtn.setToggleState (bypass, dontSendNotification);

    static const char* typeNames[] = { "Bell", "LS", "HS" };
    typeBtn.setButtonText (typeNames [jlimit (0, 2, type)]);
}

void ReverbToneEQ::BandBadge::setDetails (float q, float gain, bool /*dynOn*/, bool /*dynUp*/, float /*dynRange*/,
                                          bool /*specOn*/, const String& /*channel*/, int /*slopeDb*/, const String& /*tap*/)
{
    currentQ    = q;
    currentGain = gain;

    gainLabel.setText (String (gain, 1), dontSendNotification);
    qLabel.setText    (String (q, 2),    dontSendNotification);
}

// ─────────────────────────────────────────────────────────────────────────────
// Positioning helpers
// ─────────────────────────────────────────────────────────────────────────────

void ReverbToneEQ::positionOverlay()
{
    if (selected < 0 || selected >= (int) points.size())
        return;

    const auto r  = getLocalBounds();
    constexpr int w = 200, h = 120;

    const auto& pt = points[(size_t) selected];
    const float bandX = mapHzToX (pt.hz);
    const float bandY = mapDbToY (pt.db);

    // Start centered on the handle
    int ox = (int) bandX - w / 2;
    int oy = (int) bandY - h / 2;

    // Avoid handle overlap (radius + margin)
    constexpr int bandRadius = 12;
    constexpr int margin     = 20;

    auto overlaps = [&] { return (ox <= bandX + bandRadius + margin && ox + w >= bandX - bandRadius - margin
                               && oy <= bandY + bandRadius + margin && oy + h >= bandY - bandRadius - margin); };

    if (overlaps())
    {
        // Try right
        ox = (int) bandX + bandRadius + margin;
        oy = (int) bandY - h / 2;

        // Then left
        if (ox + w > r.getRight())
            ox = (int) bandX - w - bandRadius - margin;

        // Then above
        if (ox < r.getX() || ox + w > r.getRight())
        {
            ox = (int) bandX - w / 2;
            oy = (int) bandY - h - bandRadius - margin;
        }

        // Then below
        if (oy < r.getY())
            oy = (int) bandY + bandRadius + margin;
    }

    // Bounds clamp
    ox = jlimit (r.getX() + 10, r.getRight() - w - 10, ox);
    oy = jlimit (r.getY() + 10, r.getBottom() - h - 10, oy);

    overlay.setBounds (ox, oy, w, h);
    overlay.setVisible (true);
}

void ReverbToneEQ::positionBadgeFor (int idx)
{
    if (idx < 0 || idx >= (int) points.size()) return;

    const auto& pt = points[(size_t) idx];
    const float x  = mapHzToX (pt.hz);
    const float y  = mapDbToY (pt.db);

    constexpr int w = 120, h = 60;
    const auto pane = analyzer.getBounds();

    constexpr int bandRadius = 12;
    constexpr int margin     = 15;

    int ox = (int) x + bandRadius + margin;
    int oy = (int) y - h / 2;

    if (ox + w > pane.getRight()) ox = (int) x - w - bandRadius - margin;
    if (ox < pane.getX())         { ox = (int) x - w / 2; oy = (int) y - h - bandRadius - margin; }
    if (oy < pane.getY())         oy = (int) y + bandRadius + margin;

    // Clamp to pane bounds with small padding
    ox = jlimit (pane.getX() + 5, pane.getRight() - w - 5, ox);
    oy = jlimit (pane.getY() + 5, pane.getBottom() - h - 5, oy);

    badge.setBounds (ox, oy, w, h);
    badge.setVisible (true);
    badge.setValues (0.0f, pt.hz, pt.type, false);
    badge.setDetails (pt.q, pt.db, false, false, 0.0f, false, "St", 0, "Pre");

    // Per-band accent
    const Colour ac = bandColourFor (idx);
    badge.toFront (true);
}
