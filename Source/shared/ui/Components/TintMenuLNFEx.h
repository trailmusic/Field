#pragma once

#include <JuceHeader.h>

struct TintMenuLNFEx : public juce::LookAndFeel_V4
{
    juce::Colour defaultTint { juce::Colours::skyblue };
    juce::Array<juce::Colour> itemTints;
    bool hideChecks = true;
    mutable int paintIndex = 0; // reset on background draw

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override;
    void drawPopupMenuSeparator (juce::Graphics& g, const juce::Rectangle<int>& area);
    void drawPopupMenuSectionHeader (juce::Graphics& g, const juce::String& title,
                                     const juce::Rectangle<int>& area);
    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool /*isActive*/, bool isHighlighted, bool isTicked,
                            bool /*hasSubMenu*/, const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* /*icon*/, const juce::Colour* textColour) override;
};
