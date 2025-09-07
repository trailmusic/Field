#pragma once
#include <JuceHeader.h>
#include "MachineEngine.h"
#include "ProposalCard.h"
#include "WidthDesignerPanel.h"

class MachinePane : public juce::Component, private juce::Timer
{
public:
    MachinePane (juce::AudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf);
    ~MachinePane() override = default;

    void setSampleRate (double sr) { engine.setSampleRate (sr); }
    void pushBlock (const float* L, const float* R, int n) { engine.push (L, R, n); }

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // Minimal cell wrapper to match switch/combobox cell styling
    class SmallSwitchCell : public juce::Component
    {
    public:
        SmallSwitchCell(juce::Component& childToHost) : child(childToHost)
        {
            addAndMakeVisible (child);
        }
        void resized() override
        {
            auto b = getLocalBounds().reduced (6);
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
    };

    // UI widgets
    juce::TextButton analyzeBtn { "Learn" }, stopBtn { "" };
    juce::ComboBox   qualityBox, targetBox;
    juce::Slider     strength; // 0..1
    juce::ToggleButton freezeBtn { "Freeze" }, showPreBtn { "Use Pre" };
    juce::TextButton previewBtn { "Preview 10s" }, applyBtn { "Apply" };
    juce::TextButton ABtn { "A" }, BBtn { "B" }, CBtn { "C" }, undoBtn { "Undo" };
    juce::Component  proposalsContent; // holds ProposalCard children (no scrolling)
    std::unique_ptr<SmallSwitchCell> learnCell, stopCell;
    juce::Label freezeLabel, usePreLabel;
    juce::Rectangle<int> barArea;
    std::unique_ptr<WidthDesignerPanel> widthPanel;

    MachineEngine engine;
    juce::AudioProcessor& proc;
    juce::ValueTree& vt;
    juce::CriticalSection uiLock;
    std::vector<ParamPatch> pendingProposals;

    void timerCallback() override;
    void rebuildProposalCards();
    void applyPatches (float strength01);
    void paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area);
};


