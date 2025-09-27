#include "FieldRendering.h"
#include "IconSystem.h"

// Field Rendering System implementation
// Separated from FieldLookAndFeel for better organization

namespace FieldRendering
{
    // Panel rendering methods
    void drawNeoPanel(juce::Graphics& g, juce::Rectangle<float> r, float radius, const FieldTheme& theme)
    {
        auto inner = r.reduced(3.0f);

        // Main panel background
        g.setColour(theme.panel);
        g.fillRoundedRectangle(inner, radius);

        // Enhanced shadows for more depth
        juce::DropShadow deepShadow(theme.shadowDark.withAlpha(0.6f), 12, {-2, -2});
        juce::DropShadow lightShadow(theme.shadowLight.withAlpha(0.4f), 6, {-1, -1});

        const auto shadowRect = inner.getSmallestIntegerContainer();
        deepShadow.drawForRectangle(g, shadowRect);
        lightShadow.drawForRectangle(g, shadowRect);

        // Subtle inner rim for inset effect
        g.setColour(theme.sh.withAlpha(0.2f));
        g.drawRoundedRectangle(inner.reduced(1.0f), juce::jmax(0.0f, radius - 1.0f), 1.0f);
    }

    void paintCellPanel(juce::Graphics& g, juce::Component& c, bool showBorder, bool hover, const FieldTheme& theme)
    {
        auto r = c.getLocalBounds().toFloat();
        const float rad = 8.0f;

        g.setColour(theme.panel);
        g.fillRoundedRectangle(r.reduced(3.0f), rad);

        juce::DropShadow ds1(theme.shadowDark.withAlpha(0.35f), 12, {-1, -1});
        juce::DropShadow ds2(theme.shadowLight.withAlpha(0.25f), 6, {-1, -1});
        ds1.drawForRectangle(g, r.reduced(3.0f).getSmallestIntegerContainer());
        ds2.drawForRectangle(g, r.reduced(3.0f).getSmallestIntegerContainer());

        g.setColour(theme.sh.withAlpha(0.18f));
        g.drawRoundedRectangle(r.reduced(4.0f), rad - 1.0f, 0.8f);

        if (showBorder)
        {
            auto border = r.reduced(2.0f);
            g.setColour(theme.accentSecondary);
            if (hover)
            {
                for (int i = 1; i <= 6; ++i)
                {
                    const float t = (float)i / 6.0f;
                    const float expand = 2.0f + t * 8.0f;
                    g.setColour(theme.accentSecondary.withAlpha((1.0f - t) * 0.22f));
                    g.drawRoundedRectangle(border.expanded(expand), rad + expand * 0.35f, 2.0f);
                }
            }
            g.setColour(theme.accentSecondary);
            g.drawRoundedRectangle(border, rad, 1.5f);
        }
    }

