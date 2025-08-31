#include "KnobCell.h"
#include "FieldLookAndFeel.h"

KnobCell::KnobCell(juce::Slider& knobToHost, juce::Label& valueLabelToHost, const juce::String&)
    : knob(knobToHost), valueLabel(valueLabelToHost)
{
    // Let the children handle interactions
    setWantsKeyboardFocus (false);
    setInterceptsMouseClicks (false, true);
}

void KnobCell::setMetrics (int knobPx, int valuePx, int gapPx, int miniPx)
{
    K = juce::jmax (16, knobPx);
    V = juce::jmax (0,  valuePx);
    G = juce::jmax (0,  gapPx);
    M = juce::jmax (0,  miniPx);
    resized();
    repaint();
}

void KnobCell::setMini (juce::Slider* miniSlider, int miniHeightPx)
{
    mini = miniSlider;
    auxComponents.clear();
    M = juce::jmax (0, miniHeightPx);
    resized();
    repaint();
}

void KnobCell::setAuxComponents (const std::vector<juce::Component*>& components, int miniHeightPx)
{
    auxComponents = components;
    mini = nullptr; // prefer auxComponents when provided
    M = juce::jmax (0, miniHeightPx);
    resized();
    repaint();
}

void KnobCell::setAuxWeights (const std::vector<float>& weights)
{
    auxWeights = weights;
    resized();
    repaint();
}

// Title removed

juce::Colour KnobCell::getPanelColour() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.panel;
    return juce::Colour (0xFF3A3D45);
}

juce::Colour KnobCell::getTextColour() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.text;
    return juce::Colour (0xFFF0F2F5);
}

juce::Colour KnobCell::getAccentColour() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.accentSecondary; // use neutral dark accent for borders
    return juce::Colour (0xFF202226);
}

juce::Colour KnobCell::getShadowDark() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.shadowDark;
    return juce::Colours::black;
}

juce::Colour KnobCell::getShadowLight() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.shadowLight;
    return juce::Colours::grey;
}

juce::Colour KnobCell::getRimColour() const
{
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        return lf->theme.sh;
    return juce::Colours::black.withAlpha (0.2f);
}

void KnobCell::ensureChildrenAreHere()
{
    // Reparent the knob into this cell if needed
    if (knob.getParentComponent() != this)
        addAndMakeVisible (knob);

    // Reparent mini if present
    if (mini != nullptr && mini->getParentComponent() != this)
        addAndMakeVisible (*mini);
    if (miniLabel != nullptr && miniLabel->getParentComponent() != this)
    {
        addAndMakeVisible (*miniLabel);
        miniLabel->setInterceptsMouseClicks (false, false);
    }

    // Reparent aux components if present
    for (auto* c : auxComponents)
        if (c != nullptr && c->getParentComponent() != this)
            addAndMakeVisible (c);

    // Let the value label live inside the same parent as the knob if the caller moves it here.
    // We do not position it; the editor’s placeLabelBelow(...) handles that.
    if (valueLabel.getParentComponent() == nullptr)
        addAndMakeVisible (valueLabel);
}

