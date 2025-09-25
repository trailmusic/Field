#pragma once
#include <JuceHeader.h>
#include "../ReverbParamIDs.h"
#include "../../FieldLookAndFeel.h"

// Draggable DynEQ editor: 4 band handles on a log-frequency axis with range control.
class ReverbDynEQPane : public juce::Component, private juce::Timer
{
public:
    explicit ReverbDynEQPane (juce::AudioProcessorValueTreeState& s) : state (s)
    {
        using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
        using BA = juce::AudioProcessorValueTreeState::ButtonAttachment;
        using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

        auto styleKnob = [] (juce::Slider& k)
        {
            k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            k.setRotaryParameters (juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                   true);
        };

        auto addBand = [&] (int idx,
                             juce::ToggleButton& on, juce::ComboBox& mode,
                             juce::Slider& f, juce::Slider& g, juce::Slider& q,
                             juce::Slider& thr, juce::Slider& ratio, juce::Slider& att, juce::Slider& rel, juce::Slider& range,
                             std::unique_ptr<BA>& onA, std::unique_ptr<CA>& modeA,
                             std::unique_ptr<SA>& fA, std::unique_ptr<SA>& gA, std::unique_ptr<SA>& qA,
                             std::unique_ptr<SA>& thrA, std::unique_ptr<SA>& ratioA, std::unique_ptr<SA>& attA, std::unique_ptr<SA>& relA, std::unique_ptr<SA>& rangeA,
                             const char* onId, const char* modeId, const char* fId, const char* gId, const char* qId,
                             const char* thrId, const char* ratioId, const char* attId, const char* relId, const char* rangeId)
        {
            addAndMakeVisible (on); addAndMakeVisible (mode);
            addAndMakeVisible (f); addAndMakeVisible (g); addAndMakeVisible (q);
            addAndMakeVisible (thr); addAndMakeVisible (ratio); addAndMakeVisible (att); addAndMakeVisible (rel); addAndMakeVisible (range);
            styleKnob (f); styleKnob (g); styleKnob (q); styleKnob (thr); styleKnob (ratio); styleKnob (att); styleKnob (rel); styleKnob (range);
            mode.addItemList (juce::StringArray { "Bell", "LowShelf", "HighShelf" }, 1);
            onA   = std::make_unique<BA> (state, onId, on);
            modeA = std::make_unique<CA> (state, modeId, mode);
            fA    = std::make_unique<SA> (state, fId, f);
            gA    = std::make_unique<SA> (state, gId, g);
            qA    = std::make_unique<SA> (state, qId, q);
            thrA  = std::make_unique<SA> (state, thrId, thr);
            ratioA= std::make_unique<SA> (state, ratioId, ratio);
            attA  = std::make_unique<SA> (state, attId, att);
            relA  = std::make_unique<SA> (state, relId, rel);
            rangeA= std::make_unique<SA> (state, rangeId, range);
        };

        addBand (1, b1On, b1Mode, b1F, b1G, b1Q, b1Thr, b1Ratio, b1Att, b1Rel, b1Range,
                 b1OnA, b1ModeA, b1FA, b1GA, b1QA, b1ThrA, b1RatioA, b1AttA, b1RelA, b1RangeA,
                 ReverbIDs::dyneq1_on, ReverbIDs::dyneq1_mode, ReverbIDs::dyneq1_freqHz, ReverbIDs::dyneq1_gainDb, ReverbIDs::dyneq1_Q,
                 ReverbIDs::dyneq1_thrDb, ReverbIDs::dyneq1_ratio, ReverbIDs::dyneq1_attMs, ReverbIDs::dyneq1_relMs, ReverbIDs::dyneq1_rangeDb);
        addBand (2, b2On, b2Mode, b2F, b2G, b2Q, b2Thr, b2Ratio, b2Att, b2Rel, b2Range,
                 b2OnA, b2ModeA, b2FA, b2GA, b2QA, b2ThrA, b2RatioA, b2AttA, b2RelA, b2RangeA,
                 ReverbIDs::dyneq2_on, ReverbIDs::dyneq2_mode, ReverbIDs::dyneq2_freqHz, ReverbIDs::dyneq2_gainDb, ReverbIDs::dyneq2_Q,
                 ReverbIDs::dyneq2_thrDb, ReverbIDs::dyneq2_ratio, ReverbIDs::dyneq2_attMs, ReverbIDs::dyneq2_relMs, ReverbIDs::dyneq2_rangeDb);
        addBand (3, b3On, b3Mode, b3F, b3G, b3Q, b3Thr, b3Ratio, b3Att, b3Rel, b3Range,
                 b3OnA, b3ModeA, b3FA, b3GA, b3QA, b3ThrA, b3RatioA, b3AttA, b3RelA, b3RangeA,
                 ReverbIDs::dyneq3_on, ReverbIDs::dyneq3_mode, ReverbIDs::dyneq3_freqHz, ReverbIDs::dyneq3_gainDb, ReverbIDs::dyneq3_Q,
                 ReverbIDs::dyneq3_thrDb, ReverbIDs::dyneq3_ratio, ReverbIDs::dyneq3_attMs, ReverbIDs::dyneq3_relMs, ReverbIDs::dyneq3_rangeDb);
        addBand (4, b4On, b4Mode, b4F, b4G, b4Q, b4Thr, b4Ratio, b4Att, b4Rel, b4Range,
                 b4OnA, b4ModeA, b4FA, b4GA, b4QA, b4ThrA, b4RatioA, b4AttA, b4RelA, b4RangeA,
                 ReverbIDs::dyneq4_on, ReverbIDs::dyneq4_mode, ReverbIDs::dyneq4_freqHz, ReverbIDs::dyneq4_gainDb, ReverbIDs::dyneq4_Q,
                 ReverbIDs::dyneq4_thrDb, ReverbIDs::dyneq4_ratio, ReverbIDs::dyneq4_attMs, ReverbIDs::dyneq4_relMs, ReverbIDs::dyneq4_rangeDb);

        startTimerHz (30);
    }

