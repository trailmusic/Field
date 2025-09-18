#pragma once

#include <JuceHeader.h>
#include "DelayUiBridge.h"
#include "../../FieldLookAndFeel.h"

// Composite canvas for Delay visuals: timeline (sync/time/ghost taps), loop EQ strip, ducking meter.
class DelayVisuals : public juce::Component, private juce::Timer
{
public:
    DelayVisuals (DelayUiBridge& bridgeRef, juce::AudioProcessor* proc)
        : bridge (bridgeRef), processor (proc)
    {
        startTimerHz (60);
        setInterceptsMouseClicks (true, true);
    }

    void setScopes (std::function<int(juce::AudioBuffer<float>&, int)> pullPre,
                    std::function<int(juce::AudioBuffer<float>&, int)> pullPost)
    {
        pullPreBus  = std::move (pullPre);
        pullPostBus = std::move (pullPost);
    }

    void paint (juce::Graphics& g) override
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto bg = lf ? lf->theme.panel : juce::Colours::black;
        g.fillAll (bg);

        auto r = getLocalBounds().toFloat();
        const float pad = 8.0f;
        const float titleH = 20.0f;

        // Header with key labels
        juce::Rectangle<float> header = r.removeFromTop (titleH + 4.0f);
        paintHeader (g, header);

        auto timeline = r.removeFromTop (r.getHeight() * 0.45f).reduced (pad);
        auto eqStrip  = r.removeFromTop (r.getHeight() * 0.30f).reduced (pad);
        auto ducking  = r.reduced (pad);

        paintTimeline (g, timeline);
        paintEqStrip  (g, eqStrip);
        paintDucking  (g, ducking);
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (menuButton.contains (e.getPosition().toFloat()))
        {
            juce::PopupMenu m;
            m.addSectionHeader ("Modulation");
            m.addItem (1, "Tape wobble");
            m.addItem (2, "Chorus swirl");
            m.addSectionHeader ("Tone");
            m.addItem (3, "Tighten lows");
            m.addItem (4, "Darken highs");
            m.addSectionHeader ("Stereo/Space");
            m.addItem (5, "Enable Ping-Pong");
            m.addItem (6, "Smear tails");
            m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
                             [this](int r)
                             {
                                 if (r == 1) { setParam ("delay_mod_rate_hz", 0.35f); setParam ("delay_mod_depth_ms", 5.0f); }
                                 else if (r == 2) { setParam ("delay_mod_rate_hz", 1.8f); setParam ("delay_mod_depth_ms", 8.0f); }
                                 else if (r == 3) { setParam ("delay_hp_hz", 180.0f); }
                                 else if (r == 4) { setParam ("delay_lp_hz", 8000.0f); }
                                 else if (r == 5) { setParam ("delay_pingpong", 1.0f); }
                                 else if (r == 6) { setParam ("delay_diffusion", 0.6f); setParam ("delay_diffuse_size_ms", 24.0f); }
                             });
        }
    }

    // Host wiring: allow parent to provide a parameter setter (APVTS write-through)
    void setParamSetter (std::function<void(const juce::String&, float)> setter) { paramSetter = std::move (setter); }

