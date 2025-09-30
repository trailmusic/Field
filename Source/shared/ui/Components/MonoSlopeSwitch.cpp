#include "MonoSlopeSwitch.h"
#include "../../Core/FieldLookAndFeel.h"

void MonoSlopeSwitch::paint (juce::Graphics& g)
{
    auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
    auto accent = lf ? lf->theme.eq.hp : juce::Colour (0xFF5AA9E6);
    auto panel  = lf ? lf->theme.panel  : juce::Colour (0xFF2A2C30);
    auto sh     = lf ? lf->theme.sh     : juce::Colour (0xFF1A1C20);
    auto hl     = lf ? lf->theme.hl     : juce::Colour (0xFF4A4A4A);
    auto text   = lf ? lf->theme.text   : juce::Colours::white;

    auto b = getLocalBounds().toFloat();
    const float spacing = 6.0f;
    const float availableH = juce::jmax (0.0f, b.getHeight() - 2.0f * spacing);
    const float h = availableH / 3.0f;

    auto draw = [&](juce::Rectangle<float> r, int idx, const juce::String& lbl)
    {
        const bool on = (current == idx);
        // Elevation shadow like AUD
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour (lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour (juce::Colour (0x40000000));
        g.fillRoundedRectangle (r.translated (1.5f, 1.5f), 6.0f);

        if (on)
        {
            juce::Colour bg = accent;
            if (idx == 0) bg = accent.brighter (0.25f);    // 6 dB
            else if (idx == 2) bg = accent.darker (0.25f); // 24 dB
            g.setColour (bg);
            g.fillRoundedRectangle (r, 6.0f);
            g.setColour (bg.darker (0.30f));
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
        }
        else
        {
            // Gradient panel like ThemedIconButton::GradientPanel
            juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
            juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (r, 6.0f);
            g.setColour (sh);
            g.drawRoundedRectangle (r, 6.0f, 1.0f);
        }

        g.setColour (text);
        g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
        g.drawText (lbl, r, juce::Justification::centred);
    };

    draw ({ b.getX(), b.getY(),                     b.getWidth(), h },                 0, "6");
    draw ({ b.getX(), b.getY() + h + spacing,       b.getWidth(), h },                 1, "12");
    draw ({ b.getX(), b.getY() + 2*(h + spacing),   b.getWidth(), h },                 2, "24");
}

void MonoSlopeSwitch::mouseDown (const juce::MouseEvent& e)
{
    const float spacing = 6.0f;
    const float availableH = juce::jmax (0.0f, (float)getHeight() - 2.0f * spacing);
    const float h = availableH / 3.0f; const float y = (float) e.y;
    int idx = (y <= h) ? 0 : (y <= h * 2 + spacing ? 1 : 2);
    if (idx != current) { current = idx; repaint(); if (onChange) onChange (current); }
}
