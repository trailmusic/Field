#pragma once
#include <JuceHeader.h>
#include "MachineEngine.h"
#include "ProposalCard.h"
#include "WidthDesignerPanel.h"
#include "../../FieldLookAndFeel.h"
#include "../../IconSystem.h"

class MyPluginAudioProcessor; // fwd

class MachinePane : public juce::Component, private juce::Timer
{
public:
    MachinePane (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf);
    ~MachinePane() override = default;

    void setSampleRate (double sr) { engine.setSampleRate (sr); }
    void pushBlock (const float* L, const float* R, int n) { engine.push (L, R, n); }

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;

private:
    // Minimal cell wrapper to match switch/combobox cell styling
    class SmallSwitchCell : public juce::Component
    {
    public:
        SmallSwitchCell(juce::Component& childToHost) : child(childToHost)
        {
            addAndMakeVisible (child);
            caption.setJustificationType (juce::Justification::centred);
            caption.setInterceptsMouseClicks (false, false);
            addAndMakeVisible (caption);
        }
        void setCaption (const juce::String& text)
        {
            captionText = text;
            caption.setText (captionText, juce::dontSendNotification);
            repaint();
        }
        void resized() override
        {
            auto b = getLocalBounds().reduced (6);
            const int capH = captionText.isNotEmpty() ? 14 : 0;
            if (capH > 0)
            {
                caption.setBounds (b.removeFromTop (capH));
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                    caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
            }
            child.setBounds (b);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat().reduced (3.0f);
            g.setColour (juce::Colour (0xFF3A3E45));
            g.fillRoundedRectangle (r, 8.0f);
            g.setColour (juce::Colours::white.withAlpha (0.12f));
            g.drawRoundedRectangle (r, 8.0f, 1.5f);
        }
    private:
        juce::Component& child;
        juce::Label caption;
        juce::String captionText;
    };

    // UI widgets
    juce::TextButton analyzeBtn { "Learn" }, stopBtn { "" };
    juce::ComboBox   genreBox, venueBox, trackTypeBox;
    // Quality/time controls removed per spec
    juce::Slider     strength; // 0..1
    juce::ToggleButton showPreBtn { "" }; // Pre toggle
    juce::TextButton previewBtn { "Preview 10s" };
    juce::TextButton ABtn { "A" }, BBtn { "B" }, CBtn { "C" }, undoBtn { "Undo" };
    juce::Component  proposalsContent; // holds ProposalCard children (no scrolling)
    std::unique_ptr<SmallSwitchCell> learnCell, stopCell;
    std::unique_ptr<SmallSwitchCell> preCell;
    juce::ToggleButton listenBtn { "Listen" };
    juce::Rectangle<int> barArea;
    std::unique_ptr<WidthDesignerPanel> widthPanel;

    // Header-style bypass button for machine cards (mirrors header BypassButton)
    class CardBypassButton : public juce::TextButton, public juce::Timer
    {
    public:
        CardBypassButton() : juce::TextButton("")
        {
            setLookAndFeel(&customLookAndFeel);
            setClickingTogglesState(true);
            startTimerHz(20);
        }
        ~CardBypassButton() override
        {
            stopTimer();
            setLookAndFeel(nullptr);
        }
        void timerCallback() override
        {
            if (isShowing()) repaint();
        }
    private:
        class CustomLookAndFeel : public juce::LookAndFeel_V4
        {
        public:
            void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                      bool shouldDrawButtonAsHighlighted, bool /*shouldDrawButtonAsDown*/) override
            {
                auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);

                // Read current theme
                juce::Colour accent = juce::Colour(0xFF2196F3);
                juce::Colour textGrey = juce::Colour(0xFFB8BDC7);
                juce::Colour panel = juce::Colour(0xFF3A3D45);
                {
                    const juce::Component* c = &button;
                    while (c)
                    {
                        if (auto* lf = dynamic_cast<FieldLNF*>(&c->getLookAndFeel()))
                        {
                            accent = lf->theme.accent;
                            textGrey = lf->theme.textMuted;
                            panel = lf->theme.panel;
                            break;
                        }
                        c = c->getParentComponent();
                    }
                }

                juce::Colour baseColour;
                if (button.getToggleState())
                {
                    // Bypassed: grey body + subtle blink
                    baseColour = textGrey;
                    auto now = juce::Time::getMillisecondCounter();
                    const bool phase = ((now / 250) % 2) == 0;
                    baseColour = phase ? baseColour.darker(0.35f) : baseColour.brighter(0.05f);
                    g.setColour(baseColour.withAlpha(phase ? 0.35f : 0.18f));
                    g.fillRoundedRectangle(bounds.expanded(4.0f), 6.0f);
                }
                else
                {
                    baseColour = accent;
                }