private:
    void timerCallback() override
    {
        // Pull the latest metrics frame; if none, keep last
        DelayMetricsFrame f;
        if (bridge.pullLatest (f)) last = f;

        // Pull audio buffers for scope visualization (pre/post)
        if (pullPreBus)  pullIntoRing (pullPreBus,  preRingL, preRingR);
        if (pullPostBus) pullIntoRing (pullPostBus, postRingL, postRingR);
        repaint();
    }

    void paintTimeline (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto fg = lf ? lf->theme.text : juce::Colours::white;
        auto grid = lf ? lf->theme.sh : juce::Colours::grey;

        g.setColour (grid.withAlpha (0.5f));
        g.drawRoundedRectangle (area, 6.0f, 1.0f);

        // Draw S/D/T markers and animated playhead
        const int divs = 8;
        for (int i = 0; i <= divs; ++i)
        {
            float x = juce::jmap ((float) i, 0.0f, (float) divs, area.getX(), area.getRight());
            g.setColour (grid.withAlpha (i % 2 == 0 ? 0.45f : 0.25f));
            g.drawVerticalLine ((int) std::round (x), area.getY(), area.getBottom());
        }
        // Playhead sweeps based on delay time
        double ms = last.timeMs > 0.0 ? last.timeMs : 250.0;
        const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
        const double period = juce::jmax (0.05, ms * 0.001);
        float ph = (float) std::fmod (t, period) / (float) period;
        float xPlay = juce::jmap (ph, 0.0f, 1.0f, area.getX(), area.getRight());
        g.setColour ((lf ? lf->theme.accent : juce::Colours::orange).withAlpha (0.8f));
        g.drawLine (xPlay, area.getY(), xPlay, area.getBottom(), 1.5f);

        // Ghost taps based on feedback
        const int taps = 5;
        for (int t = 1; t <= taps; ++t)
        {
            float decay = std::pow ((float) juce::jlimit (0.0, 0.99, last.feedbackPct / 100.0), (float) t);
            float x = juce::jmap ((float) t, 0.0f, (float) taps, area.getX(), area.getRight());
            juce::Rectangle<float> mark (x - 2.0f, area.getY(), 4.0f, area.getHeight());
            g.setColour (fg.withAlpha (juce::jlimit (0.1f, 0.9f, 0.7f * (1.0f - decay))));
            g.fillRect (mark.removeFromTop (area.getHeight() * 0.6f).withY (area.getY() + area.getHeight() * 0.2f));
        }

        // Waveforms: Pre (background) and Post (accent)
        auto waves = area.reduced (6.0f);
        drawWaveform (g, waves, preRingL, preRingR, grid.withAlpha (0.5f), false);
        drawWaveform (g, waves, postRingL, postRingR, (lf?lf->theme.accent:juce::Colours::orange).withAlpha (0.8f), true);

        // Labels row
        g.setColour (fg.withAlpha (0.9f));
        auto labelRow = area.removeFromTop (18.0f);
        juce::String modeStr = (last.mode==0?"Digital": last.mode==1?"Analog":"Tape");
        juce::String syncStr = last.sync ? juce::String("Sync ") + (last.gridFlavor==1?"Dotted": last.gridFlavor==2?"Triplet":"Straight") : "Free";
        juce::String timeStr = last.sync ? juce::String(last.timeDiv) + " div" : juce::String(juce::roundToInt(last.timeMs)) + " ms";
        juce::String leftT  = juce::String("L ") + juce::String((int) last.timeSamplesL) + " smp";
        juce::String rightT = juce::String("R ") + juce::String((int) last.timeSamplesR) + " smp";
        juce::String ping   = last.pingpong ? "Ping-Pong" : "Stereo";
        juce::String feedback = juce::String("FB ") + juce::String((int) last.feedbackPct) + "%";
        juce::String wet = juce::String("Wet ") + juce::String(juce::roundToInt(last.wet01*100.0)) + "%";
        juce::Array<juce::String> labels { modeStr, syncStr, timeStr, leftT, rightT, ping, feedback, wet };
        auto eachW = labelRow.getWidth() / labels.size();
        for (int i=0;i<labels.size();++i)
        {
            auto cell = labelRow.removeFromLeft (eachW);
            g.drawFittedText (labels[i], cell.toNearestInt(), juce::Justification::centred, 1);
        }
    }

    void paintEqStrip (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto fg = lf ? lf->theme.text : juce::Colours::white;
        auto grid = lf ? lf->theme.sh : juce::Colours::grey;

        g.setColour (grid.withAlpha (0.5f));
        g.drawRoundedRectangle (area, 6.0f, 1.0f);

        // HP/LP handles
        const float minHz = 20.0f, maxHz = 20000.0f;
        auto xFromHz = [area, minHz, maxHz] (double hz)
        {
            double norm = (std::log (hz) - std::log (minHz)) / (std::log (maxHz) - std::log (minHz));
            return (float) juce::jmap (norm, 0.0, 1.0, (double) area.getX(), (double) area.getRight());
        };
        float xHP = xFromHz (last.hpHz);
        float xLP = xFromHz (last.lpHz);
        g.setColour (fg.withAlpha (0.8f));
        g.fillRect (juce::Rectangle<float> (xHP - 2.0f, area.getY(), 4.0f, area.getHeight()));
        g.fillRect (juce::Rectangle<float> (xLP - 2.0f, area.getY(), 4.0f, area.getHeight()));

        // Tilt line
        float midY = area.getCentreY();
        float tilt = (float) juce::jlimit (-12.0, 12.0, last.tiltDb);
        float slope = tilt / 24.0f; // simple visual mapping
        juce::Line<float> line (area.getX(), midY + slope * area.getHeight() * 0.4f,
                                area.getRight(), midY - slope * area.getHeight() * 0.4f);
        g.setColour (fg.withAlpha (0.4f));
        g.drawLine (line, 2.0f);

        // HP/LP/Tilt labels
        g.setColour (fg.withAlpha (0.9f));
        auto labelRow = area.removeFromTop (16.0f);
        juce::String hpS = juce::String("HP ") + juce::String((int) last.hpHz) + " Hz";
        juce::String lpS = juce::String("LP ") + juce::String((int) last.lpHz) + " Hz";
        juce::String tiltS = juce::String("Tilt ") + juce::String((int) last.tiltDb) + " dB";
        juce::String satS = juce::String("Sat ") + juce::String(juce::roundToInt(last.sat*100.0)) + "%";
        juce::Array<juce::String> labels { hpS, lpS, tiltS, satS };
        auto each = labelRow.getWidth() / labels.size();
        for (int i=0;i<labels.size();++i) g.drawFittedText (labels[i], labelRow.removeFromLeft(each).toNearestInt(), juce::Justification::centred, 1);
    }

    void paintHeader (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto fg = lf ? lf->theme.text : juce::Colours::white;
        g.setColour (fg.withAlpha (0.95f));
        juce::String title = "Delay Panel";
        g.drawFittedText (title, area.toNearestInt(), juce::Justification::centredLeft, 1);
        // Live meters summary on the right
        juce::String meters = juce::String("Pre ") + juce::String((int)(last.preRmsL*100)) + "/" + juce::String((int)(last.preRmsR*100))
                            + "  Wet " + juce::String((int)(last.wetRmsL*100)) + "/" + juce::String((int)(last.wetRmsR*100))
                            + "  GR " + juce::String(juce::roundToInt(last.duckGrDb)) + " dB";
        g.drawFittedText (meters, area.toNearestInt(), juce::Justification::centredRight, 1);

        // Mini menu button
        auto txtW = g.getCurrentFont().getStringWidthFloat (meters);
        juce::Rectangle<float> btn = area.removeFromRight (juce::jmax (20.0f, txtW * 0.0f)).withWidth (22.0f);
        btn = btn.withX (area.getRight() - 24.0f).reduced (2.0f);
        menuButton = btn;
        g.setColour ((lf?lf->theme.accentSecondary:juce::Colours::darkgrey).withAlpha (0.9f));
        g.drawRoundedRectangle (btn, 4.0f, 1.2f);
        g.drawFittedText ("⋯", btn.toNearestInt(), juce::Justification::centred, 1);
    }

    // Helpers
    void drawWaveform (juce::Graphics& g, juce::Rectangle<float> r,
                       const std::vector<float>& L, const std::vector<float>& R,
                       juce::Colour col, bool bright)
    {
        if (L.empty()) return;
        g.setColour (col);
        const int N = (int) L.size();
        auto drawCh = [&] (const std::vector<float>& ch, bool up)
        {
            juce::Path p;
            const int steps = (int) r.getWidth();
            for (int xi = 0; xi < steps; ++xi)
            {
                int idx = juce::jlimit (0, N-1, (int) ((double) xi / steps * N));
                float s = juce::jlimit (-1.0f, 1.0f, ch[idx]);
                float y = up ? (r.getCentreY() - s * r.getHeight() * 0.35f)
                             : (r.getCentreY() + s * r.getHeight() * 0.35f);
                float x = r.getX() + (float) xi;
                if (xi == 0) p.startNewSubPath (x, y); else p.lineTo (x, y);
            }
            g.strokePath (p, juce::PathStrokeType (bright ? 1.6f : 1.0f));
        };
        drawCh (L, true);
        if (!R.empty()) drawCh (R, false);
    }

    template <typename Pull>
    void pullIntoRing (Pull& pull, std::vector<float>& ringL, std::vector<float>& ringR)
    {
        static thread_local juce::AudioBuffer<float> tmp;
        const int maxPull = 1024;
        int n = pull (tmp, maxPull);
        if (n <= 0) return;
        ensureRingSize (ringL, scopeSize);
        ensureRingSize (ringR, scopeSize);
        const float* L = tmp.getReadPointer (0);
        const float* R = tmp.getNumChannels() > 1 ? tmp.getReadPointer (1) : nullptr;
        for (int i = 0; i < n; ++i)
        {
            writeToRing (ringL, L[i]);
            writeToRing (ringR, R ? R[i] : L[i]);
        }
    }

    void ensureRingSize (std::vector<float>& v, int size)
    {
        if ((int) v.size() != size) v.assign (size, 0.0f);
    }
    void writeToRing (std::vector<float>& v, float s)
    {
        if (v.empty()) return;
        ringWrite = (ringWrite + 1) % v.size();
        v[ringWrite] = s;
        // Build a linearized snapshot for painting
        rotateToLinear (v);
    }
    void rotateToLinear (std::vector<float>& v)
    {
        if (v.empty()) return;
        std::rotate (v.begin(), v.begin() + ((ringWrite + 1) % v.size()), v.end());
    }

    void setParam (const juce::String& id, float value)
    {
        if (paramSetter) paramSetter (id, value);
    }

    void paintDucking (juce::Graphics& g, juce::Rectangle<float> area)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto fg = lf ? lf->theme.text : juce::Colours::white;
        auto grid = lf ? lf->theme.sh : juce::Colours::grey;
        auto acc = lf ? lf->theme.accent : juce::Colours::orange;

        g.setColour (grid.withAlpha (0.5f));
        g.drawRoundedRectangle (area, 6.0f, 1.0f);

        // Gain reduction bar (0..24 dB visual range)
        const float maxDb = 24.0f;
        float gr = juce::jlimit (0.0f, maxDb, last.duckGrDb);
        float pct = gr / maxDb;
        auto bar = area.withTrimmedTop (area.getHeight() * (1.0f - pct));
        g.setColour (acc.withAlpha (0.7f));
        g.fillRect (bar);

        // Pre/Post RMS mini bars
        auto w = area.getWidth() / 6.0f;
        auto left = area.removeFromLeft (w);
        auto right = area.removeFromRight (w);
        auto drawRms = [&] (juce::Rectangle<float> r, float l, float rR, juce::Colour c)
        {
            float lm = juce::jlimit (0.0f, 1.0f, l);
            float rm = juce::jlimit (0.0f, 1.0f, rR);
            auto rl = r.removeFromLeft (r.getWidth() * 0.45f);
            auto rr = r.removeFromRight (r.getWidth() * 0.45f);
            g.setColour (c.withAlpha (0.6f));
            g.fillRect (rl.withTrimmedTop (rl.getHeight() * (1.0f - lm)));
            g.fillRect (rr.withTrimmedTop (rr.getHeight() * (1.0f - rm)));
        };
        drawRms (left, last.preRmsL, last.preRmsR, grid.contrasting());
        drawRms (right, last.postRmsL, last.postRmsR, grid.contrasting().brighter());
    }

    DelayUiBridge& bridge;
    juce::AudioProcessor* processor = nullptr;
    DelayMetricsFrame last;
    juce::Rectangle<float> menuButton;

    std::function<int(juce::AudioBuffer<float>&, int)> pullPreBus;
    std::function<int(juce::AudioBuffer<float>&, int)> pullPostBus;

    // Scope rings
    std::vector<float> preRingL, preRingR, postRingL, postRingR;
    int ringWrite = 0;
    const int scopeSize = 2048;

    std::function<void(const juce::String&, float)> paramSetter;
};


