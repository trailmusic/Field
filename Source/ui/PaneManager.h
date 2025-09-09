#pragma once
#include <JuceHeader.h>
#include "ProcessedSpectrumPane.h"
#include "ImagerPane.h"
#include "machine/MachinePane.h"
#include "../IconSystem.h"

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

enum class PaneID { XY=0, Spectrum=1, Imager=2, Band=3, Motion=4, Machine=5 };

static inline const char* paneKey (PaneID id)
{
    switch (id) {
        case PaneID::XY: return "xy";
        case PaneID::Spectrum: return "spec";
        case PaneID::Imager: return "imager";
        case PaneID::Band: return "band";
        case PaneID::Motion: return "motion";
        case PaneID::Machine: return "machine";
    }
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
        band = std::make_unique<juce::Component>(); // scaffold placeholder
        motion = std::make_unique<juce::Component>(); // scaffold placeholder
        mach = std::make_unique<MachinePane>(p, state, lnf);

        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) band.get(), (juce::Component*) motion.get(), (juce::Component*) mach.get() })
            addChildComponent (c);

        addAndMakeVisible (tabs);
        tabs.onSelect = [this](PaneID id){ setActive (id, true); };
        tabs.onShowMenu = [this](juce::Point<int> where)
        {
            juce::PopupMenu m;
            m.addItem (1, "XY");
            m.addItem (2, "Dynamic EQ");
            m.addItem (3, "Imager");
            m.addItem (4, "Band");
            m.addItem (5, "Motion");
            m.addItem (6, "Machine");
            m.addSeparator();
            m.addItem (7, options.keepAllWarm ? "Keep Warm: On" : "Keep Warm: Off");
            // Smoothing preset toggle for Spectrum
            if (spec)
            {
                auto preset = spec->analyzer().getSmoothingPreset();
                const bool silky = (preset == SpectrumAnalyzer::SmoothingPreset::Silky);
                m.addItem (8, juce::String("Smoothing: ") + (silky ? "Silky" : "Clean"));
            }
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&tabs),
                             [this](int r)
                             {
                                 if (r == 1) setActive (PaneID::XY, true);
                                 else if (r == 2) setActive (PaneID::Spectrum, true);
                                 else if (r == 3) setActive (PaneID::Imager, true);
                                 else if (r == 4) setActive (PaneID::Band, true);
                                 else if (r == 5) setActive (PaneID::Motion, true);
                                 else if (r == 6) setActive (PaneID::Machine, true);
                                 else if (r == 7) { setOptions({ !options.keepAllWarm }); tabs.repaint(); }
                                 else if (r == 8)
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
        PaneID initial = (s=="spec") ? PaneID::Spectrum : (s=="imager") ? PaneID::Imager : (s=="machine") ? PaneID::Machine : (s=="band") ? PaneID::Band : (s=="motion") ? PaneID::Motion : PaneID::XY;
        setActive (initial, false);
        if (spec)
        {
            if (initial == PaneID::Spectrum) spec->analyzer().resumeAudio();
            else                             spec->analyzer().pauseAudio();
        }

        for (auto id : { PaneID::XY, PaneID::Spectrum, PaneID::Imager, PaneID::Band, PaneID::Motion, PaneID::Machine })
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
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) band.get(), (juce::Component*) motion.get(), (juce::Component*) mach.get() })
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
        switch (active) { case PaneID::XY: return xy.get(); case PaneID::Spectrum: return spec.get(); case PaneID::Imager: return imgr.get(); case PaneID::Band: return band.get(); case PaneID::Motion: return motion.get(); case PaneID::Machine: return mach.get(); }
        return xy.get();
    }

    ProcessedSpectrumPane* spectrumPane() { return spec.get(); }

    void resized() override
    {
        auto full = getLocalBounds();
        // Larger tabs for easier target; no overlap with content
        const int tabHeight = 40; // was 32
        const int tabTopPad = 0;  // was 10 (overlap source)
        juce::Rectangle<int> tabsR (full.getX(), full.getY(), full.getWidth(), tabHeight + tabTopPad);
        tabs.setBounds (tabsR);

        // Content starts directly under tabs with a tiny breathing space
        auto paneTop = tabs.getBottom() + 2;
        juce::Rectangle<int> paneR (full.getX(), paneTop, full.getWidth(), full.getBottom() - paneTop);
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get(), (juce::Component*) mach.get(), (juce::Component*) band.get(), (juce::Component*) motion.get() })
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
            const int N = 6; float w = b.getWidth() / (float) N;
            auto iconFor = [] (PaneID id) -> IconSystem::IconType
            {
                switch (id)
                {
                    case PaneID::XY:       return IconSystem::Stereo;
                    case PaneID::Spectrum: return IconSystem::Tilt;      // Dynamic EQ
                    case PaneID::Imager:   return IconSystem::Width;
                    case PaneID::Band:     return IconSystem::Mono;
                    case PaneID::Motion:   return IconSystem::Pan;
                    case PaneID::Machine:  return IconSystem::CogWheel;
                }
                return IconSystem::Stereo;
            };
            auto draw = [&](int i, const juce::String& label, PaneID id)
            {
                juce::Rectangle<float> r (b.getX() + i*w, b.getY(), w, b.getHeight());
                const bool on = (current == id);
                auto rr = r.reduced (1.0f);
                // base fill
                g.setColour (juce::Colour (0xFF2C2F35));
                g.fillRoundedRectangle (rr, 9.0f);
                if (on)
                {
                    // active pill
                    g.setColour (juce::Colour (0xFF3A3E45));
                    g.fillRoundedRectangle (rr, 9.0f);
                    // bright border for active state
                    auto accent = juce::Colour (0xFF5AA9E6);
                    g.setColour (accent.withAlpha (0.85f));
                    g.drawRoundedRectangle (rr, 9.0f, 1.6f);
                    // subtle outer glow around active pill (reduced passes to keep compact)
                    for (int i = 1; i <= 3; ++i)
                    {
                        float expand = (float) i * 0.9f;
                        g.setColour (accent.withAlpha (0.14f - i * 0.03f));
                        auto glowR = rr.expanded (expand);
                        g.drawRoundedRectangle (glowR, 9.0f + i * 0.30f, 1.6f);
                    }
                }
                else
                {
                    // Make inactive look like previous active (filled pill with inner stroke)
                    g.setColour (juce::Colour (0xFF3A3E45));
                    g.fillRoundedRectangle (rr, 9.0f);
                    g.setColour (juce::Colours::white.withAlpha (0.08f));
                    g.drawRoundedRectangle (rr, 9.0f, 1.0f);
                }
                // label + icon
                auto txtCol = juce::Colours::white.withAlpha (on ? 0.95f : 0.65f);
                if (id == PaneID::Machine)
                {
                    g.setColour (txtCol);
                    g.setFont (juce::Font (juce::FontOptions (on ? 14.0f : 13.0f).withStyle ("Bold")));
                }
                else
                {
                    g.setColour (txtCol);
                    g.setFont (juce::Font (juce::FontOptions (on ? 13.0f : 12.0f).withStyle ("Bold")));
                }
                const float pad = 6.0f;
                const float iconSz = juce::jmin (rr.getHeight() - 8.0f, 16.0f);
                juce::Rectangle<float> content = rr.reduced (8.0f, 2.0f);
                float textW = g.getCurrentFont().getStringWidthFloat (label);
                float blockW = iconSz + pad + textW;
                float x0 = content.getCentreX() - blockW * 0.5f;
                juce::Rectangle<float> iconR (x0, content.getCentreY() - iconSz * 0.5f, iconSz, iconSz);
                IconSystem::drawIcon (g, iconFor (id), iconR, txtCol);
                juce::Rectangle<int> textR (juce::roundToInt (x0 + iconSz + pad), (int) content.getY(), (int) (content.getRight() - (x0 + iconSz + pad)), (int) content.getHeight());
                juce::String drawLabel = (id == PaneID::Spectrum ? juce::String ("Dynamic EQ") : label);
                g.drawFittedText (drawLabel, textR, juce::Justification::centredLeft, 1);
            };
            draw (0, "XY",         PaneID::XY);
            draw (1, "Dynamic EQ", PaneID::Spectrum);
            draw (2, "Imager",     PaneID::Imager);
            draw (3, "Band",       PaneID::Band);
            draw (4, "Motion",     PaneID::Motion);
            draw (5, "Machine",    PaneID::Machine);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            const int N = 6; int idx = juce::jlimit (0, N-1, e.x * N / juce::jmax (1, getWidth()));
            PaneID id = idx==0 ? PaneID::XY : idx==1 ? PaneID::Spectrum : idx==2 ? PaneID::Imager : idx==3 ? PaneID::Band : idx==4 ? PaneID::Motion : PaneID::Machine;
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
    std::unique_ptr<juce::Component>       band;
    std::unique_ptr<juce::Component>       motion;
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


