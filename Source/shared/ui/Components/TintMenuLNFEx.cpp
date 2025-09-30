#include "TintMenuLNFEx.h"

void TintMenuLNFEx::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    paintIndex = 0;
    auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h);
    // Use configured colours when available; avoid hardcoded hex
    auto bg = findColour (juce::PopupMenu::backgroundColourId);
    auto text = findColour (juce::PopupMenu::textColourId);
    g.setColour (bg);
    g.fillRect (r);
    g.setColour (text.withAlpha (0.06f));
    g.drawRoundedRectangle (r.reduced (1.0f), 5.0f, 1.0f);
}

void TintMenuLNFEx::drawPopupMenuSeparator (juce::Graphics& g, const juce::Rectangle<int>& area)
{
    auto r = area.toFloat().reduced (10.0f, 0.0f);
    auto text = findColour (juce::PopupMenu::textColourId);
    g.setColour (text.withAlpha (0.10f));
    g.fillRect (juce::Rectangle<float> (r.getX(), r.getCentreY() - 0.5f, r.getWidth(), 1.0f));
}

void TintMenuLNFEx::drawPopupMenuSectionHeader (juce::Graphics& g, const juce::String& title,
                                 const juce::Rectangle<int>& area)
{
    auto r = area.toFloat().reduced (8.0f, 4.0f);
    auto text = findColour (juce::PopupMenu::textColourId);
    g.setColour (text.withAlpha (0.60f));
    g.setFont (juce::Font (juce::FontOptions (12.5f)).withExtraKerningFactor (0.02f).boldened());
    g.drawFittedText (title.toUpperCase(), r.toNearestInt(), juce::Justification::centredLeft, 1);
}

void TintMenuLNFEx::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                        bool isSeparator, bool /*isActive*/, bool isHighlighted, bool isTicked,
                        bool /*hasSubMenu*/, const juce::String& text, const juce::String& shortcutKeyText,
                        const juce::Drawable* /*icon*/, const juce::Colour* textColour)
{
    if (isSeparator) { drawPopupMenuSeparator (g, area); return; }

    auto r = area.toFloat().reduced (4.0f, 2.0f);
    const juce::Colour tint = (paintIndex >= 0 && paintIndex < itemTints.size())
                              ? itemTints.getReference (paintIndex++)
                              : defaultTint;

    if (isHighlighted || isTicked)
    {
        g.setColour (tint.withAlpha (isHighlighted ? 0.90f : 0.65f));
        g.fillRoundedRectangle (r, 4.0f);
    auto text = findColour (juce::PopupMenu::textColourId);
    g.setColour (text.withAlpha (0.10f));
        g.drawRoundedRectangle (r, 4.0f, 1.0f);
    }

    auto ta = r.reduced (hideChecks ? 8.0f : 22.0f, 0.0f);
    g.setColour (textColour ? *textColour : findColour (juce::PopupMenu::textColourId).withAlpha (0.95f));
    g.setFont (juce::Font (juce::FontOptions (14.0f)));
    g.drawFittedText (text, ta.toNearestInt(), juce::Justification::centredLeft, 1);

    if (shortcutKeyText.isNotEmpty())
    {
        g.setColour (findColour (juce::PopupMenu::textColourId).withAlpha (0.55f));
        g.setFont (juce::Font (juce::FontOptions (13.0f)));
        auto rt = ta.removeFromRight (60).toNearestInt();
        g.drawFittedText (shortcutKeyText, rt, juce::Justification::centredRight, 1);
    }
}
