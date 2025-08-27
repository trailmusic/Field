// FieldLookAndFeel.h
// -----------------------------------------------------------------------------
// DEV NOTES
// - Header aligned with refactored FieldLookAndFeel.cpp
// - Added `using Theme = FieldTheme;` so cpp can refer to FieldLNF::Theme.
// - Public API unchanged in spirit; drawLabel kept inline as before.
// - Colours/themes preserved; setGreenMode() still flips full palette.
// -----------------------------------------------------------------------------

#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

// Theme palette (default "Ocean" accent). Other accents mentioned in comments.
struct FieldTheme
{
    // Lighter, more modern colours with better contrast
    juce::Colour base       { 0xFF3C3F45 }; // lighter base background
    juce::Colour panel      { 0xFF454951 }; // lighter panel background
    juce::Colour hl         { 0xFF5A5E66 }; // brighter highlight
    juce::Colour sh         { 0xFF2A2C30 }; // deeper shadow for contrast
    juce::Colour text       { 0xFFF0F2F5 }; // brighter text
    juce::Colour textMuted  { 0xFFB8BDC7 }; // lighter muted text

    // Enhanced shadows for depth
    juce::Colour shadowDark  { 0xFF1A1C20 }; // deep shadow
    juce::Colour shadowLight { 0xFF60646C }; // light shadow/highlight

    // Nature accent default: Ocean (#5AA9E6). Other options:
    // Moss #7FB069, Sand #C6AD8F, Copper #C5865C, Plum #8B6FA1
    juce::Colour accent     { 0xFF5AA9E6 };

    // EQ palette (defaults set in FieldLNF::setGreenMode(false))
    struct EqPalette {
        juce::Colour hp;
        juce::Colour lp;
        juce::Colour air;
        juce::Colour tilt;
        juce::Colour bass;
        juce::Colour scoop;
        juce::Colour monoShade;
    } eq;
};

class FieldLNF : public juce::LookAndFeel_V4
{
public:
    // Expose alias so cpp helpers can use FieldLNF::Theme
    using Theme = FieldTheme;

    explicit FieldLNF (FieldTheme t = {}) : theme (t)
    {
        setDefaultSansSerifTypefaceName ("Inter");
        // Initialize full default (colorful) palette including EQ colors
        setGreenMode (false);
    }

    // Apply theme colours to JUCE components
    void setupColours()
    {
        setColour (juce::ResizableWindow::backgroundColourId, theme.base);
        setColour (juce::Label::textColourId,                 theme.text);
        setColour (juce::Slider::textBoxTextColourId,         theme.text);
        setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
        setColour (juce::PopupMenu::backgroundColourId,       theme.panel);
        setColour (juce::PopupMenu::textColourId,             theme.text);
    }

    // Toggle green monochrome palette. Re-applies setupColours().
    void setGreenMode (bool enabled)
    {
        if (enabled)
        {
            // Green monochromatic palette
            theme.base        = juce::Colour (0xFF0D1F0D);
            theme.panel       = juce::Colour (0xFF1E2F1E);
            theme.text        = juce::Colour (0xFFE8F4E8);
            theme.textMuted   = juce::Colour (0xFFB0C5B0);
            theme.accent      = juce::Colour (0xFF5AA95A); // green accent
            theme.hl          = juce::Colour (0xFF2E4F2E);
            theme.sh          = juce::Colour (0xFF0D1E0D);
            theme.shadowDark  = juce::Colour (0xFF0D1E0D);
            theme.shadowLight = juce::Colour (0xFF4E6F4E);

            // EQ palette (monochrome green variants)
            theme.eq.hp        = juce::Colour (0xFF6FBF73);
            theme.eq.lp        = juce::Colour (0xFF66BB6A);
            theme.eq.air       = juce::Colour (0xFFA5D6A7);
            theme.eq.tilt      = juce::Colour (0xFF81C784);
            theme.eq.bass      = juce::Colour (0xFF43A047);
            theme.eq.scoop     = juce::Colour (0xFF98EE99);
            theme.eq.monoShade = juce::Colour (0xFF0D1E0D).withAlpha (0.18f);
        }
        else
        {
            // Standard blue palette
            theme.base        = juce::Colour (0xFF3C3F45);
            theme.panel       = juce::Colour (0xFF454951);
            theme.text        = juce::Colour (0xFFF0F2F5);
            theme.textMuted   = juce::Colour (0xFFB8BDC7);
            theme.accent      = juce::Colour (0xFF5AA9E6);
            theme.hl          = juce::Colour (0xFF5A5E66);
            theme.sh          = juce::Colour (0xFF2A2C30);
            theme.shadowDark  = juce::Colour (0xFF1A1C20);
            theme.shadowLight = juce::Colour (0xFF60646C);

            // EQ palette (colorful defaults)
            theme.eq.hp        = juce::Colour (0xFF42A5F5); // HP: blue
            theme.eq.lp        = juce::Colour (0xFF1E88E5); // LP: deeper blue
            theme.eq.air       = juce::Colour (0xFFFFF59D); // Air: soft yellow
            theme.eq.tilt      = juce::Colour (0xFFFFA726); // Tilt: orange (used dashed)
            theme.eq.bass      = juce::Colour (0xFF66BB6A); // Bass: green
            theme.eq.scoop     = juce::Colour (0xFFAB47BC); // Scoop: plum/purple
            theme.eq.monoShade = juce::Colour (0xFF2A2C30).withAlpha (0.15f);
        }
        setupColours();
    }

    // --- Custom primitives ----------------------------------------------------
    void drawNeoPanel (juce::Graphics& g, juce::Rectangle<float> r, float radius = 16.0f) const;

    // Label styling (inline to keep behaviour identical to your original header)
    void drawLabel (juce::Graphics& g, juce::Label& l) override
    {
        g.fillAll (juce::Colours::transparentBlack);
        g.setColour (theme.textMuted);
        g.setFont (getLabelFont (l));
        g.drawFittedText (l.getText(), l.getLocalBounds(), juce::Justification::centred, 1);
    }

    // --- Slider overrides -----------------------------------------------------
    void drawRotarySlider (juce::Graphics&, int, int, int, int,
                           float, float, float, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int, int, int, int,
                           float, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override;

    int  getSliderThumbRadius (juce::Slider&) override;

    // Gain-specific rotary with delta overlay (called by your custom GainSlider)
    void drawGainSlider (juce::Graphics& g, int x, int y, int w, int h,
                         float sliderPosProportional, float rotaryStartAngle,
                         float rotaryEndAngle, float gainDb);

    // Centered knob label helper (kept public for reuse)
    void drawKnobLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text);

    // Active theme (mutable for runtime palette switching)
    FieldTheme theme;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FieldLNF)
};
