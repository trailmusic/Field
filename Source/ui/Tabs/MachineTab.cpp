#include "MachineTab.h"
#include "../../Core/PluginProcessor.h"

MachineTab::MachineTab (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf)
    : proc (p), vt (state), engine (p, p.apvts),
      toneCard ("Tone", "Frequency balance and EQ"),
      spaceCard ("Space", "Stereo imaging and width"),
      clarityCard ("Clarity", "Transient response and dynamics")
{
    setOpaque (false);
    
    addAndMakeVisible (toneCard);
    addAndMakeVisible (spaceCard);
    addAndMakeVisible (clarityCard);
    
    // Set up timer for UI updates
    startTimerHz (30);
}

void MachineTab::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat();
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    
    if (lf)
    {
        // Draw background
        auto base = lf->theme.base;
        auto border = lf->theme.sh; // Use shadow color for border
        
        g.setColour (base);
        g.fillRoundedRectangle (b.reduced (2.0f), 8.0f);
        
        g.setColour (border);
        g.drawRoundedRectangle (b.reduced (2.0f), 8.0f, 1.0f);
        
        // Draw title
        g.setColour (lf->theme.text);
        g.setFont (juce::FontOptions (16.0f).withStyle ("Bold"));
        g.drawText ("Machine Learning", b.removeFromTop (30), juce::Justification::centred);
        
        // Draw status if learning
        if (learning)
        {
            g.setColour (lf->theme.accent);
            g.setFont (12.0f);
            g.drawText ("Learning... " + juce::String (learnRemaining, 1) + "s", b.removeFromTop (20), juce::Justification::centred);
        }
    }
}

void MachineTab::paintOverChildren (juce::Graphics& g)
{
    // Overlay effects during learning
    if (learning)
    {
        auto b = getLocalBounds().toFloat();
        g.setColour (juce::Colours::black.withAlpha (0.3f));
        g.fillRoundedRectangle (b, 8.0f);
    }
}

void MachineTab::resized()
{
    auto r = getLocalBounds();
    r.removeFromTop (50); // Space for title
    
    if (r.getHeight() > 200)
    {
        auto cardHeight = r.getHeight() / 3;
        toneCard.setBounds (r.removeFromTop (cardHeight));
        spaceCard.setBounds (r.removeFromTop (cardHeight));
        clarityCard.setBounds (r.removeFromTop (cardHeight));
    }
}

void MachineTab::startLearn()
{
    learning = true;
    learnRemaining = captureSec;
    engine.startLearn (true, captureSec);
    repaint();
}

void MachineTab::stopLearn (bool finalize)
{
    learning = false;
    engine.stopLearn (finalize);
    repaint();
}

void MachineTab::beginPreview()
{
    // Begin preview mode
    repaint();
}

void MachineTab::endPreview()
{
    // End preview mode
    repaint();
}

void MachineTab::toggleBypass()
{
    // Toggle bypass state
    repaint();
}

void MachineTab::timerCallback()
{
    if (learning)
    {
        learnRemaining = engine.getRemainingSeconds();
        if (learnRemaining <= 0.0)
        {
            stopLearn (true);
        }
        repaint();
    }
    
    // Update proposal cards
    rebuildProposalCards();
}

void MachineTab::rebuildProposalCards()
{
    auto proposals = engine.getProposals();
    if (proposals.size() >= 3)
    {
        toneCard.setProposal (proposals[0]);
        spaceCard.setProposal (proposals[1]);
        clarityCard.setProposal (proposals[2]);
    }
}

void MachineTab::applyPatches (float strength01)
{
    // Apply machine learning patches
    auto proposals = engine.getProposals();
    if (!proposals.empty())
    {
        engine.applyComposite (proposals, strength01, false);
    }
}

void MachineTab::paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    if (lf)
    {
        auto base = lf->theme.base;
        auto border = lf->theme.sh; // Use shadow color for border
        
        g.setColour (base);
        g.fillRoundedRectangle (area.toFloat(), 4.0f);
        
        g.setColour (border);
        g.drawRoundedRectangle (area.toFloat(), 4.0f, 1.0f);
    }
}