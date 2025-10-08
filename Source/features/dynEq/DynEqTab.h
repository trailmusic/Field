// Dev Note (2025-10-08): DynEqTab owns three surfaces: band curves (background),
// the floating BandOverlay (mini editor), and the BandBadge + Detector HUD pairing.
// Detector UX consolidation: HUD is the only editor for detector Source + SC HP/LP;
// overlay shows a single SC-summary chip that opens HUD; badge shows status and will
// open HUD when its detector entries are clicked. No APVTS reads on audio thread.
// Keep any detector parameter edits routed via openDetectorHUD → HUD callbacks.
#pragma once

#include <JuceHeader.h>
#include "shared/ui/Engines/SpectrumAnalyzer.h"
#include "features/dynEq/DynamicEqParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Controls/ZoomState.h"
#include "DynEqZoomSideRail.h"
#include "shared/ui/Components/BandDetectorHUDView.h"
#include "core/telemetry/DynEqTelemetry.h"

class MyPluginAudioProcessor; // fwd
class MyPluginAudioProcessorEditor; // fwd

// Dynamic EQ tab (replaces Spectrum). In-pane experience: visuals + editor.
// Scaffold component so we can integrate DSP/Editor incrementally.
enum class DetectorHUDFocus { Source, HP, LP };

class DynEqTab : public juce::Component, private juce::Timer
{
public:
    // Opens the per-band Detector HUD and optionally focuses a control
    void openDetectorHUD (int bandIdx, DetectorHUDFocus focus)
    {
        if (bandIdx < 0 || bandIdx >= (int) points.size()) return;
        selected = bandIdx;
        hudOpen = true;
        // Place badge + HUD
        positionBadgeFor (selected);
        // Ensure on top
        badge.toFront (true); detHud.toFront (true); hudButton.toFront (true);
    }
    DynEqTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p), lookAndFeelPtr (lnf), zoomState (), zoomRail (zoomState)
    {
        setOpaque (true);
        
        setLookAndFeel(lnf);
        
        startTimerHz (30);
        
        // Initialize zoom state
        zoomState.prepare (60.0);
        
        // Add zoom rail
        addAndMakeVisible (zoomRail);
        zoomRail.onZoomChanged = [this]
        {
            rebuildEqPath();
            if (selected >= 0) positionOverlay(); positionBadgeFor (selected);
            repaint();
            badge.toFront (false);
            detHud.toFront (false);
            // Keep HUD and trigger in sync with badge visibility
            if (!badge.isVisible()) { detHud.setVisible (false); hudOpen = false; }
        };
        // Overlay entry to HUD (will be set after overlay is constructed)
        zoomRail.onAutoToggled = [this] { /* persist state if needed */ };
        zoomRail.onReset = [this] { repaint(); };
        
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
        overlay.setAlwaysOnTop (true);
        badge.setAlwaysOnTop (true);

        // Detector HUD (drawer content; initially hidden)
        addAndMakeVisible (detHud);
        detHud.setVisible (false);
        detHud.setAlwaysOnTop (true);
        detHud.setMaxWidth (200);
        // HUD drawer toggle button (sits to the right of the badge)
        addAndMakeVisible (hudButton);
        hudButton.setVisible (false);
        hudButton.setButtonText ("");
        hudButton.setClickingTogglesState (true);
        hudButton.onClick = [this]
        {
            hudOpen = !hudOpen;
            hudButton.setToggleState (hudOpen, juce::dontSendNotification);
            if (selected >= 0) positionBadgeFor (selected); else detHud.setVisible (false);
            // Reassert HUD z-order robustly
            badge.toFront (false);
            detHud.toFront (false);
            hudButton.toFront (false);
        };
        detHud.onChangeSource = [this](int src)
        {
            if (selected >= 0 && selected < (int) points.size())
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynDetectorSrc, (float) juce::jlimit (0, 3, src));
            // Reassert HUD above everything after source change
            badge.toFront (false);
            detHud.toFront (false);
        };
        detHud.onChangeHP = [this](float hz)
        {
            if (selected >= 0 && selected < (int) points.size())
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynDetHPHz, juce::jlimit (20.0f, 2000.0f, hz));
        };
        detHud.onChangeLP = [this](float hz)
        {
            if (selected >= 0 && selected < (int) points.size())
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynDetLPHz, juce::jlimit (1000.0f, 20000.0f, hz));
        };
        detHud.onToggleAdaptive = [this](bool on)
        {
            if (selected >= 0 && selected < (int) points.size())
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::specAdaptive, on ? 1.0f : 0.0f);
        };
        overlay.onGainChanged = [this](float g)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                const float clamped = juce::jlimit (-24.f, 24.f, g);
                points[(size_t) selected].db = clamped;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::gainDb, clamped);
                rebuildEqPath();
                repaint();
                positionOverlay();
            }
        };
        overlay.onAttackMsChanged = [this](float ms)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynAtkMs, juce::jlimit (0.1f, 2000.0f, ms));
                repaint();
            }
        };
        overlay.onReleaseMsChanged = [this](float ms)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynRelMs, juce::jlimit (5.0f, 5000.0f, ms));
                repaint();
            }
        };
        overlay.onHoldMsChanged = [this](float ms)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynHoldMs, juce::jlimit (0.0f, 1000.0f, ms));
                repaint();
            }
        };
        overlay.onThreshDbChanged = [this](float db)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynThreshDb, juce::jlimit (-80.0f, 0.0f, db));
                repaint();
            }
        };
        overlay.onRatioChanged = [this](float r)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynRatio, juce::jlimit (1.0f, 10.0f, r));
                repaint();
            }
        };
        overlay.onKneeDbChanged = [this](float db)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynKneeDb, juce::jlimit (0.0f, 24.0f, db));
                repaint();
            }
        };
        overlay.onMakeupDbChanged = [this](float db)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynMakeupDb, juce::jlimit (-24.0f, 24.0f, db));
                repaint();
            }
        };
        overlay.onWetChanged = [this](float w)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::dynWet01, juce::jlimit (0.0f, 1.0f, w));
                repaint();
            }
        };
        // Remove inline detector edits from overlay; use HUD instead
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
            {
                overlayLastBounds = overlay.getBounds();
                // Ensure overlay/controls sit above hud and badge while dragging
                overlay.toFront (true);
            }
            else
            {
                positionOverlay();
                // Reassert desired z-order: badge above overlay, HUD above badge
                overlay.toFront (false);
                badge.toFront (true);
                detHud.toFront (true);
            }
        };
        overlay.onTypeChanged = [this](int tp)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].type = tp;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::type, (float) tp);
                rebuildEqPath();
                // reflect in badge immediately
                positionBadgeFor (selected);
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
        // SC entry from overlay
        overlay.onOpenHUD = [this](DetectorHUDFocus f){ if (selected >= 0) openDetectorHUD (selected, f); };
        overlay.onChanChanged = [this](int ch)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].channel = ch;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::channel, (float) ch);
                badge.setChannel (ch);
                repaint(); positionBadgeFor (selected);
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
                rebuildEqPath();
                // reflect in overlay immediately
                if (idx == selected) { overlay.setTypeVisual (p.type); }
                repaint(); positionBadgeFor (idx);
            }
        };
        // Badge channel change mirroring
        badge.onSetChannel = [this](int ch)
        {
            const int idx = (badgeFor >= 0 ? badgeFor : selected);
            if (idx >= 0 && idx < (int) points.size())
            {
                auto& p = points[(size_t) idx];
                p.channel = juce::jlimit (0, 4, ch);
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::channel, (float) p.channel);
                if (idx == selected) overlay.setChannelVisual (p.channel);
                repaint(); positionBadgeFor (idx);
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
                if (p.bandIdx >= 0) setBandParam (p.bandIdx, dynEq::Band::dynRangeDb, juce::jlimit (-24.0f, 24.0f, r));
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
        if (selected >= 0 && selected < (int) points.size())
        {
            const int bi = points[(size_t) selected].bandIdx;
            if (bi >= 0) detHud.setGR (field::core::telemetry::getDynEqGrDb (bi));
        }
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel = lf ? lf->theme.meters.panelDark : juce::Colour(0xFF2A2C30);
        auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
        
        // AB Button styling: Solid panel background with elevation shadow
        const float cr = 8.0f; // Match KnobCell corner radius
        
        // Elevation shadow first (AB button style)
        if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
        else g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
        
        // Solid panel background (no aliasing)
        g.setColour(panel);
        g.fillRoundedRectangle(r, cr);
        
        // Border (AB button style)
        g.setColour(sh);
        g.drawRoundedRectangle(r, cr, 1.0f);
        
        // Add 10px top and bottom padding for content
        auto contentR = r.reduced(0, 10.0f);
        
        // Draw units/grid UNDER children so controls overlay them
        drawUnits (g);
    }

    void paintOverChildren (juce::Graphics& g) override
    {
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

        // Channel-aware band points (single glyph; ring pattern encodes St/M/S/L/R) + persistent channel micro-label
        for (size_t i = 0; i < points.size(); ++i)
        {
            const auto& pt = points[i];
            const float x = mapHzToX (pt.hz);
            const float y = mapDbToY (pt.db);
            const float rRing = 8.0f;
            const float rCore = 5.0f;
            const float stroke = (selected == (int) i ? 2.0f : 1.4f);

            juce::Colour accent = applyChannelTint (bandColourFor ((int) i), pt.channel);
            // Halo
            g.setColour (accent.withAlpha (0.22f));
            g.fillEllipse (juce::Rectangle<float> (x - rRing, y - rRing, rRing * 2.0f, rRing * 2.0f));

            // Core
            g.setColour (juce::Colours::white.withAlpha (0.95f));
            g.fillEllipse (juce::Rectangle<float> (x - rCore, y - rCore, rCore * 2.0f, rCore * 2.0f));
            g.setColour (accent.withAlpha (0.95f));
            g.drawEllipse (juce::Rectangle<float> (x - rCore, y - rCore, rCore * 2.0f, rCore * 2.0f), 1.0f);

            // Channel ring pattern
            g.setColour (accent.withAlpha (0.95f));
            juce::Path ring;
            const float pi = juce::MathConstants<float>::pi;
            switch (pt.channel)
            {
                case 0: // Stereo: full ring
                {
                    g.drawEllipse (juce::Rectangle<float> (x - rRing, y - rRing, rRing * 2.0f, rRing * 2.0f), stroke);
                } break;
                case 1: // Mid: full ring + inner crossbar
                {
                    g.drawEllipse (juce::Rectangle<float> (x - rRing, y - rRing, rRing * 2.0f, rRing * 2.0f), stroke);
                    g.drawLine (x - rRing * 0.55f, y, x + rRing * 0.55f, y, stroke);
                } break;
                case 2: // Side: ticks at 3/9 o'clock
                {
                    g.drawEllipse (juce::Rectangle<float> (x - rRing, y - rRing, rRing * 2.0f, rRing * 2.0f), 1.2f);
                    g.drawLine (x + rRing, y, x + rRing - rRing * 0.28f, y, stroke);
                    g.drawLine (x - rRing, y, x - rRing + rRing * 0.28f, y, stroke);
                } break;
                case 3: // Left: left semicircle
                {
                    ring.addCentredArc (x, y, rRing, rRing, 0.0f, pi * 0.5f, pi * 1.5f, true);
                    g.strokePath (ring, juce::PathStrokeType (stroke));
                } break;
                case 4: // Right: right semicircle
                {
                    ring.addCentredArc (x, y, rRing, rRing, 0.0f, -pi * 0.5f, pi * 0.5f, true);
                    g.strokePath (ring, juce::PathStrokeType (stroke));
                } break;
                default:
                    g.drawEllipse (juce::Rectangle<float> (x - rRing, y - rRing, rRing * 2.0f, rRing * 2.0f), stroke);
                    break;
            }

            // Persistent channel micro-label
            {
                static const char* lab[5] = { "St","M","S","L","R" };
                const int ch = juce::jlimit (0, 4, pt.channel);
                auto tb = juce::Rectangle<float> (x + 10.0f, y - 8.0f, 20.0f, 14.0f);
                g.setColour (juce::Colours::black.withAlpha (0.40f));
                g.fillRoundedRectangle (tb, 3.0f);
                g.setColour (applyChannelTint (bandColourFor ((int) i), pt.channel).withAlpha (0.92f));
                g.setFont (juce::Font (10.0f));
                g.drawFittedText (lab[ch], tb.toNearestInt(), juce::Justification::centred, 1);
            }
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
            
            // Legacy band-point tooltip removed
        }
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        
        // Add 10px top and bottom padding for content
        auto contentR = r.reduced(0, 10);
        
        // Left rail outside the pane (48px width)
        auto rail = contentR.removeFromLeft (48);
        rail.removeFromRight (4); // little breathing room
        zoomRail.setBounds (rail);
        
        // Analyzer takes remaining area
        analyzer.setBounds (contentR);
        rebuildEqPath();
        positionOverlay();
    }

    // Analyzer control (PaneManager may wire these later)
    void setSampleRate (double sr) { analyzer.setSampleRate (sr); }
    void pause()  { analyzer.pauseAudio(); }
    void resume() { analyzer.resumeAudio(); }
    void pushBlock (const float* L, const float* R, int n)    { analyzer.pushBlock (L, R, n); }
    void pushBlockPre (const float* L, const float* R, int n) { analyzer.pushBlockPre (L, R, n); }
    
    // Public getter for graphics container
    SpectrumAnalyzer* getAnalyzer() const { return const_cast<SpectrumAnalyzer*>(&analyzer); }

