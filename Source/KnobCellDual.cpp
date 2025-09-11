#include "KnobCellDual.h"
#include "FieldLookAndFeel.h"

void DoubleKnobCell::ensureChildren()
{
    if (lKnob.getParentComponent() != this) addAndMakeVisible (lKnob);
    if (rKnob.getParentComponent() != this) addAndMakeVisible (rKnob);
    if (lLabel.getParentComponent() != this) addAndMakeVisible (lLabel);
    if (rLabel.getParentComponent() != this) addAndMakeVisible (rLabel);
    lLabel.setInterceptsMouseClicks (false, false);
    rLabel.setInterceptsMouseClicks (false, false);
}

void DoubleKnobCell::layoutOne (juce::Rectangle<int> area, juce::Slider& knob, juce::Label& label)
{
    const int k = juce::jmin (K, juce::jmin (area.getWidth(), area.getHeight()));
    juce::Rectangle<int> knobBox (k, k);
    knobBox = knobBox.withCentre ({ area.getCentreX(), area.getY() + k / 2 });
    knob.setBounds (knobBox);

    const int lh = (int) std::ceil (label.getFont().getHeight());
    juce::Rectangle<int> lb (knobBox.getX(), knobBox.getBottom() + G, knobBox.getWidth(), juce::jmax (V, lh));
    label.setBounds (lb);
    // label.toFront (false); // Stay behind bottomAltPanel to allow proper coverage
}

void DoubleKnobCell::drawRecessedBadge (juce::Graphics& g, juce::Label& label)
{
    if (! label.isShowing()) return;

    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto lb = label.getBounds().toFloat();
    auto f  = label.getFont();
    const juce::String txt = label.getText();

    const float th = std::ceil (f.getHeight());
    const float tw = f.getStringWidthFloat (txt);

    const float padX = 4.0f, padY = 2.0f;
    const float x = lb.getCentreX() - tw * 0.5f - padX;
    const float y = lb.getY() + (lb.getHeight() - th) * 0.5f - padY * 0.5f;
    juce::Rectangle<float> badge (x, y, tw + padX * 2.0f, th + padY);

    const float cr = 4.0f;
    auto base = lf ? lf->theme.panel : juce::Colour (0xFF2A2C30);
    auto top  = base.darker (0.70f);
    auto bot  = base.darker (0.38f);

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

void DoubleKnobCell::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
    auto sh    = lf ? lf->theme.sh    : juce::Colour (0xFF2A2C30);
    auto acc2  = lf ? lf->theme.accentSecondary : juce::Colour (0xFF202226);

    auto r = getLocalBounds().toFloat();
    const float rad = 8.0f;

    g.setColour (panel);
    g.fillRoundedRectangle (r.reduced (3.0f), rad);

    // Depth (soft) – match KnobCell
    {
        juce::DropShadow ds1 ((lf ? lf->theme.shadowDark  : juce::Colours::black).withAlpha (0.35f), 12, { -1, -1 });
        juce::DropShadow ds2 ((lf ? lf->theme.shadowLight : juce::Colours::grey ).withAlpha (0.25f),  6, { -1, -1 });
        auto ri = r.reduced (3.0f).getSmallestIntegerContainer();
        ds1.drawForRectangle (g, ri);
        ds2.drawForRectangle (g, ri);
    }

    g.setColour (sh.withAlpha (0.18f));
    g.drawRoundedRectangle (r.reduced (4.0f), rad - 1.0f, 0.8f);

    if (showBorder)
    {
        auto border = r.reduced (2.0f);
        g.setColour (acc2);
        g.drawRoundedRectangle (border, rad, 1.5f);
    }

    drawRecessedBadge (g, lLabel);
    drawRecessedBadge (g, rLabel);
}

void DoubleKnobCell::resized()
{
    ensureChildren();
    auto b = getLocalBounds().reduced (4);

    auto left  = b.removeFromLeft ((b.getWidth() - G) / 2);
    b.removeFromLeft (G);
    auto right = b;

    layoutOne (left,  lKnob, lLabel);
    layoutOne (right, rKnob, rLabel);
}


