#pragma once
#include <JuceHeader.h>
#include "../machine/MachineEngine.h"
#include "../machine/ProposalCard.h"
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"

class MyPluginAudioProcessor; // fwd

class MachineTab : public juce::Component, private juce::Timer
{
public:
    MachineTab (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf);
    ~MachineTab() override
    {
        // Stop timer before destruction to prevent use-after-free
        stopTimer();
    }

    void setSampleRate (double sr) { engine.setSampleRate (sr); }
    void pushBlock (const float* L, const float* R, int n) { engine.push (L, R, n); }

    void paint (juce::Graphics&) override;
    void paintOverChildren (juce::Graphics&) override;
    void resized() override;
    
    // Public getter for graphics container
    juce::Component* getProposalsContent() { return &proposalsContent; }

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
            
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                if (auto* button = dynamic_cast<juce::Button*>(&child))
                {
                    button->setLookAndFeel(lf);
                    juce::Logger::writeToLog("*** SmallSwitchCell: Assigned FieldLNF to child button ***");
                }
            }
            else
            {
                juce::Logger::writeToLog("*** SmallSwitchCell: FieldLNF not found for child button ***");
            }
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
            
            // Check if the child component has metallic properties
            auto metallicKind = metallicFromProps (child.getProperties());
            if (metallicKind != MetallicKind::None)
            {
                // For metallic components, don't show caption - let the button handle its own text
                caption.setVisible(false);
                child.setBounds (b);
            }
            else
            {
                // For non-metallic components, show caption above
                if (capH > 0)
                {
                    caption.setVisible(true);
                    caption.setBounds (b.removeFromTop (capH));
                    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                        caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
                }
                child.setBounds (b);
            }
        }
        void paint (juce::Graphics& g) override
        {
            // For metallic components, don't draw anything - let the child handle its own rendering
            auto metallicKind = metallicFromProps (child.getProperties());
            if (metallicKind != MetallicKind::None)
            {
                // Just ensure the child has the right bounds and let it handle everything
                auto cellBounds = getLocalBounds().reduced(3);
                child.setBounds(cellBounds);
                return; // Let the child component handle all rendering
            }
            
            // Non-metallic components use the standard SimpleSwitchCell rendering
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto r = getLocalBounds().toFloat().reduced (3.0f);
            const float rad = 8.0f;
            auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
            auto border = lf ? lf->theme.sh    : juce::Colour (0xFF2A2A2A);
            
            g.setColour (panel);
            g.fillRoundedRectangle (r, rad);
            g.setColour (border);
            g.drawRoundedRectangle (r, rad, 1.5f);
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
    juce::Component  proposalsContent; // holds ProposalCard children (no scrolling)
    std::unique_ptr<SmallSwitchCell> learnCell, stopCell;
    std::unique_ptr<SmallSwitchCell> preCell;
    std::unique_ptr<SmallSwitchCell> listenCell;
    std::unique_ptr<SmallSwitchCell> genreCell, venueCell, trackTypeCell;
    juce::ToggleButton listenBtn { "Listen" };
    juce::Rectangle<int> barArea;

    // Header-style bypass button for machine cards (mirrors header BypassButton)
    class CardBypassButton : public juce::TextButton, public juce::Timer
    {
    public:
        CardBypassButton()
        {
            setButtonText ("");
            setClickingTogglesState (true);
            setTriggeredOnMouseDown (false);
            getProperties().set ("iconType", (int) IconSystem::Bypass);
            startTimerHz (20); // Match header bypass button animation rate
        }
        
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            
            if (lf)
            {
                // Use theme-based animation colors
                auto darkColor = lf->theme.animation.bypassBlinkDark;
                auto brightColor = lf->theme.animation.bypassBlinkBright;
                auto darkAlpha = lf->theme.animation.blinkAlphaDark;
                auto brightAlpha = lf->theme.animation.blinkAlphaBright;
                
                // Animated blink effect
                const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
                const float phase = (float) std::fmod (t * 4.0, 1.0); // 4Hz blink
                const float alpha = phase < 0.5f ? 
                    juce::jmap (phase * 2.0f, 0.0f, 1.0f, darkAlpha, brightAlpha) :
                    juce::jmap ((phase - 0.5f) * 2.0f, 0.0f, 1.0f, brightAlpha, darkAlpha);
                
                auto color = darkColor.interpolatedWith (brightColor, phase);
                g.setColour (color.withAlpha (alpha));
                g.fillRoundedRectangle (r, 4.0f);
                
                // Draw icon
                IconSystem::drawIcon (g, IconSystem::Bypass, r, lf->theme.text);
            }
        }
        
        void timerCallback() override
        {
            repaint();
        }
    };

    // Machine learning proposal cards
    class MachineCard : public juce::Component
    {
    public:
        MachineCard (FieldLNF& lnf) : bypassBtn(), lnfRef(lnf)
        {
            setOpaque (false);
            addAndMakeVisible (bypassBtn);
            bypassBtn.onClick = [this] { onBypass (bypassBtn.getToggleState()); };
        }
        
        juce::String title, hint;
        bool bypassed { false };
        std::function<void(bool)> onBypass;
        
        void setMetrics (const juce::NamedValueSet& m) { metrics = m; }
        void setParams (const std::vector<ParamDelta>& p) { params = p; }
        
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            
            // Card background
            g.setColour (lnfRef.theme.panel);
            g.fillRoundedRectangle (r, 8.0f);
            
            // Border
            g.setColour (lnfRef.theme.sh);
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
            
            // Title
            g.setColour (lnfRef.theme.text);
            g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
            g.drawText (title, r.removeFromTop (20), juce::Justification::centred);
            
            // Hint
            g.setColour (lnfRef.theme.textMuted);
            g.setFont (12.0f);
            g.drawText (hint, r.removeFromTop (16), juce::Justification::centred);
            
            // Display visualization
            if (! displayA.isEmpty())
            {
                auto displayArea = r.reduced (8.0f);
                g.setColour (lnfRef.theme.accent.withAlpha (0.3f));
                for (int i = 0; i < displayArea.getWidth(); ++i)
                {
                    float t = (float) i / (displayArea.getWidth() - 1);
                    int idx = juce::jlimit (0, displayA.size() - 1, (int) (t * (displayA.size() - 1)));
                    float val = displayA[idx];
                    float h = displayArea.getHeight() * val;
                    float y = displayArea.getBottom() - h;
                    g.fillRect (displayArea.getX() + i, y, 1.0f, h);
                }
            }
        }
        
        void resized() override
        {
            auto r = getLocalBounds();
            bypassBtn.setBounds (r.removeFromTop (20).removeFromRight (20));
        }
        
        juce::Array<float> displayA, displayB;
        
    private:
        CardBypassButton bypassBtn;
        FieldLNF& lnfRef;
        juce::NamedValueSet metrics;
        std::vector<ParamDelta> params;
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