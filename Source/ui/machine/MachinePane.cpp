#include "MachinePane.h"
#include "../../FieldLookAndFeel.h"
#include "../../IconSystem.h"
#include "../../PluginProcessor.h"

MachinePane::MachinePane (MyPluginAudioProcessor& p, juce::ValueTree& state, juce::LookAndFeel* lnf)
    : proc (p), vt (state)
{
    juce::ignoreUnused (lnf);
    setOpaque (false);

    // wrapped in cells later
    addAndMakeVisible (targetBox);
    // quality/capture removed
    addAndMakeVisible (strength);
    addAndMakeVisible (showPreBtn);
    addAndMakeVisible (previewBtn);
    addAndMakeVisible (applyBtn);
    addAndMakeVisible (ABtn);
    addAndMakeVisible (BBtn);
    addAndMakeVisible (CBtn);
    addAndMakeVisible (undoBtn);
    addAndMakeVisible (proposalsContent);
    // Three cards (initially simple placeholders; wired later)
    toneCard.title = "Tone & Balance"; spaceCard.title = "Space & Width"; clarityCard.title = "Clarity & Movement";
    addAndMakeVisible (toneCard); addAndMakeVisible (spaceCard); addAndMakeVisible (clarityCard);
    addAndMakeVisible (listenBtn);
    listenBtn.setClickingTogglesState (true);
    listenBtn.setButtonText ("");
    listenBtn.getProperties().set ("iconType", (int) IconSystem::Delta);

    targetBox.addItem ("Streaming", 1); targetBox.addItem ("Club", 2); targetBox.addItem ("Podcast", 3); targetBox.addItem ("Reference", 4);
    // no quality/capture combos
    strength.setRange (0.0, 1.0, 0.01);
    strength.setNumDecimalPlacesToDisplay (2);
    strength.setTextValueSuffix (" %");
    strength.setTextBoxStyle (juce::Slider::TextBoxRight, false, 48, 18);

    startTimerHz (30);

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

    showPreBtn.setToggleState (false, juce::dontSendNotification); // default to Post
    showPreBtn.setClickingTogglesState (true);
    showPreBtn.setButtonText ("");
    // Pre icon-only (match Freeze style)
    showPreBtn.getProperties().set ("iconType", (int) IconSystem::Stereo);
    preCell = std::make_unique<SmallSwitchCell> (showPreBtn);
    addAndMakeVisible (*preCell);

    // Handlers
    analyzeBtn.onClick = [this]
    {
        startLearn();
    };
    stopBtn.onClick = [this]
    {
        stopLearn (false);
    };
    showPreBtn.onClick = [this]
    {
        engine.setUsePre (showPreBtn.getToggleState());
    };
    listenBtn.onClick = [this]
    {
        if (listenBtn.getToggleState()) beginPreview(); else endPreview();
    };
    targetBox.onChange = [this]
    {
        // Map to engine target if supported (placeholder)
        // engine.setTarget(...);
    };
    // captureSec fixed at 60s
}

void MachinePane::paint (juce::Graphics& g)
{
    auto b = getLocalBounds();
    auto bar = b.removeFromTop (56);
    // compute tight width around items
    // compute widths based on preferred sizes rather than hard-coded totals
    const int learnW = juce::jmax (160, analyzeBtn.getBestWidthForHeight (36) + 32); // extra width for container
    const int stopW  = 56; // square-ish
    const int tW     = 140;
    const int strW   = 220; // room for % suffix
    const int preW   = juce::jmax (90,  preCell ? preCell->getWidth() : 90);
    const int lisW   = 90;
    int totalW = learnW + 6 + stopW + 10 + tW + 10 + strW + 6 + preW + 10 + lisW;
    barArea = juce::Rectangle<int> (bar.getCentreX() - totalW/2, bar.getY()+4, totalW, bar.getHeight()-8);
    paintTopBarBackground (g, barArea);
}

