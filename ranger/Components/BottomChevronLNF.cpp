#include "BottomChevronLNF.h"

void BottomChevronLNF::drawButtonBackground (juce::Graphics& g, juce::Button& button, const juce::Colour&,
                           bool isOver, bool isDown)
{
    auto r = button.getLocalBounds().toFloat().reduced (2.0f);
    // Background consistent with our buttons: gradient panel + outline
    auto top = theme.panel.brighter (0.10f);
    auto bot = theme.panel.darker   (0.10f);
    g.setGradientFill (juce::ColourGradient (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false));
    // Fill entire area first to prevent white corners
    g.fillRect(r);
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (theme.sh);
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    // Subtle elevation on hover/down
    if (isOver || isDown)
    {
        juce::DropShadow ds1 (theme.shadowDark.withAlpha (0.20f), 8, { -1, -1 });
        juce::DropShadow ds2 (theme.shadowLight.withAlpha (0.18f), 5, { -1, -1 });
        ds1.drawForRectangle (g, r.getSmallestIntegerContainer());
        ds2.drawForRectangle (g, r.getSmallestIntegerContainer());
    }
}

void BottomChevronLNF::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                     bool isOver, bool /*isDown*/)
{
    // Chevron points up when not engaged, down when engaged
    const bool engaged = button.getToggleState();
    auto col = isOver ? theme.accent : theme.textMuted;
    g.setColour (col);

    auto b = button.getLocalBounds().toFloat().reduced (button.getHeight() * 0.22f, button.getHeight() * 0.30f);
    const float cx = b.getCentreX();
    const float cy = b.getCentreY();
    const float half = b.getWidth() * 0.40f;   // wider span for a flatter chevron
    const float vAmp = b.getHeight() * 0.18f;  // smaller vertical excursion for flat look

    juce::Path p;
    if (engaged)
    {
        // Down chevron (V)
        p.startNewSubPath (cx - half, cy - vAmp);
        p.lineTo          (cx,        cy + vAmp);
        p.lineTo          (cx + half, cy - vAmp);
    }
    else
    {
        // Up chevron (^)
        p.startNewSubPath (cx - half, cy + vAmp);
        p.lineTo          (cx,        cy - vAmp);
        p.lineTo          (cx + half, cy + vAmp);
    }
    g.strokePath (p, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}
