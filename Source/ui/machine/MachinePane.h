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
    juce::ComboBox   targetBox;
    // Quality/time controls removed per spec
    juce::Slider     strength; // 0..1
    juce::ToggleButton showPreBtn { "" }; // Pre toggle
    juce::TextButton previewBtn { "Preview 10s" }, applyBtn { "Apply" };
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

    // --- Three Machine cards (Tone, Space, Clarity) ---
    class MachineCard : public juce::Component
    {
    public:
        juce::String title;
        float confidence { 0.0f };
        bool  bypassed { false };
        juce::TextButton btnApply { "Apply" };
        CardBypassButton tggByp;
        juce::Array<float> displayA, displayB;
        juce::String hint;

        std::function<void()> onApply;
        std::function<void(bool)> onBypass;

        MachineCard()
        {
            addAndMakeVisible (btnApply);
            addAndMakeVisible (tggByp);
            btnApply.setClickingTogglesState (true);
            btnApply.getProperties().set ("apply_active", false);
            btnApply.onClick = [this]
            {
                const bool armed = btnApply.getToggleState();
                btnApply.getProperties().set ("apply_active", armed);
                if (armed && onApply) onApply();
            };
            tggByp.onClick   = [this]{ bypassed = tggByp.getToggleState(); if (onBypass) onBypass (bypassed); };
            btnApply.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
            btnApply.setColour (juce::TextButton::textColourOnId,  juce::Colours::white);
        }
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

            // display
            auto disp = bg.removeFromTop (juce::jlimit (100.0f, 160.0f, bg.getHeight() * 0.45f));
            g.setColour (juce::Colours::white.withAlpha (0.10f)); g.fillRoundedRectangle (disp, 6.0f);
            if (displayA.size() > 1)
            {
                juce::Path p; int W = (int) disp.getWidth(); float H = disp.getHeight();
                for (int x = 0; x < W; ++x)
                {
                    float t = (float) x / juce::jmax (1, W-1);
                    int i = juce::jlimit (0, displayA.size()-1, (int) std::floor (t * (displayA.size()-1)));
                    float y = disp.getBottom()-1.0f - juce::jlimit (0.0f, 1.0f, displayA[i]) * (H-2.0f);
                    if (x==0) p.startNewSubPath (disp.getX()+x, y); else p.lineTo (disp.getX()+x, y);
                }
                g.setColour (juce::Colours::white.withAlpha (0.80f)); g.strokePath (p, juce::PathStrokeType (1.3f));
            }
            // targets as ticks
            if (! displayB.isEmpty())
            {
                g.setColour (juce::Colour (0xFF5AA9E6).withAlpha (0.85f));
                for (auto v : displayB)
                {
                    float y = disp.getBottom()-1.0f - juce::jlimit (0.0f,1.0f,v) * (disp.getHeight()-2.0f);
                    g.drawHorizontalLine ((int) juce::roundToInt (y), disp.getX()+2.0f, disp.getRight()-2.0f);
                }
            }

            // hint
            auto foot = bg.removeFromBottom (22.0f);
            g.setColour (juce::Colours::white.withAlpha (0.50f));
            g.setFont (12.0f);
            g.drawText (hint, foot.toNearestInt(), juce::Justification::centredLeft);
        }
        void resized() override
        {
            auto r = getLocalBounds();
            auto header = r.removeFromTop (46);
            auto right = header.removeFromRight (120);
            tggByp.setBounds (right.removeFromLeft (56));
            btnApply.setBounds (right.removeFromLeft (56));
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

    MachineEngine engine;
    MyPluginAudioProcessor& proc;
    juce::ValueTree& vt;
    juce::CriticalSection uiLock;
    std::vector<ParamPatch> pendingProposals;

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


