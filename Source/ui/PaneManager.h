#pragma once
#include <JuceHeader.h>
#include "ProcessedSpectrumPane.h"
#include "ImagerPane.h"
#include "machine/MachinePane.h"

class XYPad; // forward (lives in PluginEditor.h/cpp)

// Lightweight XY wrapper so we can compile without including PluginEditor.h here
class XYPaneAdapter : public juce::Component
{
public:
    XYPaneAdapter (XYPad& padRef) : pad (padRef) { addAndMakeVisible (pad); }
    void resized() override { pad.setBounds (getLocalBounds()); }
    // Forward realtime-safe oscilloscope samples to the XYPad
    void pushWaveformSample (double L, double R) { pad.pushWaveformSample (L, R); }
private:
    XYPad& pad;
};

enum class PaneID { XY=0, Spectrum=1, Imager=2, Machine=3 };

static inline const char* paneKey (PaneID id)
{
    switch (id) { case PaneID::XY: return "xy"; case PaneID::Spectrum: return "spec"; case PaneID::Imager: return "imager"; case PaneID::Machine: return "machine"; }
    return "xy";
}

class PaneManager : public juce::Component, private juce::Timer
{
public:
    struct Options { bool keepAllWarm = false; };

    PaneManager (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf, XYPad& xyPadRef)
        : proc(p), vt(state)
    {
        xy   = std::make_unique<XYPaneAdapter> (xyPadRef);
        spec = std::make_unique<ProcessedSpectrumPane> (lnf);
        imgr = std::make_unique<ImagerPane>();
        mach = std::make_unique<MachinePane>(p, state, lnf);

        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) mach.get() })
            addChildComponent (c);

        addAndMakeVisible (tabs);
        tabs.onSelect = [this](PaneID id){ setActive (id, true); };
        tabs.onShowMenu = [this](juce::Point<int> where)
        {
            juce::PopupMenu m;
            m.addItem (1, "XY");
            m.addItem (2, "Spectrum");
            m.addItem (3, "Imager");
            m.addItem (4, "Machine");
            m.addSeparator();
            m.addItem (5, options.keepAllWarm ? "Keep Warm: On" : "Keep Warm: Off");
            // Smoothing preset toggle for Spectrum
            if (spec)
            {
                auto preset = spec->analyzer().getSmoothingPreset();
                const bool silky = (preset == SpectrumAnalyzer::SmoothingPreset::Silky);
                m.addItem (6, juce::String("Smoothing: ") + (silky ? "Silky" : "Clean"));
            }
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&tabs),
                             [this](int r)
                             {
                                 if (r == 1) setActive (PaneID::XY, true);
                                 else if (r == 2) setActive (PaneID::Spectrum, true);
                                 else if (r == 3) setActive (PaneID::Imager, true);
                                 else if (r == 4) setActive (PaneID::Machine, true);
                                 else if (r == 5) { setOptions({ !options.keepAllWarm }); tabs.repaint(); }
                                 else if (r == 6)
                                 {
                                     if (spec)
                                     {
                                         auto cur = spec->analyzer().getSmoothingPreset();
                                         auto next = (cur == SpectrumAnalyzer::SmoothingPreset::Silky)
                                                        ? SpectrumAnalyzer::SmoothingPreset::Clean
                                                        : SpectrumAnalyzer::SmoothingPreset::Silky;
                                         spec->analyzer().setSmoothingPreset (next);
                                     }
                                     tabs.repaint();
                                 }
                             });
        };

        auto s = vt.getProperty ("ui_activePane").toString();
        PaneID initial = (s=="spec") ? PaneID::Spectrum : (s=="imager") ? PaneID::Imager : (s=="machine") ? PaneID::Machine : PaneID::XY;
        setActive (initial, false);
        if (spec)
        {
            if (initial == PaneID::Spectrum) spec->analyzer().resumeAudio();
            else                             spec->analyzer().pauseAudio();
        }

        for (auto id : { PaneID::XY, PaneID::Spectrum, PaneID::Imager, PaneID::Machine })
        {
            auto key = juce::String("ui_shade_") + paneKey(id);
            if (! vt.hasProperty (key)) vt.setProperty (key, 0.0f, nullptr);
        }

        // Imager UI state defaults
        if (! vt.hasProperty ("ui_imager_showPre"))   vt.setProperty ("ui_imager_showPre",   true,  nullptr);
        if (! vt.hasProperty ("ui_imager_autoGain"))  vt.setProperty ("ui_imager_autoGain",  true,  nullptr);
        if (! vt.hasProperty ("ui_imager_quality"))   vt.setProperty ("ui_imager_quality",   30,    nullptr); // fps: 15/30/60
        if (! vt.hasProperty ("ui_imager_overlay_bounds")) vt.setProperty ("ui_imager_overlay_bounds", juce::var(), nullptr);
        applyImagerOptionsFromState();

        // restore keep-warm option
        if (vt.hasProperty ("ui_keepWarm")) options.keepAllWarm = (bool) vt.getProperty ("ui_keepWarm");
        startTimerHz (30);
        if (auto* ip = imgr.get())
        {
            ip->onUiChange = [this](const juce::String& key, const juce::var& v)
            {
                if (key == "ui_imager_overlay_bounds_query")
                {
                    // Respond with stored bounds
                    juce::ignoreUnused (v);
                    if (vt.hasProperty ("ui_imager_overlay_bounds"))
                    {
                        auto val = vt.getProperty ("ui_imager_overlay_bounds");
                        if (! val.isVoid())
                        {
                            if (auto* ip2 = imgr.get())
                            {
                                if (auto* o = val.getDynamicObject())
                                {
                                    auto x = (int) o->getProperty ("x"); auto y = (int) o->getProperty ("y");
                                    auto w = (int) o->getProperty ("w"); auto h = (int) o->getProperty ("h");
                                    ip2->setBounds (ip2->getBounds()); // no-op to ensure init
                                    if (auto* im = dynamic_cast<ImagerPane*>(ip2))
                                    {
                                        im->overlayBounds = { x,y,w,h }; im->overlayBoundsSet = true; im->resized();
                                    }
                                }
                            }
                        }
                    }
                    return;
                }
                vt.setProperty (key, v, nullptr);
                if (key == "ui_imager_quality") applyImagerOptionsFromState();
            };
            ip->onParamEdit = [this](const juce::String& id, float value)
            {
                // Route edits to APVTS by parameter ID with host notification
                setParamAutomated (id, value);
            };
            // Designer controls are initialized by the ImagerPane itself
        }
    }
    void timerCallback() override
    {
        static juce::AudioBuffer<float> tmpPre, tmpPost, tmpXY;
        const int maxPull = 2048;
        const bool wantSpec = (active == PaneID::Spectrum) || options.keepAllWarm;
        if (wantSpec && spec)
        {
            int nPost = proc.visPost.pull (tmpPost, maxPull);
            if (nPost > 0)
                spec->analyzer().pushBlock (tmpPost.getReadPointer(0), tmpPost.getNumChannels()>1?tmpPost.getReadPointer(1):nullptr, nPost);
            int nPre = proc.visPre.pull (tmpPre, maxPull);
            if (nPre > 0)
                spec->analyzer().pushBlockPre (tmpPre.getReadPointer(0), tmpPre.getNumChannels()>1?tmpPre.getReadPointer(1):nullptr, nPre);
        }
        const bool wantXY = (active == PaneID::XY) || options.keepAllWarm;
        if (wantXY && xy)
        {
            int nXY = proc.visPost.pull (tmpXY, 1024);
            if (nXY > 0)
            {
                auto* L = tmpXY.getReadPointer(0);
                auto* R = tmpXY.getNumChannels()>1 ? tmpXY.getReadPointer(1) : nullptr;
                for (int i = 0; i < nXY; i += 64)
                    xy->pushWaveformSample (L[i], R ? R[i] : L[i]);
            }
        }
        const bool wantImgr = (active == PaneID::Imager) || options.keepAllWarm;
        if (wantImgr && imgr)
        {
            if (auto* ip = dynamic_cast<ImagerPane*>(imgr.get()))
            {
                int nPost = proc.visPost.pull (tmpPost, maxPull);
                if (nPost > 0)
                    ip->pushBlock (tmpPost.getReadPointer(0), tmpPost.getNumChannels()>1?tmpPost.getReadPointer(1):nullptr, nPost, false);
                int nPre = proc.visPre.pull (tmpPre, maxPull);
                if (nPre > 0)
                    ip->pushBlock (tmpPre.getReadPointer(0), tmpPre.getNumChannels()>1?tmpPre.getReadPointer(1):nullptr, nPre, true);

                // Reflect APVTS values into Imager width editor (keeps XO/widths in sync with knobs)
                if (auto* pLo = proc.apvts.getRawParameterValue ("xover_lo_hz"))
                if (auto* pHi = proc.apvts.getRawParameterValue ("xover_hi_hz"))
                    ip->setCrossovers (*pLo, *pHi);
                if (auto* wLo = proc.apvts.getRawParameterValue ("width_lo"))
                if (auto* wMi = proc.apvts.getRawParameterValue ("width_mid"))
                if (auto* wHi = proc.apvts.getRawParameterValue ("width_hi"))
                    ip->setWidths (*wLo, *wMi, *wHi);
            }
        }
        const bool wantMach = (active == PaneID::Machine) || options.keepAllWarm;
        if (wantMach && mach)
        {
            if (auto* mp = mach.get())
            {
                int nPost = proc.visPost.pull (tmpPost, maxPull);
                if (nPost > 0)
                    mp->pushBlock (tmpPost.getReadPointer(0), tmpPost.getNumChannels()>1?tmpPost.getReadPointer(1):nullptr, nPost);
            }
        }
    }

    void setOptions (Options o) { options = o; vt.setProperty ("ui_keepWarm", options.keepAllWarm, nullptr); }
    Options getOptions() const { return options; }

    void setSampleRate (double fs)
    {
        if (spec) spec->setSampleRate (fs);
        if (auto* ip = dynamic_cast<ImagerPane*>(imgr.get())) ip->setSampleRate (fs);
        if (auto* mp = mach.get()) mp->setSampleRate (fs);
    }

    // Reflect APVTS knob changes back into Imager width editor
    void onParameterChangedForImager (const juce::String& id, float value)
    {
        if (auto* ip = dynamic_cast<ImagerPane*>(imgr.get()))
        {
            if      (id == "xover_lo_hz") ip->setCrossovers (value, (float) vt.getProperty ("xover_hi_hz", 2000.0f));
            else if (id == "xover_hi_hz") ip->setCrossovers ((float) vt.getProperty ("xover_lo_hz", 150.0f), value);
            else if (id == "width_lo")    ip->setWidths (value, ip->getWidthMid(), ip->getWidthHi());
            else if (id == "width_mid")   ip->setWidths (ip->getWidthLo(), value, ip->getWidthHi());
            else if (id == "width_hi")    ip->setWidths (ip->getWidthLo(), ip->getWidthMid(), value);
        }
    }

    void onAudioSample (double L, double R)
    {
        if (xy) xy->pushWaveformSample (L, R);
    }
    void onAudioBlock (const float* L, const float* R, int n)
    {
        auto* s = spec.get();
        if (!s) return;
        const PaneID current = getActiveID();
        if (options.keepAllWarm) { s->onAudioBlock (L, R, n); }
        else if (current == PaneID::Spectrum) { s->onAudioBlock (L, R, n); }
    }
    void onAudioBlockPre (const float* L, const float* R, int n)
    {
        auto* s = spec.get();
        if (!s) return;
        const PaneID current = getActiveID();
        if (options.keepAllWarm) { s->onAudioBlockPre (L, R, n); }
        else if (current == PaneID::Spectrum) { s->onAudioBlockPre (L, R, n); }
    }

    float getActiveShade() const
    {
        return (float) vt.getProperty (juce::String("ui_shade_") + paneKey(active), 0.0f);
    }
    void setActiveShade (float a)
    {
        vt.setProperty (juce::String("ui_shade_") + paneKey(active), juce::jlimit(0.0f,1.0f,a), nullptr);
    }

    void setActive (PaneID id, bool persist)
    {
        const bool changed = (id != active);
        // Always enforce correct visibility, even if selecting the same pane.
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) mach.get() })
            if (c) c->setVisible (false);

        if (changed)
            active = id;
        // Publish active pane for audio-thread readers
        activeAtomic.store ((int) id, std::memory_order_release);

        if (auto* a = getActive())
            a->setVisible (true);

        // Gate spectrum audio feed depending on active tab and keepWarm option
        if (spec)
        {
            if (active == PaneID::Spectrum || options.keepAllWarm) spec->analyzer().resumeAudio();
            else                                                   spec->analyzer().pauseAudio();
        }

        if (persist && changed)
            vt.setProperty ("ui_activePane", paneKey(id), nullptr);

        tabs.current = id;
        tabs.repaint();
        resized();

        if (changed && onActivePaneChanged)
            onActivePaneChanged (active);
    }

    PaneID getActiveID() const { return (PaneID) activeAtomic.load (std::memory_order_acquire); }
    juce::Component* getActive()
    {
        switch (active) { case PaneID::XY: return xy.get(); case PaneID::Spectrum: return spec.get(); case PaneID::Imager: return imgr.get(); case PaneID::Machine: return mach.get(); }
        return xy.get();
    }

    ProcessedSpectrumPane* spectrumPane() { return spec.get(); }

    void resized() override
    {
        auto r = getLocalBounds().toFloat();
        auto tabsR = r.removeFromTop (36.0f).toNearestInt();
        tabs.setBounds (tabsR);
        auto paneR = r.toNearestInt();
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) mach.get() })
            if (c) c->setBounds (paneR);
    }

    struct Tabs : public juce::Component
    {
        std::function<void(PaneID)> onSelect;
        std::function<void(juce::Point<int>)> onShowMenu;
        PaneID current { PaneID::XY };
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            const int N = 4; float w = b.getWidth() / (float) N;
            auto draw = [&](int i, const juce::String& label, PaneID id)
            {
                juce::Rectangle<float> r (b.getX() + i*w, b.getY(), w, b.getHeight());
                const bool on = (current == id);
                auto rr = r.reduced (2);
                // base fill
                g.setColour (juce::Colour (0xFF2C2F35));
                g.fillRoundedRectangle (rr, 9.0f);
                if (on)
                {
                    // active pill
                    g.setColour (juce::Colour (0xFF3A3E45));
                    g.fillRoundedRectangle (rr, 9.0f);
                    // subtle inner stroke
                    g.setColour (juce::Colours::white.withAlpha (0.08f));
                    g.drawRoundedRectangle (rr, 9.0f, 1.4f);
                    // bottom accent bar
                    g.setColour (juce::Colour (0xFF5AA9E6));
                    g.fillRect (juce::Rectangle<float> (rr.getX()+6.0f, rr.getBottom()-3.0f, rr.getWidth()-12.0f, 3.0f));
                }
                else
                {
                    // inactive stroke
                    g.setColour (juce::Colours::black.withAlpha (0.35f));
                    g.drawRoundedRectangle (rr, 9.0f, 1.0f);
                }
                // label
                g.setColour (juce::Colours::white.withAlpha (on ? 0.95f : 0.65f));
                g.drawText (label, rr.toNearestInt(), juce::Justification::centred);
            };
            draw (0, "XY",       PaneID::XY);
            draw (1, "Spectrum", PaneID::Spectrum);
            draw (2, "Imager",   PaneID::Imager);
            draw (3, "Machine",  PaneID::Machine);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            const int N = 4; int idx = juce::jlimit (0, N-1, e.x * N / juce::jmax (1, getWidth()));
            PaneID id = idx==0 ? PaneID::XY : idx==1 ? PaneID::Spectrum : idx==2 ? PaneID::Imager : PaneID::Machine;
            current = id; if (onSelect) onSelect (id); repaint();
            if (e.mods.isPopupMenu()) { if (onShowMenu) onShowMenu (e.getPosition()); }
        }
    } tabs;

    std::function<void(PaneID)> onActivePaneChanged;

private:
    MyPluginAudioProcessor& proc;
    juce::ValueTree& vt;
    Options options;
    PaneID active { PaneID::XY };
    std::atomic<int> activeAtomic { (int) PaneID::XY };

    std::unique_ptr<XYPaneAdapter>         xy;
    std::unique_ptr<ProcessedSpectrumPane> spec;
    std::unique_ptr<ImagerPane>            imgr;
    std::unique_ptr<MachinePane>           mach;

    void applyImagerOptionsFromState()
    {
        if (!imgr) return;
        ImagerPane::Options o;
        o.showPre = (bool) vt.getProperty ("ui_imager_showPre", true);
        o.autoGain = (bool) vt.getProperty ("ui_imager_autoGain", true);
        const int q = (int) vt.getProperty ("ui_imager_quality", 30);
        o.fps = juce::jlimit (10, 120, q);
        imgr->setOptions (o);
    }

    void setParamAutomated (const juce::String& paramID, float value)
    {
        if (auto* p = proc.apvts.getParameter (paramID))
        {
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*> (p))
            {
                const float norm = rp->convertTo0to1 (value);
                rp->beginChangeGesture();
                rp->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, norm));
                rp->endChangeGesture();
            }
        }
    }
};


