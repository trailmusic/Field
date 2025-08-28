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
        return lf->theme.accent;
    return juce::Colour (0xFF5AA9E6);
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
            const int stripW = juce::jmin (juce::jmax (M, content.getWidth() / 2), juce::jmax (32, content.getWidth() - 40));
            auto miniStrip = content.removeFromRight (stripW).reduced (2, 2);

            if (mini != nullptr)
            {
                // Horizontal mini bar centered vertically in the strip
                const int h = juce::jlimit (8, 24, miniThicknessPx);
                juce::Rectangle<int> bar (miniStrip.getX(), miniStrip.getCentreY() - h / 2,
                                          miniStrip.getWidth(), h);
                mini->setBounds (bar);
            }
            else if (! auxComponents.empty())
            {
                // If stacking aux controls vertically, keep them centered too
                const int count  = (int) auxComponents.size();
                const int gapY   = juce::jmax (2, G);
                const int totalG = gapY * juce::jmax (0, count - 1);
                const int cellH  = juce::jmax (1, (miniStrip.getHeight() - totalG) / juce::jmax (1, count));

                const int colH = cellH * count + totalG;
                juce::Rectangle<int> col (miniStrip.getX(), miniStrip.getCentreY() - colH / 2,
                                          miniStrip.getWidth(), colH);
                for (int i = 0; i < count; ++i)
                {
                    auto* c = auxComponents[(size_t) i];
                    if (c != nullptr)
                        c->setBounds (col.removeFromTop (cellH));
                    if (i < count - 1) col.removeFromTop (gapY);
                }
            }

            content.removeFromRight (G); // spacing between strip and knob column
        }
        else
        {
            auto miniArea = content.removeFromBottom (M).reduced (4, 2);
            if (mini != nullptr)
            {
                mini->setBounds (miniArea);
            }
            else if (! auxComponents.empty())
            {
                // Simple horizontal layout for aux components with equal widths
                const int count = (int) auxComponents.size();
                const int gap = juce::jmax (2, G);
                const int totalGap = gap * juce::jmax (0, count - 1);
                const int cellW = juce::jmax (1, (miniArea.getWidth() - totalGap) / juce::jmax (1, count));
                auto r = miniArea;
                for (int i = 0; i < count; ++i)
                {
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
    const int k = juce::jmin (K, juce::jmin (content.getWidth(), content.getHeight()));
    juce::Rectangle<int> knobBox (k, k);
    knobBox = knobBox.withCentre ({ content.getCentreX(), content.getY() + k / 2 });
    knob.setBounds (knobBox);

    // Managed value label placement (optional)
    if (valueLabelMode == ValueLabelMode::Managed)
    {
        const int h = juce::jmax (V, (int) std::ceil (valueLabel.getFont().getHeight()) + 2);
        juce::Rectangle<int> lb (knobBox.getX(), knobBox.getBottom() + valueLabelGap, knobBox.getWidth(), h);
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

    // Panel fill
    g.setColour (getPanelColour());
    g.fillRoundedRectangle (r.reduced (3.0f), rad);

    // Depth (soft)
    juce::DropShadow ds1 (getShadowDark().withAlpha (0.35f), 12, { -1, -1 });
    juce::DropShadow ds2 (getShadowLight().withAlpha (0.25f),  6, { -1, -1 });
    ds1.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());
    ds2.drawForRectangle (g, r.reduced (3.0f).getSmallestIntegerContainer());

    // Inner rim
    g.setColour (getRimColour().withAlpha (0.18f));
    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

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
}


