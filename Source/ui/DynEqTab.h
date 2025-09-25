#pragma once

#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"
#include "../dynEQ/DynamicEqParamIDs.h"
#include "../FieldLookAndFeel.h"

class MyPluginAudioProcessor; // fwd
class MyPluginAudioProcessorEditor; // fwd

// Dynamic EQ tab (replaces Spectrum). In-pane experience: visuals + editor.
// Scaffold component so we can integrate DSP/Editor incrementally.
class DynEqTab : public juce::Component, private juce::Timer
{
public:
    DynEqTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p), lookAndFeelPtr (lnf)
    {
        setOpaque (true);
        startTimerHz (30);
        addAndMakeVisible (analyzer);
        analyzer.setInterceptsMouseClicks (false, false);
        analyzer.setAutoHeadroomEnabled (true);
        analyzer.setHeadroomTargetFill (0.70f);
        SpectrumAnalyzer::Params prm; prm.fps = 30; analyzer.setParams (prm);
        analyzer.setDrawGridHorizontal (false); // we'll draw our own dB units

        // Floating per-band mini control panel (Gain/Q). Shown on selection.
        addAndMakeVisible (overlay);
        overlay.setVisible (false);
        addAndMakeVisible (badge);
        badge.setVisible (false);
        
        // Zoom slider for manual dB range control
        addAndMakeVisible (zoomSlider);
        zoomSlider.setSliderStyle (juce::Slider::LinearVertical);
        zoomSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        zoomSlider.setRange (0.0, 2.0, 0.1); // 0=±6dB, 1=±18dB, 2=±36dB
        zoomSlider.setValue (0.0); // Start at ±6dB
        zoomSlider.onValueChange = [this] { updateZoomFromSlider(); };
        overlay.onGainChanged = [this](float g)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].db = juce::jlimit (-24.f, 24.f, g);
                rebuildEqPath();
                repaint();
                positionOverlay();
            }
        };
        overlay.onQChanged = [this](float qv)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].q = juce::jlimit (0.1f, 36.0f, qv);
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::q, points[(size_t) selected].q);
                rebuildEqPath();
                repaint();
                positionOverlay();
            }
        };
        overlay.onFreqChanged = [this](float hz)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].hz = juce::jlimit (20.0f, 20000.0f, hz);
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::freqHz, points[(size_t) selected].hz);
                rebuildEqPath();
                repaint();
                positionOverlay();
            }
        };
        overlay.onDragAny = [this](bool dragging)
        {
            overlayFrozen = dragging;
            if (dragging)
                overlayLastBounds = overlay.getBounds();
            else
                positionOverlay();
        };
        overlay.onTypeChanged = [this](int tp)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].type = tp;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::type, (float) tp);
                rebuildEqPath();
                repaint();
            }
        };
        overlay.onPhaseChanged = [this](int ph)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].phase = ph;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::phase, (float) ph);
                repaint();
            }
        };
        overlay.onChanChanged = [this](int ch)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].channel = ch;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::channel, (float) ch);
                repaint();
            }
        };
        overlay.onDynChanged = [this](bool on)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                auto& p = points[(size_t) selected]; p.dynOn = on;
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::dynOn, on ? 1.0f : 0.0f);
            }
        };
        overlay.onSpecChanged = [this](bool on)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                auto& p = points[(size_t) selected]; p.specOn = on;
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::specOn, on ? 1.0f : 0.0f);
            }
        };

        badge.onDelete = [this]
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                const int bandIdx = points[(size_t) selected].bandIdx;
                if (bandIdx >= 0) setBandParam (bandIdx, dynEq::Band::active, 0.0f);
                points.erase (points.begin() + selected);
                selected = -1; rebuildEqPath(); repaint(); overlay.setVisible (false); badge.setVisible (false);
            }
        };
        badge.onBypass = [this](bool off)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                const int bandIdx = points[(size_t) selected].bandIdx;
                if (bandIdx >= 0) setBandParam (bandIdx, dynEq::Band::active, off ? 0.0f : 1.0f);
            }
        };
        badge.onSetType = [this](int tp)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx];
                p.type = juce::jlimit (0, 6, tp);
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::type, (float) p.type);
                rebuildEqPath(); repaint(); positionBadgeFor (idx);
            }
        };
        badge.onSetSlopeDb = [this](int slopeDb)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size()) { points[(size_t) idx].slopeDb = slopeDb; repaint(); positionBadgeFor (idx); }
        };
        badge.onSetTapMode = [this](int tap)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size()) { points[(size_t) idx].tapMode = juce::jlimit (0,2,tap); repaint(); positionBadgeFor (idx); }
        };
        badge.onToggleDyn = [this]
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx]; p.dynOn = !p.dynOn;
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::dynOn, p.dynOn ? 1.0f : 0.0f);
                repaint(); positionBadgeFor (idx);
            }
        };
        badge.onToggleSpec = [this]
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx]; p.specOn = !p.specOn;
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::specOn, p.specOn ? 1.0f : 0.0f);
                repaint(); positionBadgeFor (idx);
            }
        };
        badge.onSetFreq = [this](float hz)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx]; p.hz = juce::jlimit (20.f, 20000.f, hz);
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::freqHz, p.hz);
                rebuildEqPath(); repaint(); positionBadgeFor (idx);
            }
        };
        badge.onSetQ = [this](float q)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx]; p.q = juce::jlimit (0.1f, 36.0f, q);
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::q, p.q);
                rebuildEqPath(); repaint(); positionBadgeFor (idx);
            }
        };
        badge.onSetGainDb = [this](float g)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx]; p.db = juce::jlimit (-24.f, 24.f, g);
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::gainDb, p.db);
                adaptDbRangeToPoint (p.db);
                rebuildEqPath(); repaint(); positionBadgeFor (idx);
            }
        };
        badge.onSetDynRangeDb = [this](float r)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx];
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::dynRangeDb, juce::jlimit (-24.f, 24.f, r));
                rebuildEqPath(); repaint(); positionBadgeFor (idx);
            }
        };
    }

    ~DynEqTab() override
    {
        // Stop timer before destruction to prevent use-after-free
        stopTimer();
    }

    void timerCallback() override
    {
        // Drive delayed ghost repaint and hover HUD updates at 30Hz
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            // Fully opaque pane background (no see-through)
            juce::Colour top = lf->theme.shadowDark.withAlpha (1.0f);
            juce::Colour bot = lf->theme.shadowDark.withAlpha (1.0f);
            juce::ColourGradient grad (top, r.getCentreX(), r.getY(), bot, r.getCentreX(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRect (r);
            g.setColour (juce::Colours::white.withAlpha (0.10f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
        }
        else
        {
            g.fillAll (juce::Colours::darkgrey.darker (0.35f));
            g.setColour (juce::Colours::white.withAlpha (0.10f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
        }

        // background only; overlay drawn in paintOverChildren
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        // Units
        drawUnits (g);
        auto rA = analyzer.getBounds().toFloat();
        // Band-wise curves with theme-driven colours and optional fills for dyn/spec
        const bool hasAreas = bandAreas.size() == bandPaths.size();
        const bool hasDyn   = bandDynPaths.size() == bandPaths.size();
        const bool hasDynReg= bandDynRegions.size() == bandPaths.size();
        const bool hasSpecReg= bandSpecRegions.size() == bandPaths.size();
        for (size_t i = 0; i < bandPaths.size(); ++i)
        {
            juce::Colour base = bandColourFor ((int) i);
            const auto& pt = points.size() > i ? points[i] : BandPoint{};
            base = applyChannelTint (base, pt.channel); // M/S/L/R tinting

            if (hasAreas && (pt.dynOn || pt.specOn))
            {
                auto r = analyzer.getBounds().toFloat();
                if (pt.dynOn && hasDynReg)
                {
                    // Heavier near dynamic curve, lighter towards base
                    const float cx = mapHzToX (pt.hz);
                    const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
                    const float signedRange = (ptDynModeUp ((int) i) ? +1.0f : -1.0f) * ptDynRangeDb ((int) i);
                    const float offY = mapDbToY (signedRange + 18.0f) - mapDbToY (18.0f);
                    const float dynY = baseY + offY;
                    juce::Colour fillNear = base.withAlpha (0.35f);
                    juce::Colour fillFar  = base.withAlpha (0.06f);
                    juce::ColourGradient grad (fillNear, cx, dynY, fillFar, cx, baseY, false);
                    g.setGradientFill (grad);
                    g.fillPath (bandDynRegions[i]);
                }
                else if (pt.specOn && hasSpecReg)
                {
                    // Gradient between spectral curve and base band curve (heavier at spectral curve)
                    const float cx = mapHzToX (pt.hz);
                    const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
                    const float specRange = ptSpecRangeDb ((int) i);
                    const float specOff = mapDbToY (-specRange + 18.0f) - mapDbToY (18.0f);
                    const float specY = baseY + specOff;
                    juce::Colour fillNear = base.withAlpha (0.30f);
                    juce::Colour fillFar  = base.withAlpha (0.05f);
                    juce::ColourGradient grad (fillNear, cx, specY, fillFar, cx, baseY, false);
                    g.setGradientFill (grad);
                    g.fillPath (bandSpecRegions[i]);
                }
            }

            g.setColour (base.withAlpha (selected == (int) i ? 1.0f : 0.90f));
            const float width = (selected == (int) i ? 1.8f : 1.2f);
            g.strokePath (bandPaths[i], juce::PathStrokeType (width));

            // Dynamic range visualization (secondary path + indicator) — only when Dynamics is enabled
            const bool showDynElem = hasDyn && pt.dynOn;
            if (showDynElem)
            {
                juce::Colour dynCol = base.darker (0.15f).withAlpha (0.90f);
                g.setColour (dynCol);
                g.strokePath (bandDynPaths[i], juce::PathStrokeType (1.6f));

                const float cx = mapHzToX (pt.hz);
                // Approx: use band path Y (base) and offset by signed dynamic range at center (no DSP yet)
                const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
                const float signedRange = (ptDynModeUp ((int) i) ? +1.0f : -1.0f) * ptDynRangeDb ((int) i);
                const float offsetY = mapDbToY (signedRange + 18.0f) - mapDbToY (18.0f);
                const float cy = baseY + offsetY;
                // Unique vertical indicator with grab handle
                g.drawLine (cx, baseY, cx, cy, 2.0f);
                g.setColour (juce::Colours::white.withAlpha (0.90f));
                g.fillRoundedRectangle (juce::Rectangle<float> (cx-6.0f, cy-6.0f, 12.0f, 12.0f), 3.0f);
                g.setColour (dynCol);
                g.drawRoundedRectangle (juce::Rectangle<float> (cx-6.0f, cy-6.0f, 12.0f, 12.0f), 3.0f, 1.2f);
            }

            // Spectral attenuation preview path (if Spec ON)
            if (pt.specOn && bandSpecPaths.size() > i)
            {
                g.setColour (base.withAlpha (0.75f));
                g.strokePath (bandSpecPaths[i], juce::PathStrokeType (1.4f));
                // Draw spectral amount indicator similar to dynamics (always downward)
                const float cx = mapHzToX (pt.hz);
                const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
                const float specRange = ptSpecRangeDb ((int) i);
                const float offsetY = mapDbToY (-specRange + 18.0f) - mapDbToY (18.0f);
                const float cy = baseY + offsetY;
                g.setColour (base.darker (0.10f).withAlpha (0.90f));
                g.drawLine (cx, baseY, cx, cy, 2.0f);
                g.setColour (juce::Colours::white.withAlpha (0.90f));
                g.fillEllipse (juce::Rectangle<float> (cx-5.0f, cy-5.0f, 10.0f, 10.0f));
                g.setColour (base.darker (0.10f).withAlpha (0.90f));
                g.drawEllipse (juce::Rectangle<float> (cx-5.0f, cy-5.0f, 10.0f, 10.0f), 1.2f);
            }

            // Hover/selection gradient for inactive bands (neither dyn nor spec):
            // Fill area between band curve and 0 dB line; heavier near curve.
            const bool inactiveBand = !pt.dynOn && !pt.specOn;
            const bool hoverThis = ((int) i == hover);
            const bool selectThis = ((int) i == selected);
            if (inactiveBand && (hoverThis || selectThis))
            {
                auto r = analyzer.getBounds().toFloat();
                const int N = juce::jmax (64, (int) (r.getWidth() * 0.5f));
                const float y0 = mapDbToY (0.0f);
                juce::Path area;
                // forward along curve
                for (int k = 0; k < N; ++k)
                {
                    const double t = (double) k / (double) (N - 1);
                    const double minHz=20.0, maxHz=20000.0;
                    const double a=std::log10(minHz), bL=std::log10(maxHz);
                    const double logF=juce::jmap(t,0.0,1.0,a,bL);
                    const double hz=std::pow(10.0, logF);
                    const float x = r.getX() + (float) t * r.getWidth();
                    const float y = mapDbToY (bandDbAtForPaint (pt, (float) hz));
                    if (k == 0) area.startNewSubPath (x, y); else area.lineTo (x, y);
                }
                // down to 0 line and back along 0 line
                area.lineTo (r.getRight(), y0);
                for (int k = N-1; k >= 0; --k)
                {
                    const double t = (double) k / (double) (N - 1);
                    const float x = r.getX() + (float) t * r.getWidth();
                    area.lineTo (x, y0);
                }
                area.closeSubPath();

                // Gradient: heavier near curve center, lighter towards 0 line
                const float cx = mapHzToX (pt.hz);
                const float curveY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
                const float alphaNear = selectThis ? 0.26f : 0.16f;
                const float alphaFar  = selectThis ? 0.08f : 0.04f;
                juce::Colour cNear = base.withAlpha (alphaNear);
                juce::Colour cFar  = base.withAlpha (alphaFar);
                juce::ColourGradient grad (cNear, cx, curveY, cFar, cx, y0, false);
                g.setGradientFill (grad);
                g.fillPath (area);
            }
        }
        // Combined EQ curve (macro) slightly more prominent
        g.setColour (macroColour());
        g.strokePath (eqPath, juce::PathStrokeType (3.0f));

        g.setColour (juce::Colours::yellow.withAlpha (0.95f));
        for (const auto& pt : points)
        {
            const float x = mapHzToX (pt.hz);
            const float y = mapDbToY (pt.db);
            g.fillEllipse (x-8, y-8, 16, 16);
        }

        if (selected >= 0 && selected < (int) points.size())
        {
            const auto& pt = points[(size_t) selected];
            const float x = mapHzToX (pt.hz);
            const float y = mapDbToY (pt.db);
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.drawEllipse (x-12, y-12, 24, 24, 1.6f);
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
            g.setColour (juce::Colours::white.withAlpha (alpha));
            const float x = (float) hoverPos.x;
            // Main center line
            g.drawLine (x, rGuide.getY(), x, rGuide.getBottom(), ghostOn ? 1.4f : 1.0f);
            // Side fades
            g.setColour (juce::Colours::white.withAlpha (alphaFade));
            g.drawLine (x-12.0f, rGuide.getY(), x-12.0f, rGuide.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.drawLine (x+12.0f, rGuide.getY(), x+12.0f, rGuide.getBottom(), ghostOn ? 1.0f : 0.8f);
            g.setColour (juce::Colours::white.withAlpha (alphaFade * 0.6f));
            g.drawLine (x-24.0f, rGuide.getY(), x-24.0f, rGuide.getBottom(), 0.8f);
            g.drawLine (x+24.0f, rGuide.getY(), x+24.0f, rGuide.getBottom(), 0.8f);

            // Hz readout near bottom and top (follow cursor)
            g.setColour (juce::Colours::white.withAlpha (0.60f));
            juce::String hzText;
            if (hoverHz >= 1000.0f && hoverHz < 10000.0f) hzText = juce::String (hoverHz / 1000.0f, 1) + "k";
            else if (hoverHz >= 10000.0f) hzText = juce::String ((int) std::round (hoverHz/1000.0f)) + "k";
            else hzText = juce::String ((int) hoverHz);
            juce::String lbl = hzText + " Hz";
            auto tb = juce::Rectangle<float> ((float) hoverPos.x - 32.0f, rA.getBottom() - 20.0f, 64.0f, 14.0f);
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.fillRoundedRectangle (tb, 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.80f));
            g.drawFittedText (lbl, tb.toNearestInt(), juce::Justification::centred, 1);
            // Top badge
            auto tt = juce::Rectangle<float> ((float) hoverPos.x - 28.0f, rA.getY() + 6.0f, 56.0f, 14.0f);
            g.setColour (juce::Colours::black.withAlpha (0.40f));
            g.fillRoundedRectangle (tt, 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.85f));
            g.drawFittedText (lbl, tt.toNearestInt(), juce::Justification::centred, 1);

            // Predictive ghost: show faint HP/LP in low/high zones, else Bell
            const bool showGhost = ghostOn;
            if (showGhost)
            {
                // Suppress ghost if near an existing point (avoid conflicts)
                const float suppressRadiusPx = 24.0f;
                bool nearPoint = false;
                for (const auto& pt : points)
                {
                    if (juce::Point<float> (mapHzToX (pt.hz), mapDbToY (pt.db)).getDistanceFrom (hoverPos.toFloat()) <= suppressRadiusPx)
                    { nearPoint = true; break; }
                }
                if (! nearPoint)
                {
                // Build full ghost
                juce::Path ghost;
                const bool mouseAbove0 = mapYToDb (hoverPos.y) > 0.0f;
                auto makeGhost = [&](int type, float amtDb){ BandPoint b; b.type = type; b.hz = hoverHz; b.db = amtDb; b.q = 0.9f; const int N = juce::jmax (64, (int) rA.getWidth()); for (int i=0;i<N;++i){ const double minHz=20.0, maxHz=20000.0; const double t=(double)i/(double)(N-1); const double a=std::log10(minHz), bL=std::log10(maxHz); const double logF=juce::jmap(t,0.0,1.0,a,bL); const double hz=std::pow(10.0, logF); const float x=rA.getX() + (float) i/(float)(N-1)*rA.getWidth(); const float y=mapDbToY (bandDbAtForPaint (b, (float) hz)); if (i==0) ghost.startNewSubPath (x, y); else ghost.lineTo (x, y);} };
                // Predictive: shelves in low/high, bell elsewhere; sign by mouse Y
                if (hoverHz <= 50.0f)
                {
                    makeGhost (1 /*LowShelf*/, mouseAbove0 ? +3.0f : -3.0f);
                }
                else if (hoverHz >= 10000.0f)
                {
                    makeGhost (2 /*HighShelf*/, mouseAbove0 ? +3.0f : -3.0f);
                }
                else
                {
                    makeGhost (0 /*Bell*/, mouseAbove0 ? +3.0f : -3.0f);
                }
                // Radial fade around cursor to softly reveal only local part
                juce::Path clipped; clipped.addEllipse ((float) hoverPos.x - rA.getWidth()*0.05f, (float) hoverPos.y - rA.getHeight()*0.15f, rA.getWidth()*0.10f, rA.getHeight()*0.30f);
                juce::Graphics::ScopedSaveState ss (g);
                g.reduceClipRegion (clipped);
                g.setColour (juce::Colours::white.withAlpha (0.16f));
                g.strokePath (ghost, juce::PathStrokeType (1.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
                }
            }
        }
        
        // Draw tooltip if active
        if (showTooltip && currentTooltipText.isNotEmpty())
        {
            // Position tooltip near the anchor point
            auto tooltipRect = currentTooltipRect.expanded (8, 8);
            tooltipRect = tooltipRect.translated (0, -tooltipRect.getHeight() - 8);
            
            // Ensure tooltip stays within bounds
            if (tooltipRect.getY() < getLocalBounds().getY())
                tooltipRect = tooltipRect.translated (0, tooltipRect.getHeight() + 16);
            if (tooltipRect.getX() < getLocalBounds().getX())
                tooltipRect = tooltipRect.translated (getLocalBounds().getX() - tooltipRect.getX(), 0);
            if (tooltipRect.getRight() > getLocalBounds().getRight())
                tooltipRect = tooltipRect.translated (getLocalBounds().getRight() - tooltipRect.getRight(), 0);
            
            // Draw tooltip background
            g.setColour (juce::Colours::black.withAlpha (0.85f));
            g.fillRoundedRectangle (tooltipRect.toFloat(), 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawRoundedRectangle (tooltipRect.toFloat(), 4.0f, 1.0f);
            
            // Draw tooltip text
            g.setColour (juce::Colours::white);
            g.setFont (12.0f);
            g.drawFittedText (currentTooltipText, tooltipRect, juce::Justification::centred, 1);
        }
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        
        // Position zoom slider on the left edge
        const int zoomWidth = 20;
        const int zoomMargin = 8;
        zoomSlider.setBounds (zoomMargin, r.getY() + 40, zoomWidth, r.getHeight() - 80);
        
        // Adjust analyzer bounds to account for zoom slider
        analyzer.setBounds (r.reduced (zoomWidth + zoomMargin * 2, 0));
        rebuildEqPath();
        positionOverlay();
    }

    // Analyzer control (PaneManager may wire these later)
    void setSampleRate (double sr) { analyzer.setSampleRate (sr); }
    void pause()  { analyzer.pauseAudio(); }
    void resume() { analyzer.resumeAudio(); }
    void pushBlock (const float* L, const float* R, int n)    { analyzer.pushBlock (L, R, n); }
    void pushBlockPre (const float* L, const float* R, int n) { analyzer.pushBlockPre (L, R, n); }

private:
    // Adaptive dB zoom (starts at ±6, expands to ±24, then full)
    float currentDbTop { 6.0f };
    float currentDbBottom { -6.0f };
    void adaptDbRangeToPoint (float db)
    {
        if (currentDbTop == 6.0f && currentDbBottom == -6.0f)
        {
            if (db > 6.0f || db < -6.0f) { currentDbTop = 18.0f; currentDbBottom = -18.0f; }
        }
        if (currentDbTop == 18.0f && currentDbBottom == -18.0f)
        {
            if (db > 18.0f || db < -18.0f) { currentDbTop = 18.0f; currentDbBottom = -36.0f; }
        }
    }
    
    void updateZoomFromSlider()
    {
        const float zoomValue = (float) zoomSlider.getValue();
        if (zoomValue <= 0.5f) {
            // ±6dB zoom
            currentDbTop = 6.0f;
            currentDbBottom = -6.0f;
        } else if (zoomValue <= 1.5f) {
            // ±18dB zoom
            currentDbTop = 18.0f;
            currentDbBottom = -18.0f;
        } else {
            // ±36dB zoom
            currentDbTop = 18.0f;
            currentDbBottom = -36.0f;
        }
        rebuildEqPath();
        repaint();
    }
    struct BandPoint { float hz=1000.f; float db=0.f; float q=0.707f; int type=0; int phase=1; int channel=0; int bandIdx=-1; bool dynOn=false; bool specOn=false; int slopeDb=12; int tapMode=1; };
    std::vector<BandPoint> points;
    int selected { -1 };
    int hover { -1 };
    bool hoverInPane { false };
    juce::Point<int> hoverPos { 0, 0 };
    float hoverHz { 0.0f };
    juce::int64 lastMouseMoveMs { 0 };
    int ghostDelayMs { 220 };
    int badgeFor { -1 };
    juce::Path eqPath;
    std::vector<juce::Path> bandPaths;
    std::vector<juce::Path> bandAreas;
    std::vector<juce::Path> bandDynPaths;
    std::vector<juce::Path> bandDynRegions;
    std::vector<juce::Path> bandSpecPaths;
    std::vector<juce::Path> bandSpecRegions;

    // Floating band editor overlay
    class BandOverlay : public juce::Component
    {
    public:
        std::function<void(float)> onGainChanged;
        std::function<void(float)> onQChanged;
        std::function<void(float)> onFreqChanged;
        std::function<void(int)>   onTypeChanged;
        std::function<void(int)>   onPhaseChanged;
        std::function<void(int)>   onChanChanged;
        std::function<void(bool)>  onDynChanged;
        std::function<void(bool)>  onSpecChanged;
        std::function<void(bool)>  onDragAny; // notify begin/end of any slider drag
        BandOverlay()
        {
            setInterceptsMouseClicks (true, true);
            gain.setSliderStyle (juce::Slider::LinearHorizontal);
            gain.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);
            gain.setRange (-24.0, 24.0, 0.1);
            gain.onValueChange = [this]{ if (!updating && onGainChanged) onGainChanged ((float) gain.getValue()); };
            gain.onDragStart = [this]{ if (onDragAny) onDragAny (true); };
            gain.onDragEnd   = [this]{ if (onDragAny) onDragAny (false); };
            addAndMakeVisible (gain);

            q.setSliderStyle (juce::Slider::LinearHorizontal);
            q.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);
            q.setRange (0.1, 36.0, 0.01);
            q.onValueChange = [this]{ if (!updating && onQChanged) onQChanged ((float) q.getValue()); };
            q.onDragStart = [this]{ if (onDragAny) onDragAny (true); };
            q.onDragEnd   = [this]{ if (onDragAny) onDragAny (false); };
            addAndMakeVisible (q);

            freq.setSliderStyle (juce::Slider::LinearHorizontal);
            freq.setTextBoxStyle (juce::Slider::TextBoxRight, false, 64, 18);
            freq.setRange (20.0, 20000.0, 0.01);
            freq.setSkewFactorFromMidPoint (1000.0);
            freq.onValueChange = [this]{ if (!updating && onFreqChanged) onFreqChanged ((float) freq.getValue()); };
            freq.onDragStart = [this]{ if (onDragAny) onDragAny (true); };
            freq.onDragEnd   = [this]{ if (onDragAny) onDragAny (false); };
            addAndMakeVisible (freq);

            gainLabel.setText ("GAIN", juce::dontSendNotification);
            gainLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (gainLabel);

            qLabel.setText ("Q", juce::dontSendNotification);
            qLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (qLabel);

            freqLabel.setText ("FREQ", juce::dontSendNotification);
            freqLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (freqLabel);

            // Type icon acts as trigger for popup menu; we hide the combo for space
            addAndMakeVisible (typeIcon);
            typeIcon.onClick = [this]{ showTypeMenuToggle(); };
            typeCb.addItemList (juce::StringArray{ "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" }, 1);
            typeCb.setVisible (false);

            phaseCb.addItemList (juce::StringArray{ "Zero","Natural","Linear" }, 1);
            phaseCb.onChange = [this]{ if (!updating && onPhaseChanged) onPhaseChanged (phaseCb.getSelectedItemIndex()); };
            addAndMakeVisible (phaseCb);

            chanLabel.setText ("CHAN", juce::dontSendNotification);
            chanLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (chanLabel);
            chanCb.addItemList (juce::StringArray{ "Stereo","Mid","Side","Left","Right" }, 1);
            chanCb.onChange = [this]{ if (!updating && onChanChanged) onChanChanged (chanCb.getSelectedItemIndex()); };
            addAndMakeVisible (chanCb);

            // Dynamic / Spectral toggles
            dynToggle.setButtonText ("DYN");
            dynToggle.onClick = [this]{ if (!updating && onDynChanged) onDynChanged (dynToggle.getToggleState()); };
            addAndMakeVisible (dynToggle);
            specToggle.setButtonText ("SPEC");
            specToggle.onClick = [this]{ if (!updating && onSpecChanged) onSpecChanged (specToggle.getToggleState()); };
            addAndMakeVisible (specToggle);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Lighter dark background for overlay
            juce::Colour bg = juce::Colours::darkgrey.darker (0.20f);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) bg = lf->theme.shadowDark.brighter (0.20f);
            g.setColour (bg.withAlpha (0.96f));
            g.fillRoundedRectangle (r, 6.0f);
            // Subtle outline (slightly stronger)
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
            // Accent vertical strip on the left (bottom-to-top gradient)
            juce::Rectangle<float> strip = r.removeFromLeft (2.0f).reduced (0.5f, 1.5f);
            // Top-down gradient for border accent
            juce::Colour a0 = overlayAccent.withAlpha (0.36f);
            juce::Colour a1 = overlayAccent.withAlpha (0.12f);
            juce::ColourGradient grad (a0, strip.getCentreX(), strip.getY(), a1, strip.getCentreX(), strip.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRect (strip);
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced (8);
            auto row = r.removeFromTop (22);
            gainLabel.setBounds (row.removeFromLeft (40));
            gain.setBounds (row);
            r.removeFromTop (6);
            row = r.removeFromTop (22);
            qLabel.setBounds (row.removeFromLeft (40));
            q.setBounds (row);
            r.removeFromTop (6);
            row = r.removeFromTop (22);
            freqLabel.setBounds (row.removeFromLeft (40));
            freq.setBounds (row);

            r.removeFromTop (8);
            auto half = r.removeFromTop (24);
            typeIcon.setBounds (half.removeFromLeft (28));
            // Expand remaining controls into freed space
            phaseCb.setBounds (half.removeFromLeft (160));
            dynToggle.setBounds (half.removeFromLeft (64));
            specToggle.setBounds (half.removeFromLeft (64));

            auto half2 = r.removeFromTop (24);
            chanLabel.setBounds (half2.removeFromLeft (40));
            chanCb.setBounds (half2.removeFromLeft (120));
        }
        void setValues (float gainDb, float qVal, float freqHz, int typeIdx, int phaseIdx, int chanIdx, bool dynOn, bool specOn)
        {
            juce::ScopedValueSetter<bool> sv (updating, true);
            gain.setValue (gainDb, juce::dontSendNotification);
            q.setValue (qVal, juce::dontSendNotification);
            freq.setValue (freqHz, juce::dontSendNotification);
            typeCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, typeCb.getNumItems()-1), typeIdx), juce::dontSendNotification);
            phaseCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, phaseCb.getNumItems()-1), phaseIdx), juce::dontSendNotification);
            chanCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, chanCb.getNumItems()-1), chanIdx), juce::dontSendNotification);
            typeIcon.setType (typeIdx);
            dynToggle.setToggleState (dynOn, juce::dontSendNotification);
            specToggle.setToggleState (specOn, juce::dontSendNotification);
        }
        void setAccentColour (juce::Colour c) { overlayAccent = c; repaint(); }
    private:
        juce::Slider gain, q, freq;
        juce::Label gainLabel, qLabel, freqLabel, /*typeLabel, phaseLabel,*/ chanLabel;
        juce::ComboBox typeCb, phaseCb, chanCb;
        juce::ToggleButton dynToggle, specToggle;
        struct SmallCurveIcon : public juce::Component {
            int type { 0 };
            bool hovered { false };
            std::function<void()> onClick;
            void setType (int t){ type = t; repaint(); }
            void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
            void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
            void mouseUp    (const juce::MouseEvent&) override { if (onClick) onClick(); }
            void paint (juce::Graphics& g) override {
                auto r = getLocalBounds().toFloat();
                juce::Colour col = juce::Colours::white.withAlpha (0.75f);
                if (hovered)
                {
                    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) col = lf->theme.accent.withAlpha (0.95f);
                }
                g.setColour (col);
                juce::Path p; const int N = 28;
                auto mapx = [&](int i){ return r.getX() + (float) i / (float) (N-1) * r.getWidth(); };
                auto mapy = [&](float v01){ return juce::jmap (v01, 0.0f, 1.0f, r.getBottom(), r.getY()); };
                auto shape = [&](float t){
                    if (type == 0) return 0.5f + 0.35f * std::sin ((t-0.5f) * juce::MathConstants<float>::pi);
                    if (type == 1) return 0.35f + 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.45f)));
                    if (type == 2) return 0.65f - 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.55f)));
                    if (type == 3) return juce::jlimit (0.0f,1.0f, (t*1.6f));
                    if (type == 4) return juce::jlimit (0.0f,1.0f, 1.0f-(t*1.6f));
                    if (type == 5) return 0.5f + 0.45f * std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi);
                    if (type == 6) return 0.5f + 0.45f * std::abs (std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi));
                    return 0.5f;
                };
                p.startNewSubPath (mapx(0), mapy (shape(0)));
                for (int i=1;i<N;++i) p.lineTo (mapx(i), mapy (shape((float) i/(N-1))));
                g.strokePath (p, juce::PathStrokeType (1.2f));
            }
        } typeIcon;
        bool updating { false };
        bool typeMenuOpen { false };
        void showTypeMenuToggle()
        {
            if (typeMenuOpen) { juce::PopupMenu::dismissAllActiveMenus(); typeMenuOpen = false; return; }
            juce::PopupMenu m; juce::StringArray names { "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" };
            for (int i = 0; i < names.size(); ++i) m.addItem (i+1, names[i]);
            typeMenuOpen = true;
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                             [this](int r)
                             {
                                 typeMenuOpen = false;
                                 if (r > 0 && !updating)
                                 {
                                     const int idx = r-1;
                                     typeIcon.setType (idx);
                                     if (onTypeChanged) onTypeChanged (idx);
                                 }
                             });
        }
        juce::Colour overlayAccent { juce::Colours::white.withAlpha (0.4f) };
    } overlay;

    // Compact per-band badge (GR, freq, type, delete, bypass)
    class BandBadge : public juce::Component
    {
    public:
        std::function<void()> onDelete;
        std::function<void(bool)> onBypass;
        std::function<void(int)> onSetType;
        std::function<void(int)> onSetSlopeDb;
        std::function<void(int)> onSetTapMode;
        std::function<void()> onToggleDyn;
        std::function<void()> onToggleSpec;
        std::function<void(float)> onSetFreq;
        std::function<void(float)> onSetQ;
        std::function<void(float)> onSetGainDb;
        std::function<void(float)> onSetDynRangeDb;
        void setValues (float grDb, float freqHz, int typeIdx, bool bypass)
        {
            gainReductionDb = grDb; freq = freqHz; type = typeIdx; bypassed = bypass; repaint();
        }
        void setAccentColour (juce::Colour c) { badgeAccent = c; repaint(); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Lighter dark background for badge
            juce::Colour bg = juce::Colours::darkgrey.darker (0.22f);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) bg = lf->theme.shadowDark.brighter (0.18f);
            g.setColour (bg.withAlpha (0.96f));
            g.fillRoundedRectangle (r, 5.0f);
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawRoundedRectangle (r, 5.0f, 0.9f);
            // Accent vertical strip on the left
            juce::Rectangle<float> strip = r.withX (r.getX()).withWidth (2.0f).reduced (0.5f, 1.5f);
            // Top-down gradient for border accent
            juce::Colour a0 = badgeAccent.withAlpha (0.32f);
            juce::Colour a1 = badgeAccent.withAlpha (0.10f);
            juce::ColourGradient grad (a0, strip.getCentreX(), strip.getY(), a1, strip.getCentreX(), strip.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRect (strip);
            auto area = r.reduced (6.0f);
            auto row1 = area.removeFromTop (16.0f);
            auto row2 = area.removeFromTop (16.0f);

            // Row 1: Type glyph | FREQ | Q | GAIN | GR | (spacer) | Power | X
            g.setColour (juce::Colours::white.withAlpha (0.90f));
            g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));

            typeRect = row1.removeFromLeft (24).toNearestInt();
            // clickable type glyph with hover accent
            bool overType = typeRect.contains (getMouseXYRelative());
            juce::Colour glyphCol = juce::Colours::white.withAlpha (0.75f);
            if (overType)
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) glyphCol = lf->theme.accent.withAlpha (0.95f);
            drawTypeGlyphWithColour (g, typeRect.toFloat(), glyphCol);
            // invisible popup trigger areas: we handle showing menus in mouseDown to avoid accidental drags

            auto cell = [&](juce::Rectangle<float>& row, const juce::String& label){ auto c = row.removeFromLeft (48.0f); g.drawText (label, c, juce::Justification::centredLeft); return c.toNearestInt(); };
            juce::String f;
            if (freq >= 1000.f && freq < 10000.f) f = juce::String (freq / 1000.0f, 1) + "k";
            else if (freq >= 10000.f) f = juce::String ((int) std::round (freq/1000.f)) + "k";
            else f = juce::String ((int) freq);
            freqRect = cell (row1, f + " Hz");
            qRect    = cell (row1, juce::String (qVal, 2));
            gainRect = cell (row1, juce::String (gainDb, 1) + " dB");
            grRect   = cell (row1, juce::String (gainReductionDb, 1) + " dB");

            row1.removeFromLeft (4.0f);
            powerRect = row1.removeFromRight (18.0f).toNearestInt();
            xRect     = row1.removeFromRight (18.0f).toNearestInt();
            IconSystem::drawIcon (g, IconSystem::Power, powerRect.toFloat(), bypassed ? juce::Colours::orange : juce::Colours::white.withAlpha (0.90f));
            IconSystem::drawIcon (g, IconSystem::X,     xRect.toFloat(), juce::Colours::white.withAlpha (0.90f));

            // Row 2 chips: Dyn | Spec | Chan | Slope | Tap
            g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")));
            auto chip = [&](juce::Rectangle<float>& row, const juce::String& txt, juce::Colour col){ auto c = row.removeFromLeft (54.0f).reduced (1.0f); g.setColour (col.withAlpha (0.22f)); g.fillRoundedRectangle (c, 4.0f); g.setColour (col.withAlpha (0.85f)); g.drawRoundedRectangle (c, 4.0f, 1.0f); g.drawText (txt, c, juce::Justification::centred); return c.toNearestInt(); };
            auto col = juce::Colours::white;
            dynRect   = chip (row2, dynUp ? (juce::String ("Dyn ") + (dynUp?"Up":"Dn") + " " + juce::String (dynRangeDb,1)+"dB") : (juce::String ("Dyn ") + (dynUp?"Up":"Dn")), col);
            specRect  = chip (row2, specOn ? juce::String ("Spec ON") : juce::String ("Spec"), col);
            chanRect  = chip (row2, chanLabel, col);
            slopeRect = chip (row2, juce::String (slopeDb) + " dB", col);
            tapRect   = chip (row2, tapLabel, col);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            if (xRect.contains (e.getPosition())) { if (onDelete) onDelete(); }
            else if (powerRect.contains (e.getPosition())) { bypassed = !bypassed; if (onBypass) onBypass (bypassed); repaint(); }
            else if (dynRect.contains (e.getPosition())) { if (onToggleDyn) onToggleDyn(); }
            else if (specRect.contains (e.getPosition())) { if (onToggleSpec) onToggleSpec(); }
            else if (typeRect.contains (e.getPosition())) { showTypeMenu(); }
            else if (slopeRect.contains (e.getPosition())) { showSlopeMenu(); }
            else if (tapRect.contains (e.getPosition())) { showTapMenu(); }
        }
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (typeRect.contains (e.getPosition())) { showTypeMenu(); return; }
            if (slopeRect.contains (e.getPosition())) { showSlopeMenu(); return; }
            if (tapRect.contains (e.getPosition())) { showTapMenu(); return; }
        }
        void showTypeMenu()
        {
            if (!onSetType) return; juce::PopupMenu m; juce::StringArray names { "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" };
            for (int i = 0; i < names.size(); ++i) m.addItem (i+1, names[i]);
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this), [this](int r){ if (r > 0 && onSetType) onSetType (r-1); });
        }
        void showSlopeMenu()
        {
            if (!onSetSlopeDb) return; juce::PopupMenu m; int slopes[] = {6,12,18,24,36,48,72,96};
            for (int i=0;i<8;++i) m.addItem (i+1, juce::String (slopes[i]) + " dB/oct");
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this), [this](int r){ if (r > 0 && onSetSlopeDb) { int slopes[] = {6,12,18,24,36,48,72,96}; onSetSlopeDb (slopes[r-1]); } });
        }
        void showTapMenu()
        {
            if (!onSetTapMode) return; juce::PopupMenu m; m.addItem (1, "Pre XY"); m.addItem (2, "Post XY"); m.addItem (3, "External");
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this), [this](int r){ if (r > 0 && onSetTapMode) onSetTapMode (r-1); });
        }
        void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
        {
            const float d = (float) wheel.deltaY; const bool fast = e.mods.isShiftDown();
            if (freqRect.contains (e.getPosition())) { float ratio = fast ? 1.0f + d * 0.25f : 1.0f + d * 0.08f; ratio = juce::jlimit (0.5f, 2.0f, ratio); if (onSetFreq) onSetFreq (juce::jlimit (20.0f, 20000.0f, freq * ratio)); return; }
            if (qRect.contains (e.getPosition()))    { float nf = juce::jlimit (0.1f, 36.0f, qVal * (1.0f + d * (fast ? 0.50f : 0.15f))); if (onSetQ) onSetQ (nf); return; }
            if (gainRect.contains (e.getPosition())) { float step = fast ? 1.0f : 0.2f; if (onSetGainDb) onSetGainDb (juce::jlimit (-24.0f, 24.0f, gainDb + d * step)); return; }
            if (grRect.contains (e.getPosition()))   { float step = fast ? 1.0f : 0.2f; if (onSetDynRangeDb) onSetDynRangeDb (juce::jlimit (-24.0f, 24.0f, dynRangeDb + d * step)); return; }
            if (slopeRect.contains (e.getPosition())) { static const int steps[] = {6,12,18,24,36,48,72,96}; int idx=0; for (int i=0;i<8;++i) if (steps[i]==slopeDb){idx=i;break;} idx = juce::jlimit (0,7, idx + (d>0?1:-1)); if (onSetSlopeDb) onSetSlopeDb (steps[idx]); return; }
            if (tapRect.contains (e.getPosition()))   { int nm = juce::jlimit (0,2, (tapLabel=="Pre"?0:tapLabel=="Post"?1:2) + (d>0?1:-1)); if (onSetTapMode) onSetTapMode (nm); return; }
            if (typeRect.contains (e.getPosition()))  { int nt = juce::jlimit (0,6, type + (d>0?1:-1)); if (onSetType) onSetType (nt); return; }
        }
        void setDetails (float q, float gDb, bool dyn_on, bool dyn_up, float dyn_range, bool spec_on,
                         const juce::String& ch, int slope_db, const juce::String& tap)
        { qVal = q; gainDb = gDb; dynOn = dyn_on; dynUp = dyn_up; dynRangeDb = dyn_range; specOn = spec_on; chanLabel = ch; slopeDb = slope_db; tapLabel = tap; }
    private:
        void drawTypeGlyphWithColour (juce::Graphics& g, juce::Rectangle<float> r, juce::Colour c)
        {
            g.setColour (c);
            juce::Path p; const int N = 20;
            auto mapx = [&](int i){ return r.getX() + (float) i / (float) (N-1) * r.getWidth(); };
            auto mapy = [&](float v01){ return juce::jmap (v01, 0.0f, 1.0f, r.getBottom(), r.getY()); };
            auto shape = [&](float t){
                if (type == 0) return 0.5f + 0.35f * std::sin ((t-0.5f) * juce::MathConstants<float>::pi);
                if (type == 1) return 0.35f + 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.45f)));
                if (type == 2) return 0.65f - 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.55f)));
                if (type == 3) return juce::jlimit (0.0f,1.0f, (t*1.6f));
                if (type == 4) return juce::jlimit (0.0f,1.0f, 1.0f-(t*1.6f));
                if (type == 5) return 0.5f + 0.45f * std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi);
                if (type == 6) return 0.5f + 0.45f * std::abs (std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi));
                return 0.5f;
            };
            p.startNewSubPath (mapx(0), mapy (shape(0)));
            for (int i=1;i<N;++i) p.lineTo (mapx(i), mapy (shape((float) i/(N-1))));
            g.strokePath (p, juce::PathStrokeType (1.2f));
        }
        float gainReductionDb { 0.0f }, freq { 1000.0f }; int type { 0 }; bool bypassed { false };
        float qVal { 0.707f }, gainDb { 0.0f };
        bool dynOn { false }, dynUp { false }, specOn { false };
        float dynRangeDb { 0.0f };
        juce::String chanLabel { "St" }; int slopeDb { 12 }; juce::String tapLabel { "Post" };
        juce::Rectangle<int> powerRect, xRect, typeRect, freqRect, qRect, gainRect, grRect;
        juce::Rectangle<int> dynRect, specRect, chanRect, slopeRect, tapRect;
        juce::Colour badgeAccent { juce::Colours::white.withAlpha (0.4f) };
    } badge;

    // Simple mapping helpers (20..20k Hz, -36..+18 dB)
    float mapHzToX (float hz) const
    {
        auto r = analyzer.getBounds().toFloat();
        const float minHz = 20.f, maxHz = 20000.f;
        const float t = (float) (std::log10 (juce::jlimit (minHz, maxHz, hz) / minHz) / std::log10 (maxHz / minHz));
        return r.getX() + t * r.getWidth();
    }
    float mapDbToY (float dB) const
    {
        auto r = analyzer.getBounds().toFloat();
        const float top = r.getY()+8.f, bottom = r.getBottom()-8.f;
        return juce::jmap (dB, currentDbTop, currentDbBottom, top, bottom);
    }
    float mapXToHz (int px) const
    {
        auto r = analyzer.getBounds();
        const float minHz = 20.f, maxHz = 20000.f;
        const float t = juce::jlimit (0.0f, 1.0f, (px - (float) r.getX()) / (float) r.getWidth());
        const float a = std::log10 (minHz), b = std::log10 (maxHz);
        return std::pow (10.0f, juce::jmap (t, 0.0f, 1.0f, a, b));
    }
    float mapYToDb (int py) const
    {
        auto r = analyzer.getBounds();
        return juce::jmap ((float) py, (float) r.getY(), (float) r.getBottom(), currentDbTop, currentDbBottom);
    }

    int hitTestPoint (juce::Point<int> p) const
    {
        const float radius = 12.0f;
        for (int i = (int) points.size()-1; i >= 0; --i)
        {
            const float x = mapHzToX (points[(size_t) i].hz);
            const float y = mapDbToY (points[(size_t) i].db);
            if (juce::Point<float> (x, y).getDistanceFrom (p.toFloat()) <= radius)
                return i;
        }
        return -1;
    }

    int hitTestDynHandle (juce::Point<int> p) const
    {
        if (bandDynPaths.size() != points.size()) return -1;
        for (int i = (int) points.size()-1; i >= 0; --i)
        {
            const auto& pt = points[(size_t) i];
            if (!pt.dynOn && i != selected) continue; // allow selected band even if dyn off
            const float cx = mapHzToX (pt.hz);
            const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
            const float signedRange = (ptDynModeUp (i) ? +1.0f : -1.0f) * ptDynRangeDb (i);
            const float offsetY = mapDbToY (signedRange + 18.0f) - mapDbToY (18.0f);
            const float cy = baseY + offsetY;
            if (juce::Point<float> (cx, cy).getDistanceFrom (p.toFloat()) <= 10.0f)
                return i;
        }
        return -1;
    }

    int hitTestSpecHandle (juce::Point<int> p) const
    {
        if (bandSpecPaths.size() != points.size()) return -1;
        for (int i = (int) points.size()-1; i >= 0; --i)
        {
            const auto& pt = points[(size_t) i];
            if (!pt.specOn && i != selected) continue; // allow selected band even if spec off
            const float cx = mapHzToX (pt.hz);
            const float baseY = mapDbToY (bandDbAtForPaint (pt, pt.hz));
            const float specRange = ptSpecRangeDb (i);
            const float offsetY = mapDbToY (-specRange + 18.0f) - mapDbToY (18.0f);
            const float cy = baseY + offsetY;
            if (juce::Point<float> (cx, cy).getDistanceFrom (p.toFloat()) <= 10.0f)
                return i;
        }
        return -1;
    }

    void rebuildEqPath()
    {
        eqPath.clear();
        bandPaths.clear();
        bandAreas.clear();
        bandDynPaths.clear();
        bandDynRegions.clear();
        bandSpecPaths.clear();
        bandSpecRegions.clear();
        auto r = analyzer.getBounds().toFloat();
        if (r.isEmpty()) return;

        const int N = juce::jmax (128, (int) r.getWidth());
        auto bandDbAt = [this](const BandPoint& b, double hz)
        {
            const double logHz = std::log10 (juce::jlimit (20.0, 20000.0, hz));
            const double logC  = std::log10 (juce::jlimit (20.0f, 20000.0f, b.hz));
            const double q     = juce::jlimit (0.1, 36.0, (double) b.q);
            const double width = juce::jlimit (0.02, 0.50, 0.22 / q);
            const double d     = (logHz - logC) / width;
            switch (b.type)
            {
                case 0: { // Bell
                    const float w = (float) std::exp (-0.5 * d * d);
                    return b.db * w;
                }
                case 1: { // LowShelf
                    const double k = 8.0 * juce::jlimit (0.2, 3.0, q * 0.25);
                    const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
                    return (float) (b.db * s);
                }
                case 2: { // HighShelf
                    const double k = 8.0 * juce::jlimit (0.2, 3.0, q * 0.25);
                    const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC)));
                    return (float) (b.db * (1.0 - s));
                }
                case 3: { // HP
                    const double n = 2.0; // ~12 dB/oct
                    const double fc = std::pow (10.0, logC);
                    const double ratio = juce::jlimit (1e-6, 1e6, fc / juce::jlimit (20.0, 20000.0, hz));
                    const double att = -std::abs ((double) b.db <= 0.01 ? 24.0 : (double) b.db);
                    const double mag = 1.0 / std::sqrt (1.0 + std::pow (ratio, 2.0 * n));
                    return (float) (att * (1.0 - mag));
                }
                case 4: { // LP
                    const double n = 2.0;
                    const double fc = std::pow (10.0, logC);
                    const double ratio = juce::jlimit (1e-6, 1e6, juce::jlimit (20.0, 20000.0, hz) / fc);
                    const double att = -std::abs ((double) b.db <= 0.01 ? 24.0 : (double) b.db);
                    const double mag = 1.0 / std::sqrt (1.0 + std::pow (ratio, 2.0 * n));
                    return (float) (att * (1.0 - mag));
                }
                case 5: { // Notch
                    const float depth = -std::abs (b.db);
                    const float w = (float) std::exp (-0.5 * d * d);
                    return depth * w;
                }
                case 6: { // BandPass
                    const float w = (float) std::exp (-0.5 * d * d);
                    return std::abs (b.db) * w;
                }
                default: // AllPass
                    return 0.0f;
            }
        };
        auto totalDbAt = [&](double hz)
        {
            float s = 0.0f;
            for (const auto& b : points) s += bandDbAt (b, hz);
            return s;
        };

        auto mapX = [&](int i)
        {
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double) i / (double) (N - 1);
            const double a = std::log10 (minHz), b = std::log10 (maxHz);
            const double logF = juce::jmap (t, 0.0, 1.0, a, b);
            const double hz = std::pow (10.0, logF);
            return std::pair<float,float> ((float) hz, mapDbToY (totalDbAt (hz)));
        };

        auto p0 = mapX (0); eqPath.startNewSubPath (r.getX(), p0.second);
        for (int i = 1; i < N; ++i)
        {
            auto p = mapX (i);
            const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
            eqPath.lineTo (x, p.second);
        }

        // Per-band paths
        bandPaths.resize (points.size());
        bandAreas.resize (points.size());
        bandDynPaths.resize (points.size());
        bandDynRegions.resize (points.size());
        bandSpecPaths.resize (points.size());
        bandSpecRegions.resize (points.size());
        for (size_t bi = 0; bi < points.size(); ++bi)
        {
            auto& bp = bandPaths[bi];
            auto& ba = bandAreas[bi];
            auto& bd = bandDynPaths[bi];
            auto& br = bandDynRegions[bi];
            auto& sp = bandSpecPaths[bi];
            auto& sr = bandSpecRegions[bi];
            auto mapBand = [&](int i){
                const double minHz = 20.0, maxHz = 20000.0;
                const double t = (double) i / (double) (N - 1);
                const double a = std::log10 (minHz), b = std::log10 (maxHz);
                const double logF = juce::jmap (t, 0.0, 1.0, a, b);
                const double hz = std::pow (10.0, logF);
                return std::pair<float,float> ((float) hz, mapDbToY (bandDbAt (points[bi], hz)));
            };
            auto q0 = mapBand (0); bp.startNewSubPath (r.getX(), q0.second);
            ba.startNewSubPath (r.getX(), r.getBottom());
            ba.lineTo (r.getX(), q0.second);
            bd.startNewSubPath (r.getX(), q0.second);
            sp.startNewSubPath (r.getX(), q0.second);
            sr.startNewSubPath (r.getX(), q0.second);
            br.clear();
            br.startNewSubPath (r.getX(), q0.second);
            for (int i = 1; i < N; ++i)
            {
                auto q = mapBand (i);
                const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
                bp.lineTo (x, q.second);
                ba.lineTo (x, q.second);
                // dynamic offset curve (gaussian around band center, scaled by dynRange)
                const auto& bpt = points[bi];
                const double minHz = 20.0, maxHz = 20000.0;
                const double t = (double) i / (double) (N - 1);
                const double logF = juce::jmap (t, std::log10 (minHz), std::log10 (maxHz));
                const double hz   = std::pow (10.0, logF);
                const double logHz= std::log10 (juce::jlimit (20.0, 20000.0, hz));
                const double logC = std::log10 (juce::jlimit (20.0f, 20000.0f, bpt.hz));
                const double qv   = juce::jlimit (0.1, 36.0, (double) bpt.q);
                const double width= juce::jlimit (0.02, 0.50, 0.22 / qv);
                const double d    = (logHz - logC) / width;
                const float  w    = (float) std::exp (-0.5 * d * d);
                const float  range= ptDynRangeDb ((int) bi);
                const bool   up   = ptDynModeUp ((int) bi);
                const float  signedRange = (up ? +1.0f : -1.0f) * range * w;
                const float  baseY = q.second;
                const float  offsetY = mapDbToY (signedRange + 18.0f) - mapDbToY (18.0f);
                bd.lineTo (x, baseY + offsetY);
                br.lineTo (x, baseY + offsetY);

                // spectral attenuation visualization: similar shape, always downward, scaled by Spec Range param
                const float specAmt = bpt.specOn ? (w * ptSpecRangeDb ((int) bi)) : 0.0f;
                const float specOff = mapDbToY (-specAmt + 18.0f) - mapDbToY (18.0f);
                sp.lineTo (x, baseY + specOff);
                sr.lineTo (x, baseY + specOff);
            }
            ba.lineTo (r.getRight(), r.getBottom());
            ba.closeSubPath();
            // Close dynamic region back along the MACRO curve (reverse)
            for (int i = N-1; i >= 0; --i)
            {
                const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
                const double minHz = 20.0, maxHz = 20000.0;
                const double t = (double) i / (double) (N - 1);
                const double logF = juce::jmap (t, std::log10 (minHz), std::log10 (maxHz));
                const double hz = std::pow (10.0, logF);
                const float macroY = mapDbToY (totalDbAt (hz));
                br.lineTo (x, macroY);
            }
            br.closeSubPath();

            // Close spectral region back along the MACRO curve (reverse)
            for (int i = N-1; i >= 0; --i)
            {
                const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
                const double minHz = 20.0, maxHz = 20000.0;
                const double t = (double) i / (double) (N - 1);
                const double logF = juce::jmap (t, std::log10 (minHz), std::log10 (maxHz));
                const double hz = std::pow (10.0, logF);
                const float macroY = mapDbToY (totalDbAt (hz));
                sr.lineTo (x, macroY);
            }
            sr.closeSubPath();
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        selected = hitTestPoint (e.getPosition());
        // If no point hit and user clicked near a dynamic handle, select that band for range drag
        if (selected < 0)
        {
            const int idx = hitTestDynHandle (e.getPosition());
            if (idx >= 0) { selected = idx; draggingDynHandle = true; draggingSpecHandle = false; dragStartY = (float) e.getPosition().y; startDynRange = ptDynRangeDb (idx); dragDynModeUp = ptDynModeUp (idx); return; }
        }
        if (selected < 0)
        {
            const int idx = hitTestSpecHandle (e.getPosition());
            if (idx >= 0) { selected = idx; draggingSpecHandle = true; draggingDynHandle = false; dragStartY = (float) e.getPosition().y; startSpecRange = ptSpecRangeDb (idx); return; }
        }
        // Single-click creates a band when empty area is clicked
        if (selected < 0 && !e.mods.isPopupMenu())
        {
            BandPoint bp; bp.hz = juce::jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x)); bp.db = juce::jlimit (-24.f, 24.f, mapYToDb (e.getPosition().y));
            if (bp.hz <= 50.0f) { bp.type = 3; bp.db = -12.0f; }
            else if (bp.hz >= 10000.0f) { bp.type = 4; bp.db = -12.0f; }
            else { bp.type = 0; }
            adaptDbRangeToPoint (bp.db);
            const int slot = allocateBandSlot();
            if (slot >= 0)
            {
                bp.bandIdx = slot;
                setBandParam (slot, dynEq::Band::active, 1.0f);
                setBandParam (slot, dynEq::Band::freqHz, bp.hz);
                setBandParam (slot, dynEq::Band::gainDb, bp.db);
                setBandParam (slot, dynEq::Band::q, bp.q);
                setBandParam (slot, dynEq::Band::type, (float) bp.type);
                setBandParam (slot, dynEq::Band::phase, (float) bp.phase);
                setBandParam (slot, dynEq::Band::channel, (float) bp.channel);
            }
            points.push_back (bp);
            selected = (int) points.size() - 1;
            rebuildEqPath(); repaint();
        }
        if (e.mods.isPopupMenu())
        {
            juce::PopupMenu m;
            m.addItem (1, "Delete band", selected >= 0);
            m.addItem (2, "Reset Q", selected >= 0);
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                [this](int r)
                {
                    if (r == 1 && selected >= 0 && selected < (int) points.size()) { points.erase (points.begin() + selected); selected = -1; rebuildEqPath(); repaint(); }
                    if (r == 2 && selected >= 0 && selected < (int) points.size()) { points[(size_t) selected].q = 0.707f; rebuildEqPath(); repaint(); }
                });
            return;
        }

        // Show/update overlay when a band is selected
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            overlay.setValues (pt.db, pt.q, pt.hz, pt.type, pt.phase, pt.channel, pt.dynOn, pt.specOn);
            overlay.setVisible (true);
            positionOverlay();
            positionBadgeFor (selected);
        }
        else
        {
            overlay.setVisible (false);
            badge.setVisible (false);
        }
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (draggingDynHandle && selected >= 0 && selected < (int) points.size())
        {
            const float dy = (float) e.getPosition().y - dragStartY;
            const float dbPerPx = 0.10f; // sensitivity
            // When mode=Down (dragDynModeUp=false), dragging up (dy<0) should increase attenuation (more negative)
            // Flip sign accordingly so UX is intuitive: up = more effect regardless of mode
            float signedDy = -dy; // up negative -> positive effect
            float delta = signedDy * dbPerPx;
            float newRange = startDynRange + (dragDynModeUp ? delta : -delta);
            newRange = juce::jlimit (-24.0f, 24.0f, newRange);
            const int bi = selected;
            setBandParam (points[(size_t) bi].bandIdx, dynEq::Band::dynRangeDb, newRange);
            rebuildEqPath(); repaint();
            return;
        }
        if (draggingSpecHandle && selected >= 0 && selected < (int) points.size())
        {
            const float dy = (float) e.getPosition().y - dragStartY;
            const float dbPerPx = 0.10f;
            // Down drag increases attenuation (more downward), up reduces
            float delta = dy * dbPerPx;
            float newRange = juce::jlimit (0.0f, 24.0f, startSpecRange + delta);
            const int bi = selected;
            if (points[(size_t) bi].bandIdx >= 0)
                setBandParam (points[(size_t) bi].bandIdx, dynEq::Band::specRangeDb, newRange);
            rebuildEqPath(); repaint();
            return;
        }
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.hz = juce::jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x));
            pt.db = juce::jlimit (-24.f, 24.f, mapYToDb (e.getPosition().y));
            adaptDbRangeToPoint (pt.db);
            if (pt.bandIdx >= 0)
            {
                setBandParam (pt.bandIdx, dynEq::Band::freqHz, pt.hz);
                setBandParam (pt.bandIdx, dynEq::Band::gainDb, pt.db);
            }
            rebuildEqPath();
            repaint();
            overlay.setValues (pt.db, pt.q, pt.hz, pt.type, pt.phase, pt.channel, pt.dynOn, pt.specOn);
            positionOverlay();
            positionBadgeFor (selected);
        }
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        const int idx = hitTestPoint (e.getPosition());
        if (idx >= 0 && idx < (int) points.size())
        {
            // Double-click on an existing point deletes it
            const int bandIdx = points[(size_t) idx].bandIdx;
            if (bandIdx >= 0)
                setBandParam (bandIdx, dynEq::Band::active, 0.0f);
            points.erase (points.begin() + idx);
            if (selected == idx) selected = -1; else if (selected > idx) --selected;
            rebuildEqPath();
            repaint();
            if (selected < 0) overlay.setVisible (false); else { auto& pt2 = points[(size_t) selected]; overlay.setValues (pt2.db, pt2.q, pt2.hz, pt2.type, pt2.phase, pt2.channel, pt2.dynOn, pt2.specOn); positionOverlay(); }
            return;
        }

        // Double-click on empty area adds a new band
        BandPoint bp; bp.hz = juce::jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x)); bp.db = juce::jlimit (-24.f, 24.f, mapYToDb (e.getPosition().y));
        if (bp.hz <= 50.0f) { bp.type = 3; bp.db = -12.0f; }
        else if (bp.hz >= 10000.0f) { bp.type = 4; bp.db = -12.0f; }
        else { bp.type = 0; }
        adaptDbRangeToPoint (bp.db);
        const int slot = allocateBandSlot();
        if (slot >= 0)
        {
            bp.bandIdx = slot;
            setBandParam (slot, dynEq::Band::active, 1.0f);
            setBandParam (slot, dynEq::Band::freqHz, bp.hz);
            setBandParam (slot, dynEq::Band::gainDb, bp.db);
            setBandParam (slot, dynEq::Band::q, bp.q);
            setBandParam (slot, dynEq::Band::type, (float) bp.type);
            setBandParam (slot, dynEq::Band::phase, (float) bp.phase);
            setBandParam (slot, dynEq::Band::channel, (float) bp.channel);
        }
        points.push_back (bp);
        selected = (int) points.size() - 1;
        rebuildEqPath();
        repaint();
        if (selected >= 0) { auto& pt = points[(size_t) selected]; overlay.setValues (pt.db, pt.q, pt.hz, pt.type, pt.phase, pt.channel, pt.dynOn, pt.specOn); overlay.setVisible (true); positionOverlay(); positionBadgeFor (selected); }
    }

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            const float delta = (float) (wheel.deltaY * (e.mods.isShiftDown() ? 1.0 : 0.2));
            pt.q = juce::jlimit (0.1f, 36.0f, pt.q * (1.0f + delta));
            if (pt.bandIdx >= 0)
                setBandParam (pt.bandIdx, dynEq::Band::q, pt.q);
            rebuildEqPath();
            repaint();
            overlay.setValues (pt.db, pt.q, pt.hz, pt.type, pt.phase, pt.channel, pt.dynOn, pt.specOn);
            positionOverlay();
            positionBadgeFor (selected);
        }
    }

    void mouseUp (const juce::MouseEvent&) override
    {
        draggingDynHandle = false;
        draggingSpecHandle = false;
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int h = hitTestPoint (e.getPosition());
        if (h != hover)
        {
            hover = h;
            if (selected < 0)
            {
                if (hover >= 0) positionBadgeFor (hover);
                else badge.setVisible (false);
            }
            else
            {
                // When selected exists, allow hover to preview a different band's badge
                if (hover >= 0) positionBadgeFor (hover);
                else positionBadgeFor (selected);
            }
        }
        hoverPos = e.getPosition();
        auto r = analyzer.getBounds();
        hoverInPane = r.contains (hoverPos);
        if (hoverInPane)
            hoverHz = juce::jlimit (20.0f, 20000.0f, mapXToHz (hoverPos.x));
        lastMouseMoveMs = (juce::int64) juce::Time::getMillisecondCounterHiRes();
        
        // Tooltip support - check if we're over a control that needs tooltips
        updateTooltipForPosition (e.getPosition());
        
        repaint();
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        if (selected < 0) badge.setVisible (false);
        hover = -1;
        hoverInPane = false;
        repaint();
    }

    // Tooltip support for Dynamic EQ controls
    void updateTooltipForPosition (juce::Point<int> pos);
    
    // Simple tooltip display
    juce::String currentTooltipText;
    juce::Rectangle<int> currentTooltipRect;
    bool showTooltip = false;

    void positionOverlay()
    {
        if (! overlay.isVisible() || selected < 0 || selected >= (int) points.size()) return;
        if (overlayFrozen)
        {
            if (overlayLastBounds.isEmpty()) overlayLastBounds = overlay.getBounds();
            overlay.setBounds (overlayLastBounds);
            overlay.toFront (false);
            return;
        }
        const float x = mapHzToX (points[(size_t) selected].hz);
        auto pane = getLocalBounds();
        const int w = 360, h = 132;
        // Fixed Y higher up to avoid Hz units; X follows the point's latitude
        int oy = pane.getBottom() - h - 60; // Moved up 48px to clear Hz units
        // Center overlay around the point's X, clamped within pane
        int ox = (int) x - (w / 2);
        if (ox < pane.getX()) ox = pane.getX() + 12;
        if (ox + w > pane.getRight()) ox = pane.getRight() - w - 12;
        overlayLastBounds = juce::Rectangle<int> (ox, oy, w, h);
        overlay.setBounds (overlayLastBounds);
        overlay.toFront (false);
    }

    void positionBadgeFor (int idx)
    {
        if (idx < 0 || idx >= (int) points.size()) return;
        const auto& pt = points[(size_t) idx];
        const float x = mapHzToX (pt.hz);
        const float y = mapDbToY (pt.db);
        const int w = 212, h = 40;
        int ox = (int) x + 14, oy = (int) y - h - 8;
        auto pane = getLocalBounds();
        if (ox + w > pane.getRight()) ox = (int) x - w - 14;
        if (oy < pane.getY()) oy = (int) y + 8;
        badge.setBounds (ox, oy, w, h);
        badge.setVisible (true);
        // Approx details for badge
        float gr = 0.0f; bool dynUp = ptDynModeUp (pt.bandIdx >= 0 ? pt.bandIdx : 0);
        float range = ptDynRangeDb (pt.bandIdx >= 0 ? pt.bandIdx : 0);
        if (pt.dynOn) gr = std::abs (range);
        badge.setValues (gr, pt.hz, pt.type, false);
        juce::String tap = (pt.tapMode == 0 ? "Pre" : pt.tapMode == 1 ? "Post" : "Ext");
        badge.setDetails (pt.q, pt.db, pt.dynOn, dynUp, range, pt.specOn, channelLabel (pt.channel), pt.slopeDb, tap);
        // per-band accent on badge and overlay
        juce::Colour accent = applyChannelTint (bandColourFor (idx), pt.channel);
        badge.setAccentColour (accent);
        overlay.setAccentColour (accent);
        badge.toFront (true);
    }

    static juce::String channelLabel (int ch)
    {
        switch (ch) { case 1: return "M"; case 2: return "S"; case 3: return "L"; case 4: return "R"; default: return "St"; }
    }

    // Units and grid (lightweight, muted)
    void drawUnits (juce::Graphics& g)
    {
        auto r = analyzer.getBounds().toFloat();
        if (r.isEmpty()) return;
        g.setFont (12.0f);
        auto gridCol = juce::Colours::white.withAlpha (0.10f);
        auto textCol = juce::Colours::white.withAlpha (0.45f);
        g.setColour (gridCol);

        // dB ticks
        const float dbVals[] = { 18, 12, 6, 0, -6, -12, -18, -24, -30, -36 };
        for (float dbv : dbVals)
        {
            if (dbv > currentDbTop || dbv < currentDbBottom) continue;
            const float y = mapDbToY (dbv);
            g.setColour (gridCol);
            g.drawLine (r.getX(), y, r.getRight(), y, dbv == 0 ? 1.2f : 0.6f);
            g.setColour (textCol);
            juce::String lbl = juce::String ((int) dbv) + " dB";
            g.drawFittedText (lbl, juce::Rectangle<int> ((int) r.getX()+4, (int) y-8, 44, 16), juce::Justification::centredLeft, 1);
        }

        // Hz ticks (moved down slightly to avoid control bar)
        const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000 };
        for (double hz : hzTicks)
        {
            const float x = mapHzToX ((float) hz);
            g.setColour (gridCol);
            g.drawLine (x, r.getBottom()-16.0f, x, r.getBottom(), 0.8f); // Moved down 4px
            g.setColour (textCol);
            juce::String lbl;
            if (hz >= 1000.0 && hz < 10000.0) lbl = juce::String (hz/1000.0, 1) + "k";
            else if (hz >= 10000.0) lbl = juce::String ((int) std::round (hz/1000.0)) + "k";
            else lbl = juce::String ((int) hz);
            g.drawFittedText (lbl, juce::Rectangle<int> ((int) x-18, (int) r.getBottom()-30, 36, 14), juce::Justification::centred, 1); // Moved down 4px
        }
    }

    // --- Colour system (theme-driven) ---
    juce::Colour macroColour() const
    {
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            return lf->theme.accent.withAlpha (0.95f);
        return juce::Colours::cyan.withAlpha (0.95f);
    }

    // Lightweight helpers for dynamic visuals (read-only APVTS)
    float getBandParamFloat (int bandIdx, const char* baseId, float fallback) const
    {
        auto id = bandId (baseId, bandIdx);
        if (auto* v = proc.apvts.getRawParameterValue (id)) return v->load();
        return fallback;
    }
    // Approximate band dB at a given Hz using current visual model
    float bandDbAtForPaint (const BandPoint& b, float hz) const
    {
        const double logHz = std::log10 (juce::jlimit (20.0f, 20000.0f, hz));
        const double logC  = std::log10 (juce::jlimit (20.0f, 20000.0f, b.hz));
        const double q     = juce::jlimit (0.1, 36.0, (double) b.q);
        const double width = juce::jlimit (0.02, 0.50, 0.22 / q);
        const double d     = (logHz - logC) / width;
        switch (b.type)
        {
            case 0: { const float w = (float) std::exp (-0.5 * d * d); return b.db * w; }
            case 1: { const double k = 8.0 * juce::jlimit (0.2, 3.0, q * 0.25); const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC))); return (float) (b.db * s); }
            case 2: { const double k = 8.0 * juce::jlimit (0.2, 3.0, q * 0.25); const double s = 1.0 / (1.0 + std::exp (-k * (logHz - logC))); return (float) (b.db * (1.0 - s)); }
            case 3: { const double n = 2.0; const double fc = std::pow (10.0, logC); const double ratio = juce::jlimit (1e-6, 1e6, fc / juce::jlimit (20.0, 20000.0, (double) hz)); const double att = -std::abs ((double) b.db <= 0.01 ? 24.0 : (double) b.db); const double mag = 1.0 / std::sqrt (1.0 + std::pow (ratio, 2.0 * n)); return (float) (att * (1.0 - mag)); }
            case 4: { const double n = 2.0; const double fc = std::pow (10.0, logC); const double ratio = juce::jlimit (1e-6, 1e6, juce::jlimit (20.0, 20000.0, (double) hz) / fc); const double att = -std::abs ((double) b.db <= 0.01 ? 24.0 : (double) b.db); const double mag = 1.0 / std::sqrt (1.0 + std::pow (ratio, 2.0 * n)); return (float) (att * (1.0 - mag)); }
            case 5: { const float depth = -std::abs (b.db); const float w = (float) std::exp (-0.5 * d * d); return depth * w; }
            case 6: { const float w = (float) std::exp (-0.5 * d * d); return std::abs (b.db) * w; }
            default: return 0.0f;
        }
    }
    float ptDynRangeDb (int bandIdx) const { return getBandParamFloat (bandIdx, dynEq::Band::dynRangeDb, -3.0f); }
    bool  ptDynModeUp  (int bandIdx) const { return (int) std::round (getBandParamFloat (bandIdx, dynEq::Band::dynMode, 0.0f)) == 1; }
    float ptSpecRangeDb (int bandIdx) const { return getBandParamFloat (bandIdx, dynEq::Band::specRangeDb, 3.0f); }

    // Drag state for dynamic handle
    bool  draggingDynHandle { false };
    bool  draggingSpecHandle { false };
    bool  dragDynModeUp { false };
    float dragStartY { 0.0f };
    float startDynRange { 0.0f };
    float startSpecRange { 0.0f };

    // Overlay positioning freeze while dragging overlay sliders
    bool overlayFrozen { false };
    juce::Rectangle<int> overlayLastBounds;
    juce::Colour bandColourFor (int bandIdx) const
    {
        juce::Colour accent = juce::Colours::deepskyblue;
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            accent = lf->theme.accent;
        const float baseHue = accent.getHue();
        const float baseSat = juce::jlimit (0.25f, 0.95f, accent.getSaturation());
        const float baseBrt = juce::jlimit (0.35f, 0.95f, accent.getBrightness());
        const float golden = 0.61803398875f;
        float hue = std::fmod (baseHue + golden * (float) (bandIdx + 1), 1.0f);
        hue = juce::jlimit (0.0f, 1.0f, 0.65f * hue + 0.35f * baseHue);
        float sat = juce::jlimit (0.30f, 0.95f, baseSat * 0.9f + 0.1f);
        float brt = juce::jlimit (0.40f, 0.95f, baseBrt * 0.9f + 0.1f);
        return juce::Colour::fromHSV (hue, sat, brt, 1.0f);
    }
    static juce::Colour applyChannelTint (juce::Colour c, int channel)
    {
        // 0=Stereo,1=M,2=S,3=L,4=R
        switch (channel)
        {
            case 2: return c.withSaturation (juce::jlimit (0.0f, 1.0f, c.getSaturation() * 1.10f))
                             .withBrightness (juce::jlimit (0.0f, 1.0f, c.getBrightness() * 1.06f));
            case 3: return c.withHue (std::fmod (c.getHue() - 0.03f + 1.0f, 1.0f));
            case 4: return c.withHue (std::fmod (c.getHue() + 0.03f, 1.0f));
            default: return c;
        }
    }

    // ----- APVTS helpers -----
    static constexpr int kMaxBands = 24;
    static juce::String bandId (const char* base, int idx) { return juce::String (base) + "_" + juce::String (idx); }
    int allocateBandSlot()
    {
        for (int i = 0; i < kMaxBands; ++i)
        {
            auto id = bandId (dynEq::Band::active, i);
            if (auto* v = proc.apvts.getRawParameterValue (id))
            {
                if (v->load() < 0.5f)
                    return i;
            }
        }
        return -1;
    }
    void setBandParam (int bandIdx, const char* baseId, float value)
    {
        auto id = bandId (baseId, bandIdx);
        if (auto* p = proc.apvts.getParameter (id))
        {
            const float norm = p->convertTo0to1 (value);
            p->setValueNotifyingHost (norm);
        }
    }

    MyPluginAudioProcessor& proc;
    juce::LookAndFeel* lookAndFeelPtr { nullptr };
    SpectrumAnalyzer analyzer;
    
    // Zoom slider for manual dB range control
    juce::Slider zoomSlider;
};

