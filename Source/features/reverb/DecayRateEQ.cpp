#include "DecayRateEQ.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "features/dynEq/FilterFactory.h"
#include "shared/Core/PluginProcessor.h"

DecayRateEQ::DecayRateEQ(MyPluginAudioProcessor& p)
    : proc(p)
{
    setOpaque(true);
    startTimerHz(30);
    
    // Initialize zoom state for decay multiplier range
    zoomState.prepare(1.5f); // 0.5x to 2.0x range
    
    addAndMakeVisible(analyzer);
    analyzer.setInterceptsMouseClicks(false, false);
    analyzer.setAutoHeadroomEnabled(true);
    analyzer.setHeadroomTargetFill(0.70f);
    SpectrumAnalyzer::Params prm; 
    prm.fps = 30; 
    analyzer.setParams(prm);
    analyzer.setDrawGridHorizontal(false); // we'll draw our own multiplier units
    
    // Add per-band modules
    addAndMakeVisible(overlay);
    overlay.setVisible(false);
    addAndMakeVisible(badge);
    badge.setVisible(false);
    
    // Setup overlay callbacks
    overlay.onMultChanged = [this](float m) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].mult = juce::jlimit(0.5f, 2.0f, m);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::DecayBand::decayMult, m);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onQChanged = [this](float q) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].q = juce::jlimit(0.1f, 36.0f, q);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::DecayBand::q, q);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onFreqChanged = [this](float f) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].hz = juce::jlimit(20.f, 20000.f, f);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::DecayBand::freqHz, f);
            rebuildEqPath(); repaint();
        }
    };
    
    overlay.onTypeChanged = [this](int t) {
        if (selected >= 0 && selected < (int)points.size()) {
            points[(size_t)selected].type = juce::jlimit(0, 2, t);
            if (points[(size_t)selected].bandIdx >= 0)
                setBandParam(points[(size_t)selected].bandIdx, ReverbEQParams::DecayBand::dynAmt, (float)t);
            rebuildEqPath(); repaint();
        }
    };
    
    // Setup badge callbacks
    badge.onDelete = [this] {
        if (selected >= 0 && selected < (int)points.size()) {
            const int bandIdx = points[(size_t)selected].bandIdx;
            if (bandIdx >= 0) setBandParam(bandIdx, ReverbEQParams::DecayBand::active, 0.0f);
            points.erase(points.begin() + selected);
            selected = -1; rebuildEqPath(); repaint(); overlay.setVisible(false); badge.setVisible(false);
        }
    };
    
    badge.onBypass = [this](bool off) {
        if (selected >= 0 && selected < (int)points.size()) {
            const int bandIdx = points[(size_t)selected].bandIdx;
            if (bandIdx >= 0) setBandParam(bandIdx, ReverbEQParams::DecayBand::active, off ? 0.0f : 1.0f);
        }
    };
    
    badge.onSetType = [this](int tp) {
        const int idx = (badgeFor >= 0 ? badgeFor : selected);
        if (idx >= 0 && idx < (int)points.size()) {
            auto& p = points[(size_t)idx];
            p.type = juce::jlimit(0, 2, tp);
            if (p.bandIdx >= 0) setBandParam(p.bandIdx, ReverbEQParams::DecayBand::dynAmt, (float)p.type);
            rebuildEqPath(); repaint(); positionBadgeFor(idx);
        }
    };
}

DecayRateEQ::~DecayRateEQ()
{
    stopTimer();
}

void DecayRateEQ::timerCallback()
{
    // Drive delayed ghost repaint and hover HUD updates at 30Hz
    repaint();
}

void DecayRateEQ::lookAndFeelChanged()
{
    // LNF object changed → reattach listener and repaint
    parentHierarchyChanged();
    repaint();
}

void DecayRateEQ::parentHierarchyChanged()
{
    // Re-wire listener whenever LNF or parent changes
    if (auto* old = listeningTo) old->removeChangeListener(this);
    listeningTo = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    if (listeningTo) {
        listeningTo->addChangeListener(this);
        juce::Logger::writeToLog("DecayRateEQ: Connected to FieldLNF ChangeBroadcaster");
    } else {
        juce::Logger::writeToLog("DecayRateEQ: Failed to connect to FieldLNF ChangeBroadcaster");
    }
}

void DecayRateEQ::changeListenerCallback(juce::ChangeBroadcaster* src)
{
    // If the active LNF is our FieldLNF, repaint on its change signals
    if (src == dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        juce::Logger::writeToLog("DecayRateEQ: ChangeListener callback triggered!");
        repaint();
    }
}