private:
    // Auto-zoom logic for when bands exceed current range
    void adaptDbRangeToPoint (float db)
    {
        if (zoomRail.isAutoMode())
        {
            const float currentRange = zoomState.getCurrent();
            if (std::abs (db) > currentRange)
            {
                // Suggest a new range that accommodates this point
                const float suggestedRange = std::max (currentRange * 1.5f, std::abs (db) * 1.2f);
                const float clampedRange = juce::jlimit (zoomState.getMin(), zoomState.getMax(), suggestedRange);
                zoomState.setTarget (clampedRange);
            }
        }
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
        std::function<void(float)> onThreshDbChanged;
        std::function<void(float)> onRatioChanged;
        std::function<void(float)> onKneeDbChanged;
        std::function<void(float)> onMakeupDbChanged;
        std::function<void(float)> onWetChanged;
        std::function<void(float)> onSCHPChanged;
        std::function<void(float)> onSCLPChanged;
        std::function<void(int)>   onDetSrcChanged;
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
            // Hide legacy combo; we will render channel chips in paint()
            chanCb.addItemList (juce::StringArray{ "Stereo","Mid","Side","Left","Right" }, 1);
            chanCb.onChange = [this]{ if (!updating && onChanChanged) onChanChanged (chanCb.getSelectedItemIndex()); };
            chanCb.setVisible (false);

            // Dynamic / Spectral toggles
            dynToggle.setButtonText ("DYN");
            dynToggle.onClick = [this]{ if (!updating && onDynChanged) onDynChanged (dynToggle.getToggleState()); };
            addAndMakeVisible (dynToggle);
            specToggle.setButtonText ("SPEC");
            specToggle.onClick = [this]{ if (!updating && onSpecChanged) onSpecChanged (specToggle.getToggleState()); };
            addAndMakeVisible (specToggle);
            
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                dynToggle.setLookAndFeel(lf);
                specToggle.setLookAndFeel(lf);
            }
            
            setAreaMetallicForCell (dynToggle, MetallicKind::Band); // Use Band metallic for DynEQ buttons
            setAreaMetallicForCell (specToggle, MetallicKind::Band);

            // Attack / Release / Hold sliders → mini knobs
            auto makeMiniKnob = [this](juce::Slider& s){
                s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                s.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
                if (auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel())) s.setLookAndFeel (lf2);
            };
            makeMiniKnob (gain); makeMiniKnob (q); makeMiniKnob (freq);
            atkMs.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            atkMs.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            atkMs.setRange (0.1, 2000.0, 0.1);
            atkMs.onValueChange = [this]{ if (!updating && onAttackMsChanged) onAttackMsChanged ((float) atkMs.getValue()); };
            addAndMakeVisible (atkMs);
            atkLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (atkLabel);

            relMs.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            relMs.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            relMs.setRange (5.0, 5000.0, 0.1);
            relMs.onValueChange = [this]{ if (!updating && onReleaseMsChanged) onReleaseMsChanged ((float) relMs.getValue()); };
            addAndMakeVisible (relMs);
            relLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (relLabel);

            holdMs.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            holdMs.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            holdMs.setRange (0.0, 1000.0, 0.1);
            holdMs.onValueChange = [this]{ if (!updating && onHoldMsChanged) onHoldMsChanged ((float) holdMs.getValue()); };
            addAndMakeVisible (holdMs);
            holdLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (holdLabel);

            // Threshold / Ratio / Knee / Makeup / Wet → mini knobs
            threshDb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            threshDb.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            threshDb.setRange (-80.0, 0.0, 0.1);
            threshDb.onValueChange = [this]{ if (!updating && onThreshDbChanged) onThreshDbChanged ((float) threshDb.getValue()); };
            addAndMakeVisible (threshDb);

            ratio.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            ratio.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            ratio.setRange (1.0, 10.0, 0.1);
            ratio.onValueChange = [this]{ if (!updating && onRatioChanged) onRatioChanged ((float) ratio.getValue()); };
            addAndMakeVisible (ratio);

            kneeDb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            kneeDb.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            kneeDb.setRange (0.0, 24.0, 0.1);
            kneeDb.onValueChange = [this]{ if (!updating && onKneeDbChanged) onKneeDbChanged ((float) kneeDb.getValue()); };
            addAndMakeVisible (kneeDb);

            makeupDb.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            makeupDb.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            makeupDb.setRange (-24.0, 24.0, 0.1);
            makeupDb.onValueChange = [this]{ if (!updating && onMakeupDbChanged) onMakeupDbChanged ((float) makeupDb.getValue()); };
            addAndMakeVisible (makeupDb);

            wet01.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            wet01.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            wet01.setRange (0.0, 1.0, 0.01);
            wet01.onValueChange = [this]{ if (!updating && onWetChanged) onWetChanged ((float) wet01.getValue()); };
            addAndMakeVisible (wet01);

            // (detector controls removed; SC chip opens HUD)
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Lighter dark background for overlay
            juce::Colour bg = juce::Colours::darkgrey.darker (0.20f);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) bg = lf->theme.shadowDark.brighter (0.20f);
            g.setColour (bg.withAlpha (0.96f));
            g.fillRoundedRectangle (r, 8.0f);
            // Subtle outline (slightly stronger)
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
            // Accent vertical strip on the left (bottom-to-top gradient)
            juce::Rectangle<float> strip = r.removeFromLeft (2.0f).reduced (0.5f, 1.5f);
            // Top-down gradient for border accent
            juce::Colour a0 = overlayAccent.withAlpha (0.36f);
            juce::Colour a1 = overlayAccent.withAlpha (0.12f);
            juce::ColourGradient grad (a0, strip.getCentreX(), strip.getY(), a1, strip.getCentreX(), strip.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRect (strip);

            // Draw top control bar icons
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto fg = lf ? lf->theme.textPrimary : juce::Colours::white;
            g.setColour (fg.withAlpha (0.9f));
            IconSystem::drawIcon (g, IconSystem::Power, hdrPowerR_.toFloat(), fg);
            IconSystem::drawIcon (g, IconSystem::Audition, hdrAudR_.toFloat(), fg);
            IconSystem::drawIcon (g, IconSystem::X, hdrCloseR_.toFloat(), fg);

            // SC chip (status)
            if (! scChipRect_.isEmpty())
            {
                auto chip = scChipRect_.toFloat();
                auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                auto bgc = lf2 ? lf2->theme.chipBg : juce::Colours::darkgrey;
                auto fgc = lf2 ? lf2->theme.textPrimary : juce::Colours::white;
                g.setColour (bgc.withAlpha (0.28f)); g.fillRoundedRectangle (chip.reduced (1.f), 6.f);
                g.setColour (bgc.withAlpha (0.85f)); g.drawRoundedRectangle (chip.reduced (1.f), 6.f, 1.f);
                g.setColour (fgc.withAlpha (0.95f));
                g.drawFittedText ("SC  " + scSummary_, scChipRect_, juce::Justification::centred, 1);
            }

            // Channel chips row (bottom) — paint from rects computed in resized()
            auto paintChip = [&](juce::Rectangle<int> b, const juce::String& t, bool on)
            {
                auto bgc = lf ? lf->theme.chipBg : juce::Colours::darkgrey;
                auto fgc = lf ? lf->theme.textPrimary : juce::Colours::white;
                if (on) bgc = lf ? lf->theme.chipBgActive : bgc.brighter (0.25f);
                g.setColour (bgc); g.fillRoundedRectangle (b.toFloat(), 6.f);
                g.setColour (fgc.withAlpha (on ? 0.95f : 0.75f));
                g.drawFittedText (t, b, juce::Justification::centred, 1);
            };
            paintChip (rSt_, "St", channel == 0);
            paintChip (rM_,  "M",  channel == 1);
            paintChip (rS_,  "S",  channel == 2);
            paintChip (rL_,  "L",  channel == 3);
            paintChip (rR_,  "R",  channel == 4);
        }
        void resized() override
        {
            auto r = getLocalBounds().reduced (8);
            const int kRow = 28; const int kG = 6; const int kKnob = 22; const int kLabW = 40; const int kGap = 10;
            auto placeKnob = [&](juce::Rectangle<int> area, juce::Slider& s){
                auto c = area.getCentre(); s.setBounds (c.x - kKnob/2, c.y - kKnob/2, kKnob, kKnob);
            };

            r.removeFromTop (8);
            // Top control bar
            {
                auto header = r.removeFromTop (20);
                const int icon = 16;
                hdrPowerR_ = header.removeFromLeft (icon);
                header.removeFromLeft (4);
                hdrAudR_   = header.removeFromLeft (icon);
                hdrCloseR_ = header.removeFromRight (icon);
            }
            auto half = r.removeFromTop (24);
            typeIcon.setBounds (half.removeFromLeft (28));
            // Expand remaining controls into freed space
            phaseCb.setBounds (half.removeFromLeft (160));
            dynToggle.setBounds (half.removeFromLeft (64));
            specToggle.setBounds (half.removeFromLeft (64));

            // Row: Gain, Q, Freq (third row)
            {
                auto row = r.removeFromTop (kRow);
                // Gain
                gainLabel.setBounds (row.removeFromLeft (kLabW));
                placeKnob (row.removeFromLeft (kKnob), gain);
                row.removeFromLeft (kGap);
                // Q
                qLabel.setBounds (row.removeFromLeft (kLabW));
                placeKnob (row.removeFromLeft (kKnob), q);
                row.removeFromLeft (kGap);
                // Freq
                freqLabel.setBounds (row.removeFromLeft (kLabW));
                placeKnob (row.removeFromLeft (kKnob), freq);
            }

            auto half2 = r.removeFromTop (24);
            chanLabel.setBounds (half2.removeFromLeft (40));
            chanCb.setBounds (half2.removeFromLeft (120));
            // Attack / Release / Hold row
            auto half3 = r.removeFromTop (kRow);
            atkLabel.setBounds (half3.removeFromLeft (36));
            placeKnob (half3.removeFromLeft (kRow), atkMs);
            relLabel.setBounds (half3.removeFromLeft (36));
            placeKnob (half3.removeFromLeft (kRow), relMs);
            holdLabel.setBounds (half3.removeFromLeft (44));
            placeKnob (half3.removeFromLeft (kRow), holdMs);

            // Threshold / Ratio / Knee / Makeup / Wet row
            auto half4 = r.removeFromTop (kRow);
            placeKnob (half4.removeFromLeft (kRow), threshDb);
            placeKnob (half4.removeFromLeft (kRow), ratio);
            placeKnob (half4.removeFromLeft (kRow), kneeDb);
            placeKnob (half4.removeFromLeft (kRow), makeupDb);
            placeKnob (half4.removeFromLeft (kRow), wet01);

            // SC chip row (status-only) — opens HUD on click
            auto scRow = r.removeFromTop (kRow);
            auto chip  = scRow.removeFromLeft (200);
            scChipRect_ = chip.toNearestInt();
            // Paint happens in paint(); here we just set bounds

            // Bottom channel row (28 px)
            const int rowH = 28, gutter = 8;
            auto rAll = getLocalBounds().reduced (12);
            auto rowChan = rAll.removeFromBottom (rowH);
            auto slice = [&](int w){ auto s = rowChan.removeFromLeft (w); if (rowChan.getWidth() > 0) rowChan.removeFromLeft (gutter); return s; };
            rSt_ = slice (48); rM_ = slice (40); rS_ = slice (40); rL_ = slice (40); rR_ = slice (40);
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
            channel = juce::jlimit (0, 4, chanIdx);
            typeIcon.setType (typeIdx);
            dynToggle.setToggleState (dynOn, juce::dontSendNotification);
            specToggle.setToggleState (specOn, juce::dontSendNotification);
        }
        void setAccentColour (juce::Colour c) { overlayAccent = c; repaint(); }
        void setDynParams (float atk_ms, float rel_ms, float hold_ms)
        {
            juce::ScopedValueSetter<bool> sv (updating, true);
            atkMs.setValue (atk_ms, juce::dontSendNotification);
            relMs.setValue (rel_ms, juce::dontSendNotification);
            holdMs.setValue (hold_ms, juce::dontSendNotification);
        }
        // Hooks to parent for parameter binding
        std::function<void(float)> onAttackMsChanged;
        std::function<void(float)> onReleaseMsChanged;
        std::function<void(float)> onHoldMsChanged;
    private:
        juce::Slider gain, q, freq;
        juce::Slider atkMs, relMs, holdMs;
        juce::Slider threshDb, ratio, kneeDb, makeupDb, wet01;
        // Removed inline detector controls; use HUD via SC chip
        juce::Label gainLabel, qLabel, freqLabel, /*typeLabel, phaseLabel,*/ chanLabel;
        juce::Label atkLabel { "ATK", "ATK" }, relLabel { "REL", "REL" }, holdLabel { "HOLD", "HOLD" };
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
        // Expose a visual setter so outer class can sync icon without accessing private members
    public:
        void setTypeVisual (int t)
        {
            juce::ScopedValueSetter<bool> sv (updating, true);
            typeIcon.setType (t);
            typeCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, typeCb.getNumItems()-1), t), juce::dontSendNotification);
            repaint();
        }
    private:
        bool updating { false };
        bool typeMenuOpen { false };
        void showTypeMenuToggle()
        {
            if (typeMenuOpen) { juce::PopupMenu::dismissAllActiveMenus(); typeMenuOpen = false; return; }
            juce::PopupMenu m; juce::StringArray names { "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" };
            struct TypeMenuComp : public juce::PopupMenu::CustomComponent {
                juce::String text; int idx; TypeMenuComp(const juce::String& t, int i) : juce::PopupMenu::CustomComponent(true), text(t), idx(i) {}
                void getIdealSize (int& w, int& h) override { w = 220; h = 24; }
                void paint (juce::Graphics& g) override {
                    auto r = getLocalBounds().toFloat();
                    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                    auto fg = lf ? lf->theme.textPrimary : juce::Colours::white;
                    auto ic = r.removeFromRight (20.0f).toFloat().reduced (2.0f);
                    g.setColour (fg);
                    juce::Path p; const int N = 14;
                    auto mapx = [&](int i){ return ic.getX() + (float) i / (float) (N-1) * ic.getWidth(); };
                    auto mapy = [&](float v01){ return juce::jmap (v01, 0.0f, 1.0f, ic.getBottom(), ic.getY()); };
                    auto shape = [&](float t){
                        if (idx == 0) return 0.5f + 0.35f * std::sin ((t-0.5f) * juce::MathConstants<float>::pi);
                        if (idx == 1) return 0.35f + 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.45f)));
                        if (idx == 2) return 0.65f - 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.55f)));
                        if (idx == 3) return juce::jlimit (0.0f,1.0f, (t*1.6f));
                        if (idx == 4) return juce::jlimit (0.0f,1.0f, 1.0f-(t*1.6f));
                        if (idx == 5) return 0.5f + 0.45f * std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi);
                        if (idx == 6) return 0.5f + 0.45f * std::abs (std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi));
                        return 0.5f;
                    };
                    p.startNewSubPath (mapx(0), mapy (shape(0)));
                    for (int i=1;i<N;++i) p.lineTo (mapx(i), mapy (shape((float) i/(N-1))));
                    g.strokePath (p, juce::PathStrokeType (1.2f));
                    g.setColour (fg);
                    g.drawFittedText (text, getLocalBounds().withTrimmedRight (24), juce::Justification::centredLeft, 1);
                }
            };
            for (int i = 0; i < names.size(); ++i) m.addCustomItem (i+1, std::make_unique<TypeMenuComp>(names[i], i));
            typeMenuOpen = true;
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                             [this](int r)
                             {
                                 typeMenuOpen = false;
                                 if (r > 0 && !updating)
                                 {
                                     const int idx = r-1;
                                     setTypeVisual (idx);
                                     if (onTypeChanged) onTypeChanged (idx);
                                 }
                             });
        }
        juce::Colour overlayAccent { juce::Colours::white.withAlpha (0.4f) };
        
        // --- Channel chips (bottom row) ---
    public:
        void setChannelVisual (int ch)
        {
            juce::ScopedValueSetter<bool> sv (updating, true);
            channel = juce::jlimit (0, 4, ch);
            chanCb.setSelectedItemIndex (channel, juce::dontSendNotification);
            repaint();
        }
    private:
        // External hooks
    public:
        // HUD entry
        std::function<void(DetectorHUDFocus)> onOpenHUD;
        void setDetectorSummaryText (const juce::String& txt) { scSummary_ = txt; repaint (scChipRect_); }
    private:
        int channel { 0 };
        juce::Rectangle<int> rSt_, rM_, rS_, rL_, rR_;
        juce::Rectangle<int> scChipRect_;
        juce::String scSummary_ { "Pre • HP — • LP —" };
        juce::Rectangle<int> hdrPowerR_, hdrAudR_, hdrCloseR_;
        void mouseUp (const juce::MouseEvent& e) override
        {
            auto p = e.getPosition();
            if (hdrPowerR_.contains (p)) { if (onDynChanged) onDynChanged (false); return; }
            if (hdrAudR_.contains   (p)) { /* audition (overlay) TODO */ return; }
            if (hdrCloseR_.contains (p)) { this->setVisible (false); return; }
            if (scChipRect_.contains (p)) { if (onOpenHUD) onOpenHUD (DetectorHUDFocus::Source); return; }
            if (rSt_.contains (p)) { if (onChanChanged) onChanChanged (0); return; }
            if (rM_.contains  (p)) { if (onChanChanged) onChanChanged (1); return; }
            if (rS_.contains  (p)) { if (onChanChanged) onChanChanged (2); return; }
            if (rL_.contains  (p)) { if (onChanChanged) onChanChanged (3); return; }
            if (rR_.contains  (p)) { if (onChanChanged) onChanChanged (4); return; }
        }
    } overlay;

    // Compact per-band badge (GR, freq, type, delete, bypass)
    class BandBadge : public juce::Component
    {
    public:
        BandBadge() { setOpaque (true); }
        void setChannel (int ch)
        {
            channel_ = juce::jlimit (0, 4, ch);
            static const char* lab[5] = { "St","M","S","L","R" };
            chanLabel = juce::String (lab[channel_]);
            repaint();
        }
        std::function<void()> onDelete;
        std::function<void(bool)> onBypass;
        std::function<void(int)> onSetType;
        std::function<void(int)> onSetSlopeDb;
        std::function<void(int)> onSetTapMode;
        std::function<void(int)> onSetChannel;
        std::function<void(bool)> onToggleAudition;
        std::function<void()> onToggleDyn;
        std::function<void()> onToggleSpec;
        std::function<void(float)> onSetFreq;
        std::function<void(float)> onSetQ;
        std::function<void(float)> onSetGainDb;
        std::function<void(float)> onSetDynRangeDb;
        std::function<void()> onClose; // close badge without deleting
        void setValues (float grDb, float freqHz, int typeIdx, bool bypass)
        {
            gainReductionDb = grDb; freq = freqHz; type = typeIdx; bypassed = bypass; repaint();
        }
        void setAccentColour (juce::Colour c) { badgeAccent = c; repaint(); }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            // Opaque background for badge (theme-driven)
            juce::Colour bg = juce::Colours::black;
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) bg = lf->theme.drawerBg;
            g.setColour (bg);
            g.fillRoundedRectangle (r, 5.0f);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour (lf->theme.drawerBorder);
            else g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.drawRoundedRectangle (r, 5.0f, 0.9f);
            // Accent vertical strip on the left
            juce::Rectangle<float> strip = r.withX (r.getX()).withWidth (2.0f).reduced (0.5f, 1.5f);
            // Top-down gradient for border accent
            juce::Colour a0 = badgeAccent.withAlpha (0.65f);
            juce::Colour a1 = badgeAccent.withAlpha (0.25f);
            juce::ColourGradient grad (a0, strip.getCentreX(), strip.getY(), a1, strip.getCentreX(), strip.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRect (strip);
            auto area = r.reduced (6.0f);
            // Header: power (left) and close X (right)
            const float headerH = 20.0f;
            auto header = area.removeFromTop (headerH);
            // separator line under header (avoid touching outside badge)
            juce::Colour borderCol = (dynamic_cast<FieldLNF*>(&getLookAndFeel()) ? ((FieldLNF&) getLookAndFeel()).theme.drawerBorder
                                                                                : juce::Colours::white.withAlpha (0.25f));
            g.setColour (borderCol.withAlpha (0.6f));
            g.fillRect (juce::Rectangle<float> (r.getX()+6.0f, header.getBottom()+5.0f, r.getWidth()-12.0f, 1.0f));
            const float headerIconSz = juce::jmax (14.0f, headerH * 0.9f);
            powerRect = header.removeFromLeft (headerIconSz).toNearestInt();
            auditionRect = header.removeFromLeft (headerIconSz).toNearestInt();
            xRect     = header.removeFromRight (headerIconSz).toNearestInt();
            auto iconCol = juce::Colours::white.withAlpha (0.90f);
            IconSystem::drawIcon (g, IconSystem::Power, powerRect.toFloat(), bypassed ? juce::Colours::orange : iconCol);
            IconSystem::drawIcon (g, IconSystem::Audition, auditionRect.toFloat(), auditionOn ? juce::Colours::deepskyblue : iconCol);
            IconSystem::drawIcon (g, IconSystem::X,     xRect.toFloat(), iconCol);
            // Centered type glyph button
            {
                const float typeW = header.getHeight() * 0.9f;
                juce::Rectangle<float> typeBox (header.getCentreX() - typeW * 0.5f,
                                                header.getY() + (header.getHeight() - typeW) * 0.5f,
                                                typeW, typeW);
                typeRect = typeBox.toNearestInt();
            bool overType = typeRect.contains (getMouseXYRelative());
            juce::Colour glyphCol = juce::Colours::white.withAlpha (0.75f);
            if (overType)
                {
                    if (auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel())) glyphCol = lf2->theme.accent.withAlpha (0.95f);
                    // highlight only the glyph bounds
                    g.setColour (glyphCol.withAlpha (0.12f));
                    g.fillRoundedRectangle (typeBox.expanded (2.0f), 3.0f);
                }
                drawTypeGlyphWithColour (g, typeBox, glyphCol);
            }
            area.removeFromTop (4.0f);
            // Responsive rows based on available height
            const float gap = 6.0f;
            const float totalH = area.getHeight();
            const float rowH  = juce::jmax (14.0f, (totalH - gap) * 0.5f);
            auto row1 = area.removeFromTop (rowH);
            area.removeFromTop (gap);
            auto row2 = area.removeFromTop (rowH);

            // Row 1: Type glyph | FREQ | Q | GAIN | GR | (spacer) | Power | X
            g.setColour (juce::Colours::white.withAlpha (0.97f));
            g.setFont (juce::Font (juce::FontOptions (juce::jmax (10.0f, rowH * 0.65f)).withStyle ("Bold")));

            // Type glyph moved to header; leave initial spacing tighter
            // invisible popup trigger areas: we handle showing menus in mouseDown to avoid accidental drags

            // Distribute four cells across remaining width (freq, Q, Gain, GR)
            const int numCells = 4;
            const float colGap = 6.0f;
            float rw = row1.getWidth() - (colGap * (numCells - 1));
            float cellW = juce::jmax (44.0f, rw / (float) numCells);
            auto cell = [&](juce::Rectangle<float>& row, const juce::String& label)
            {
                auto c  = row.removeFromLeft (cellW);
                auto ci = c.toNearestInt();
                // subtle cell background to align top row
                juce::Colour cellBg = juce::Colours::white.withAlpha (0.06f);
                juce::Colour cellBorder = juce::Colours::white.withAlpha (0.18f);
                juce::Colour cellFg = juce::Colours::white.withAlpha (0.90f);
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) { cellBg = lf->theme.chipBg; cellBorder = lf->theme.drawerBorder; cellFg = lf->theme.textPrimary; }
                g.setColour (cellBg.withAlpha (0.28f));
                g.fillRoundedRectangle (c.reduced (1.0f), 4.0f);
                // accent top border
                juce::Colour accent = (dynamic_cast<FieldLNF*>(&getLookAndFeel()) ? ((FieldLNF&) getLookAndFeel()).theme.accent
                                                                                   : juce::Colours::deepskyblue);
                g.setColour (accent.withAlpha (0.55f));
                g.fillRect (juce::Rectangle<float> (c.getX()+1.0f, c.getY()+1.0f, c.getWidth()-2.0f, 1.0f));
                g.setColour (cellBorder.withAlpha (0.95f));
                g.drawRoundedRectangle (c.reduced (1.0f), 4.0f, 0.8f);
                g.setColour (cellFg);
                g.drawFittedText (label, ci, juce::Justification::centredLeft, 1);
                if (row.getWidth() > 0) row.removeFromLeft (colGap);
                return ci;
            };
            juce::String f;
            if (freq >= 1000.f && freq < 10000.f) f = juce::String (freq / 1000.0f, 1) + "k";
            else if (freq >= 10000.f) f = juce::String ((int) std::round (freq/1000.f)) + "k";
            else f = juce::String ((int) freq);
            freqRect = cell (row1, f + " Hz");
            qRect    = cell (row1, juce::String (qVal, 2));
            gainRect = cell (row1, juce::String (gainDb, 1) + " dB");
            grRect   = cell (row1, juce::String (gainReductionDb, 1) + " dB");

            row1.removeFromLeft (4.0f); // header holds icons now

            // Row 2 chips: Dyn | Spec | Chan | Slope | Tap
            g.setFont (juce::Font (juce::FontOptions (juce::jmax (9.0f, rowH * 0.52f)).withStyle ("Bold")));
            const int numChips = 5;
            const float chipGap = 6.0f;
            float rw2 = row2.getWidth() - chipGap * (numChips - 1);
            float chipW = juce::jmax (52.0f, rw2 / (float) numChips);
            const float chipR = 4.0f;
            auto chip = [&](juce::Rectangle<float>& row, const juce::String& txt, juce::Colour col)
            {
                auto c  = row.removeFromLeft (chipW).reduced (1.0f);
                auto ci = c.toNearestInt();
                g.setColour (col.withAlpha (0.22f)); g.fillRoundedRectangle (c, chipR);
                g.setColour (col.withAlpha (0.85f)); g.drawRoundedRectangle (c, chipR, 1.0f);
                g.drawFittedText (txt, ci, juce::Justification::centred, 1);
                if (row.getWidth() > 0) row.removeFromLeft (chipGap);
                return ci;
            };
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
            else if (auditionRect.contains (e.getPosition())) { auditionOn = !auditionOn; if (onToggleAudition) onToggleAudition (auditionOn); repaint(); }
            else if (dynRect.contains (e.getPosition())) { if (onToggleDyn) onToggleDyn(); }
            else if (specRect.contains (e.getPosition())) { if (onToggleSpec) onToggleSpec(); }
            else if (typeRect.contains (e.getPosition())) { showTypeMenu(); }
            else if (slopeRect.contains (e.getPosition())) { showSlopeMenu(); }
            else if (tapRect.contains (e.getPosition())) { showTapMenu(); }
            else if (chanRect.contains (e.getPosition())) { showChanMenu(); }
        }
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (typeRect.contains (e.getPosition())) { showTypeMenu(); return; }
            if (slopeRect.contains (e.getPosition())) { showSlopeMenu(); return; }
            if (tapRect.contains (e.getPosition())) { showTapMenu(); return; }
        }
        void mouseDoubleClick (const juce::MouseEvent& e) override
        {
            if (typeRect.contains (e.getPosition())) { if (onClose) onClose(); }
        }
        void showTypeMenu()
        {
            if (!onSetType) return; juce::PopupMenu m; juce::StringArray names { "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" };
            struct TypeMenuComp : public juce::PopupMenu::CustomComponent {
                juce::String text; int idx; TypeMenuComp(const juce::String& t, int i) : juce::PopupMenu::CustomComponent(true), text(t), idx(i) {}
                void getIdealSize (int& w, int& h) override { w = 220; h = 24; }
                void paint (juce::Graphics& g) override {
                    auto r = getLocalBounds().toFloat();
                    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                    auto fg = lf ? lf->theme.textPrimary : juce::Colours::white;
                    auto ic = r.removeFromLeft (20.0f).toFloat().reduced (2.0f);
                    g.setColour (fg);
                    juce::Path p; const int N = 14;
                    auto mapx = [&](int i){ return ic.getX() + (float) i / (float) (N-1) * ic.getWidth(); };
                    auto mapy = [&](float v01){ return juce::jmap (v01, 0.0f, 1.0f, ic.getBottom(), ic.getY()); };
                    auto shape = [&](float t){
                        if (idx == 0) return 0.5f + 0.35f * std::sin ((t-0.5f) * juce::MathConstants<float>::pi);
                        if (idx == 1) return 0.35f + 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.45f)));
                        if (idx == 2) return 0.65f - 0.5f * 1.0f / (1.0f + std::exp (-10.0f*(t-0.55f)));
                        if (idx == 3) return juce::jlimit (0.0f,1.0f, (t*1.6f));
                        if (idx == 4) return juce::jlimit (0.0f,1.0f, 1.0f-(t*1.6f));
                        if (idx == 5) return 0.5f + 0.45f * std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi);
                        if (idx == 6) return 0.5f + 0.45f * std::abs (std::sin ((t-0.5f) * juce::MathConstants<float>::twoPi));
                        return 0.5f;
                    };
                    p.startNewSubPath (mapx(0), mapy (shape(0)));
                    for (int i=1;i<N;++i) p.lineTo (mapx(i), mapy (shape((float) i/(N-1))));
                    g.strokePath (p, juce::PathStrokeType (1.2f));
                    g.setColour (fg);
                    g.drawFittedText (text, getLocalBounds().withTrimmedLeft (24), juce::Justification::centredLeft, 1);
                }
            };
            for (int i = 0; i < names.size(); ++i) m.addCustomItem (i+1, std::make_unique<TypeMenuComp>(names[i], i));
            // Anchor at the type glyph, hinting to open upward from header
            auto screenRect = localAreaToGlobal (typeRect).expanded (2);
            m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (screenRect), [this](int r){ if (r > 0 && onSetType) onSetType (r-1); });
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
        void showChanMenu()
        {
            juce::PopupMenu m; juce::StringArray names { "Stereo","Mid","Side","Left","Right" };
            for (int i = 0; i < names.size(); ++i) m.addItem (i+1, names[i]);
            auto screenRect = localAreaToGlobal (chanRect).expanded (2);
            m.showMenuAsync (juce::PopupMenu::Options().withTargetScreenArea (screenRect),
                             [this](int r)
                             {
                                 if (r > 0)
                                 {
                                     const int ch = r - 1;
                                     setChannel (ch); // immediate UI update
                                     if (onSetChannel) onSetChannel (ch);
                                 }
                             });
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
        juce::String chanLabel { "St" }; int channel_ { 0 }; int slopeDb { 12 }; juce::String tapLabel { "Post" };
        juce::Rectangle<int> powerRect, auditionRect, xRect, typeRect, freqRect, qRect, gainRect, grRect;
        juce::Rectangle<int> dynRect, specRect, chanRect, slopeRect, tapRect;
        juce::Colour badgeAccent { juce::Colours::white.withAlpha (0.4f) };
        bool auditionOn { false };
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
        const float halfRange = zoomState.getCurrent();
        return juce::jmap (dB, +halfRange, -halfRange, top, bottom);
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
        const float halfRange = zoomState.getCurrent();
        return juce::jmap ((float) py, (float) r.getY(), (float) r.getBottom(), +halfRange, -halfRange);
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
            if (juce::Point<float> (cx, cy).getDistanceFrom (p.toFloat()) <= 14.0f)
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
            if (juce::Point<float> (cx, cy).getDistanceFrom (p.toFloat()) <= 14.0f)
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
            // Do not auto-open HUD on new point
            hudOpen = false; detHud.setVisible (false); if (hudButton.getToggleState()) hudButton.setToggleState(false, juce::dontSendNotification);
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
            // Check if clicking on already selected point - toggle controls visibility
            if (selected == hitTestPoint (e.getPosition()) && overlay.isVisible())
            {
                // Toggle off controls
                overlay.setVisible (false);
                badge.setVisible (false);
                selected = -1;
            }
            else
            {
                // Select point and show controls
                auto& pt = points[(size_t) selected];
                overlay.setValues (pt.db, pt.q, pt.hz, pt.type, pt.phase, pt.channel, pt.dynOn, pt.specOn);
                overlay.setVisible (true);
                positionOverlay();
                positionBadgeFor (selected);
                // Update HUD state from APVTS for selected band
                DetectorHUDState st;
                st.bandIndex = points[(size_t) selected].bandIdx;
                // Source
                {
                    const auto id = bandId (dynEq::Band::dynDetectorSrc, st.bandIndex);
                    int srcIdx = 0; if (auto* p = proc.apvts.getParameter (id)) srcIdx = (int) std::round (p->getValue() * 3.0f);
                    st.source = (srcIdx == 1 ? "post" : srcIdx == 2 ? "ext1" : srcIdx == 3 ? "ext2" : "pre");
                }
                st.hpHz = getBandParamFloat (st.bandIndex, dynEq::Band::dynDetHPHz, 60.0f);
                st.lpHz = getBandParamFloat (st.bandIndex, dynEq::Band::dynDetLPHz, 8000.0f);
                st.adaptive = getBandParamFloat (st.bandIndex, dynEq::Band::specAdaptive, 0.0f) > 0.5f;
                // Pull GR preview from telemetry
                st.grPreviewDb = field::core::telemetry::getDynEqGrDb (st.bandIndex);
                // External activation state (stub: mark inactive for ext1/2; future: query host bus)
                st.extActive = !(st.source.startsWithIgnoreCase ("ext"));
                detHud.setState (st);
                detHud.setVisible (true);
            }
        }
        else
        {
            overlay.setVisible (false);
            badge.setVisible (false);
            detHud.setVisible (false);
            hudOpen = false; hudButton.setVisible(false); hudButton.setToggleState(false, juce::dontSendNotification);
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
                else { badge.setVisible (false); detHud.setVisible(false); hudOpen = false; hudButton.setVisible(false); hudButton.setToggleState(false, juce::dontSendNotification); }
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
        if (selected < 0)
        {
            badge.setVisible (false);
            detHud.setVisible (false);
            hudOpen = false;
            hudButton.setVisible (false);
            hudButton.setToggleState (false, juce::dontSendNotification);
        }
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
        
        const auto& pt = points[(size_t) selected];
        const float x = mapHzToX (pt.hz);
        const float y = mapDbToY (pt.db);
        auto pane = getLocalBounds();

        // Compute dynamic overlay size from row specs
        const int pad = 12, gutter = 8;
        const int row22 = 22, row24 = 24, row28 = 28;
        const int headerH = 20;
        const int rowsH = headerH + gutter + row22 + gutter + row22 + gutter + row22 + gutter
                        + row24 + gutter + row24 + gutter + row24 + gutter
                        + row24 + gutter + row28;
        // Row width requirements (fixed controls)
        const int wTypeRow = 28 + gutter + 160 + gutter + 64 + gutter + 64;            // 28|160|64|64 with 3 gutters
        const int wAtkRow  = (36+120) + gutter + (36+120) + gutter + (44+120);         // labels+sliders with 2 gutters
        const int wTRKMW   = (120*5) + (gutter*4);                                     // THR/RATIO/KNEE/MAKEUP/WET
        const int wDetRow  = (140*3) + (gutter*2);                                     // SRC + HP + LP
        const int wChanRow = 48 + gutter + 40 + gutter + 40 + gutter + 40 + gutter + 40; // St M S L R
        const int contentW = juce::jmax (wTypeRow, juce::jmax (wAtkRow, juce::jmax (wTRKMW, juce::jmax (wDetRow, wChanRow))));
        const int minW = 360; // floor for readability
        const int w = juce::jmax (minW, pad + contentW + pad);
        const int h = pad + rowsH + pad;
        
        // Start with band point position
        int ox = (int) x - (w / 2);
        int oy = pane.getBottom() - h - 60; // prefer lower area but above Hz labels
        
        // Smart positioning to avoid overlap with band point
        const int bandRadius = 12; // Band point click radius
        const int margin = 20; // Additional margin from band point
        
        // Check if overlay would overlap with band point
        bool overlapsBand = (ox <= x + bandRadius + margin &&
                           ox + w >= x - bandRadius - margin &&
                           oy <= y + bandRadius + margin &&
                           oy + h >= y - bandRadius - margin);
        
        if (overlapsBand)
        {
            // Position overlay to the right of band point
            ox = (int) x + bandRadius + margin;
            
            // If that goes off screen, try to the left
            if (ox + w > pane.getRight())
            {
                ox = (int) x - w - bandRadius - margin;
            }
            
            // If still off screen, try above
            if (ox < pane.getX() || ox + w > pane.getRight())
            {
                ox = (int) x - w/2;
                oy = (int) y - h - bandRadius - margin;
            }
            
            // If still off screen, try below
            if (oy < pane.getY())
            {
                oy = (int) y + bandRadius + margin;
            }
        }
        
        // Final bounds checking
        if (ox < pane.getX()) ox = pane.getX() + 12;
        if (ox + w > pane.getRight()) ox = pane.getRight() - w - 12;
        if (oy < pane.getY()) oy = pane.getY() + 12;
        if (oy + h > pane.getBottom()) oy = pane.getBottom() - h - 12;
        
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
        // Compute badge size from content (two rows + header)
        const int pad = 6;
        const int headerH = 20;
        const int gap = 6;
        // Row widths
        const int numCells = 4; const int colGap = 6; const int minCellW = 44; // Row1
        const int row1W = (minCellW * numCells) + (colGap * (numCells - 1));
        const int numChips = 5; const int chipGap = 6; const int minChipW = 52; // Row2
        const int row2W = (minChipW * numChips) + (chipGap * (numChips - 1));
        const int contentW = juce::jmax (row1W, row2W);
        const int minW = 260; // floor
        const int w = juce::jmax (minW, pad + contentW + pad);
        // Height: header + sep + row1 + gap + row2 + pads
        const int rowH = 22; // approximate dynamic row height
        const int h = pad + headerH + 5 /*sep+margin*/ + rowH + gap + rowH + pad;
        auto pane = getLocalBounds();
        
        // Start with band point position
        int ox = (int) x + 14;
        int oy = (int) y - h - 8;
        
        // Smart positioning to avoid overlap with band point
        const int bandRadius = 12; // Band point click radius
        const int margin = 20; // Additional margin from band point
        
        // Check if badge would overlap with band point
        bool overlapsBand = (ox <= x + bandRadius + margin &&
                           ox + w >= x - bandRadius - margin &&
                           oy <= y + bandRadius + margin &&
                           oy + h >= y - bandRadius - margin);
        
        if (overlapsBand)
        {
            // Position badge to the right of band point
            ox = (int) x + bandRadius + margin;
            oy = (int) y - h/2;
            
            // If that goes off screen, try to the left
            if (ox + w > pane.getRight())
            {
                ox = (int) x - w - bandRadius - margin;
            }
            
            // If still off screen, try above
            if (ox < pane.getX() || ox + w > pane.getRight())
            {
                ox = (int) x - w/2;
                oy = (int) y - h - bandRadius - margin;
            }
            
            // If still off screen, try below
            if (oy < pane.getY())
            {
                oy = (int) y + bandRadius + margin;
            }
        }
        
        // Final bounds checking
        if (ox < pane.getX()) ox = pane.getX() + 12;
        if (ox + w > pane.getRight()) ox = pane.getRight() - w - 12;
        if (oy < pane.getY()) oy = pane.getY() + 12;
        if (oy + h > pane.getBottom()) oy = pane.getBottom() - h - 12;
        
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

        // Drawer toggle button on the right of badge (wider for easier click)
        const int btnGap = 8;
        const int btnW = 26;
        hudButton.setBounds (ox + w + btnGap, oy, btnW, h); // vertical bar button full badge height
        hudButton.setVisible (true);
        // Style as vertical accent bar
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            hudButton.setColour (juce::TextButton::buttonColourId, lf->theme.accent.withAlpha (hudOpen ? 0.85f : 0.35f));
        else
            hudButton.setColour (juce::TextButton::buttonColourId, juce::Colours::deepskyblue.withAlpha (hudOpen ? 0.85f : 0.35f));
        hudButton.toFront (true);

        // Position HUD as a drawer coming off the bottom of the badge when open
        if (hudOpen)
        {
            const int hudW = 220;
            const int hudH = juce::jmax (detHud.getCollapsedHeight(), h);
            int hudX = ox; // align to badge left by default
            int hudY = oy + h + 8; // below badge
            if (hudX + hudW > pane.getRight()) hudX = pane.getRight() - hudW - 8;
            if (hudX < pane.getX()) hudX = pane.getX() + 8;
            if (hudY + hudH > pane.getBottom()) hudY = oy - hudH - 8; // if off bottom, place above badge
            detHud.setBounds (hudX, hudY, hudW, hudH);
            detHud.setVisible (true);
            detHud.toFront (true);
            hudButton.toFront (true);
        }
        else
        {
            detHud.setVisible (false);
        }
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
        const float halfRange = zoomState.getCurrent();
        const float dbVals[] = { 18, 12, 6, 0, -6, -12, -18, -24, -30, -36 };
        for (float dbv : dbVals)
        {
            if (dbv > halfRange || dbv < -halfRange) continue;
            const float y = mapDbToY (dbv);
            g.setColour (gridCol);
            g.drawLine (r.getX(), y, r.getRight(), y, dbv == 0 ? 1.2f : 0.6f);
            g.setColour (textCol);
            juce::String lbl = juce::String ((int) dbv) + " dB";
            g.drawFittedText (lbl, juce::Rectangle<int> ((int) r.getX()+4, (int) y-8, 44, 16), juce::Justification::centredLeft, 1);
        }

        // Hz ticks (full analyzer height; keep labels near bottom)
        const double hzTicks[] = { 20, 50, 100, 200, 500, 1000, 1500, 2000, 3000, 4000, 5000, 7000, 8000, 10000, 20000 };
        for (double hz : hzTicks)
        {
            const float x = mapHzToX ((float) hz);
            g.setColour (gridCol);
            g.drawLine (x, r.getY(), x, r.getBottom(), 0.6f);
            g.setColour (textCol);
            juce::String lbl;
            if (hz >= 1000.0 && hz < 10000.0) lbl = juce::String (hz/1000.0, 1) + "k";
            else if (hz >= 10000.0) lbl = juce::String ((int) std::round (hz/1000.0)) + "k";
            else lbl = juce::String ((int) hz);
            g.drawFittedText (lbl, juce::Rectangle<int> ((int) x-18, (int) r.getBottom()-18, 36, 14), juce::Justification::centred, 1);
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
    
    // Zoom state and rail
    ZoomState zoomState;
    DynEqZoomSideRail zoomRail;
    BandDetectorHUDView detHud;
    juce::TextButton hudButton;
    bool hudOpen { false };

    // Helper: launch HP/LP popover anchored to overlay chip
    void showFreqPopover (juce::Rectangle<int> chipLocalToOverlay, bool isHP)
    {
        const int bi = (selected >= 0 ? points[(size_t) selected].bandIdx : -1); if (bi < 0) return;
        auto anchor = chipLocalToOverlay; // chip rect in overlay space
        auto screen = overlay.getScreenBounds();
        anchor.setPosition (screen.getX() + chipLocalToOverlay.getX(), screen.getY() + chipLocalToOverlay.getY());
        struct FreqPane : juce::Component
        {
            juce::Slider s; juce::Label title, val;
            std::function<void(float)> onCommit;
            void resized() override {
                auto r = getLocalBounds().reduced(12);
                title.setBounds(r.removeFromTop(18));
                r.removeFromTop(6);
                s.setBounds(r.removeFromTop(28));
                r.removeFromTop(6);
                val.setBounds(r.removeFromTop(18));
            }
            FreqPane(const juce::String& t, float startHz, std::function<void(float)> commit)
            {
                onCommit = std::move(commit);
                title.setText(t, juce::dontSendNotification);
                addAndMakeVisible(title);
                s.setRange(20.0, 20000.0, 0.01);
                s.setSkewFactorFromMidPoint(1000.0);
                s.setValue(startHz, juce::dontSendNotification);
                s.onValueChange = [this]{ val.setText(hzText((float)s.getValue()), juce::dontSendNotification); };
                s.onDragEnd = [this]{ if (onCommit) onCommit((float) s.getValue()); };
                addAndMakeVisible(s);
                val.setJustificationType(juce::Justification::centredRight);
                addAndMakeVisible(val);
                setSize(300, 80);
            }
            static juce::String hzText(float hz){
                return hz >= 1000.f ? juce::String(hz/1000.f, hz<10000?1:0) + " kHz" : juce::String((int)hz) + " Hz";
            }
        };
        const float startHz = (isHP ? currentHpHz(bi) : currentLpHz(bi));
        auto pane = std::make_unique<FreqPane>(isHP ? "High-pass" : "Low-pass", startHz,
            [this, bi, isHP](float hz){ setBandParam(bi, isHP ? dynEq::Band::dynDetHPHz : dynEq::Band::dynDetLPHz, hz); });
        juce::CallOutBox::launchAsynchronously(std::move(pane), anchor, nullptr);
    }

    private:
    float currentHpHz (int bandIdx) const { return getBandParamFloat (bandIdx, dynEq::Band::dynDetHPHz, 20.0f); }
    float currentLpHz (int bandIdx) const { return getBandParamFloat (bandIdx, dynEq::Band::dynDetLPHz, 20000.0f); }
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


