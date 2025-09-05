#pragma once
#include <JuceHeader.h>
#include "ProcessedSpectrumPane.h"

class XYPad; // forward (lives in PluginEditor.h/cpp)

// Lightweight XY wrapper so we can compile without including PluginEditor.h here
class XYPaneAdapter : public juce::Component
{
public:
    XYPaneAdapter (XYPad& padRef) : pad (padRef) { addAndMakeVisible (pad); }
    void resized() override { pad.setBounds (getLocalBounds()); }
private:
    XYPad& pad;
};

enum class PaneID { XY=0, Spectrum=1, Imager=2 };

static inline const char* paneKey (PaneID id)
{
    switch (id) { case PaneID::XY: return "xy"; case PaneID::Spectrum: return "spec"; case PaneID::Imager: return "imager"; }
    return "xy";
}

class PaneManager : public juce::Component
{
public:
    struct Options { bool keepAllWarm = false; };

    PaneManager (juce::ValueTree& state, juce::LookAndFeel* lnf, XYPad& xyPadRef)
        : vt(state)
    {
        xy   = std::make_unique<XYPaneAdapter> (xyPadRef);
        spec = std::make_unique<ProcessedSpectrumPane> (lnf);
        imgr = std::make_unique<juce::Component>(); // placeholder

        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get() })
            addChildComponent (c);

        addAndMakeVisible (tabs);
        tabs.onSelect = [this](PaneID id){ setActive (id, true); };
        tabs.onShowMenu = [this](juce::Point<int> where)
        {
            juce::PopupMenu m;
            m.addItem (1, "XY");
            m.addItem (2, "Spectrum");
            m.addItem (3, "Imager");
            m.addSeparator();
            m.addItem (4, options.keepAllWarm ? "Keep Warm: On" : "Keep Warm: Off");
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&tabs),
                             [this](int r)
                             {
                                 if (r == 1) setActive (PaneID::XY, true);
                                 else if (r == 2) setActive (PaneID::Spectrum, true);
                                 else if (r == 3) setActive (PaneID::Imager, true);
                                 else if (r == 4) { setOptions({ !options.keepAllWarm }); tabs.repaint(); }
                             });
        };

        auto s = vt.getProperty ("ui_activePane").toString();
        PaneID initial = (s=="spec") ? PaneID::Spectrum : (s=="imager") ? PaneID::Imager : PaneID::XY;
        setActive (initial, false);

        for (auto id : { PaneID::XY, PaneID::Spectrum, PaneID::Imager })
        {
            auto key = juce::String("ui_shade_") + paneKey(id);
            if (! vt.hasProperty (key)) vt.setProperty (key, 0.0f, nullptr);
        }

        // restore keep-warm option
        if (vt.hasProperty ("ui_keepWarm")) options.keepAllWarm = (bool) vt.getProperty ("ui_keepWarm");
    }

    void setOptions (Options o) { options = o; vt.setProperty ("ui_keepWarm", options.keepAllWarm, nullptr); }
    Options getOptions() const { return options; }

    void setSampleRate (double fs)
    {
        if (spec) spec->setSampleRate (fs);
    }

    void onAudioSample (double, double) {}
    void onAudioBlock (const float* L, const float* R, int n)
    {
        if (options.keepAllWarm) { if (spec) spec->onAudioBlock (L,R,n); }
        else                      { if (getActiveID()==PaneID::Spectrum && spec) spec->onAudioBlock (L,R,n); }
    }
    void onAudioBlockPre (const float* L, const float* R, int n)
    {
        if (options.keepAllWarm) { if (spec) spec->onAudioBlockPre (L,R,n); }
        else                      { if (getActiveID()==PaneID::Spectrum && spec) spec->onAudioBlockPre (L,R,n); }
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
        if (id == active) return;
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get() })
            c->setVisible (false);
        active = id;
        getActive()->setVisible (true);
        if (persist) vt.setProperty ("ui_activePane", paneKey(id), nullptr);
        tabs.current = id; tabs.repaint(); resized();
        if (onActivePaneChanged) onActivePaneChanged (active);
    }

    PaneID getActiveID() const { return active; }
    juce::Component* getActive()
    {
        switch (active) { case PaneID::XY: return xy.get(); case PaneID::Spectrum: return spec.get(); case PaneID::Imager: return imgr.get(); }
        return xy.get();
    }

    ProcessedSpectrumPane* spectrumPane() { return spec.get(); }

    void resized() override
    {
        auto r = getLocalBounds().toFloat();
        auto tabsR = r.removeFromBottom (28.0f).toNearestInt();
        tabs.setBounds (tabsR);
        auto paneR = r.toNearestInt();
        for (auto* c : { (juce::Component*) xy.get(), (juce::Component*) spec.get(), (juce::Component*) imgr.get() })
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
            const int N = 3; float w = b.getWidth() / (float) N;
            auto draw = [&](int i, const juce::String& label, PaneID id)
            {
                juce::Rectangle<float> r (b.getX() + i*w, b.getY(), w, b.getHeight());
                const bool on = (current == id);
                auto rr = r.reduced (2);
                // base fill
                g.setColour (juce::Colour (0xFF2C2F35));
                g.fillRoundedRectangle (rr, 7.0f);
                if (on)
                {
                    // active pill
                    g.setColour (juce::Colour (0xFF3A3E45));
                    g.fillRoundedRectangle (rr, 7.0f);
                    // subtle inner stroke
                    g.setColour (juce::Colours::white.withAlpha (0.08f));
                    g.drawRoundedRectangle (rr, 7.0f, 1.2f);
                    // bottom accent bar
                    g.setColour (juce::Colour (0xFF5AA9E6));
                    g.fillRect (juce::Rectangle<float> (rr.getX()+6.0f, rr.getBottom()-2.5f, rr.getWidth()-12.0f, 2.5f));
                }
                else
                {
                    // inactive stroke
                    g.setColour (juce::Colours::black.withAlpha (0.35f));
                    g.drawRoundedRectangle (rr, 7.0f, 1.0f);
                }
                // label
                g.setColour (juce::Colours::white.withAlpha (on ? 0.95f : 0.65f));
                g.drawText (label, rr.toNearestInt(), juce::Justification::centred);
            };
            draw (0, "XY",       PaneID::XY);
            draw (1, "Spectrum", PaneID::Spectrum);
            draw (2, "Imager",   PaneID::Imager);
        }
        void mouseUp (const juce::MouseEvent& e) override
        {
            const int N = 3; int idx = juce::jlimit (0, N-1, e.x * N / juce::jmax (1, getWidth()));
            PaneID id = idx==0 ? PaneID::XY : idx==1 ? PaneID::Spectrum : PaneID::Imager;
            current = id; if (onSelect) onSelect (id); repaint();
            if (e.mods.isPopupMenu()) { if (onShowMenu) onShowMenu (e.getPosition()); }
        }
    } tabs;

    std::function<void(PaneID)> onActivePaneChanged;

private:
    juce::ValueTree& vt;
    Options options;
    PaneID active { PaneID::XY };

    std::unique_ptr<XYPaneAdapter>         xy;
    std::unique_ptr<ProcessedSpectrumPane> spec;
    std::unique_ptr<juce::Component>       imgr;
};