void DecayRateEQ::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    auto& lf = getLookAndFeel();
    
    // Get theme colors from LNF
    auto border = lf.findColour(FieldLNF::eqBorderColourId);
    auto panel = lf.findColour(juce::ResizableWindow::backgroundColourId);
    auto accent = lf.findColour(FieldLNF::eqLabelTextColourId);
    auto zeroLine = lf.findColour(FieldLNF::eqZeroLineColourId);
    auto gridLine = lf.findColour(FieldLNF::eqGridLineColourId);
    auto handle = lf.findColour(FieldLNF::eqBandHandleColourId);
    auto handleActive = lf.findColour(FieldLNF::eqBandHandleActiveId);
    auto trace = lf.findColour(FieldLNF::eqAnalyzerTraceColourId);
    
    // Anti-aliasing fix: Fill entire area first, then rounded rectangle
    const float cr = 8.0f;
    
    // Fill entire rectangular area to prevent white corners
    g.setColour(panel);
    g.fillRect(r);
    
    // Then draw rounded rectangle on top
    g.fillRoundedRectangle(r, cr);
    
    // Border for definition
    g.setColour(border);
    g.drawRoundedRectangle(r.reduced(1.0f), cr - 1.0f, 1.5f);
    
    // Draw units and grid
    drawUnits(g);
    
    // Draw EQ curves
    auto rA = analyzer.getBounds().toFloat();
    if (!rA.isEmpty())
    {
        // Combined EQ curve (macro) slightly more prominent
        g.setColour(accent.withAlpha(0.95f));
        g.strokePath(eqPath, juce::PathStrokeType(3.0f));
        
        // Per-band curves
        for (size_t i = 0; i < bandPaths.size(); ++i)
        {
            juce::Colour base = bandColourFor((int)i);
            g.setColour(base.withAlpha(0.90f));
            g.strokePath(bandPaths[i], juce::PathStrokeType(1.2f));
        }
        
        // Draw band points
        g.setColour(accent.withAlpha(0.95f));
        for (const auto& pt : points)
        {
            const float x = mapHzToX(pt.hz);
            const float y = mapMultToY(pt.mult);
            g.fillEllipse(x-8, y-8, 16, 16);
        }
        
        // Selected point highlight
        if (selected >= 0 && selected < (int)points.size())
        {
            const auto& pt = points[(size_t)selected];
            const float x = mapHzToX(pt.hz);
            const float y = mapMultToY(pt.mult);
            g.setColour(border.withAlpha(0.6f));
            g.drawEllipse(x-12, y-12, 24, 24, 1.6f);
        }
        
        // Hover readout and predictive ghost
        if (hoverInPane)
        {
            // Vertical guide lines that track the cursor (soft when moving, stronger when ghost reveals)
            const juce::int64 nowMs = (juce::int64) juce::Time::getMillisecondCounterHiRes();
            const bool ghostOn = (nowMs - lastMouseMoveMs) >= (juce::int64) ghostDelayMs;
            auto rGuide = rA;
            // Smooth fade for center line based on time since last move
            const float tSince = (float) juce::jlimit<juce::int64> (0, ghostDelayMs, nowMs - lastMouseMoveMs);
            const float aMove = juce::jmap (tSince, 0.0f, (float) ghostDelayMs, 0.26f, 0.18f); // while moving
            const float aGhost= 0.34f; // when ghost is on
            float alpha = ghostOn ? aGhost : aMove;
            float alphaFade = ghostOn ? 0.12f : 0.06f;
            g.setColour (accent.withAlpha (alpha));
            const float x = (float) hoverPos.x;
            // Main center line
            g.drawLine (x, rGuide.getY(), x, rGuide.getBottom(), ghostOn ? 1.4f : 1.0f);
            // Side fades
            g.setColour (accent.withAlpha (alphaFade));
            g.drawLine (x-12.0f, rGuide.getY(), x-12.0f, rGuide.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.drawLine (x+12.0f, rGuide.getY(), x+12.0f, rGuide.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.setColour (accent.withAlpha (alphaFade * 0.6f));
            g.drawLine (x-24.0f, rGuide.getY(), x-24.0f, rGuide.getBottom(), 0.8f);
            g.drawLine (x+24.0f, rGuide.getY(), x+24.0f, rGuide.getBottom(), 0.8f);

            // Hz readout near bottom and top (follow cursor)
            g.setColour (accent.withAlpha (0.60f));
            juce::String hzText;
            if (hoverHz >= 1000.0f && hoverHz < 10000.0f) hzText = juce::String (hoverHz / 1000.0f, 1) + "k";
            else if (hoverHz >= 10000.0f) hzText = juce::String ((int) std::round (hoverHz/1000.0f)) + "k";
            else hzText = juce::String ((int) hoverHz);
            juce::String lbl = hzText + " Hz";
            auto tb = juce::Rectangle<float> ((float) hoverPos.x - 32.0f, rA.getBottom() - 20.0f, 64.0f, 14.0f);
            g.setColour (border.withAlpha (0.45f));
            g.fillRoundedRectangle (tb, 4.0f);
            g.setColour (accent.withAlpha (0.80f));
            g.drawFittedText (lbl, tb.toNearestInt(), juce::Justification::centred, 1);
            // Top badge
            auto tt = juce::Rectangle<float> ((float) hoverPos.x - 28.0f, rA.getY() + 6.0f, 56.0f, 14.0f);
            g.setColour (border.withAlpha (0.40f));
            g.fillRoundedRectangle (tt, 4.0f);
            g.setColour (accent.withAlpha (0.85f));
            g.drawFittedText (lbl, tt.toNearestInt(), juce::Justification::centred, 1);

            // Predictive ghost: show faint TiltLo/TiltHi in low/high zones, else Bell
            const bool showGhost = ghostOn;
            if (showGhost)
            {
                // Suppress ghost if near an existing point (avoid conflicts)
                const float suppressRadiusPx = 24.0f;
                bool nearPoint = false;
                for (const auto& pt : points)
                {
                    if (juce::Point<float> (mapHzToX (pt.hz), mapMultToY (pt.mult)).getDistanceFrom (hoverPos.toFloat()) <= suppressRadiusPx)
                    { nearPoint = true; break; }
                }
                if (! nearPoint)
                {
                // Build full ghost
                juce::Path ghost;
                const bool mouseAbove1 = mapYToMult (hoverPos.y) > 1.0f;
                auto makeGhost = [&](int type, float amtMult){ DecayBandPoint b; b.type = type; b.hz = hoverHz; b.mult = amtMult; b.q = 0.9f; const int N = juce::jmax (64, (int) rA.getWidth()); for (int i=0;i<N;++i){ const double minHz=20.0, maxHz=20000.0; const double t=(double)i/(double)(N-1); const double a=std::log10(minHz), bL=std::log10(maxHz); const double logF=juce::jmap(t,0.0,1.0,a,bL); const double hz=std::pow(10.0, logF); const float x=rA.getX() + (float) i/(float)(N-1)*rA.getWidth(); const float y=mapMultToY (bandMultAtForPaint (b, (float) hz)); if (i==0) ghost.startNewSubPath (x, y); else ghost.lineTo (x, y);} };
                // Predictive: tilts in low/high, bell elsewhere; sign by mouse Y
                if (hoverHz <= 50.0f)
                {
                    makeGhost (1 /*TiltLo*/, mouseAbove1 ? 1.3f : 0.7f);
                }
                else if (hoverHz >= 10000.0f)
                {
                    makeGhost (2 /*TiltHi*/, mouseAbove1 ? 1.3f : 0.7f);
                }
                else
                {
                    makeGhost (0 /*Bell*/, mouseAbove1 ? 1.3f : 0.7f);
                }
                // Radial fade around cursor to softly reveal only local part
                juce::Path clipped; clipped.addEllipse ((float) hoverPos.x - rA.getWidth()*0.05f, (float) hoverPos.y - rA.getHeight()*0.15f, rA.getWidth()*0.10f, rA.getHeight()*0.30f);
                juce::Graphics::ScopedSaveState ss (g);
                g.reduceClipRegion (clipped);
                g.setColour (lf.findColour(FieldLNF::eqLabelTextColourId).withAlpha (0.16f));
                g.strokePath (ghost, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
            }
        }
    }
}

void DecayRateEQ::resized()
{
    auto r = getLocalBounds().reduced(6);
    analyzer.setBounds(r);
    rebuildEqPath();
}

void DecayRateEQ::rebuildEqPath()
{
    eqPath.clear();
    bandPaths.clear();
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;

    const int N = juce::jmax(128, (int)r.getWidth());
    
    // Build combined EQ curve
    auto totalMultAt = [this](double hz)
    {
        float s = 1.0f; // Start with 1.0x (no change)
        for (const auto& b : points) 
        {
            s *= bandMultAtForPaint(b, (float)hz);
        }
        return s;
    };
    
    auto mapX = [&](int i)
    {
        const double minHz = 20.0, maxHz = 20000.0;
        const double t = (double)i / (double)(N - 1);
        const double a = std::log10(minHz), b = std::log10(maxHz);
        const double logF = juce::jmap(t, 0.0, 1.0, a, b);
        const double hz = std::pow(10.0, logF);
        return std::pair<float,float>((float)hz, mapMultToY(totalMultAt(hz)));
    };

    auto p0 = mapX(0); 
    eqPath.startNewSubPath(r.getX(), p0.second);
    for (int i = 1; i < N; ++i)
    {
        auto p = mapX(i);
        const float x = r.getX() + (float)i / (float)(N - 1) * r.getWidth();
        eqPath.lineTo(x, p.second);
    }
    
    // Per-band paths
    bandPaths.resize(points.size());
    for (size_t bi = 0; bi < points.size(); ++bi)
    {
        auto& bp = bandPaths[bi];
        auto mapBand = [&](int i){
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double)i / (double)(N - 1);
            const double a = std::log10(minHz), b = std::log10(maxHz);
            const double logF = juce::jmap(t, 0.0, 1.0, a, b);
            const double hz = std::pow(10.0, logF);
            return std::pair<float,float>((float)hz, mapMultToY(bandMultAtForPaint(points[bi], (float)hz)));
        };
        auto q0 = mapBand(0); 
        bp.startNewSubPath(r.getX(), q0.second);
        for (int i = 1; i < N; ++i)
        {
            auto q = mapBand(i);
            const float x = r.getX() + (float)i / (float)(N - 1) * r.getWidth();
            bp.lineTo(x, q.second);
        }
    }
}

void DecayRateEQ::drawUnits(juce::Graphics& g)
{
    auto r = analyzer.getBounds().toFloat();
    if (r.isEmpty()) return;
    
    // Get theme colors from LNF
    auto& lf = getLookAndFeel();
    auto gridCol = lf.findColour(FieldLNF::eqGridLineColourId);
    auto textCol = lf.findColour(FieldLNF::eqLabelTextColourId).withAlpha(0.45f);
    auto zeroCol = lf.findColour(FieldLNF::eqZeroLineColourId);
    
    g.setFont(12.0f);
    g.setColour(gridCol);

    // Decay multiplier ticks
    const float multVals[] = { 2.0f, 1.5f, 1.0f, 0.75f, 0.5f };
    for (float mult : multVals)
    {
        const float y = mapMultToY(mult);
        
        // Use special color for 1.0x line
        if (mult == 1.0f) {
            g.setColour(zeroCol);
            g.drawLine(r.getX(), y, r.getRight(), y, 1.2f);
        } else {
            g.setColour(gridCol);
            g.drawLine(r.getX(), y, r.getRight(), y, 0.6f);
        }
        
        g.setColour(textCol);
        juce::String lbl = juce::String(mult, 1) + "×";
        g.drawFittedText(lbl, juce::Rectangle<int>((int)r.getX()+4, (int)y-8, 44, 16), juce::Justification::centredLeft, 1);
    }

    // Hz ticks
    const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000 };
    for (double hz : hzTicks)
    {
        const float x = mapHzToX((float)hz);
        g.setColour(gridCol);
        g.drawLine(x, r.getBottom()-16.0f, x, r.getBottom(), 0.8f);
        g.setColour(textCol);
        juce::String lbl;
        if (hz >= 1000.0 && hz < 10000.0) lbl = juce::String(hz/1000.0, 1) + "k";
        else if (hz >= 10000.0) lbl = juce::String((int)std::round(hz/1000.0)) + "k";
        else lbl = juce::String((int)hz);
        g.drawFittedText(lbl, juce::Rectangle<int>((int)x-18, (int)r.getBottom()-30, 36, 14), juce::Justification::centred, 1);
    }
}

// Mouse interaction
void DecayRateEQ::mouseDown(const juce::MouseEvent& e)
{
    const int h = hitTestPoint(e.getPosition());
    if (h >= 0)
    {
        selected = h;
        auto& pt = points[(size_t)selected];
        overlay.setValues(pt.mult, pt.q, pt.hz, pt.type);
        overlay.setVisible(true);
        positionOverlay();
        positionBadgeFor(selected);
    }
    else if (!e.mods.isPopupMenu())
    {
        // Check band limit (3 bands max for Decay-Rate EQ)
        if (points.size() >= 3)
        {
            // Show tooltip or visual feedback that limit is reached
            return;
        }
        
        // Create new band
        DecayBandPoint bp; 
        bp.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x)); 
        bp.mult = juce::jlimit(0.5f, 2.0f, mapYToMult(e.getPosition().y));
        if (bp.hz <= 50.0f) { bp.type = 1; } // TiltLo
        else if (bp.hz >= 10000.0f) { bp.type = 2; } // TiltHi
        else { bp.type = 0; } // Bell
        
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
            setBandParam(slot, ReverbEQParams::DecayBand::active, 1.0f);
            setBandParam(slot, ReverbEQParams::DecayBand::freqHz, bp.hz);
            setBandParam(slot, ReverbEQParams::DecayBand::decayMult, bp.mult);
            setBandParam(slot, ReverbEQParams::DecayBand::q, bp.q);
        }
        points.push_back(bp);
        selected = (int)points.size() - 1;
        overlay.setValues(bp.mult, bp.q, bp.hz, bp.type);
        overlay.setVisible(true);
        positionOverlay();
        positionBadgeFor(selected);
        rebuildEqPath(); 
        repaint();
    }
    else
    {
        overlay.setVisible(false);
        badge.setVisible(false);
    }
}

