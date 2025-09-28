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
            auto r = getLocalBounds();
            auto captionArea = r.removeFromBottom (16);
            child.setBounds (r);
            caption.setBounds (captionArea);
        }
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            if (lf)
            {
                // Draw cell background with metallic styling
                auto base = lf->theme.base;
                auto border = lf->theme.sh; // Use shadow color for border
                auto text = lf->theme.text;
                
                g.setColour (base);
                g.fillRoundedRectangle (r, 4.0f);
                
                g.setColour (border);
                g.drawRoundedRectangle (r, 4.0f, 1.0f);
                
                g.setColour (text.withAlpha (0.8f));
                g.setFont (10.0f);
                g.drawText (captionText, r, juce::Justification::centredBottom);
            }
        }
    private:
        juce::Component& child;
        juce::Label caption;
        juce::String captionText;
    };

    // Top bar with learn controls
    class TopBar : public juce::Component
    {
    public:
        TopBar (MachineTab& parent) : owner (parent)
        {
            addAndMakeVisible (learnBtn);
            addAndMakeVisible (stopBtn);
            addAndMakeVisible (tggByp);
            
            learnBtn.setButtonText ("Learn");
            stopBtn.setButtonText ("Stop");
            tggByp.setButtonText ("Bypass");
            
            learnBtn.onClick = [this] { owner.startLearn(); };
            stopBtn.onClick = [this] { owner.stopLearn (true); };
            tggByp.onClick = [this] { owner.toggleBypass(); };
        }
        
        void paint (juce::Graphics& g) override
        {
            owner.paintTopBarBackground (g, getLocalBounds());
        }
        
        void resized() override
        {
            auto r = getLocalBounds();
            auto left = r.removeFromLeft (120);
            learnBtn.setBounds (left.removeFromLeft (50));
            stopBtn.setBounds (left.removeFromLeft (50));
            tggByp.setBounds (r.removeFromRight (60));
        }
        
    private:
        MachineTab& owner;
        juce::TextButton learnBtn, stopBtn;
        juce::ToggleButton tggByp;
    };

    // Proposal card for displaying machine learning results
    class MachineCard : public juce::Component
    {
    public:
        MachineCard (const juce::String& title, const juce::String& subtitle)
            : cardTitle (title), cardSubtitle (subtitle)
        {
            setOpaque (false);
        }
        
        void setProposal (const Proposal& p)
        {
            proposal = p;
            repaint();
        }
        
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            
            if (lf)
            {
                // Draw card background
                auto base = lf->theme.base;
                auto border = lf->theme.sh; // Use shadow color for border
                auto text = lf->theme.text;
                auto accent = lf->theme.accent;
                
                g.setColour (base);
                g.fillRoundedRectangle (r, 8.0f);
                
                g.setColour (border);
                g.drawRoundedRectangle (r, 8.0f, 1.0f);
                
                // Draw title
                g.setColour (text);
                g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
                g.drawText (cardTitle, r.removeFromTop (20), juce::Justification::centred);
                
                // Draw subtitle
                g.setColour (text.withAlpha (0.8f));
                g.setFont (12.0f);
                g.drawText (cardSubtitle, r.removeFromTop (16), juce::Justification::centred);
                
                // Draw proposal content if available
                if (proposal.id.isNotEmpty())
                {
                    g.setColour (accent);
                    g.setFont (10.0f);
                    g.drawText (proposal.summary, r, juce::Justification::centred);
                }
            }
        }
        
    private:
        juce::String cardTitle, cardSubtitle;
        Proposal proposal;
    };

    // Status display for learning progress
    class StatusDisplay : public juce::Component
    {
    public:
        StatusDisplay() { setOpaque (false); }
        
        void setStatus (const juce::String& status, const juce::String& info)
        {
            statusText = status;
            infoText = info;
            repaint();
        }
        
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            
            if (lf)
            {
                auto text = lf->theme.text;
                auto muted = lf->theme.textMuted;
                
                g.setColour (text);
                g.setFont (juce::FontOptions (12.0f).withStyle ("Bold"));
                g.drawText (statusText, r.removeFromTop (16), juce::Justification::centred);
                
                g.setColour (muted);
                g.setFont (10.0f);
                g.drawText (infoText, r, juce::Justification::centred);
            }
        }
        
    private:
        juce::String statusText, infoText;
    };

    // Hint text for user guidance
    class HintText : public juce::Component
    {
    public:
        HintText() { setOpaque (false); }
        
        void setHint (const juce::String& hint)
        {
            hintText = hint;
            repaint();
        }
        
        void paint (juce::Graphics& g) override
        {
            auto r = getLocalBounds().toFloat();
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            
            if (lf)
            {
                auto muted = lf->theme.textMuted;
                g.setColour (muted);
                g.setFont (10.0f);
                g.drawFittedText (hintText, r.toNearestInt(), juce::Justification::centredLeft, 3);
            }
        }
        
    private:
        juce::String hintText;
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
    void toggleBypass();

    void timerCallback() override;
    void rebuildProposalCards();
    void applyPatches (float strength01);
    void paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area);
};