// Tooltip implementation for Dynamic EQ controls
inline void DynEqTab::updateTooltipForPosition (juce::Point<int> pos)
{
    juce::String tooltipText;
    juce::Rectangle<int> anchorRect;
    
    // Check if we're over band points
    if (hover >= 0 && hover < (int) points.size())
    {
        const auto& pt = points[(size_t) hover];
        const float x = mapHzToX (pt.hz);
        const float y = mapDbToY (pt.db);
        anchorRect = juce::Rectangle<int> ((int) x - 8, (int) y - 8, 16, 16);
        tooltipText = "Drag to set Freq (X) and Gain (Y). Scroll to adjust Q";
    }
    // Check if we're over dynamic/spectral handles
    else if (selected >= 0 && selected < (int) points.size())
    {
        const auto& pt = points[(size_t) selected];
        const float x = mapHzToX (pt.hz);
        
        // Check dynamic handle
        if (pt.dynOn)
        {
            const float dynY = mapDbToY (pt.db + ptDynRangeDb (selected));
            if (std::abs (pos.x - (int) x) < 8 && std::abs (pos.y - (int) dynY) < 8)
            {
                anchorRect = juce::Rectangle<int> ((int) x - 8, (int) dynY - 8, 16, 16);
                tooltipText = "Drag the center handle to set ±dB dynamic range";
            }
        }
        
        // Check spectral handle
        if (pt.specOn)
        {
            const float specY = mapDbToY (pt.db - ptSpecRangeDb (selected));
            if (std::abs (pos.x - (int) x) < 8 && std::abs (pos.y - (int) specY) < 8)
            {
                anchorRect = juce::Rectangle<int> ((int) x - 8, (int) specY - 8, 16, 16);
                tooltipText = "Drag to set spectral attenuation range";
            }
        }
    }
    
    // Update tooltip state
    if (tooltipText.isNotEmpty())
    {
        currentTooltipText = tooltipText;
        currentTooltipRect = anchorRect;
        showTooltip = true;
    }
    else
    {
        showTooltip = false;
    }
}


