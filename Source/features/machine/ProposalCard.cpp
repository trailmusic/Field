#include "ProposalCard.h"

void ProposalCard::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.fillRoundedRectangle (r, 7.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (r, 7.0f, 1.2f);

    g.setColour (juce::Colours::white.withAlpha (0.90f));
    g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle ("Bold")));
    g.drawText (patch.paramID, r.removeFromTop (18.0f).toNearestInt(), juce::Justification::centredLeft);

    auto row = r.removeFromTop (20.0f);
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    juce::String vals = juce::String (patch.current, 2) + " → " + juce::String (patch.target, 2);
    g.drawText (vals, row.toNearestInt(), juce::Justification::centredLeft);

    auto row2 = r.removeFromTop (18.0f);
    g.setColour (juce::Colours::white.withAlpha (0.60f));
    juce::String meta = juce::String ((int) std::round (patch.confidence * 100.0f)) + "%  •  " + patch.rationale;
    g.drawText (meta, row2.toNearestInt(), juce::Justification::centredLeft);
}

void ProposalCard::resized()
{
    auto r = getLocalBounds().reduced (6);
    auto buttons = r.removeFromRight (200);
    int w = (buttons.getWidth() - 8) / 3;
    applyBtn.setBounds (buttons.removeFromLeft (w)); buttons.removeFromLeft (4);
    bypassBtn.setBounds (buttons.removeFromLeft (w)); buttons.removeFromLeft (4);
    widthBtn.setBounds  (buttons.removeFromLeft (w));
    if (widthBtn.getParentComponent() != this) addAndMakeVisible (widthBtn);
    if (applyBtn.getParentComponent() != this) addAndMakeVisible (applyBtn);
    if (bypassBtn.getParentComponent() != this) addAndMakeVisible (bypassBtn);
}