void DecayRateEQ::mouseDrag(const juce::MouseEvent& e)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        pt.hz = juce::jlimit(20.f, 20000.f, mapXToHz(e.getPosition().x));
        pt.mult = juce::jlimit(0.5f, 2.0f, mapYToMult(e.getPosition().y));
        if (pt.bandIdx >= 0)
        {
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::freqHz, pt.hz);
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::decayMult, pt.mult);
        }
        rebuildEqPath();
        repaint();
    }
}

void DecayRateEQ::mouseUp(const juce::MouseEvent&)
{
    dragging = false;
}

void DecayRateEQ::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    if (selected >= 0 && selected < (int)points.size())
    {
        auto& pt = points[(size_t)selected];
        const float delta = (float)(wheel.deltaY * (e.mods.isShiftDown() ? 1.0 : 0.2));
        pt.q = juce::jlimit(0.1f, 36.0f, pt.q * (1.0f + delta));
        if (pt.bandIdx >= 0)
            setBandParam(pt.bandIdx, ReverbEQParams::DecayBand::q, pt.q);
        rebuildEqPath();
        repaint();
    }
}

void DecayRateEQ::mouseDoubleClick(const juce::MouseEvent& e)
{
    const int idx = hitTestPoint(e.getPosition());
    if (idx >= 0 && idx < (int)points.size())
    {
        // Double-click on an existing point deletes it
        const int bandIdx = points[(size_t)idx].bandIdx;
        if (bandIdx >= 0)
            setBandParam(bandIdx, ReverbEQParams::DecayBand::active, 0.0f);
        points.erase(points.begin() + idx);
        if (selected == idx) selected = -1; 
        else if (selected > idx) --selected;
        rebuildEqPath();
        repaint();
        if (selected < 0) 
        {
            overlay.setVisible(false);
            badge.setVisible(false);
        }
        else 
        {
            auto& pt2 = points[(size_t)selected];
            overlay.setValues(pt2.mult, pt2.q, pt2.hz, pt2.type);
            overlay.setVisible(true);
            positionOverlay();
            positionBadgeFor(selected);
        }
        return;
    }
}

