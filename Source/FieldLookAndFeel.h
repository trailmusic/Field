#pragma once
#include <juce_gui_extra/juce_gui_extra.h>

struct FieldTheme {
    // Lighter, more modern colors with better contrast
    juce::Colour base      { 0xFF3C3F45 }; // lighter base background
    juce::Colour panel     { 0xFF454951 }; // lighter panel background
    juce::Colour hl        { 0xFF5A5E66 }; // brighter highlight
    juce::Colour sh        { 0xFF2A2C30 }; // deeper shadow for contrast
    juce::Colour text      { 0xFFF0F2F5 }; // brighter text
    juce::Colour textMuted { 0xFFB8BDC7 }; // lighter muted text
    // Enhanced shadows for depth
    juce::Colour shadowDark  { 0xFF1A1C20 }; // deep shadow
    juce::Colour shadowLight { 0xFF60646C }; // light shadow/highlight
    // Nature accent default: Ocean (#5AA9E6). Other options:
    // Moss #7FB069, Sand #C6AD8F, Copper #C5865C, Plum #8B6FA1
    juce::Colour accent    { 0xFF5AA9E6 };
};

class FieldLNF : public juce::LookAndFeel_V4
{
public:
    explicit FieldLNF (FieldTheme t = {}) : theme (t)
    {
        setDefaultSansSerifTypefaceName ("Inter");
        setupColours();
    }

    void setupColours()
    {
        setColour (juce::ResizableWindow::backgroundColourId, theme.base);
        setColour (juce::Label::textColourId, theme.text);
        setColour (juce::Slider::textBoxTextColourId, theme.text);
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour (juce::PopupMenu::backgroundColourId, theme.panel);
        setColour (juce::PopupMenu::textColourId, theme.text);
    }
    
    void setGreenMode(bool enabled)
    {
        if (enabled) {
            // Green monochromatic palette - more opaque and matching blue accent's saturation
            theme.base = juce::Colour (0xFF0D1F0D);
            theme.panel = juce::Colour (0xFF1E2F1E);
            theme.text = juce::Colour (0xFFE8F4E8);
            theme.textMuted = juce::Colour (0xFFB0C5B0);
            theme.accent = juce::Colour (0xFF5AA95A); // Matching blue accent's opacity and saturation
            theme.hl = juce::Colour (0xFF2E4F2E);
            theme.sh = juce::Colour (0xFF0D1E0D);
            theme.shadowDark = juce::Colour (0xFF0D1E0D);
            theme.shadowLight = juce::Colour (0xFF4E6F4E);
        } else {
            // Standard blue palette
            theme.base = juce::Colour (0xFF3C3F45);
            theme.panel = juce::Colour (0xFF454951);
            theme.text = juce::Colour (0xFFF0F2F5);
            theme.textMuted = juce::Colour (0xFFB8BDC7);
            theme.accent = juce::Colour (0xFF5AA9E6);
            theme.hl = juce::Colour (0xFF5A5E66);
            theme.sh = juce::Colour (0xFF2A2C30);
            theme.shadowDark = juce::Colour (0xFF1A1C20);
            theme.shadowLight = juce::Colour (0xFF60646C);
        }
        setupColours();
    }

    void drawNeoPanel (juce::Graphics& g, juce::Rectangle<float> r, float radius = 16.0f) const;

    void drawLabel (juce::Graphics& g, juce::Label& l) override
    {
        g.fillAll (juce::Colours::transparentBlack);
        g.setColour (theme.textMuted);
        g.setFont (getLabelFont (l));
        g.drawFittedText (l.getText(), l.getLocalBounds(), juce::Justification::centred, 1);
    }

    void drawRotarySlider (juce::Graphics&, int, int, int, int,
                           float, float, float, juce::Slider&) override;
    
    void drawLinearSlider (juce::Graphics&, int, int, int, int,
                          float, float, float, juce::Slider::SliderStyle, juce::Slider&) override;
    
    int getSliderThumbRadius (juce::Slider&) override;
    
    void drawGainSlider (juce::Graphics& g, int x, int y, int w, int h,
                        float sliderPosProportional, float rotaryStartAngle,
                        float rotaryEndAngle, float gainDb);
    
    void drawKnobLabel (juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text);

    FieldTheme theme;
}; 