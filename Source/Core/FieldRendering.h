#pragma once
#include <JuceHeader.h>
#include "FieldTheme.h"
#include "FieldMetallic.h"

// Field Rendering System - Component-specific rendering methods
// Separated from FieldLookAndFeel for better organization

namespace FieldRendering
{
    // Panel rendering methods
    void drawNeoPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius, const FieldTheme& theme);
    void paintCellPanel(juce::Graphics& g, juce::Component& c, bool showBorder, bool hover, const FieldTheme& theme);

    // Button rendering methods
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool isMouseOver, bool isButtonDown, const FieldTheme& theme);
    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour, 
                             bool isMouseOver, bool isButtonDown, const FieldTheme& theme);

    // Slider rendering methods
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider, const FieldTheme& theme);
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider, const FieldTheme& theme);
    void drawGainSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPosProportional,
                       float rotaryStartAngle, float rotaryEndAngle, float gainDb, const FieldTheme& theme);
    int getSliderThumbRadius(juce::Slider& slider);

    // ComboBox rendering methods
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box, const FieldTheme& theme);
    void positionComboBoxText(juce::ComboBox& box, juce::Label& label);

    // PopupMenu rendering methods
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height, const FieldTheme& theme);
    void drawPopupMenuSeparator(juce::Graphics& g, const juce::Rectangle<int>& area, const FieldTheme& theme);
    void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area,
                                   const juce::String& sectionName, const FieldTheme& theme);
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                          bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                          const juce::Drawable* icon, const juce::Colour* textColour, const FieldTheme& theme);
    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                  int& idealWidth, int& idealHeight);

    // Special rendering methods
    void drawRotationPad(juce::Graphics& g, juce::Rectangle<float> bounds, float rotationDeg, float asymmetry,
                        juce::Colour accent, juce::Colour text, juce::Colour panel);
    void drawTabPill(juce::Graphics& g, juce::Rectangle<float> r, bool active, const FieldTheme& theme);
    void drawKnobLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, const FieldTheme& theme);

    // Label rendering
    void drawLabel(juce::Graphics& g, juce::Label& label, const FieldTheme& theme);
}