    ~ReverbDynEQPane() override
    {
        // Stop timer before destruction to prevent use-after-free
        stopTimer();
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (6);
        const int rowH = juce::jmax (24, r.getHeight() / 5);
        auto row = [&](int i){ return juce::Rectangle<int> (r.getX(), r.getY() + (i-1)*rowH, r.getWidth(), rowH).reduced (0, 2); };
        auto placeBand = [&] (int rowIdx, juce::ToggleButton& on, juce::ComboBox& mode,
                              juce::Slider& f, juce::Slider& g, juce::Slider& q,
                              juce::Slider& thr, juce::Slider& ratio, juce::Slider& att, juce::Slider& rel, juce::Slider& range)
        {
            auto rr = row (rowIdx);
            auto c = rr;
            const int w = juce::jmax (40, rr.getWidth() / 10);
            on.setBounds   (c.removeFromLeft (w)); c.removeFromLeft (4);
            mode.setBounds (c.removeFromLeft (w)); c.removeFromLeft (8);
            auto knobW = w; auto placeKnob = [&](juce::Slider& s){ s.setBounds (c.removeFromLeft (knobW)); c.removeFromLeft (4); };
            placeKnob (f); placeKnob (g); placeKnob (q); placeKnob (thr); placeKnob (ratio); placeKnob (att); placeKnob (rel); placeKnob (range);
        };
        placeBand (1, b1On, b1Mode, b1F, b1G, b1Q, b1Thr, b1Ratio, b1Att, b1Rel, b1Range);
        placeBand (2, b2On, b2Mode, b2F, b2G, b2Q, b2Thr, b2Ratio, b2Att, b2Rel, b2Range);
        placeBand (3, b3On, b3Mode, b3F, b3G, b3Q, b3Thr, b3Ratio, b3Att, b3Rel, b3Range);
        placeBand (4, b4On, b4Mode, b4F, b4G, b4Q, b4Thr, b4Ratio, b4Att, b4Rel, b4Range);
    }

    void paint (juce::Graphics& g) override
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        FieldLNF def; const auto& th = lf ? lf->theme : def.theme;
        auto r = getLocalBounds().toFloat().reduced (6);

        // Top: grid/labels; Bottom: editor canvas
        const float headerH = juce::jmin (r.getHeight() * 0.32f, 56.0f);
        auto editor = r.withY (r.getY() + headerH).withHeight (r.getHeight() - headerH);

        // Background
        g.setColour (th.panel.withAlpha (0.6f));
        g.fillRoundedRectangle (editor, 6.0f);
        g.setColour (th.sh);
        g.drawRoundedRectangle (editor, 6.0f, 1.0f);

        // Grid
        g.saveState(); g.reduceClipRegion (editor.toNearestInt());
        g.setColour (th.text.withAlpha (0.10f));
        for (float hz : { 20.f, 50.f, 100.f, 200.f, 500.f, 1000.f, 2000.f, 5000.f, 10000.f, 20000.f })
        {
            const float x = xFromHz (hz, editor);
            g.drawVerticalLine ((int) x, editor.getY(), editor.getBottom());
        }
        for (float db : { 0.f, 6.f, 12.f, 18.f, 24.f })
        {
            const float y = yFromRangeDb (db, editor);
            g.drawHorizontalLine ((int) y, editor.getX(), editor.getRight());
        }