void DecayRateEQ::mouseMove(const juce::MouseEvent& e)
{
    const int h = hitTestPoint(e.getPosition());
    if (h != hover)
    {
        hover = h;
        repaint();
    }
    hoverPos = e.getPosition();
    auto r = analyzer.getBounds();
    hoverInPane = r.contains(hoverPos);
    if (hoverInPane)
        hoverHz = juce::jlimit(20.0f, 20000.0f, mapXToHz(hoverPos.x));
    lastMouseMoveMs = (juce::int64) juce::Time::getMillisecondCounterHiRes();
    repaint();
}

void DecayRateEQ::mouseExit(const juce::MouseEvent&)
{
    hover = -1;
    hoverInPane = false;
    repaint();
}

// Mapping helpers
float DecayRateEQ::mapHzToX(float hz) const
{
    auto r = analyzer.getBounds().toFloat();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = (float)(std::log10(juce::jlimit(minHz, maxHz, hz) / minHz) / std::log10(maxHz / minHz));
    return r.getX() + t * r.getWidth();
}

float DecayRateEQ::mapMultToY(float mult) const
{
    auto r = analyzer.getBounds().toFloat();
    const float top = r.getY()+8.f, bottom = r.getBottom()-8.f;
    const float halfRange = zoomState.getCurrent();
    return juce::jmap(mult, 2.0f, 0.5f, top, bottom);
}

