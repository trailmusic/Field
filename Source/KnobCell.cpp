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
    {
        auto base = lf->theme.panel;
        const double brighten = (double) getProperties().getWithDefault ("panelBrighten", 0.0);
        if (brighten > 0.0) return base.brighter ((float) brighten);
        if (brighten < 0.0) return base.darker   ((float) -brighten);
        return base;
    }
    auto base = juce::Colour (0xFF3A3D45);
    const double brighten = (double) getProperties().getWithDefault ("panelBrighten", 0.0);
    if (brighten > 0.0) return base.brighter ((float) brighten);
    if (brighten < 0.0) return base.darker   ((float) -brighten);
    return base;
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
    {
        auto base = lf->theme.accentSecondary; // neutral dark accent for borders
        const double brighten = (double) getProperties().getWithDefault ("borderBrighten", 0.0);
        if (brighten > 0.0) return base.brighter ((float) brighten);
        if (brighten < 0.0) return base.darker   ((float) -brighten);
        return base;
    }
    auto base = juce::Colour (0xFF202226);
    const double brighten = (double) getProperties().getWithDefault ("borderBrighten", 0.0);
    if (brighten > 0.0) return base.brighter ((float) brighten);
    if (brighten < 0.0) return base.darker   ((float) -brighten);
    return base;
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

    // Ensure the value label is parented; Managed mode positions it in resized().
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
                    // Natural weighted vertical stack (robust; always assigns bounds)
                    const int count  = (int) auxComponents.size();
                    const int gapY   = juce::jmax (6, G);
                    const int totalG = gapY * juce::jmax (0, count - 1);
                    const int H      = juce::jmax (1, miniStrip.getHeight() - totalG);

                    juce::Array<float> weights;
                    if ((int) auxWeights.size() == count) {
                        for (float v : auxWeights) weights.add (juce::jmax (0.0f, v));
                    } else {
                        for (int i = 0; i < count; ++i) weights.add (1.0f);
                    }
                    float sum = 0.0f; for (auto w : weights) sum += w; if (sum <= 0.0001f) sum = (float) count;

                    juce::Array<int> heights; heights.resize (count);
                    int acc = 0;
                    for (int i = 0; i < count; ++i) { int h = (int) std::round (H * (weights[i] / sum)); heights.set (i, h); acc += h; }
                    for (int d = 0; d < H - acc; ++d) heights.set (d % count, heights[d % count] + 1);

                    juce::Rectangle<int> col = miniStrip;
                    col.setY (miniStrip.getCentreY() - (H + totalG) / 2);
                    col.setHeight (H + totalG);
                    for (int i = 0; i < count; ++i)
                    {
                        auto* c = auxComponents[(size_t) i];
                        auto rCell = col.removeFromTop (heights[i]).reduced (2, 2);
                        if (c != nullptr)
                            c->setBounds (rCell);
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

    // Panel fill (optional) with metallic mode
    const bool metallic = (bool) getProperties().getWithDefault ("metallic", false);
    if (showPanel)
    {
        auto rr = r.reduced (3.0f);
        if (metallic)
        {
            // Brushed-metal gradient; allow Motion variant to lean cooler to match purple border
            const bool motionGreen = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
            // Reverb keeps neutral steel; Motion tilts cooler/darker
            // Motion variant: deeper bluish-purple to match the purple border
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                juce::Colour top = motionGreen ? lf->theme.motionPanelTop : juce::Colour (0xFF9AA0A7);
                juce::Colour bot = motionGreen ? lf->theme.motionPanelBot : juce::Colour (0xFF7F858D);
                juce::ColourGradient grad (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (rr, rad);
            }
            else
            {
                juce::Colour top = motionGreen ? juce::Colour (0xFF7B81C1) : juce::Colour (0xFF9AA0A7);
                juce::Colour bot = motionGreen ? juce::Colour (0xFF555A99) : juce::Colour (0xFF7F858D);
                juce::ColourGradient grad (top, rr.getX(), rr.getY(), bot, rr.getX(), rr.getBottom(), false);
                g.setGradientFill (grad);
                g.fillRoundedRectangle (rr, rad);
            }

            // Subtle horizontal brushing lines (slightly denser)
            g.setColour (juce::Colours::white.withAlpha (0.045f));
            const int step = 1;
            for (int y = (int) rr.getY() + step; y < rr.getBottom(); y += step)
                g.fillRect (juce::Rectangle<int> ((int) rr.getX() + 4, y, (int) rr.getWidth() - 8, 1));

            // Fine grain noise overlay (very low alpha)
            {
                juce::Random rng ((int) juce::Time::getMillisecondCounter());
                g.setColour (juce::Colours::black.withAlpha (0.040f));
                const int noiseRows = juce::jmax (1, (int) rr.getHeight() / 4);
                for (int i = 0; i < noiseRows; ++i)
                {
                    const int y = (int) rr.getY() + 2 + i * 4 + (rng.nextInt (3) - 1);
                    const int w = juce::jmax (8, (int) rr.getWidth() - 8 - rng.nextInt (12));
                    const int x = (int) rr.getX() + 4 + rng.nextInt (12);
                    g.fillRect (juce::Rectangle<int> (x, y, w, 1));
                }
            }

            // Diagonal micro-scratches
            {
                juce::Random rng ((int) juce::Time::getMillisecondCounter() ^ 0xA5A5);
                const int scratches = juce::jmax (6, (int) rr.getWidth() / 22);
                g.setColour (juce::Colours::white.withAlpha (0.035f));
                for (int i = 0; i < scratches; ++i)
                {
                    float sx = rr.getX() + 6 + rng.nextFloat() * (rr.getWidth() - 12);
                    float sy = rr.getY() + 6 + rng.nextFloat() * (rr.getHeight() - 12);
                    float len = 10.0f + rng.nextFloat() * 18.0f;
                    float dx = len * 0.86f; // cos(~40deg)
                    float dy = len * 0.50f; // sin(~30deg)
                    g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                }
                g.setColour (juce::Colours::black.withAlpha (0.025f));
                for (int i = 0; i < scratches; ++i)
                {
                    float sx = rr.getX() + 6 + rng.nextFloat() * (rr.getWidth() - 12);
                    float sy = rr.getY() + 6 + rng.nextFloat() * (rr.getHeight() - 12);
                    float len = 8.0f + rng.nextFloat() * 14.0f;
                    float dx = len * -0.80f;
                    float dy = len * 0.58f;
                    g.drawLine (sx, sy, sx + dx, sy + dy, 1.0f);
                }
            }

            // Vignette to reduce perceived brightness near edges (slightly stronger for Motion)
            {
                const float edgeAlpha = motionGreen ? 0.22f : 0.16f;
                juce::ColourGradient vg (juce::Colours::transparentBlack, rr.getCentreX(), rr.getCentreY(),
                                         juce::Colours::black.withAlpha (edgeAlpha), rr.getCentreX(), rr.getCentreY() - rr.getHeight() * 0.6f, true);
                g.setGradientFill (vg);
                g.fillRoundedRectangle (rr, rad);
            }
        }
        else
        {
            g.setColour (getPanelColour());
            g.fillRoundedRectangle (rr, rad);
        }
    }

    // Depth (soft)
    if (showPanel)
    {
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        if (metallic)
        {
            // Softer, slightly cooler shadows for metal
            juce::DropShadow ds1 (juce::Colours::black.withAlpha (0.28f), 10, { -1, -1 });
            juce::DropShadow ds2 (juce::Colours::white.withAlpha (0.18f),  5, { -1, -1 });
            ds1.drawForRectangle (g, ri);
            ds2.drawForRectangle (g, ri);
        }
        else
        {
            juce::DropShadow ds1 (getShadowDark().withAlpha (0.35f), 12, { -1, -1 });
            juce::DropShadow ds2 (getShadowLight().withAlpha (0.25f),  6, { -1, -1 });
            ds1.drawForRectangle (g, ri);
            ds2.drawForRectangle (g, ri);
        }
    }

    // Inner rim
    if (showPanel)
    {
        if (auto* lf2 = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            const bool motionGreen2 = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
            auto rimCol = metallic && motionGreen2 ? lf2->theme.motionBorder : (metallic ? juce::Colour (0xFF51565D) : getRimColour());
            g.setColour (rimCol.withAlpha (0.16f));
        }
        else
        {
            g.setColour ((metallic ? juce::Colour (0xFF51565D) : getRimColour()).withAlpha (0.16f));
        }
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
        // Use theme text colour for delay-themed cells' borders if requested via property,
        // or a deep blue/purple border for motion cells,
        // or a vintage orange/red maroon border for reverb cells
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        const bool delayTheme = (bool) getProperties().getWithDefault ("delayThemeBorderTextGrey", false);
        const bool motionGreen = (bool) getProperties().getWithDefault ("motionPurpleBorder", (bool) getProperties().getWithDefault ("motionGreenBorder", false));
        const bool reverbMaroon = (bool) getProperties().getWithDefault ("reverbMaroonBorder", false);
        if (motionGreen)
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour (lf->theme.motionBorder);
            else
                g.setColour (juce::Colour (0xFF4A4A8E));
        }
        else if (reverbMaroon)
            g.setColour (juce::Colour (0xFF8E3A2F)); // Vintage orange-red maroon
        else if (delayTheme && lf != nullptr)
            g.setColour (lf->theme.text.withAlpha (0.85f));
        else
            g.setColour (getAccentColour());
        g.drawRoundedRectangle (border, rad, 1.5f);
    }

// Title: draw caption inside the cell using LookAndFeel helper when present

    // Draw caption/name above knob using LNF helper if available
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        juce::String caption;
        if (getProperties().contains ("caption"))
            caption = getProperties()["caption"].toString();
        if (caption.isNotEmpty())
        {
            auto r = getLocalBounds().toFloat();
            r.removeFromBottom (juce::jmax (2, V + G)); // leave space for value label band
            lf->drawKnobLabel (g, r, caption);
        }
    }

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