void MachinePane::paintOverChildren (juce::Graphics& g)
{
    if (! learning) return;
    auto bounds = getLocalBounds().toFloat();

    // Snapshot cards content (children only), blur, and draw back
    juce::Rectangle<int> cardsR = bounds.toNearestInt().withTrimmedTop (56).reduced (8, 8);
    if (cardsR.getWidth() > 0 && cardsR.getHeight() > 0)
    {
        juce::Image cardsSnap (juce::Image::ARGB, cardsR.getWidth(), cardsR.getHeight(), true);
        {
            juce::Graphics g2 (cardsSnap);
            g2.fillAll (juce::Colours::transparentBlack);
            auto drawChild = [&](juce::Component& c)
            {
                if (! c.isVisible()) return;
                auto local = c.getLocalBounds();
                if (local.isEmpty()) return;
                auto s = c.createComponentSnapshot (local, true);
                if (s.isValid())
                {
                    const int dx = c.getX() - cardsR.getX();
                    const int dy = c.getY() - cardsR.getY();
                    g2.drawImageAt (s, dx, dy);
                }
            };
            drawChild (toneCard);
            drawChild (spaceCard);
            drawChild (clarityCard);
            drawChild (proposalsContent);
        }
        if (blurredCards.isNull() || blurredCards.getWidth() != cardsSnap.getWidth() || blurredCards.getHeight() != cardsSnap.getHeight())
            blurredCards = juce::Image (juce::Image::ARGB, juce::jmax (1, cardsSnap.getWidth()), juce::jmax (1, cardsSnap.getHeight()), true);
        blurredCards.clear (blurredCards.getBounds(), juce::Colours::transparentBlack);
        juce::ImageEffects::applyGaussianBlurEffect (9.0f, cardsSnap, blurredCards);
        g.setOpacity (0.95f);
        g.drawImageAt (blurredCards, cardsR.getX(), cardsR.getY());
        g.setOpacity (1.0f);
    }
    // Dim blurred cards slightly
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.fillRect (cardsR);

    // Single large animated bar region (letterless)
    const float pad = 16.0f;
    const float cellW = bounds.getWidth() - pad * 2.0f;
    const float cellH = bounds.getHeight() * 0.34f;
    const float baseY = bounds.getCentreY() - cellH * 0.5f;
    const double t = juce::Time::getMillisecondCounterHiRes() * 0.001;
    const juce::Colour accent (0xFF5AA9E6);

    juce::Rectangle<float> inner (pad, baseY, cellW, cellH);
    const int bars = 42;
    const float gap = 2.0f;
    const float bw = juce::jmax (1.0f, (inner.getWidth() - (bars-1)*gap) / (float) bars);
    for (int i = 0; i < bars; ++i)
    {
        float x = inner.getX() + i * (bw + gap);
        float ph = (float) i / (float) bars;
        // Base motion (sin)
        float baseAmp = 0.40f + 0.35f * (0.5f * (1.0f + std::sin ((float) (t*2.1 + ph * juce::MathConstants<float>::twoPi + i*0.16f))));
        // Audio-react and progress sweep highlight
        const float sweep = scanPos.getCurrentValue();
        const float xNorm = juce::jlimit (0.0f, 1.0f, (x + bw*0.5f - inner.getX()) / inner.getWidth());
        const float sigma = 0.12f;
        const float hl = std::exp (-0.5f * (xNorm - sweep)*(xNorm - sweep) / (sigma*sigma));
        baseAmp += overlayLevel01 * 0.25f + 0.20f * hl;
        // Taller bars
        float amp = juce::jlimit (0.0f, 1.0f, baseAmp * 3.0f);
        float h = inner.getHeight() * juce::jlimit (0.10f, 1.0f, amp);
        float y = inner.getY() + (inner.getHeight() - h) * 0.5f;
        float mix = juce::jlimit (0.1f, 1.0f, 0.25f + 0.55f * hl);
        g.setColour (juce::Colour (0xFF3A3E45).interpolatedWith (accent, mix).withAlpha (0.95f));
        g.fillRoundedRectangle ({ x, y, bw, h }, 2.0f);
    }

    // Countdown overlay text (right-bottom)
    g.setColour (juce::Colours::white.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (16.0f).withStyle ("Bold")));
    g.drawFittedText (juce::String ((int) std::ceil (learnRemaining)) + "s", bounds.toNearestInt().removeFromBottom(28).removeFromRight(60), juce::Justification::centredRight, 1);
}