void KnobCell::resized()
{
    ensureChildrenAreHere();

    auto b = getLocalBounds().reduced (4);
    const int rimR = 6;

    // Mini/aux placement: bottom or right
    auto content = b;
    if (M > 0)
    {
        if (miniOnRight)
        {
            // Wider, easier-to-hit right strip
            int stripWtmp = juce::jmin (juce::jmax (M, content.getWidth() / 2), juce::jmax (32, content.getWidth() - 40));
            if (! showKnob)
                stripWtmp = content.getWidth(); // when no knob, let the aux/right strip take full width
            const int stripW = stripWtmp;
            auto miniStrip = content.removeFromRight (stripW).reduced (2, 2);

            if (mini != nullptr)
            {
                // Horizontal mini bar centered vertically in the strip
                const int h = juce::jlimit (8, 24, miniThicknessPx);
                juce::Rectangle<int> bar (miniStrip.getX(), miniStrip.getCentreY() - h / 2,
                                          miniStrip.getWidth(), h);
                mini->setBounds (bar);
                if (miniLabel != nullptr)
                {
                    const int lh = (int) std::ceil (miniLabel->getFont().getHeight());
                    const int smallGap = 2; // very small vertical spacing
                    miniLabel->setBounds (bar.withY (bar.getBottom() + smallGap).withHeight (juce::jmax (V, lh)));
                    miniLabel->toFront (false);
                }
            }
            else if (! auxComponents.empty())
            {
                if (auxAsBars)
                {
                    // Centered thin bars (EQ-mini style)
                    const int count  = (int) auxComponents.size();
                    const int gapY   = juce::jmax (6, G); // add more vertical padding between minis
                    const int hBar   = juce::jlimit (8, 24, miniThicknessPx);
                    const int totalH = count * hBar + gapY * juce::jmax (0, count - 1);
                    juce::Rectangle<int> col = miniStrip;
                    col.setY (miniStrip.getCentreY() - totalH / 2);
                    col.setHeight (totalH);
                    for (int i = 0; i < count; ++i)
                    {
                        auto* c = auxComponents[(size_t) i];
                        if (c != nullptr)
                            c->setBounds (col.removeFromTop (hBar));
                        if (i < count - 1) col.removeFromTop (gapY);
                    }
                }
                else
                {
                    // Natural weighted vertical stack (with special handling for square buttons and micro bars)
                    const int count  = (int) auxComponents.size();
                    const int gapY   = juce::jmax (2, G);
                    const int totalG = gapY * juce::jmax (0, count - 1);
                    const int H      = juce::jmax (1, miniStrip.getHeight() - totalG);
                    juce::Array<float> normH;
                    float sum = 0.0f;
                    if ((int) auxWeights.size() == count)
                    {
                        for (float v : auxWeights) { float t = juce::jmax (0.0f, v); normH.add (t); sum += t; }
                    }
                    if (sum <= 0.0001f) { normH.clear(); for (int i = 0; i < count; ++i) { normH.add (1.0f); } sum = (float) count; }

                    juce::Rectangle<int> col = miniStrip;
                    int used = 0;
                    for (int i = 0; i < count; ++i) used += (int) std::round (H * (normH[i] / sum));
                    used += totalG;
                    col.setY (miniStrip.getCentreY() - used / 2);
                    col.setHeight (used);

                    for (int i = 0; i < count; ++i)
                    {
                        const int cellH = (int) std::round (H * (normH[i] / sum));
                        auto* c = auxComponents[(size_t) i];
                        juce::Rectangle<int> rCell = col.removeFromTop (cellH);
                        if (c != nullptr)
                        {
                            // If first component wants a specific aspect (height/width), center rectangle accordingly
                            if (i == 0)
                            {
                                const bool wantsSquare = (bool) c->getProperties().getWithDefault ("square", false);
                                const float aspect = (float) c->getProperties().getWithDefault ("aspect", wantsSquare ? 1.0f : 0.0f); // height/width
                                if (aspect > 0.0f || wantsSquare || dynamic_cast<juce::Button*>(c) != nullptr)
                                {
                                    float useAspect = aspect;
                                    if (useAspect <= 0.0f) useAspect = 1.0f; // square fallback
                                    // Compute size that fits inside rCell with given aspect (height = aspect * width)
                                    int maxW = rCell.getWidth();
                                    int maxH = rCell.getHeight();
                                    int wA = juce::jmin (maxW, (int) std::round ((double) maxH / (double) useAspect));
                                    int hA = (int) std::round ((double) wA * (double) useAspect);
                                    juce::Rectangle<int> ar (wA, hA);
                                    ar = ar.withCentre (rCell.getCentre());
                                    // Clip to bounds just in case
                                    if (ar.getX() < rCell.getX()) ar.setX (rCell.getX());
                                    if (ar.getY() < rCell.getY()) ar.setY (rCell.getY());
                                    if (ar.getRight() > rCell.getRight()) ar.setRight (rCell.getRight());
                                    if (ar.getBottom() > rCell.getBottom()) ar.setBottom (rCell.getBottom());
                                    c->setBounds (ar);
                                    goto placed;
                                }
                            }
                            else
                            {
                                // Micro sliders: render as thin bars centered vertically
                                const bool isMicro = (bool) c->getProperties().getWithDefault ("micro", false);
                                if (isMicro)
                                {
                                    const int hBar = juce::jlimit (8, 24, miniThicknessPx);
                                    juce::Rectangle<int> bar (rCell.getX(), rCell.getCentreY() - hBar / 2, rCell.getWidth(), hBar);
                                    c->setBounds (bar);
                                }
                                else
                                {
                                    c->setBounds (rCell);
                                }
                            }
                        placed: ;
                        }
                        if (i < count - 1) col.removeFromTop (gapY);
                    }
                }
            }

            content.removeFromRight (G); // spacing between strip and knob column
        }
        else
        {
            auto miniArea = content.removeFromBottom (M).reduced (4, 2);
            if (mini != nullptr)
            {
                const int barH = juce::jlimit (8, 24, miniThicknessPx);
                juce::Rectangle<int> bar = miniArea.removeFromTop (barH);
                mini->setBounds (bar);
                if (miniLabel != nullptr)
                {
                    const int lh = (int) std::ceil (miniLabel->getFont().getHeight());
                    const int smallGap = 2; // very small vertical spacing
                    auto lb = miniArea;
                    lb.removeFromTop (smallGap);
                    miniLabel->setBounds (lb.withHeight (juce::jmax (V, lh)));
                    miniLabel->toFront (false);
                }
            }
            else if (! auxComponents.empty())
            {
                // Horizontal layout for aux components; respect optional weights
                const int count = (int) auxComponents.size();
                const int gap = juce::jmax (2, G);
                const int totalGap = gap * juce::jmax (0, count - 1);
                const int w = miniArea.getWidth() - totalGap;
                juce::Array<float> normW;
                float sum = 0.0f;
                if ((int) auxWeights.size() == count)
                {
                    for (float v : auxWeights) { float t = juce::jmax (0.0f, v); normW.add (t); sum += t; }
                }
                if (sum <= 0.0001f) { normW.clear(); for (int i = 0; i < count; ++i) { normW.add (1.0f); } sum = (float) count; }
                auto r = miniArea;
                for (int i = 0; i < count; ++i)
                {
                    const int cellW = (int) std::round (w * (normW[i] / sum));
                    auto* c = auxComponents[(size_t) i];
                    if (c != nullptr)
                        c->setBounds (r.removeFromLeft (cellW));
                    if (i < count - 1)
                        r.removeFromLeft (gap);
                }
            }
            content.removeFromBottom (G);
        }
    }

    // Reserve space for the value label (caller will position it relative to the knob)
    if (V > 0)
        content.removeFromBottom (V + G);

    // Fit the knob at the top-center with requested diameter K
    if (showKnob)
    {
        const int k = juce::jmin (K, juce::jmin (content.getWidth(), content.getHeight()));
        juce::Rectangle<int> knobBox (k, k);
        knobBox = knobBox.withCentre ({ content.getCentreX(), content.getY() + k / 2 });
        knob.setBounds (knobBox);
    }

    // Managed value label placement (optional)
    if (valueLabelMode == ValueLabelMode::Managed)
    {
        const int h = juce::jmax (V, (int) std::ceil (valueLabel.getFont().getHeight()));
        int lx = content.getX();
        int lw = content.getWidth();
        int ly = content.getY() + valueLabelGap + (showKnob ? 0 : 0);
        if (showKnob)
        {
            const int k = juce::jmin (K, juce::jmin (content.getWidth(), content.getHeight()));
            juce::Rectangle<int> kb (k, k);
            kb = kb.withCentre ({ content.getCentreX(), content.getY() + k / 2 });
            lx = kb.getX(); lw = kb.getWidth(); ly = kb.getBottom() + valueLabelGap;
        }
        juce::Rectangle<int> lb (lx, ly, lw, h);
        if (valueLabel.getParentComponent() != this)
            addAndMakeVisible (valueLabel);
        valueLabel.setBounds (lb);
        valueLabel.toFront (false);
        valueLabel.setInterceptsMouseClicks (false, false);
    }

    // Leave any remaining space as breathing room; label will be placed by caller beneath `knob`.
    ignoreUnused (rimR);
}