                if (shouldDrawButtonAsHighlighted)
                    baseColour = baseColour.brighter(0.1f);

                // shadow
                g.setColour(juce::Colour(0x40000000));
                g.fillRoundedRectangle(bounds.translated(2.0f, 2.0f), 4.0f);

                // bg + border with animated stroke width when bypassed
                g.setColour(baseColour);
                g.fillRoundedRectangle(bounds, 4.0f);
                auto now2 = juce::Time::getMillisecondCounter();
                const float pulse = (float) ((now2 / 200) % 2 == 0 ? 2.0f : 1.0f);
                g.setColour(baseColour.darker(0.45f));
                g.drawRoundedRectangle(bounds, 4.0f, pulse);
            }

            void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                bool, bool) override
            {
                auto bounds = button.getLocalBounds().toFloat();
                FieldLNF* lf = nullptr;
                {
                    const juce::Component* c = &button;
                    while (c)
                    {
                        if (auto* l = dynamic_cast<FieldLNF*>(&c->getLookAndFeel())) { lf = l; break; }
                        c = c->getParentComponent();
                    }
                }
                juce::Colour textGrey = lf ? lf->theme.textMuted : juce::Colour(0xFFB8BDC7);
                juce::Colour iconColour = button.getToggleState() ? juce::Colours::black : textGrey;

                auto icon = button.getToggleState() ? IconSystem::X : IconSystem::Power;
                auto pad = button.getToggleState() ? 6.0f : 4.0f;
                IconSystem::drawIcon(g, icon, bounds.reduced(pad), iconColour);
            }
        };
        CustomLookAndFeel customLookAndFeel;
    };

    // --- Three Machine cards (Tone, Reverb/Delay/Motion, Clarity) ---
    class MachineCard : public juce::Component
    {
    public:
        juce::String title;
        float confidence { 0.0f };
        bool  bypassed { false };
        CardBypassButton tggByp;
        juce::Array<float> displayA, displayB;
        juce::String hint;
        juce::NamedValueSet metrics;
        std::vector<ParamDelta> deltas;

        std::function<void(bool)> onBypass;

        MachineCard()
        {
            addAndMakeVisible (tggByp);
            tggByp.onClick   = [this]{ bypassed = tggByp.getToggleState(); if (onBypass) onBypass (bypassed); };
        }
        void setMetrics (const juce::NamedValueSet& m) { metrics = m; repaint(); }
        void setParams (const std::vector<ParamDelta>& p) { deltas = p; repaint(); }
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            auto bg = b.reduced (2.0f);
            g.setColour (juce::Colour (0xFF3A3E45)); g.fillRoundedRectangle (bg, 10.0f);
            g.setColour (juce::Colours::white.withAlpha (0.08f)); g.drawRoundedRectangle (bg, 10.0f, 1.0f);

            auto header = bg.removeFromTop (44.0f);
            header = header.reduced (10.0f, 0.0f);
            g.setFont (juce::Font (15.0f, juce::Font::bold));
            g.setColour (juce::Colours::white.withAlpha (0.92f));
            g.drawText (title, header.removeFromLeft (header.getWidth() - 120.0f).toNearestInt(), juce::Justification::centredLeft);

            // (confidence pill removed to avoid visual conflict near buttons)

            // display removed (was generic graph); keep spacing minimal and draw section headings/dividers instead
            auto disp = bg.removeFromTop (juce::jlimit (22.0f, 26.0f, bg.getHeight() * 0.10f));
            // Section heading: Analysis
            g.setColour (juce::Colours::white.withAlpha (0.70f));
            g.setFont (12.0f);
            g.drawFittedText ("Analysis", disp.toNearestInt(), juce::Justification::centredLeft, 1);
            // divider under heading
            {
                auto dv = disp.withY (disp.getBottom()).withHeight (1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.10f));
                g.fillRect (dv);
            }

            // metrics: small bar group under display
            auto met = bg.removeFromTop (juce::jmin (90.0f, bg.getHeight() * 0.35f));
            if (metrics.size() > 0)
            {
                g.saveState();
                g.reduceClipRegion (met.toNearestInt());
                const int rows = 3;
                auto rowH = (int) std::floor (met.getHeight() / (float) rows);
                int row = 0;
                auto drawMetric = [&](const juce::String& name, float value01)
                {
                    if (row >= rows) return;
                    auto rr = met.removeFromTop ((float) rowH); ++row;
                    rr = rr.reduced (10.0f, 2.0f);
                    auto label = rr.removeFromLeft (90.0f);
                    g.setColour (juce::Colours::white.withAlpha (0.60f));
                    g.setFont (12.0f);
                    g.drawFittedText (name, label.toNearestInt(), juce::Justification::centredLeft, 1);
                    auto bar = rr;
                    g.setColour (juce::Colours::white.withAlpha (0.10f)); g.fillRoundedRectangle (bar, 3.0f);
                    float w = bar.getWidth() * juce::jlimit (0.0f, 1.0f, value01);
                    g.setColour (juce::Colour (0xFF5AA9E6).withAlpha (0.95f));
                    g.fillRoundedRectangle ({ bar.getX(), bar.getY(), w, bar.getHeight() }, 3.0f);
                };
                auto get01 = [&](const juce::String& key, std::function<float(float)> map) -> bool
                {
                    if (auto* v = metrics.getVarPointer (key))
                    {
                        float raw = (float) v->operator float();
                        // Friendly labels
                        juce::String label = key;
                        if (key == "full_corr") label = "Full Corr";
                        else if (key == "corr_low") label = "Low Corr";
                        else if (key == "corr_mid") label = "Mid Corr";
                        else if (key == "corr_hi")  label = "High Corr";
                        else if (key == "slope_db_per_oct") label = "Slope (dB/oct)";
                        else if (key == "dryness_index") label = "Dryness";
                        else if (key == "avg_flux") label = "Flux";
                        else if (key == "Wlo") label = "Width Low";
                        else if (key == "Wmid") label = "Width Mid";
                        else if (key == "Whi") label = "Width High";
                        else if (key == "lf_rumble") label = "Rumble";
                        else if (key == "hf_fizz")   label = "Fizz";
                        else if (key == "sibilance") label = "Sibilance";
                        else if (key == "crest") label = "Crest";
                        drawMetric (label, map (raw));
                        return true;
                    }
                    return false;
                };
                // Show up to 3 common metrics if present
                get01 ("full_corr", [](float x){ return juce::jlimit (0.0f, 1.0f, (x + 1.0f) * 0.5f); });
                get01 ("corr_low",  [](float x){ return juce::jlimit (0.0f, 1.0f, (x + 1.0f) * 0.5f); });
                if (row < rows) get01 ("avg_flux",  [](float x){ return juce::jlimit (0.0f, 1.0f, x); });
                if (row < rows) get01 ("Wmid",      [](float x){ return juce::jlimit (0.0f, 1.0f, x); });
                if (row < rows) get01 ("sibilance", [](float x){ return juce::jlimit (0.0f, 1.0f, x); });
                g.restoreState();
            }

            // divider between metrics and params
            {
                auto dv = met.withY (met.getBottom() + 4.0f).withHeight (1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.08f));
                g.fillRect (dv);
            }

            // Section heading: Recommendations
            {
                auto hd = bg.removeFromTop (20.0f);
                g.setColour (juce::Colours::white.withAlpha (0.70f));
                g.setFont (12.0f);
                g.drawFittedText ("Recommendations", hd.toNearestInt(), juce::Justification::centredLeft, 1);
            }
            // params: simple rows with current->target and mini knob arc
            auto paramsArea = bg.removeFromTop (juce::jmin (84.0f, bg.getHeight() * 0.40f));
            if (! deltas.empty())
            {
                g.saveState();
                g.reduceClipRegion (paramsArea.toNearestInt());
                auto rr = paramsArea.reduced (10.0f, 2.0f);
                const int per = 2; // show up to 2 rows
                for (int i = 0; i < juce::jmin (per, (int) deltas.size()); ++i)
                {
                    auto rowR = rr.removeFromTop (24.0f);
                    const auto& d = deltas[(size_t) i];
                    g.setColour (juce::Colours::white.withAlpha (0.75f));
                    g.setFont (12.0f);
                    // Friendly param name
                    juce::String pname = d.id;
                    if (pname == "mono_hz") pname = "Mono (Hz)";
                    else if (pname == "width_lo") pname = "Width Low";
                    else if (pname == "width_mid") pname = "Width Mid";
                    else if (pname == "width_hi") pname = "Width High";
                    else if (pname == "rotation_deg") pname = "Rotation";
                    else if (pname == "tilt") pname = "Tilt (dB)";
                    else if (pname == "hp_hz") pname = "HP (Hz)";
                    else if (pname == "lp_hz") pname = "LP (Hz)";
                    else if (pname == "depth") pname = "Reverb Depth";
                    else if (pname == "space_algo") pname = "Reverb Type";
                    else if (pname == "ducking") pname = "Ducking";
                    else if (pname == "duck_attack_ms") pname = "Duck Attack";
                    else if (pname == "duck_release_ms") pname = "Duck Release";
                    else if (pname == "duck_threshold_db") pname = "Duck Threshold";
                    else if (pname == "duck_ratio") pname = "Duck Ratio";
                    g.drawText (pname, rowR.removeFromLeft (120).toNearestInt(), juce::Justification::centredLeft);
                    float cur = d.current; float tgt = d.target;
                    float norm = (d.hi - d.lo) > 1e-9f ? (cur - d.lo) / (d.hi - d.lo) : 0.0f;
                    float normT = (d.hi - d.lo) > 1e-9f ? (tgt - d.lo) / (d.hi - d.lo) : 0.0f;
                    auto knob = rowR.removeFromLeft (80).reduced (8.0f, 2.0f).toFloat();
                    juce::Path arc;
                    auto r = knob.reduced (6.0f);
                    float a0 = juce::MathConstants<float>::pi * 1.2f;
                    float a1 = juce::MathConstants<float>::pi * (1.2f + 1.6f * norm);
                    float aT = juce::MathConstants<float>::pi * (1.2f + 1.6f * normT);
                    arc.addCentredArc (r.getCentreX(), r.getCentreY(), r.getWidth()*0.5f, r.getHeight()*0.5f, 0.0f, a0, a1, true);
                    // Elevated arc + target tick
                    g.setColour (juce::Colours::white.withAlpha (0.25f));
                    g.strokePath (arc, juce::PathStrokeType (3.0f));
                    g.setColour (juce::Colours::white.withAlpha (0.85f));
                    g.strokePath (arc, juce::PathStrokeType (1.6f));
                    g.setColour (juce::Colour (0xFF5AA9E6).withAlpha (0.98f));
                    juce::Path tick; tick.addCentredArc (r.getCentreX(), r.getCentreY(), r.getWidth()*0.5f, r.getHeight()*0.5f, 0.0f, aT-0.02f, aT+0.02f, true);
                    g.strokePath (tick, juce::PathStrokeType (3.2f));
                    juce::String txt = juce::String (cur, 2) + " → " + juce::String (tgt, 2);
                    g.setColour (juce::Colours::white.withAlpha (0.65f));
                    g.drawFittedText (txt, rowR.toNearestInt(), juce::Justification::centredLeft, 1);
                }
                g.restoreState();
            }

            // hint (max two lines)
            // divider directly above hint area
            {
                auto dv = bg.withHeight (1.0f);
                g.setColour (juce::Colours::white.withAlpha (0.08f));
                g.fillRect (dv);
            }
            const int hintLines = 2;
            const float lineH = 14.0f;
            auto hintArea = bg.removeFromBottom (lineH * hintLines + 6.0f);
            g.setColour (juce::Colours::white.withAlpha (0.50f));
            g.setFont (12.0f);
            g.drawFittedText (hint, hintArea.toNearestInt(), juce::Justification::centredLeft, hintLines);
        }
        void resized() override
        {
            auto r = getLocalBounds();
            auto header = r.removeFromTop (46);
            auto right = header.removeFromRight (64);
            tggByp.setBounds (right.removeFromLeft (56));
        }
    };

    MachineCard toneCard, spaceCard, clarityCard;

    // --- Overlay animation caches/state ---
    juce::Image glyphMask[5];
    juce::Path  glyphPath[5];
    bool overlayReady { false };

    int   overlayBars { 24 };
    float overlayGap  { 2.0f };
    float overlayHeightFrac { 0.32f };

    std::vector<float> barSeed[5];
    std::vector<float> barValue[5];
    std::vector<float> barTarget[5];

    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> overlayAlpha { 0.0f };
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> scanPos      { 0.0f };
    float overlayLevel01 { 0.0f };

    // Temp audio buffer for overlay level metering
    juce::AudioBuffer<float> overlayBuf;

    // Snapshot-based blur of cards during Learn
    juce::Image blurredCards;
    juce::Rectangle<int> blurredArea;

    // Dim cards during learning (simulated blur with heavy dim in overlay paint)

    MyPluginAudioProcessor& proc;
    juce::ValueTree& vt;
    MachineEngine engine;
    juce::CriticalSection uiLock;
    std::vector<Proposal> pendingProposals;

    // Learn/preview state
    bool learning { false };
    double captureSec { 60.0 };
    double lastTickMs { -1.0 };
    double learnRemaining { 0.0 };
    bool finalizeQueued { false };

    void startLearn();
    void stopLearn (bool finalize);
    void beginPreview();
    void endPreview();

    void timerCallback() override;
    void rebuildProposalCards();
    void applyPatches (float strength01);
    void paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area);
};