float DecayRateEQ::mapXToHz(int px) const
{
    auto r = analyzer.getBounds();
    const float minHz = 20.f, maxHz = 20000.f;
    const float t = juce::jlimit(0.0f, 1.0f, (px - (float)r.getX()) / (float)r.getWidth());
    const float a = std::log10(minHz), b = std::log10(maxHz);
    return std::pow(10.0f, juce::jmap(t, 0.0f, 1.0f, a, b));
}

float DecayRateEQ::mapYToMult(int py) const
{
    auto r = analyzer.getBounds();
    const float halfRange = zoomState.getCurrent();
    return juce::jmap((float)py, (float)r.getY(), (float)r.getBottom(), 2.0f, 0.5f);
}

// Hit testing
int DecayRateEQ::hitTestPoint(juce::Point<int> p) const
{
    const float radius = 12.0f;
    for (int i = (int)points.size()-1; i >= 0; --i)
    {
        const float x = mapHzToX(points[(size_t)i].hz);
        const float y = mapMultToY(points[(size_t)i].mult);
        if (juce::Point<float>(x, y).getDistanceFrom(p.toFloat()) <= radius)
            return i;
        }
    return -1;
}

// Band management
int DecayRateEQ::allocateBandSlot()
{
    for (int i = 0; i < kMaxBands; ++i)
    {
        auto id = bandId(ReverbEQParams::DecayBand::active, i);
        if (auto* v = proc.apvts.getRawParameterValue(id))
        {
            if (v->load() < 0.5f)
                return i;
        }
    }
    return -1;
}

