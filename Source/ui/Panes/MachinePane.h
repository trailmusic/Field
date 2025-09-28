#pragma once
#include <JuceHeader.h>

// Minimal skeleton for the Machine pane (analyze → propose → apply)
class MachinePane : public juce::Component
{
public:
    MachinePane (juce::AudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf)
        : proc (p), vt (state)
    {
        juce::ignoreUnused (lnf);
        setOpaque (false);
    }

    void setSampleRate (double srIn) { sr = (srIn > 0 ? srIn : 48000.0); }

    // UI-thread push from PaneManager
    void pushBlock (const float* L, const float* R, int n)
    {
        juce::ignoreUnused (L, R, n);
        // TODO: feed rolling buffer for feature extraction
    }

    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillRoundedRectangle (b.reduced (2.0f), 8.0f);

        auto inner = b.reduced (10.0f);
        g.setColour (juce::Colours::white.withAlpha (0.10f));
        g.drawRoundedRectangle (inner, 6.0f, 1.2f);

        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
        g.drawText ("Machine (Analyze → Propose → Apply)", inner.toNearestInt(), juce::Justification::centredTop);
    }

private:
    juce::AudioProcessor& proc;
    juce::ValueTree& vt;
    double sr { 48000.0 };
};