void MachinePane::resized()
{
    auto r = getLocalBounds();
    // Recompute tight bar area (keep layout independent of paint)
    auto bar = r.removeFromTop (56);
    const int learnW = 180, stopW = 56, tW = 140, strW = 220, preW=90, lisW=90;
    int totalW = learnW + 6 + stopW + 10 + tW + 10 + strW + 6 + preW + 10 + lisW;
    auto ba = juce::Rectangle<int> (bar.getCentreX() - totalW/2, bar.getY()+4, totalW, bar.getHeight()-8);
    // add padding inside the container
    auto ia = ba.reduced (10, 6);
    auto place = [&](juce::Component& c, int w){ c.setBounds (ia.removeFromLeft (w)); ia.removeFromLeft (6); };
    if (learnCell) place (*learnCell, learnW);
    if (stopCell)  place (*stopCell,  stopW);
    place (targetBox,  tW);
    ia.removeFromLeft (10);
    place (strength,   strW);
    if (preCell)    place (*preCell,    preW);
    ia.removeFromLeft (10);
    place (listenBtn, lisW);
    // content occupies the rest without scroll; place cards
    auto content = r.withTrimmedTop (2).reduced (8, 8);
    const int cols = content.getWidth() > 1200 ? 3 : (content.getWidth() > 800 ? 2 : 1);
    const int gutter = 12;
    if (cols == 3)
    {
        int w = (content.getWidth() - 2*gutter) / 3;
        toneCard.setBounds   (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        spaceCard.setBounds  (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        clarityCard.setBounds(content.removeFromLeft (content.getWidth()).reduced (2));
    }
    else if (cols == 2)
    {
        int w = (content.getWidth() - gutter) / 2;
        toneCard.setBounds   (content.removeFromLeft (w).reduced (2)); content.removeFromLeft (gutter);
        spaceCard.setBounds  (content.removeFromLeft (content.getWidth()).reduced (2));
        // next row
        content = r.withTrimmedTop (2).reduced (8, 8);
        content.removeFromTop (toneCard.getBottom() + gutter - r.getY());
        clarityCard.setBounds(content.removeFromLeft (content.getWidth()).reduced (2));
    }
    else
    {
        int h = content.getHeight() / 3;
        toneCard.setBounds   (content.removeFromTop (h).reduced (2)); content.removeFromTop (gutter);
        spaceCard.setBounds  (content.removeFromTop (h).reduced (2)); content.removeFromTop (gutter);
        clarityCard.setBounds(content.reduced (2));
    }
}

void MachinePane::timerCallback()
{
    // Audio-react level and progress sweep
    if (engine.hasResults())
    {
        auto res = engine.takeResults();
        {
            const juce::ScopedLock sl (uiLock);
            pendingProposals = std::move (res);
        }
        rebuildProposalCards();
    }

    if (learning)
    {
        // pull latest samples for overlay level
        const int pulled = proc.visPost.pull (overlayBuf, 1024);
        const int n = overlayBuf.getNumSamples();
        if (n > 0)
        {
            double s=0.0; const float* L = overlayBuf.getReadPointer(0); const float* R = overlayBuf.getNumChannels()>1?overlayBuf.getReadPointer(1):nullptr;
            for (int i=0;i<n;++i){ const float l=L[i], r= R?R[i]:l; s += 0.5*(l*l + r*r); }
            const float rms = (float) std::sqrt (s / juce::jmax (1, n));
            const float db = juce::Decibels::gainToDecibels (juce::jlimit (1e-6f, 1.f, rms));
            const float lev = juce::jlimit (0.f, 1.f, 1.f - (float)((-db) / 60.f));
            overlayLevel01 = overlayLevel01*0.85f + lev*0.15f;
        }
        const double prog = juce::jlimit (0.0, 1.0, (60.0 - learnRemaining) / 60.0);
        scanPos.setTargetValue ((float) prog);
        repaint();
    }
    // countdown update when learning
    const double nowMs = juce::Time::getMillisecondCounterHiRes();
    if (learning)
    {
        if (lastTickMs < 0.0) lastTickMs = nowMs;
        const double dt = (nowMs - lastTickMs) * 0.001;
        lastTickMs = nowMs;
        learnRemaining = juce::jmax (0.0, learnRemaining - dt);
        if (learnRemaining <= 0.0)
        {
            learning = false;
            // trigger finalize if supported by engine (placeholder: cancel→results path diverges in current engine)
        }
    }
    // UI animation: ensure Learn button (blink/countdown) repaints periodically
    analyzeBtn.getProperties().set ("countdown_secs", learnRemaining);
    analyzeBtn.getProperties().set ("learn_active", learning);
    analyzeBtn.repaint();
}

void MachinePane::startLearn()
{
    learning = true;
    learnRemaining = captureSec;
    lastTickMs = -1.0;
    // Placeholder: engine analyze start
    engine.analyzeAsync (MachineEngine::Target::Streaming, MachineEngine::Quality::Standard);
}

void MachinePane::stopLearn (bool finalize)
{
    engine.cancel();
    learning = false;
    learnRemaining = 0.0;
}

void MachinePane::beginPreview()
{
    // Placeholder: implement APVTS snapshot + strength blend apply
}

void MachinePane::endPreview()
{
    // Placeholder: restore snapshot
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


