#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "FieldTheme.h"
#include "FieldMetallic.h"
#include "FieldRendering.h"

// FieldLookAndFeel - Core Look and Feel class
// Reorganized for better maintainability and separation of concerns

class FieldLNF : public juce::LookAndFeel_V4, public juce::ChangeBroadcaster
{
public:
    // EQ-specific color IDs for consistent theming
    enum ColourIds
    {
        eqLabelTextColourId     = 0x6101001,
        eqBorderColourId        = 0x6101002,
        eqZeroLineColourId      = 0x6101003,
        eqGridLineColourId      = 0x6101004,
        eqBandHandleColourId    = 0x6101005,
        eqBandHandleActiveId    = 0x6101006,
        eqAnalyzerTraceColourId = 0x6101007,
    };

    // Expose theme alias for compatibility
    using Theme = FieldTheme;

    explicit FieldLNF(FieldTheme theme = {}) : theme(theme)
    {
        setDefaultSansSerifTypefaceName("Inter");
        // Initialize with Ocean theme by default
        setTheme(ThemeVariant::Ocean);
    }

    // Theme management
    void setTheme(ThemeVariant variant)
    {
        ThemeManager::applyTheme(theme, variant);
        currentVariant = variant;
        setupColours();
    }

    // Apply theme and broadcast changes
    void applyTheme(ThemeVariant variant)
    {
        juce::Logger::writeToLog("FieldLNF: applyTheme called with variant " + juce::String((int)variant));
        ThemeManager::applyTheme(theme, variant);
        currentVariant = variant;
        setupColours();
        sendChangeMessage(); // Notify listeners that colours changed
        juce::Logger::writeToLog("FieldLNF: sendChangeMessage() called");
    }

    // Legacy compatibility - redirects to new theme system
    void setGreenMode(bool enabled)
    {
        setTheme(enabled ? ThemeVariant::Green : ThemeVariant::Ocean);
    }

    // Apply theme colours to JUCE components
    void setupColours()
    {
        setColour(juce::ResizableWindow::backgroundColourId, theme.base);
        setColour(juce::Label::textColourId, theme.text);
        setColour(juce::Slider::textBoxTextColourId, theme.text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::PopupMenu::backgroundColourId, theme.panel);
        setColour(juce::PopupMenu::textColourId, theme.text);
        
        // Set EQ-specific colors
        setColour(eqLabelTextColourId, theme.accent);
        setColour(eqBorderColourId, theme.accent.withAlpha(0.90f));
        setColour(eqZeroLineColourId, theme.accent.withAlpha(0.35f));
        setColour(eqGridLineColourId, theme.accent.withAlpha(0.12f));
        setColour(eqBandHandleColourId, theme.text.withAlpha(0.70f));
        setColour(eqBandHandleActiveId, theme.accent);
        setColour(eqAnalyzerTraceColourId, theme.text.withAlpha(0.30f));
    }

    // Popup menu per-item tinting support
    void setPopupItemTints(const juce::Array<juce::Colour>& tints)
    {
        popupItemTints = tints;
    }

    // Centralized knob styling function to eliminate redundancy
    // Moved to FieldRendering::styleKnob()

    // --- JUCE LookAndFeel overrides (delegate to FieldRendering) ---
    
    // Button overrides
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool isMouseOverButton, bool isButtonDown) override
    {
        FieldRendering::drawToggleButton(g, button, isMouseOverButton, isButtonDown, theme);
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                             const juce::Colour& backgroundColour,
                             bool isMouseOverButton, bool isButtonDown) override
    {
        FieldRendering::drawButtonBackground(g, button, backgroundColour, isMouseOverButton, isButtonDown, theme);
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                       bool isMouseOverButton, bool isButtonDown) override
    {
        FieldRendering::drawButtonText(g, button, isMouseOverButton, isButtonDown, theme);
    }

    // Slider overrides
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
    {
        FieldRendering::drawRotarySlider(g, x, y, width, height, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, slider, theme);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        FieldRendering::drawLinearSlider(g, x, y, width, height, sliderPosProportional, minSliderPos, maxSliderPos, style, slider, theme);
    }

    int getSliderThumbRadius(juce::Slider& slider) override
    {
        return FieldRendering::getSliderThumbRadius(slider);
    }

    // ComboBox overrides
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box) override
    {
        FieldRendering::drawComboBox(g, width, height, isButtonDown, buttonX, buttonY, buttonW, buttonH, box, theme);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label) override
    {
        FieldRendering::positionComboBoxText(box, label);
    }

    // PopupMenu overrides
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height) override
    {
        FieldRendering::drawPopupMenuBackground(g, width, height, theme);
    }

    void drawPopupMenuSeparator(juce::Graphics& g, const juce::Rectangle<int>& area)
    {
        FieldRendering::drawPopupMenuSeparator(g, area, theme);
    }

    void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area,
                                   const juce::String& sectionName) override
    {
        FieldRendering::drawPopupMenuSectionHeader(g, area, sectionName, theme);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                             bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                             bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                          const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        FieldRendering::drawPopupMenuItem(g, area, isSeparator, isActive, isHighlighted, isTicked,
                                        hasSubMenu, text, shortcutKeyText, icon, textColour, theme);
    }

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                  int& idealWidth, int& idealHeight) override
    {
        FieldRendering::getIdealPopupMenuItemSize(text, isSeparator, standardMenuItemHeight, idealWidth, idealHeight);
    }

    // Label override
    void drawLabel(juce::Graphics& g, juce::Label& label) override
    {
        FieldRendering::drawLabel(g, label, theme);
    }

    // --- Public API methods ---
    
    // Panel rendering
    void drawNeoPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius = 16.0f) const;
    void paintCellPanel(juce::Graphics& g, juce::Component& c, bool showBorder, bool hover) const;

    // Theme change broadcasting
    static void broadcastThemeChanged(juce::Component& root)
    {
        // Call lookAndFeelChanged() on the whole subtree so everyone refreshes
        std::function<void(juce::Component*)> walk = [&](juce::Component* c)
        {
            if (!c) return;
            c->lookAndFeelChanged();
            for (int i = 0; i < c->getNumChildComponents(); ++i)
                walk(c->getChildComponent(i));
        };
        walk(&root);
    }

    // Special rendering
    void drawRotationPad(juce::Graphics& g, juce::Rectangle<float> bounds, float rotationDeg, float asymmetry,
                        juce::Colour accent, juce::Colour text, juce::Colour panel) const;
    void drawGainSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPosProportional,
                   float rotaryStartAngle, float rotaryEndAngle, float gainDb);
    void drawKnobLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, juce::Component* component = nullptr);
    void drawTabPill(juce::Graphics& g, juce::Rectangle<float> r, bool active, bool hover = false) const;

    // Metallic rendering (delegate to MetallicRenderer)
    // MetallicRenderer::paintMetal() and MetallicRenderer::paintPhaseMetal()

    // Active theme (mutable for runtime palette switching)
    FieldTheme theme;
    ThemeVariant currentVariant{ThemeVariant::Ocean};

private:
    // Popup menu per-item tinting support
    juce::Array<juce::Colour> popupItemTints;
    mutable int popupPaintIndex{0};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FieldLNF)
};
