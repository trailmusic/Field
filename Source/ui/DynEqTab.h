#pragma once

#include <JuceHeader.h>
#include "SpectrumAnalyzer.h"
#include "../dynEQ/DynamicEqParamIDs.h"

class MyPluginAudioProcessor; // fwd

// Dynamic EQ tab (replaces Spectrum). In-pane experience: visuals + editor.
// Scaffold component so we can integrate DSP/Editor incrementally.
class DynEqTab : public juce::Component
{
public:
    DynEqTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p), lookAndFeelPtr (lnf)
    {
        setOpaque (true);
        addAndMakeVisible (analyzer);
        analyzer.setInterceptsMouseClicks (false, false);
        analyzer.setAutoHeadroomEnabled (true);
        analyzer.setHeadroomTargetFill (0.70f);
        SpectrumAnalyzer::Params prm; prm.fps = 30; analyzer.setParams (prm);

        // Floating per-band mini control panel (Gain/Q). Shown on selection.
        addAndMakeVisible (overlay);
        overlay.setVisible (false);
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
        overlay.onTypeChanged = [this](int tp)
        {
            if (selected >= 0 && selected < (int) points.size())
            {
                points[(size_t) selected].type = tp;
                if (points[(size_t) selected].bandIdx >= 0)
                    setBandParam (points[(size_t) selected].bandIdx, dynEq::Band::type, (float) tp);
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
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        g.fillAll (juce::Colours::darkgrey);
        g.setColour (juce::Colours::white.withAlpha (0.12f));
        g.drawRoundedRectangle (r, 8.0f, 1.2f);

        // background only; overlay drawn in paintOverChildren
    }

    void paintOverChildren (juce::Graphics& g) override
    {
        // Points overlay on top of analyzer
        // Combined EQ curve (approximate visual)
        g.setColour (juce::Colours::cyan.withAlpha (0.85f));
        g.strokePath (eqPath, juce::PathStrokeType (1.6f));

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
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        analyzer.setBounds (r);
        positionOverlay();
    }

    // Analyzer control (PaneManager may wire these later)
    void setSampleRate (double sr) { analyzer.setSampleRate (sr); }
    void pause()  { analyzer.pauseAudio(); }
    void resume() { analyzer.resumeAudio(); }
    void pushBlock (const float* L, const float* R, int n)    { analyzer.pushBlock (L, R, n); }
    void pushBlockPre (const float* L, const float* R, int n) { analyzer.pushBlockPre (L, R, n); }

private:
    struct BandPoint { float hz=1000.f; float db=0.f; float q=0.707f; int type=0; int phase=1; int channel=0; int bandIdx=-1; };
    std::vector<BandPoint> points;
    int selected { -1 };
    juce::Path eqPath;

    // Floating band editor overlay
    class BandOverlay : public juce::Component
    {
    public:
        std::function<void(float)> onGainChanged;
        std::function<void(float)> onQChanged;
        std::function<void(int)>   onTypeChanged;
        std::function<void(int)>   onPhaseChanged;
        std::function<void(int)>   onChanChanged;
        BandOverlay()
        {
            setInterceptsMouseClicks (true, true);
            gain.setSliderStyle (juce::Slider::LinearHorizontal);
            gain.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);
            gain.setRange (-24.0, 24.0, 0.1);
            gain.onValueChange = [this]{ if (!updating && onGainChanged) onGainChanged ((float) gain.getValue()); };
            addAndMakeVisible (gain);

            q.setSliderStyle (juce::Slider::LinearHorizontal);
            q.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);
            q.setRange (0.1, 36.0, 0.01);
            q.onValueChange = [this]{ if (!updating && onQChanged) onQChanged ((float) q.getValue()); };
            addAndMakeVisible (q);

            gainLabel.setText ("GAIN", juce::dontSendNotification);
            gainLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (gainLabel);

            qLabel.setText ("Q", juce::dontSendNotification);
            qLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (qLabel);

            // Type / Phase / Channel selectors
            typeLabel.setText ("TYPE", juce::dontSendNotification);
            typeLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (typeLabel);
            typeCb.addItemList (juce::StringArray{ "Bell","LowShelf","HighShelf","HP","LP","Notch","BandPass","AllPass" }, 1);
            typeCb.onChange = [this]{ if (!updating && onTypeChanged) onTypeChanged (typeCb.getSelectedItemIndex()); };
            addAndMakeVisible (typeCb);

            phaseLabel.setText ("PHASE", juce::dontSendNotification);
            phaseLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (phaseLabel);
            phaseCb.addItemList (juce::StringArray{ "Zero","Natural","Linear" }, 1);
            phaseCb.onChange = [this]{ if (!updating && onPhaseChanged) onPhaseChanged (phaseCb.getSelectedItemIndex()); };
            addAndMakeVisible (phaseCb);

            chanLabel.setText ("CHAN", juce::dontSendNotification);
            chanLabel.setJustificationType (juce::Justification::centredLeft);
            addAndMakeVisible (chanLabel);
            chanCb.addItemList (juce::StringArray{ "Stereo","Mid","Side","Left","Right" }, 1);
            chanCb.onChange = [this]{ if (!updating && onChanChanged) onChanChanged (chanCb.getSelectedItemIndex()); };
            addAndMakeVisible (chanCb);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            g.setColour (juce::Colours::black.withAlpha (0.55f));
            g.fillRoundedRectangle (r, 6.0f);
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
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

            r.removeFromTop (8);
            auto half = r.removeFromTop (22);
            typeLabel.setBounds (half.removeFromLeft (40));
            typeCb.setBounds (half.removeFromLeft (90));
            phaseLabel.setBounds (half.removeFromLeft (50));
            phaseCb.setBounds (half);

            auto half2 = r.removeFromTop (22);
            chanLabel.setBounds (half2.removeFromLeft (40));
            chanCb.setBounds (half2.removeFromLeft (120));
        }
        void setValues (float gainDb, float qVal, int typeIdx, int phaseIdx, int chanIdx)
        {
            juce::ScopedValueSetter<bool> sv (updating, true);
            gain.setValue (gainDb, juce::dontSendNotification);
            q.setValue (qVal, juce::dontSendNotification);
            typeCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, typeCb.getNumItems()-1), typeIdx), juce::dontSendNotification);
            phaseCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, phaseCb.getNumItems()-1), phaseIdx), juce::dontSendNotification);
            chanCb.setSelectedItemIndex (juce::jlimit (0, juce::jmax (0, chanCb.getNumItems()-1), chanIdx), juce::dontSendNotification);
        }
    private:
        juce::Slider gain, q;
        juce::Label gainLabel, qLabel, typeLabel, phaseLabel, chanLabel;
        juce::ComboBox typeCb, phaseCb, chanCb;
        bool updating { false };
    } overlay;

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
        return juce::jmap (dB, 18.f, -36.f, top, bottom);
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
        return juce::jmap ((float) py, (float) r.getY(), (float) r.getBottom(), 18.f, -36.f);
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

    void rebuildEqPath()
    {
        eqPath.clear();
        auto r = analyzer.getBounds().toFloat();
        if (r.isEmpty()) return;

        const int N = juce::jmax (128, (int) r.getWidth());
        auto dbAt = [this](double hz)
        {
            if (points.empty()) return 0.0f;
            // Approximate bell contributions in log domain using Gaussian shaped gain with Q
            const double logHz = std::log10 (juce::jlimit (20.0, 20000.0, hz));
            float sumDb = 0.0f;
            for (const auto& pt : points)
            {
                const double logC = std::log10 (juce::jlimit (20.0f, 20000.0f, pt.hz));
                const double bw = juce::jlimit (0.02, 0.50, 0.22 / juce::jmax (0.1, (double) pt.q)); // heuristic width
                const double d = (logHz - logC) / bw;
                const float w = (float) std::exp (-0.5 * d * d);
                sumDb += pt.db * w;
            }
            return sumDb;
        };

        auto mapX = [&](int i)
        {
            const double minHz = 20.0, maxHz = 20000.0;
            const double t = (double) i / (double) (N - 1);
            const double a = std::log10 (minHz), b = std::log10 (maxHz);
            const double logF = juce::jmap (t, 0.0, 1.0, a, b);
            const double hz = std::pow (10.0, logF);
            return std::pair<float,float> ((float) hz, mapDbToY (dbAt (hz)));
        };

        auto p0 = mapX (0); eqPath.startNewSubPath (r.getX(), p0.second);
        for (int i = 1; i < N; ++i)
        {
            auto p = mapX (i);
            const float x = r.getX() + (float) i / (float) (N - 1) * r.getWidth();
            eqPath.lineTo (x, p.second);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        selected = hitTestPoint (e.getPosition());
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

        if (selected < 0 && e.mods.isLeftButtonDown())
        {
            // Add new band at click
            BandPoint bp; bp.hz = juce::jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x)); bp.db = juce::jlimit (-24.f, 24.f, mapYToDb (e.getPosition().y));
            // Allocate APVTS band slot and sync
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
        }

        // Show/update overlay when a band is selected
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            overlay.setValues (pt.db, pt.q, pt.type, pt.phase, pt.channel);
            overlay.setVisible (true);
            positionOverlay();
        }
        else
        {
            overlay.setVisible (false);
        }
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (selected >= 0 && selected < (int) points.size())
        {
            auto& pt = points[(size_t) selected];
            pt.hz = juce::jlimit (20.f, 20000.f, mapXToHz (e.getPosition().x));
            pt.db = juce::jlimit (-24.f, 24.f, mapYToDb (e.getPosition().y));
            if (pt.bandIdx >= 0)
            {
                setBandParam (pt.bandIdx, dynEq::Band::freqHz, pt.hz);
                setBandParam (pt.bandIdx, dynEq::Band::gainDb, pt.db);
            }
            rebuildEqPath();
            repaint();
            overlay.setValues (pt.db, pt.q, pt.type, pt.phase, pt.channel);
            positionOverlay();
        }
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        const int idx = hitTestPoint (e.getPosition());
        if (idx >= 0 && idx < (int) points.size())
        {
            const int bandIdx = points[(size_t) idx].bandIdx;
            if (bandIdx >= 0)
                setBandParam (bandIdx, dynEq::Band::active, 0.0f);
            points.erase (points.begin() + idx);
            if (selected == idx) selected = -1; else if (selected > idx) --selected;
            rebuildEqPath();
            repaint();
            if (selected < 0) overlay.setVisible (false); else { auto& pt2 = points[(size_t) selected]; overlay.setValues (pt2.db, pt2.q, pt2.type, pt2.phase, pt2.channel); positionOverlay(); }
        }
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
            overlay.setValues (pt.db, pt.q, pt.type, pt.phase, pt.channel);
            positionOverlay();
        }
    }

    void positionOverlay()
    {
        if (! overlay.isVisible() || selected < 0 || selected >= (int) points.size()) return;
        const float x = mapHzToX (points[(size_t) selected].hz);
        auto pane = getLocalBounds();
        const int w = 360, h = 84;
        // Fixed Y near bottom; X follows the point's latitude
        int oy = pane.getBottom() - h - 12;
        // Center overlay around the point's X, clamped within pane
        int ox = (int) x - (w / 2);
        if (ox < pane.getX()) ox = pane.getX() + 12;
        if (ox + w > pane.getRight()) ox = pane.getRight() - w - 12;
        overlay.setBounds (juce::Rectangle<int> (ox, oy, w, h));
        overlay.toFront (false);
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
};