        // Draw handles
        auto drawHandle = [&] (int i, juce::Colour c)
        {
            const auto hz = getF (bandIds[i].freqId, bandDefaults[i].freq);
            const auto range = getF (bandIds[i].rangeId, 6.f);
            const auto on = getB (bandIds[i].onId, false);
            const float x = xFromHz (hz, editor);
            const float y = yFromRangeDb (range, editor);
            const float rad = 6.0f + (i == selected ? 2.0f : 0.0f);
            g.setColour ((on ? c : th.text.withAlpha (0.25f)).withAlpha (0.85f));
            g.fillEllipse (x - rad, y - rad, rad*2, rad*2);
            g.setColour (th.sh);
            g.drawEllipse (x - rad, y - rad, rad*2, rad*2, 1.0f);
        };
        drawHandle (0, juce::Colours::orange.withAlpha (0.9f));
        drawHandle (1, juce::Colours::yellow.withAlpha (0.9f));
        drawHandle (2, juce::Colours::aqua.withAlpha (0.9f));
        drawHandle (3, juce::Colours::lime.withAlpha (0.9f));

        g.restoreState();
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        auto editor = getEditorBounds();
        selected = pickNearestHandle (e.position, editor);
        lastDragPos = e.position;
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        if (selected < 0) return;
        auto editor = getEditorBounds();
        auto p0 = lastDragPos; auto p1 = e.position; lastDragPos = p1;
        // Map position to params
        const float newHz = hzFromX (juce::jlimit (editor.getX(), editor.getRight(), p1.x), editor);
        const float newRange = juce::jlimit (0.f, 24.f, rangeDbFromY (juce::jlimit (editor.getY(), editor.getBottom(), p1.y), editor));
        setF (bandIds[selected].freqId, newHz);
        if (e.mods.isShiftDown())
        {
            // Shift adjusts Q
            const float qOld = getF (bandIds[selected].qId, 1.0f);
            const float qNew = juce::jlimit (0.3f, 8.0f, qOld * std::pow (2.0f, (p0.y - p1.y) / 80.0f));
            setF (bandIds[selected].qId, qNew);
        }
        else if (e.mods.isAltDown())
        {
            // Alt adjusts makeup gain
            const float gOld = getF (bandIds[selected].gainId, 0.0f);
            const float gNew = juce::jlimit (-12.f, 12.f, gOld + (p0.y - p1.y) * 0.05f);
            setF (bandIds[selected].gainId, gNew);
        }
        else
        {
            // Default: adjust dynamic range (max reduction)
            setF (bandIds[selected].rangeId, newRange);
        }
        repaint();
    }

    void mouseDoubleClick (const juce::MouseEvent& e) override
    {
        auto editor = getEditorBounds();
        const int i = pickNearestHandle (e.position, editor);
        if (i >= 0)
        {
            const bool on = getB (bandIds[i].onId, false);
            setB (bandIds[i].onId, !on);
            repaint();
        }
    }