void DecayRateEQ::setBandParam(int bandIdx, const char* baseId, float value)
{
    auto id = bandId(baseId, bandIdx);
    if (auto* p = proc.apvts.getParameter(id))
    {
        const float norm = p->convertTo0to1(value);
        p->setValueNotifyingHost(norm);
    }
}

float DecayRateEQ::getBandParamFloat(int bandIdx, const char* baseId, float fallback) const
{
    auto id = bandId(baseId, bandIdx);
    if (auto* v = proc.apvts.getRawParameterValue(id)) return v->load();
    return fallback;
}

// Visual helpers
juce::Colour DecayRateEQ::bandColourFor(int bandIdx) const
{
    auto& lf = getLookAndFeel();
    auto accent = lf.findColour(FieldLNF::eqLabelTextColourId);
    const float baseHue = accent.getHue();
    const float baseSat = juce::jlimit(0.25f, 0.95f, accent.getSaturation());
    const float baseBrt = juce::jlimit(0.35f, 0.95f, accent.getBrightness());
    const float golden = 0.61803398875f;
    float hue = std::fmod(baseHue + golden * (float)(bandIdx + 1), 1.0f);
    hue = juce::jlimit(0.0f, 1.0f, 0.65f * hue + 0.35f * baseHue);
    float sat = juce::jlimit(0.30f, 0.95f, baseSat * 0.9f + 0.1f);
    float brt = juce::jlimit(0.40f, 0.95f, baseBrt * 0.9f + 0.1f);
    return juce::Colour::fromHSV(hue, sat, brt, 1.0f);
}

