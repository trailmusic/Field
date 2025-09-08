#pragma once
#include <JuceHeader.h>
#include "MachineEngine.h"
#include "ProposalCard.h"
#include "WidthDesignerPanel.h"
#include "../../FieldLookAndFeel.h"

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

    // --- Three Machine cards (Tone, Space, Clarity) ---
    class MachineCard : public juce::Component
    {
    public:
        juce::String title;
        float confidence { 0.0f };
        bool  bypassed { false };
        juce::TextButton btnApply { "Apply" };
        juce::ToggleButton tggByp { "Bypass" };
        juce::Array<float> displayA, displayB;
        juce::String hint;

        std::function<void()> onApply;
        std::function<void(bool)> onBypass;

        MachineCard()
        {
            addAndMakeVisible (btnApply);
            addAndMakeVisible (tggByp);
            btnApply.onClick = [this]{ if (onApply) onApply(); };
            tggByp.onClick   = [this]{ bypassed = tggByp.getToggleState(); if (onBypass) onBypass (bypassed); };
        }
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            auto bg = b.reduced (2.0f);
            g.setColour (juce::Colour (0xFF3A3E45)); g.fillRoundedRectangle (bg, 10.0f);
            g.setColour (juce::Colours::white.withAlpha (0.08f)); g.drawRoundedRectangle (bg, 10.0f, 1.0f);

            auto header = bg.removeFromTop (44.0f);
            g.setFont (juce::Font (15.0f, juce::Font::bold));
            g.setColour (juce::Colours::white.withAlpha (0.92f));
            g.drawText (title, header.removeFromLeft (header.getWidth() - 120.0f).toNearestInt(), juce::Justification::centredLeft);

            // confidence pill
            auto confR = header.removeFromRight (80.0f);
            auto pill = confR.reduced (0, confR.getHeight()*0.35f);
            auto c = juce::Colour (0xFF5AA9E6).interpolatedWith (juce::Colours::grey, 1.0f - juce::jlimit (0.0f, 1.0f, confidence));
            g.setColour (c.withAlpha (0.18f)); g.fillRoundedRectangle (pill, 8.0f);
            g.setColour (c.withAlpha (0.75f)); g.drawRoundedRectangle (pill, 8.0f, 1.0f);
            g.setFont (juce::Font (12.0f, juce::Font::bold));
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawFittedText (juce::String ((int) juce::roundToInt (confidence * 100.0f)) + "%", pill.toNearestInt(), juce::Justification::centred, 1);

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
            auto right = header.removeFromRight (160);
            tggByp.setBounds (right.removeFromLeft (80));
            btnApply.setBounds (right.removeFromLeft (70));
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