private:
    juce::AudioProcessorValueTreeState& state;

    // Band controls (top rows)
    juce::ToggleButton b1On, b2On, b3On, b4On;
    juce::ComboBox b1Mode, b2Mode, b3Mode, b4Mode;
    juce::Slider b1F, b1G, b1Q, b1Thr, b1Ratio, b1Att, b1Rel, b1Range;
    juce::Slider b2F, b2G, b2Q, b2Thr, b2Ratio, b2Att, b2Rel, b2Range;
    juce::Slider b3F, b3G, b3Q, b3Thr, b3Ratio, b3Att, b3Rel, b3Range;
    juce::Slider b4F, b4G, b4Q, b4Thr, b4Ratio, b4Att, b4Rel, b4Range;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> b1OnA, b2OnA, b3OnA, b4OnA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> b1ModeA, b2ModeA, b3ModeA, b4ModeA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> b1FA, b1GA, b1QA, b1ThrA, b1RatioA, b1AttA, b1RelA, b1RangeA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> b2FA, b2GA, b2QA, b2ThrA, b2RatioA, b2AttA, b2RelA, b2RangeA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> b3FA, b3GA, b3QA, b3ThrA, b3RatioA, b3AttA, b3RelA, b3RangeA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> b4FA, b4GA, b4QA, b4ThrA, b4RatioA, b4AttA, b4RelA, b4RangeA;
    // Editor state
    int selected { -1 };
    juce::Point<float> lastDragPos;

    struct Ids { const char* onId; const char* modeId; const char* freqId; const char* gainId; const char* qId; const char* thrId; const char* ratioId; const char* attId; const char* relId; const char* rangeId; };
    const Ids bandIds[4] = {
        { ReverbIDs::dyneq1_on, ReverbIDs::dyneq1_mode, ReverbIDs::dyneq1_freqHz, ReverbIDs::dyneq1_gainDb, ReverbIDs::dyneq1_Q, ReverbIDs::dyneq1_thrDb, ReverbIDs::dyneq1_ratio, ReverbIDs::dyneq1_attMs, ReverbIDs::dyneq1_relMs, ReverbIDs::dyneq1_rangeDb },
        { ReverbIDs::dyneq2_on, ReverbIDs::dyneq2_mode, ReverbIDs::dyneq2_freqHz, ReverbIDs::dyneq2_gainDb, ReverbIDs::dyneq2_Q, ReverbIDs::dyneq2_thrDb, ReverbIDs::dyneq2_ratio, ReverbIDs::dyneq2_attMs, ReverbIDs::dyneq2_relMs, ReverbIDs::dyneq2_rangeDb },
        { ReverbIDs::dyneq3_on, ReverbIDs::dyneq3_mode, ReverbIDs::dyneq3_freqHz, ReverbIDs::dyneq3_gainDb, ReverbIDs::dyneq3_Q, ReverbIDs::dyneq3_thrDb, ReverbIDs::dyneq3_ratio, ReverbIDs::dyneq3_attMs, ReverbIDs::dyneq3_relMs, ReverbIDs::dyneq3_rangeDb },
        { ReverbIDs::dyneq4_on, ReverbIDs::dyneq4_mode, ReverbIDs::dyneq4_freqHz, ReverbIDs::dyneq4_gainDb, ReverbIDs::dyneq4_Q, ReverbIDs::dyneq4_thrDb, ReverbIDs::dyneq4_ratio, ReverbIDs::dyneq4_attMs, ReverbIDs::dyneq4_relMs, ReverbIDs::dyneq4_rangeDb },
    };
    struct Defaults { float freq; };
    const Defaults bandDefaults[4] { 220.f, 1200.f, 3500.f, 8000.f };

    // Helpers
    juce::Rectangle<float> getEditorBounds() const
    {
        auto r = getLocalBounds().toFloat().reduced (6);
        const float headerH = juce::jmin (r.getHeight() * 0.32f, 56.0f);
        return r.withY (r.getY() + headerH).withHeight (r.getHeight() - headerH);
    }
    static float log01FromHz (float hz)
    {
        const float a = std::log10 (20.f), b = std::log10 (20000.f);
        return juce::jlimit (0.f, 1.f, (std::log10 (juce::jlimit (20.f, 20000.f, hz)) - a) / (b - a));
    }
    static float hzFromLog01 (float n01)
    {
        const float a = std::log10 (20.f), b = std::log10 (20000.f);
        return std::pow (10.f, juce::jlimit (a, b, juce::jmap (n01, 0.f, 1.f, a, b)));
    }
    static float xFromHz (float hz, juce::Rectangle<float> r)
    { return r.getX() + log01FromHz (hz) * r.getWidth(); }
    static float hzFromX (float x, juce::Rectangle<float> r)
    { return hzFromLog01 ((x - r.getX()) / juce::jmax (1.f, r.getWidth())); }
    static float yFromRangeDb (float db, juce::Rectangle<float> r)
    { return juce::jmap (juce::jlimit (0.f, 24.f, db), 0.f, 24.f, r.getBottom() - 8.f, r.getY() + 8.f); }
    static float rangeDbFromY (float y, juce::Rectangle<float> r)
    { return juce::jmap (y, r.getBottom() - 8.f, r.getY() + 8.f, 0.f, 24.f); }
    int pickNearestHandle (juce::Point<float> p, juce::Rectangle<float> editor) const
    {
        int best = -1; float bestD = 1.0e9f;
        for (int i=0;i<4;++i)
        {
            const float x = xFromHz (getF (bandIds[i].freqId, bandDefaults[i].freq), editor);
            const float y = yFromRangeDb (getF (bandIds[i].rangeId, 6.f), editor);
            const float d = p.getDistanceFrom ({ x, y });
            if (d < bestD) { bestD = d; best = i; }
        }
        return (bestD <= 20.f ? best : -1);
    }
    float getF (const char* id, float fb) const
    {
        if (auto* p = state.getRawParameterValue (id)) return p->load();
        return fb;
    }
    bool getB (const char* id, bool fb) const
    {
        if (auto* p = state.getParameter (id)) return p->getValue() >= 0.5f;
        return fb;
    }
    void setF (const char* id, float v)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterFloat*>(state.getParameter (id))) p->operator= (v);
    }
    void setB (const char* id, bool v)
    {
        if (auto* p = dynamic_cast<juce::AudioParameterBool*>(state.getParameter (id))) p->operator= (v);
    }

    void timerCallback() override { repaint(); }
};