    // Button rendering methods
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button, bool isMouseOver, bool isButtonDown, const FieldTheme& theme)
    {
        auto r = button.getLocalBounds().toFloat().reduced(2.0f);
        auto bg = theme.panel;
        auto sh = theme.sh;
        auto hl = theme.hl;
        auto accent = theme.accent;
        auto grey = theme.panel;

        const bool invert = (bool)button.getProperties().getWithDefault("invertActive", false);
        const bool active = invert ? (!button.getToggleState()) : button.getToggleState();

        juce::Colour fill = active ? accent : grey;
        if (isButtonDown) fill = fill.darker(0.25f);
        else if (isMouseOver) fill = fill.brighter(0.10f);

        // Fill square/rounded rect
        const float cr = 4.0f;
        g.setColour(fill);
        g.fillRoundedRectangle(r, cr);

        // Border contrasts better when active (darker tone), otherwise panel shadow
        g.setColour(active ? fill.darker(0.35f) : sh);
        g.drawRoundedRectangle(r, cr, 1.5f);

        // Icon rendering (iconOnly style via property 'iconType')
        int iconInt = (int)button.getProperties().getWithDefault("iconType", -1);
        if (iconInt >= 0)
        {
            auto inner = r.reduced(4.0f);
            juce::Colour iconCol = active ? accent : theme.text.withAlpha(0.75f);
            // Shadow pass for weight
            g.setColour(juce::Colours::black.withAlpha(0.18f));
            IconSystem::drawIcon(g, (IconSystem::IconType)iconInt, inner.translated(0.7f, 1.0f), iconCol);
            // Main icon
            g.setColour(iconCol);
            IconSystem::drawIcon(g, (IconSystem::IconType)iconInt, inner, iconCol);
        }
    }

    void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour& backgroundColour,
                             bool isMouseOver, bool isButtonDown, const FieldTheme& theme)
    {
        auto r = button.getLocalBounds().toFloat().reduced(2.0f);
        auto accent = theme.accent;
        auto panel = theme.panel;

        // Check for metallic properties first
        auto metallicKind = metallicFromProps(button.getProperties());
        
        // Identify special buttons by text; fallback to default look otherwise
        juce::String txt = button.getButtonText().trim();

        bool isLearn = txt.equalsIgnoreCase("Learn");
        bool isStop = txt == juce::String::fromUTF8("\u25A0") || txt.equalsIgnoreCase("Stop");
        bool isApply = txt.equalsIgnoreCase("Apply");

        juce::Colour fill = panel;

        if (isLearn)
        {
            fill = juce::Colour(0xFF4CAF50); // Green for Learn
            if (isButtonDown) fill = fill.darker(0.20f);
        }
        else if (isStop)
        {
            fill = juce::Colour(0xFFF44336); // Red for Stop
            if (isButtonDown) fill = fill.darker(0.20f);
        }
        else if (isApply)
        {
            fill = juce::Colour(0xFF2196F3); // Blue for Apply
            if (isButtonDown) fill = fill.darker(0.20f);
        }
        else if (metallicKind != MetallicKind::None)
        {
            // Metallic buttons - use metallic rendering system
            if (button.getToggleState())
            {
                // Toggled state - use metallic colors
                auto metalColors = MetallicRenderer::getMetallicColors(theme, metallicKind);
                MetallicRenderer::paintMetal(g, r, metalColors, 6.0f);
                return; // Early return for metallic rendering
            }
            else
            {
                // Untoggled state - use panel with metallic border
                fill = isMouseOver ? panel.brighter(0.06f) : panel;
                if (isButtonDown) fill = fill.darker(0.12f);
            }
        }
        else
        {
            // Default
            fill = isMouseOver ? panel.brighter(0.06f) : panel;
            if (isButtonDown) fill = fill.darker(0.12f);
        }

        const float cr = 6.0f;
        g.setColour(fill);
        g.fillRoundedRectangle(r, cr);

        // Border - use metallic colors for metallic buttons
        if (metallicKind != MetallicKind::None && !button.getToggleState())
        {
            auto metalColors = MetallicRenderer::getMetallicColors(theme, metallicKind);
            g.setColour(metalColors.bottom.darker(0.2f));
        }
        else
        {
            g.setColour(fill.darker(0.3f));
        }
        g.drawRoundedRectangle(r, cr, 1.0f);
    }

    // Slider rendering methods (simplified - full implementation would be much longer)
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider, const FieldTheme& theme)
    {
        // Simplified implementation - full version would include all the complex rotary rendering
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        
        // Basic rotary slider rendering
        g.setColour(theme.panel);
        g.fillEllipse(bounds.reduced(4.0f));
        
        g.setColour(theme.accent);
        g.drawEllipse(bounds.reduced(4.0f), 2.0f);
        
        // Draw thumb
        auto thumbAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
        auto thumbRadius = bounds.getWidth() * 0.3f;
        auto thumbX = bounds.getCentreX() + thumbRadius * std::cos(thumbAngle - juce::MathConstants<float>::halfPi);
        auto thumbY = bounds.getCentreY() + thumbRadius * std::sin(thumbAngle - juce::MathConstants<float>::halfPi);
        
        g.setColour(theme.accent);
        g.fillEllipse(thumbX - 4, thumbY - 4, 8, 8);
    }

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float minSliderPos, float maxSliderPos, juce::Slider::SliderStyle style, juce::Slider& slider, const FieldTheme& theme)
    {
        // Simplified linear slider implementation
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        
        g.setColour(theme.panel);
        g.fillRoundedRectangle(bounds, 2.0f);
        
        g.setColour(theme.accent);
        g.drawRoundedRectangle(bounds, 2.0f, 1.0f);
    }

    void drawGainSlider(juce::Graphics& g, int x, int y, int w, int h, float sliderPosProportional,
                       float rotaryStartAngle, float rotaryEndAngle, float gainDb, const FieldTheme& theme)
    {
        // Simplified gain slider implementation
        auto bounds = juce::Rectangle<int>(x, y, w, h).toFloat();
        
        g.setColour(theme.panel);
        g.fillEllipse(bounds.reduced(4.0f));
        
        g.setColour(theme.accent);
        g.drawEllipse(bounds.reduced(4.0f), 2.0f);
    }

    int getSliderThumbRadius(juce::Slider& slider)
    {
        return 8;
    }

    // ComboBox rendering methods
    void drawComboBox(juce::Graphics& g, int width, int height, bool isButtonDown,
                     int buttonX, int buttonY, int buttonW, int buttonH, juce::ComboBox& box, const FieldTheme& theme)
    {
        auto r = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(2.0f);
        auto accent = theme.accent;

        // Check for metallic properties first
        auto metallicKind = metallicFromProps(box.getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Metallic ComboBox - use metallic rendering system
            auto metalColors = MetallicRenderer::getMetallicColors(theme, metallicKind);
            MetallicRenderer::paintMetal(g, r, metalColors, 5.0f);
        }
        else
        {
            // Background: mimic SwitchCell panel (mode cell style)
            g.setColour(theme.panel);
            g.fillRoundedRectangle(r, 5.0f);
        }

        // Border
        g.setColour(theme.sh);
        g.drawRoundedRectangle(r, 5.0f, 1.0f);

        // Arrow
        juce::Path p;
        p.addTriangle(r.getCentreX() - 4, r.getCentreY() - 2,
                     r.getCentreX() + 4, r.getCentreY() - 2,
                     r.getCentreX(), r.getCentreY() + 2);
        g.setColour(accent);
        g.fillPath(p);
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label)
    {
        // Simplified ComboBox text positioning
        label.setBounds(box.getLocalBounds().reduced(4));
    }

    // PopupMenu rendering methods (simplified)
    void drawPopupMenuBackground(juce::Graphics& g, int width, int height, const FieldTheme& theme)
    {
        g.setColour(theme.panel);
        g.fillRoundedRectangle(0, 0, width, height, 4.0f);
    }

    void drawPopupMenuSeparator(juce::Graphics& g, const juce::Rectangle<int>& area, const FieldTheme& theme)
    {
        g.setColour(theme.sh);
        g.drawHorizontalLine(area.getCentreY(), area.getX(), area.getRight());
    }

    void drawPopupMenuSectionHeader(juce::Graphics& g, const juce::Rectangle<int>& area,
                                   const juce::String& sectionName, const FieldTheme& theme)
    {
        g.setColour(theme.textMuted);
        g.setFont(12.0f);
        g.drawText(sectionName, area, juce::Justification::centred);
    }

    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                          bool hasSubMenu, const juce::String& text, const juce::String& shortcutKeyText,
                          const juce::Drawable* icon, const juce::Colour* textColour, const FieldTheme& theme)
    {
        if (isSeparator)
        {
            drawPopupMenuSeparator(g, area, theme);
            return;
        }

        if (isHighlighted)
        {
            g.setColour(theme.accent.withAlpha(0.3f));
            g.fillRoundedRectangle(area.toFloat(), 2.0f);
        }

        g.setColour(theme.text);
        g.setFont(14.0f);
        g.drawText(text, area.reduced(8, 0), juce::Justification::left);
    }

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator, int standardMenuItemHeight,
                                  int& idealWidth, int& idealHeight)
    {
        idealHeight = standardMenuItemHeight;
        idealWidth = text.length() * 8 + 32; // Rough estimate
    }

    // Special rendering methods
    void drawRotationPad(juce::Graphics& g, juce::Rectangle<float> bounds, float rotationDeg, float asymmetry,
                        juce::Colour accent, juce::Colour text, juce::Colour panel)
    {
        // Simplified rotation pad implementation
        bounds = bounds.reduced(4.0f);
        auto centre = bounds.getCentre();
        auto radius = 0.5f * std::min(bounds.getWidth(), bounds.getHeight()) - 4.0f;

        // Energy circle
        g.setColour(panel.brighter(0.25f));
        g.drawEllipse(centre.x - radius, centre.y - radius, 2 * radius, 2 * radius, 1.6f);

        // Basis vectors
        const float th = juce::degreesToRadians(rotationDeg);
        auto u = juce::Point<float>(std::cos(th), -std::sin(th));
        auto v = juce::Point<float>(std::sin(th), std::cos(th));

        // Draw basis vectors
        g.setColour(accent.withAlpha(0.85f));
        g.drawLine(centre.x - radius * u.x, centre.y - radius * u.y,
                  centre.x + radius * u.x, centre.y + radius * u.y, 2.0f);
        g.drawLine(centre.x - radius * v.x, centre.y - radius * v.y,
                  centre.x + radius * v.x, centre.y + radius * v.y, 1.6f);
    }

    void drawTabPill(juce::Graphics& g, juce::Rectangle<float> r, bool active, const FieldTheme& theme)
    {
        if (active)
        {
            g.setColour(theme.accent);
            g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        }
        else
        {
            g.setColour(theme.panel);
            g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        }
    }

    void drawKnobLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, const FieldTheme& theme)
    {
        g.setColour(theme.textMuted);
        g.setFont(12.0f);
        g.drawText(text, bounds, juce::Justification::centred);
    }

    void drawLabel(juce::Graphics& g, juce::Label& label, const FieldTheme& theme)
    {
        g.fillAll(juce::Colours::transparentBlack);
        g.setColour(theme.textMuted);
        g.setFont(12.0f);
        g.drawFittedText(label.getText(), label.getLocalBounds(), juce::Justification::centred, 1);
    }
}
