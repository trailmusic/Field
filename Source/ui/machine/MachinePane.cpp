#include "MachinePane.h"
#include "../../FieldLookAndFeel.h"

MachinePane::MachinePane (juce::AudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf)
    : proc (p), vt (state)
{
    juce::ignoreUnused (lnf);
    setOpaque (false);

    // wrapped in cells later
    addAndMakeVisible (qualityBox);
    addAndMakeVisible (targetBox);
    addAndMakeVisible (strength);
    addAndMakeVisible (freezeBtn);
    addAndMakeVisible (showPreBtn);
    addAndMakeVisible (freezeLabel);
    addAndMakeVisible (usePreLabel);
    addAndMakeVisible (previewBtn);
    addAndMakeVisible (applyBtn);
    addAndMakeVisible (ABtn);
    addAndMakeVisible (BBtn);
    addAndMakeVisible (CBtn);
    addAndMakeVisible (undoBtn);
    addAndMakeVisible (proposalsContent);

    qualityBox.addItem ("Fast", 1); qualityBox.addItem ("Standard", 2); qualityBox.addItem ("Deep", 3);
    targetBox.addItem ("Streaming", 1); targetBox.addItem ("Club", 2); targetBox.addItem ("Podcast", 3); targetBox.addItem ("Reference", 4);
    strength.setRange (0.0, 1.0, 0.001);

    startTimerHz (15);

    // style: cell-like buttons (larger for bottom bar)
    auto styleCell = [&](juce::Button& b)
    {
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF3A3E45));
        b.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xFF5AA9E6));
        b.setColour (juce::TextButton::textColourOffId, juce::Colours::white.withAlpha (0.92f));
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
        b.setConnectedEdges (juce::Button::ConnectedOnLeft | juce::Button::ConnectedOnRight);
    };
    styleCell (analyzeBtn);
    styleCell (stopBtn);
    analyzeBtn.setTriggeredOnMouseDown (false);
    stopBtn.setTriggeredOnMouseDown (false);
    stopBtn.setButtonText (juce::String::fromUTF8 ("\u25A0")); // square icon
    analyzeBtn.setButtonText ("Learn");

    // Wrap Learn/Stop into small cells to match delay-style cells
    learnCell = std::make_unique<SmallSwitchCell> (analyzeBtn);
    stopCell  = std::make_unique<SmallSwitchCell> (stopBtn);
    addAndMakeVisible (*learnCell);
    addAndMakeVisible (*stopCell);

    // Label style like icon labels near combo boxes
    auto setCap = [&](juce::Label& l, const juce::String& t)
    {
        l.setText (t, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setInterceptsMouseClicks (false, false);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            l.setColour (juce::Label::textColourId, lf->theme.textMuted);
    };
    setCap (freezeLabel, "Freeze");
    setCap (usePreLabel, "Use Pre");
}

void MachinePane::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    auto bar = b.removeFromBottom (56);
    // compute tight width around items
    const int learnW = 160, stopW = 100, qW = 120, tW = 140, strW = 180, frW=90, upW=100;
    int totalW = learnW + 6 + stopW + 10 + qW + 6 + tW + 10 + strW + 6 + frW + 6 + upW;
    barArea = juce::Rectangle<int> (bar.getCentreX() - totalW/2, bar.getY()+4, totalW, bar.getHeight()-8);
    paintTopBarBackground (g, barArea);
}

void MachinePane::resized()
{
    auto r = getLocalBounds();
    // Recompute tight bar area (keep layout independent of paint)
    auto bar = r.removeFromBottom (56);
    const int learnW = 160, stopW = 100, qW = 120, tW = 140, strW = 180, frW=90, upW=100;
    int totalW = learnW + 6 + stopW + 10 + qW + 6 + tW + 10 + strW + 6 + frW + 6 + upW;
    auto ba = juce::Rectangle<int> (bar.getCentreX() - totalW/2, bar.getY()+4, totalW, bar.getHeight()-8);
    auto place = [&](juce::Component& c, int w){ c.setBounds (ba.removeFromLeft (w)); ba.removeFromLeft (6); };
    if (learnCell) place (*learnCell, learnW);
    if (stopCell)  place (*stopCell,  stopW);
    ba.removeFromLeft (10);
    place (qualityBox, qW);
    place (targetBox,  tW);
    ba.removeFromLeft (10);
    place (strength,   strW);
    // place toggle + labels stacked tightly
    auto freezeArea = ba.removeFromLeft (frW);
    freezeBtn.setBounds (freezeArea.removeFromTop ((int) (bar.getHeight() * 0.6f)));
    freezeLabel.setBounds (freezeArea);
    auto usePreArea = ba.removeFromLeft (upW);
    showPreBtn.setBounds (usePreArea.removeFromTop ((int) (bar.getHeight() * 0.6f)));
    usePreLabel.setBounds (usePreArea);
    // content occupies the rest without scroll
    proposalsContent.setBounds (r.reduced (8));
}

void MachinePane::timerCallback()
{
    if (engine.hasResults())
    {
        auto res = engine.takeResults();
        {
            const juce::ScopedLock sl (uiLock);
            pendingProposals = std::move (res);
        }
        rebuildProposalCards();
    }
}

void MachinePane::rebuildProposalCards()
{
    proposalsContent.removeAllChildren();
    int y = 0; const int h = 74;
    const juce::ScopedLock sl (uiLock);
    for (const auto& p : pendingProposals)
    {
        auto* card = new ProposalCard();
        card->setPatch (p);
        proposalsContent.addAndMakeVisible (card);
        card->setBounds (8, y, proposalsContent.getWidth() - 16, h);
        y += h + 6;
    }
}

void MachinePane::applyPatches (float)
{
    // MVP stub
}

void MachinePane::paintTopBarBackground (juce::Graphics& g, juce::Rectangle<int> area)
{
    auto r = area.toFloat();
    // Space-age texture: subtle starfield + gradient
    juce::ColourGradient grad (
        juce::Colour::fromHSV (0.60f, 0.25f, 0.10f, 0.90f), r.getX(), r.getY(),
        juce::Colour::fromHSV (0.62f, 0.20f, 0.16f, 0.90f), r.getX(), r.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (r, 10.0f);
    g.saveState();
    juce::Random rnd (0xC0FFEE);
    for (int i = 0; i < 40; ++i)
    {
        float sx = r.getX() + rnd.nextFloat() * r.getWidth();
        float sy = r.getY() + rnd.nextFloat() * r.getHeight();
        float sz = 0.8f + rnd.nextFloat() * 1.6f;
        g.setColour (juce::Colours::white.withAlpha (0.06f + rnd.nextFloat()*0.06f));
        g.fillEllipse (sx, sy, sz, sz);
    }
    g.restoreState();
    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (r.reduced (1.0f), 9.0f, 1.2f);
}


