#include "KnobCellQuad.h"

void QuadKnobCell::setMetrics (int knobPx, int valuePx, int gapPx)
{
    K = juce::jmax (16, knobPx);
    V = juce::jmax (0,  valuePx);
    G = juce::jmax (0,  gapPx);
    resized(); repaint();
}

void QuadKnobCell::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;
    juce::Colour panel = juce::Colour (0xFF3A3D45);
    juce::Colour sh    = juce::Colour (0xFF2A2C30);
    juce::Colour acc2  = juce::Colour (0xFF202226);
    juce::Colour shDark = juce::Colours::black;
    juce::Colour shLight= juce::Colours::grey;
    if (lf)
    {
        panel  = lf->theme.panel;
        sh     = lf->theme.sh;
        acc2   = lf->theme.accentSecondary;
        shDark = lf->theme.shadowDark;
        shLight= lf->theme.shadowLight;
    }

    g.setColour (panel);
    g.fillRoundedRectangle (r.reduced (3.0f), rad);

    juce::DropShadow ds1 (shDark.withAlpha (0.35f), 12, { -1, -1 });
    juce::DropShadow ds2 (shLight.withAlpha (0.25f),  6, { -1, -1 });
    auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
    ds1.drawForRectangle (g, ri);
    ds2.drawForRectangle (g, ri);

    g.setColour (sh.withAlpha (0.18f));
    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

    if (showBorder)
    {
        auto border = r.reduced (2.0f);
        g.setColour (acc2);
        g.drawRoundedRectangle (border, rad, 1.5f);
    }
}

void QuadKnobCell::resized()
{
    ensureChildren();
    auto b = getLocalBounds().reduced (4);
    // Trim 2 px on the right to visually match row grid gap rendering
    b.removeFromRight (2);

    // Split into two rows
    auto top = b.removeFromTop ((b.getHeight() - G) / 2);
    b.removeFromTop (G);
    auto bottom = b;

    // Top row: two knobs (HP, LP)
    auto left  = top.removeFromLeft ((top.getWidth() - G) / 2);
    top.removeFromLeft (G);
    auto right = top;
    layoutKnob (left,  hp, hpVal);
    layoutKnob (right, lp, lpVal);

    // Bottom row: Q (knob) + Cluster (component)
    auto leftB  = bottom.removeFromLeft ((bottom.getWidth() - G) / 2);
    bottom.removeFromLeft (G);
    auto rightB = bottom;
    layoutKnob (leftB, q, qVal);
    cluster.setBounds (rightB);
    cluster.toFront (false);
}

void QuadKnobCell::ensureChildren()
{
    auto adopt = [this](juce::Component& c){ if (c.getParentComponent() != this) addAndMakeVisible (c); };
    adopt (hp); adopt (lp); adopt (q); adopt (hpVal); adopt (lpVal); adopt (qVal); adopt (cluster);
    hpVal.setInterceptsMouseClicks (false, false);
    lpVal.setInterceptsMouseClicks (false, false);
    qVal .setInterceptsMouseClicks (false, false);
}

void QuadKnobCell::layoutKnob (juce::Rectangle<int> area, juce::Slider& knob, juce::Label& label)
{
    const int k = juce::jmin (K, juce::jmin (area.getWidth(), area.getHeight()));
    juce::Rectangle<int> knobBox (k, k);
    knobBox = knobBox.withCentre ({ area.getCentreX(), area.getY() + k / 2 });
    knob.setBounds (knobBox);

    const int lh = (int) std::ceil (label.getFont().getHeight());
    juce::Rectangle<int> lb (knobBox.getX(), knobBox.getBottom() + G, knobBox.getWidth(), juce::jmax (V, lh));
    label.setBounds (lb);
    label.toFront (false);
}