void KnobCell::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    // Panel fill (optional)
    if (showPanel)
    {
        g.setColour (getPanelColour());
        g.fillRoundedRectangle (r.reduced (3.0f), rad);
    }

    // Depth (soft)
    if (showPanel)
    {
        juce::DropShadow ds1 (getShadowDark().withAlpha (0.35f), 12, { -1, -1 });
        juce::DropShadow ds2 (getShadowLight().withAlpha (0.25f),  6, { -1, -1 });
        ds1.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
        ds2.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
    }

    // Inner rim
    if (showPanel)
    {
        g.setColour (getRimColour().withAlpha (0.18f));
        g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);
    }

    if (showBorder)
    {
        // Hover halo
        const bool over = isMouseOverOrDragging();
        auto border = r.reduced (2.0f);
        g.setColour (getAccentColour());
        if (over || hoverActive)
        {
            for (int i = 1; i <= 6; ++i)
            {
                const float t = (float) i / 6.0f;
                const float expand = 2.0f + t * 8.0f;
                g.setColour (getAccentColour().withAlpha ((1.0f - t) * 0.22f));
                g.drawRoundedRectangle (border.expanded (expand), rad + expand * 0.35f, 2.0f);
            }
        }
        g.setColour (getAccentColour());
        g.drawRoundedRectangle (border, rad, 1.5f);
    }

    // No title: knob name already shown by LNF; retain only numeric/value labels via valueLabel

    // Recessed background badge behind value label text (slightly larger + darker + stronger inner shadow)
    if (valueLabel.isShowing())
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto lb = valueLabel.getBounds().toFloat();
        auto f  = valueLabel.getFont();
        const juce::String txt = valueLabel.getText();

        const float th = std::ceil (f.getHeight());
        const float tw = f.getStringWidthFloat (txt);

        // Slightly larger than text bounds
        const float padX = 4.0f;
        const float padY = 2.0f;

        const float x = lb.getCentreX() - tw * 0.5f - padX;
        const float y = lb.getY() + (lb.getHeight() - th) * 0.5f - padY * 0.5f;
        juce::Rectangle<float> badge (x, y, tw + padX * 2.0f, th + padY);

        const float cr = 4.0f;

        juce::Colour base = lf ? lf->theme.panel : juce::Colour (0xFF2A2C30);
        juce::Colour top  = base.darker (0.70f);  // darker overall
        juce::Colour bot  = base.darker (0.38f);

        juce::ColourGradient grad (top, badge.getX(), badge.getY(),
                                   bot, badge.getX(), badge.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (badge, cr);

        // Stronger inner shadow using multiple inset strokes
        for (int i = 0; i < 3; ++i)
        {
            const float inset = 0.8f + i * 0.8f;
            const float alpha = 0.28f - i * 0.06f;
            g.setColour (juce::Colours::black.withAlpha (alpha));
            g.drawRoundedRectangle (badge.reduced (inset), juce::jmax (0.0f, cr - inset * 0.6f), 1.2f);
        }

        // Top inner highlight and bottom inner shadow lines for accent
        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawLine (badge.getX() + 1.0f, badge.getY() + 1.0f,
                    badge.getRight() - 1.0f, badge.getY() + 1.0f, 1.0f);

        g.setColour (juce::Colours::black.withAlpha (0.22f));
        g.drawLine (badge.getX() + 1.0f, badge.getBottom() - 1.0f,
                    badge.getRight() - 1.0f, badge.getBottom() - 1.0f, 1.0f);
    }

    // Draw mini label badge (if present) with the same style but smaller padding
    if (miniLabel != nullptr && miniLabel->isShowing())
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto lb = miniLabel->getBounds().toFloat();
        auto f  = miniLabel->getFont();
        const juce::String txt = miniLabel->getText();

        const float th = std::ceil (f.getHeight());
        const float tw = f.getStringWidthFloat (txt);

        const float padX = 3.0f; // slightly smaller than main
        const float padY = 1.0f;

        const float x = lb.getCentreX() - tw * 0.5f - padX;
        const float y = lb.getY() + (lb.getHeight() - th) * 0.5f - padY * 0.5f;
        juce::Rectangle<float> badge (x, y, tw + padX * 2.0f, th + padY);

        const float cr = 4.0f;

        juce::Colour base = lf ? lf->theme.panel : juce::Colour (0xFF2A2C30);
        juce::Colour top  = base.darker (0.70f);
        juce::Colour bot  = base.darker (0.38f);

        juce::ColourGradient grad (top, badge.getX(), badge.getY(),
                                   bot, badge.getX(), badge.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (badge, cr);

        for (int i = 0; i < 3; ++i)
        {
            const float inset = 0.8f + i * 0.8f;
            const float alpha = 0.28f - i * 0.06f;
            g.setColour (juce::Colours::black.withAlpha (alpha));
            g.drawRoundedRectangle (badge.reduced (inset), juce::jmax (0.0f, cr - inset * 0.6f), 1.2f);
        }

        g.setColour (juce::Colours::white.withAlpha (0.18f));
        g.drawLine (badge.getX() + 1.0f, badge.getY() + 1.0f,
                    badge.getRight() - 1.0f, badge.getY() + 1.0f, 1.0f);

        g.setColour (juce::Colours::black.withAlpha (0.22f));
        g.drawLine (badge.getX() + 1.0f, badge.getBottom() - 1.0f,
                    badge.getRight() - 1.0f, badge.getBottom() - 1.0f, 1.0f);
    }

    // Draw value badges for right-strip micro aux sliders to match EQ mini labels
    if (! auxComponents.empty())
    {
        for (auto* c : auxComponents)
        {
            if (auto* s = dynamic_cast<juce::Slider*>(c))
            {
                const bool isMicro = (bool) s->getProperties().getWithDefault ("micro", false);
                if (!isMicro || !s->isShowing())
                    continue;

                auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
                auto bar = s->getBounds().toFloat();
                const float lh = (float) juce::jmax (V, 12);
                const float smallGap = 2.0f;
                juce::Rectangle<float> lb (bar.getX(), bar.getBottom() + smallGap, bar.getWidth(), lh);

                juce::String txt = juce::String (s->getValue(), 2);
                auto f = valueLabel.getFont();
                const float th = std::ceil (f.getHeight());
                const float tw = f.getStringWidthFloat (txt);
                const float padX = 3.0f;
                const float padY = 1.0f;
                const float x = lb.getCentreX() - tw * 0.5f - padX;
                const float y = lb.getY() + (lb.getHeight() - th) * 0.5f - padY * 0.5f;
                juce::Rectangle<float> badge (x, y, tw + padX * 2.0f, th + padY);
                const float cr = 4.0f;
                juce::Colour base = lf ? lf->theme.panel : juce::Colour (0xFF2A2C30);
                juce::Colour top  = base.darker (0.70f);
                juce::Colour bot  = base.darker (0.38f);
                juce::ColourGradient grad (top, badge.getX(), badge.getY(), bot, badge.getX(), badge.getBottom(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (badge, cr);
                for (int i = 0; i < 3; ++i)
                {
                    const float inset = 0.8f + i * 0.8f;
                    const float alpha = 0.28f - i * 0.06f;
                    g.setColour (juce::Colours::black.withAlpha (alpha));
                    g.drawRoundedRectangle (badge.reduced (inset), juce::jmax (0.0f, cr - inset * 0.6f), 1.2f);
                }
                g.setColour (juce::Colours::white.withAlpha (0.18f));
                g.drawLine (badge.getX() + 1.0f, badge.getY() + 1.0f,
                            badge.getRight() - 1.0f, badge.getY() + 1.0f, 1.0f);
                g.setColour (juce::Colours::black.withAlpha (0.22f));
                g.drawLine (badge.getX() + 1.0f, badge.getBottom() - 1.0f,
                            badge.getRight() - 1.0f, badge.getBottom() - 1.0f, 1.0f);
                // Draw text
                g.setColour (getTextColour());
                g.setFont (f);
                g.drawFittedText (txt, badge.toNearestInt(), juce::Justification::centred, 1);
            }
        }
    }
}