float DecayRateEQ::bandMultAtForPaint(const DecayBandPoint& b, float hz) const
{
    const double logHz = std::log10(juce::jlimit(20.0f, 20000.0f, hz));
    const double logC = std::log10(juce::jlimit(20.0f, 20000.0f, b.hz));
    const double q = juce::jlimit(0.1, 36.0, (double)b.q);
    const double width = juce::jlimit(0.02, 0.50, 0.22 / q);
    const double d = (logHz - logC) / width;
    
    switch (b.type)
    {
        case 0: { // Bell
            const float w = (float)std::exp(-0.5 * d * d);
            return juce::jmap(w, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 1: { // TiltLo
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return juce::jmap((float)s, 0.0f, 1.0f, 1.0f, b.mult);
        }
        case 2: { // TiltHi
            const double k = 8.0 * juce::jlimit(0.2, 3.0, q * 0.25);
            const double s = 1.0 / (1.0 + std::exp(-k * (logHz - logC)));
            return juce::jmap((float)(1.0 - s), 0.0f, 1.0f, 1.0f, b.mult);
        }
        default: return 1.0f;
    }
}

// Per-band module implementations
void DecayRateEQ::positionOverlay()
{
    if (selected < 0 || selected >= (int)points.size()) return;
    
    auto r = getLocalBounds();
    const int w = 200, h = 120;
    
    // Get the selected band point position
    const auto& pt = points[(size_t)selected];
    const float bandX = mapHzToX(pt.hz);
    const float bandY = mapMultToY(pt.mult);
    
    // Start with band point position
    int ox = (int)bandX - w/2;
    int oy = (int)bandY - h/2;
    
    // Smart positioning to avoid overlap with band point
    const int bandRadius = 12; // Band point click radius
    const int margin = 20; // Additional margin from band point
    
    // Check if overlay would overlap with band point
    bool overlapsBand = (ox <= bandX + bandRadius + margin && 
                        ox + w >= bandX - bandRadius - margin &&
                        oy <= bandY + bandRadius + margin && 
                        oy + h >= bandY - bandRadius - margin);
    
    if (overlapsBand)
    {
        // Position overlay to the right of band point
        ox = (int)bandX + bandRadius + margin;
        oy = (int)bandY - h/2;
        
        // If that goes off screen, try to the left
        if (ox + w > r.getRight())
        {
            ox = (int)bandX - w - bandRadius - margin;
        }
        
        // If still off screen, try above
        if (ox < r.getX() || ox + w > r.getRight())
        {
            ox = (int)bandX - w/2;
            oy = (int)bandY - h - bandRadius - margin;
        }
        
        // If still off screen, try below
        if (oy < r.getY())
        {
            oy = (int)bandY + bandRadius + margin;
        }
    }
    
    // Final bounds checking
    if (ox < r.getX()) ox = r.getX() + 10;
    if (ox + w > r.getRight()) ox = r.getRight() - w - 10;
    if (oy < r.getY()) oy = r.getY() + 10;
    if (oy + h > r.getBottom()) oy = r.getBottom() - h - 10;
    
    overlay.setBounds(ox, oy, w, h);
    overlay.setVisible(true);
}

void DecayRateEQ::positionBadgeFor(int idx)
{
    if (idx < 0 || idx >= (int)points.size()) return;
    const auto& pt = points[(size_t)idx];
    const float x = mapHzToX(pt.hz);
    const float y = mapMultToY(pt.mult);
    
    const int w = 120, h = 60;
    auto pane = analyzer.getBounds();
    
    // Smart positioning to avoid overlap with band point
    const int bandRadius = 12;
    const int margin = 15;
    
    // Try positioning to the right first
    int ox = (int)x + bandRadius + margin;
    int oy = (int)y - h/2;
    
    // If that goes off screen, try to the left
    if (ox + w > pane.getRight())
    {
        ox = (int)x - w - bandRadius - margin;
    }
    
    // If still off screen, try above
    if (ox < pane.getX())
    {
        ox = (int)x - w/2;
        oy = (int)y - h - bandRadius - margin;
    }
    
    // If still off screen, try below
    if (oy < pane.getY())
    {
        oy = (int)y + bandRadius + margin;
    }
    
    // Final bounds checking
    if (ox < pane.getX()) ox = pane.getX() + 5;
    if (ox + w > pane.getRight()) ox = pane.getRight() - w - 5;
    if (oy < pane.getY()) oy = pane.getY() + 5;
    if (oy + h > pane.getBottom()) oy = pane.getBottom() - h - 5;
    
    badge.setBounds(ox, oy, w, h);
    badge.setVisible(true);
    badge.setValues(pt.mult, pt.hz, pt.type, false);
    badge.setDetails(pt.q, pt.mult, false, false, 0.0f, false, "St", 0, "Pre");
    
    // Per-band accent color
    juce::Colour accent = bandColourFor(idx);
    badge.toFront(true);
}

// BandOverlay implementation for DecayRateEQ
DecayRateEQ::BandOverlay::BandOverlay()
{
    setInterceptsMouseClicks(true, true);
    
    // Setup sliders
    mult.setSliderStyle(juce::Slider::LinearHorizontal);
    mult.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    mult.setRange(0.5, 2.0, 0.01);
    mult.onValueChange = [this] { if (!updating && onMultChanged) onMultChanged((float)mult.getValue()); };
    mult.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    mult.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(mult);
    
    q.setSliderStyle(juce::Slider::LinearHorizontal);
    q.setTextBoxStyle(juce::Slider::TextBoxRight, false, 48, 18);
    q.setRange(0.1, 36.0, 0.01);
    q.onValueChange = [this] { if (!updating && onQChanged) onQChanged((float)q.getValue()); };
    q.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    q.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(q);
    
    freq.setSliderStyle(juce::Slider::LinearHorizontal);
    freq.setTextBoxStyle(juce::Slider::TextBoxRight, false, 64, 18);
    freq.setRange(20.0, 20000.0, 0.01);
    freq.setSkewFactorFromMidPoint(1000.0);
    freq.onValueChange = [this] { if (!updating && onFreqChanged) onFreqChanged((float)freq.getValue()); };
    freq.onDragStart = [this] { if (onDragAny) onDragAny(true); };
    freq.onDragEnd = [this] { if (onDragAny) onDragAny(false); };
    addAndMakeVisible(freq);
    
    // Setup labels
    multLabel.setText("MULT", juce::dontSendNotification);
    multLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(multLabel);
    
    qLabel.setText("Q", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(qLabel);
    
    freqLabel.setText("FREQ", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(freqLabel);
    
    // Setup type combo
    typeCb.addItemList(juce::StringArray{"Bell", "TiltLo", "TiltHi"}, 1);
    typeCb.onChange = [this] { if (!updating && onTypeChanged) onTypeChanged(typeCb.getSelectedItemIndex()); };
    addAndMakeVisible(typeCb);
    
    typeLabel.setText("TYPE", juce::dontSendNotification);
    typeLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(typeLabel);
}

DecayRateEQ::BandOverlay::~BandOverlay() {}

void DecayRateEQ::BandOverlay::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    // Get theme colors
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
    
    // Background with accent
    juce::Colour bg = th.panel.darker(0.20f);
    g.setColour(bg.withAlpha(0.96f));
    g.fillRoundedRectangle(r, 8.0f);
    
    // Accent border
    g.setColour(th.accent.withAlpha(0.8f));
    g.drawRoundedRectangle(r, 8.0f, 2.0f);
    
    // Accent strip
    juce::Rectangle<float> strip = r.removeFromLeft(3.0f).reduced(0.5f, 2.0f);
    g.setColour(th.accent);
    g.fillRoundedRectangle(strip, 1.5f);
}

void DecayRateEQ::BandOverlay::resized()
{
    auto r = getLocalBounds().reduced(8);
    const int labelW = 40, sliderH = 20, gap = 4;
    
    // Mult row
    multLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    mult.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Q row
    qLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    q.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Freq row
    freqLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    freq.setBounds(r.removeFromTop(sliderH));
    r.removeFromTop(gap);
    
    // Type row
    typeLabel.setBounds(r.removeFromTop(sliderH).removeFromLeft(labelW));
    typeCb.setBounds(r.removeFromTop(sliderH));
}

void DecayRateEQ::BandOverlay::setValues(float multVal, float qVal, float freqVal, int type)
{
    updating = true;
    mult.setValue(multVal, juce::dontSendNotification);
    q.setValue(qVal, juce::dontSendNotification);
    freq.setValue(freqVal, juce::dontSendNotification);
    typeCb.setSelectedItemIndex(type, juce::dontSendNotification);
    updating = false;
}


// BandBadge implementation for DecayRateEQ
DecayRateEQ::BandBadge::BandBadge()
{
    setInterceptsMouseClicks(true, true);
    
    // Setup buttons
    deleteBtn.setButtonText("×");
    deleteBtn.onClick = [this] { if (onDelete) onDelete(); };
    addAndMakeVisible(deleteBtn);
    
    bypassBtn.setButtonText("BYP");
    bypassBtn.onClick = [this] { if (onBypass) onBypass(bypassBtn.getToggleState()); };
    addAndMakeVisible(bypassBtn);
    
    typeBtn.setButtonText("Bell");
    typeBtn.onClick = [this] { if (onSetType) onSetType(0); };
    addAndMakeVisible(typeBtn);
    
    // Setup labels
    freqLabel.setText("1.0k", juce::dontSendNotification);
    freqLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(freqLabel);
    
    multLabel.setText("1.0x", juce::dontSendNotification);
    multLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(multLabel);
    
    qLabel.setText("0.7", juce::dontSendNotification);
    qLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(qLabel);
}

DecayRateEQ::BandBadge::~BandBadge() {}

void DecayRateEQ::BandBadge::paint(juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    
    // Get theme colors from LNF
    auto& lf = getLookAndFeel();
    auto accent = lf.findColour(FieldLNF::eqLabelTextColourId);
    auto panel = lf.findColour(juce::ResizableWindow::backgroundColourId);
    
    // Background
    juce::Colour bg = panel.darker(0.30f);
    g.setColour(bg.withAlpha(0.95f));
    g.fillRoundedRectangle(r, 6.0f);
    
    // Accent border
    g.setColour(accent.withAlpha(0.9f));
    g.drawRoundedRectangle(r, 6.0f, 1.5f);
    
    // Accent strip
    juce::Rectangle<float> strip = r.removeFromLeft(2.0f).reduced(0.5f, 1.0f);
    g.setColour(accent);
    g.fillRoundedRectangle(strip, 1.0f);
}

void DecayRateEQ::BandBadge::resized()
{
    auto r = getLocalBounds().reduced(4);
    const int btnW = 24, btnH = 16;
    
    // Top row: buttons
    deleteBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    bypassBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    typeBtn.setBounds(r.removeFromTop(btnH).removeFromLeft(btnW));
    
    // Bottom row: labels
    freqLabel.setBounds(r.removeFromTop(btnH));
    multLabel.setBounds(r.removeFromTop(btnH));
    qLabel.setBounds(r.removeFromTop(btnH));
}

void DecayRateEQ::BandBadge::setValues(float mult, float freq, int type, bool bypass)
{
    currentMult = mult;
    currentFreq = freq;
    currentType = type;
    currentBypass = bypass;
    
    freqLabel.setText(juce::String(freq, freq >= 1000.0f ? 1 : 0) + (freq >= 1000.0f ? "k" : ""), juce::dontSendNotification);
    multLabel.setText(juce::String(mult, 1) + "x", juce::dontSendNotification);
    bypassBtn.setToggleState(bypass, juce::dontSendNotification);
    
    const char* typeNames[] = {"Bell", "TiltLo", "TiltHi"};
    typeBtn.setButtonText(typeNames[juce::jlimit(0, 2, type)]);
}

void DecayRateEQ::BandBadge::setDetails(float q, float mult, bool dynOn, bool dynUp, float dynRange, bool specOn, const juce::String& channel, int slopeDb, const juce::String& tap)
{
    currentQ = q;
    currentMult = mult;
    
    multLabel.setText(juce::String(mult, 1) + "x", juce::dontSendNotification);
    qLabel.setText(juce::String(q, 2), juce::dontSendNotification);
}
