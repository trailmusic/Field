#include "FieldRendering.h"
#include "IconSystem.h"
#include "../ui/Controls/SimpleSwitchCell.h"

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
            // Metallic buttons - use metallic rendering system with KnobCell styling
            auto metalColors = MetallicRenderer::getMetallicColors(theme, metallicKind);
            
            if (button.getToggleState())
            {
                // Toggled state - use full metallic colors with KnobCell styling
                MetallicRenderer::paintMetal(g, r, metalColors, 8.0f); // Match KnobCell corner radius
                
                // Add KnobCell-style shadows for metallic buttons
                auto ri = r.reduced(3.0f).getSmallestIntegerContainer();
                juce::DropShadow ds1(juce::Colours::black.withAlpha(0.28f), 10, {-1, -1});
                juce::DropShadow ds2(juce::Colours::white.withAlpha(0.18f), 5, {-1, -1});
                ds1.drawForRectangle(g, ri);
                ds2.drawForRectangle(g, ri);
                
                // Add inner rim like KnobCell
                g.setColour(juce::Colour(0xFF51565D).withAlpha(0.16f));
                g.drawRoundedRectangle(r.reduced(4.0f), 8.0f - 1.0f, 0.8f);
                
                // Add KnobCell-style border
                g.setColour(theme.accentSecondary.withAlpha(0.85f));
                g.drawRoundedRectangle(r.reduced(2.0f), 8.0f, 1.5f);
                
                return; // Early return for metallic rendering
            }
            else
            {
                // Untoggled state - use metallic colors with reduced intensity and KnobCell styling
                auto reducedMetal = metalColors;
                reducedMetal.top = reducedMetal.top.withAlpha(0.6f);
                reducedMetal.bottom = reducedMetal.bottom.withAlpha(0.6f);
                MetallicRenderer::paintMetal(g, r, reducedMetal, 8.0f); // Match KnobCell corner radius
                
                // Add KnobCell-style shadows for metallic buttons
                auto ri = r.reduced(3.0f).getSmallestIntegerContainer();
                juce::DropShadow ds1(juce::Colours::black.withAlpha(0.28f), 10, {-1, -1});
                juce::DropShadow ds2(juce::Colours::white.withAlpha(0.18f), 5, {-1, -1});
                ds1.drawForRectangle(g, ri);
                ds2.drawForRectangle(g, ri);
                
                // Add inner rim like KnobCell
                g.setColour(juce::Colour(0xFF51565D).withAlpha(0.16f));
                g.drawRoundedRectangle(r.reduced(4.0f), 8.0f - 1.0f, 0.8f);
                
                // Add KnobCell-style border
                g.setColour(theme.accentSecondary.withAlpha(0.85f));
                g.drawRoundedRectangle(r.reduced(2.0f), 8.0f, 1.5f);
                
                return; // Early return for metallic rendering
            }
        }
        else
        {
            // Default - use KnobCell styling for consistency
            fill = isMouseOver ? panel.brighter(0.06f) : panel;
            if (isButtonDown) fill = fill.darker(0.12f);
        }

        const float cr = 8.0f; // Match KnobCell corner radius
        g.setColour(fill);
        g.fillRoundedRectangle(r, cr);

        // Add KnobCell-style shadows for all buttons
        auto ri = r.reduced(3.0f).getSmallestIntegerContainer();
        juce::DropShadow ds1(juce::Colours::black.withAlpha(0.28f), 10, {-1, -1});
        juce::DropShadow ds2(juce::Colours::white.withAlpha(0.18f), 5, {-1, -1});
        ds1.drawForRectangle(g, ri);
        ds2.drawForRectangle(g, ri);

        // Add inner rim like KnobCell
        g.setColour(juce::Colour(0xFF51565D).withAlpha(0.16f));
        g.drawRoundedRectangle(r.reduced(4.0f), 8.0f - 1.0f, 0.8f);

        // Border - use KnobCell styling for all buttons
        g.setColour(theme.accentSecondary.withAlpha(0.85f)); // Match KnobCell border color
        g.drawRoundedRectangle(r.reduced(2.0f), cr, 1.5f); // Match KnobCell border thickness
    }

    void drawButtonText(juce::Graphics& g, juce::TextButton& button, bool isMouseOver, bool isButtonDown, const FieldTheme& theme)
    {
        // Let JUCE handle the default button text rendering
        // This ensures text labels are properly displayed
        juce::LookAndFeel_V4 lf;
        lf.drawButtonText(g, button, isMouseOver, isButtonDown);
    }

    // Sophisticated rotary slider rendering with tick system and tracks
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPosProportional,
                         float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider, const FieldTheme& theme)
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
        auto centre = bounds.getCentre();
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
        
        // Check for hover state and apply raise effect
        bool isHovered = slider.isMouseOver();
        float hoverOffset = isHovered ? 2.0f : 0.0f;
        
        // Check for double-click flash effect
        bool isFlashing = slider.getProperties().getWithDefault("flash", false);
        float flashIntensity = isFlashing ? 1.0f : 0.0f;
        
        // Apply hover raise effect
        if (isHovered)
        {
            bounds = bounds.translated(0, -hoverOffset);
            centre = bounds.getCentre();
        }
        
        // Calculate the actual angle range (typically π to π + 2π for full rotation)
        auto angleRange = rotaryEndAngle - rotaryStartAngle;
        auto currentAngle = rotaryStartAngle + sliderPosProportional * angleRange;
        
        // Draw the track background (outer ring) - slightly smaller knob
        auto trackRadius = radius * 0.80f;  // Reduced from 0.85f to 0.80f
        auto trackThickness = 4.0f;  // Reduced from 6.0f to 4.0f (thinner slider)
        
        // Draw shadow first, before any other knob elements
        // Create ring shadow that only affects the outer track area, not the center
        // This prevents darkening the center where the knob name appears
        auto shadowRadius = trackRadius + trackThickness * 0.5f; // Slightly larger than track
        auto innerShadowRadius = trackRadius - trackThickness * 0.5f; // Inner edge of track
        
        juce::Path shadowRing;
        shadowRing.addEllipse(centre.x - shadowRadius, centre.y - shadowRadius, 
                             shadowRadius * 2, shadowRadius * 2);
        shadowRing.addEllipse(centre.x - innerShadowRadius, centre.y - innerShadowRadius, 
                             innerShadowRadius * 2, innerShadowRadius * 2);
        
        // Heavy shadow - only around the track ring (reduced intensity)
        g.setColour(theme.shadowDark.withAlpha(isHovered ? 0.2f : 0.1f));
        g.fillPath(shadowRing);
        
        // Light shadow overlay - only around the track ring (reduced intensity)
        g.setColour(theme.shadowLight.withAlpha(isHovered ? 0.1f : 0.05f));
        g.fillPath(shadowRing);
        
        // Flash effect overlay (if double-clicked)
        if (isFlashing)
        {
            juce::Colour flashColor = theme.accent.brighter(0.8f).withAlpha(flashIntensity * 0.6f);
            g.setColour(flashColor);
            g.fillEllipse(centre.x - trackRadius, centre.y - trackRadius, 
                         trackRadius * 2, trackRadius * 2);
        }
        
        // Track background
        g.setColour(theme.panel.darker(0.1f));
        g.drawEllipse(centre.x - trackRadius, centre.y - trackRadius, 
                     trackRadius * 2, trackRadius * 2, trackThickness);
        
        // Track highlight
        g.setColour(theme.hl);
        g.drawEllipse(centre.x - trackRadius, centre.y - trackRadius, 
                     trackRadius * 2, trackRadius * 2, 1.0f);
        
        // Draw quarter marks (12, 3, 6, 9 o'clock positions)
        // Starting at 6 o'clock (π), then 9 (3π/2), 12 (2π), 3 (π/2)
        std::vector<float> quarterAngles = {
            juce::MathConstants<float>::pi,                    // 6 o'clock
            juce::MathConstants<float>::pi * 1.5f,            // 9 o'clock  
            juce::MathConstants<float>::twoPi,                // 12 o'clock
            juce::MathConstants<float>::halfPi                // 3 o'clock
        };
        
        
        for (auto angle : quarterAngles)
        {
            // Place ticks in the center of the track
            auto tickRadius = trackRadius;  // Center of the track
            auto tickX = centre.x + tickRadius * std::cos(angle - juce::MathConstants<float>::halfPi);
            auto tickY = centre.y + tickRadius * std::sin(angle - juce::MathConstants<float>::halfPi);
            
            // Quarter tick dots - accent color dots on the knob arc
            g.setColour(theme.accent);
            g.fillEllipse(tickX - 2, tickY - 2, 4, 4);  // Smaller size: 4x4 instead of 16x16
        }
        
        // Draw the value arc (filled portion)
        auto valueAngle = rotaryStartAngle + sliderPosProportional * angleRange;
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, trackRadius, trackRadius,
                             0.0f, rotaryStartAngle, valueAngle, true);
        
        // Value arc with gradient
        juce::ColourGradient gradient(theme.accent, centre.x, centre.y,
                                     theme.accent.brighter(0.3f), 
                                     centre.x + trackRadius * 0.5f, centre.y + trackRadius * 0.5f, true);
        g.setGradientFill(gradient);
        g.strokePath(valueArc, juce::PathStrokeType(trackThickness));
        
        // Draw the thumb (current position indicator)
        auto thumbRadius = trackRadius;
        auto thumbX = centre.x + thumbRadius * std::cos(currentAngle - juce::MathConstants<float>::halfPi);
        auto thumbY = centre.y + thumbRadius * std::sin(currentAngle - juce::MathConstants<float>::halfPi);
        
        // Thumb shadow
        g.setColour(theme.shadowDark.withAlpha(0.3f));
        g.fillEllipse(thumbX - 4, thumbY - 4, 8, 8);  // Reduced from 12x12 to 8x8
        
        // Thumb body
        g.setColour(theme.accent);
        g.fillEllipse(thumbX - 3, thumbY - 3, 6, 6);  // Reduced from 10x10 to 6x6
        
        // Thumb highlight
        g.setColour(theme.accent.brighter(0.4f));
        g.fillEllipse(thumbX - 2, thumbY - 2, 4, 4);  // Reduced from 6x6 to 4x4
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
        // Check if this ComboBox is in a SimpleSwitchCell with metallic properties
        auto* parent = box.getParentComponent();
        auto metallicKind = metallicFromProps(box.getProperties());
        
        juce::Rectangle<float> r;
        if (parent && dynamic_cast<SimpleSwitchCell*>(parent) && metallicKind != MetallicKind::None)
        {
            // For metallic ComboBoxes in SimpleSwitchCell, use full bounds to match KnobCell
            r = juce::Rectangle<float>(0, 0, (float)width, (float)height);
        }
        else
        {
            // Standard ComboBox with padding
            r = juce::Rectangle<float>(0, 0, (float)width, (float)height).reduced(2.0f);
        }
        
        auto accent = theme.accent;

        // Check for metallic properties first
        if (metallicKind != MetallicKind::None)
        {
            // Metallic ComboBox - use metallic rendering system with KnobCell styling
            auto metalColors = MetallicRenderer::getMetallicColors(theme, metallicKind);
            MetallicRenderer::paintMetal(g, r, metalColors, 8.0f); // Match KnobCell corner radius
            
            // Add KnobCell-style shadows for metallic ComboBoxes
            auto ri = r.reduced(3.0f).getSmallestIntegerContainer();
            juce::DropShadow ds1(juce::Colours::black.withAlpha(0.28f), 10, {-1, -1});
            juce::DropShadow ds2(juce::Colours::white.withAlpha(0.18f), 5, {-1, -1});
            ds1.drawForRectangle(g, ri);
            ds2.drawForRectangle(g, ri);
            
            // Add inner rim like KnobCell
            g.setColour(juce::Colour(0xFF51565D).withAlpha(0.16f));
            g.drawRoundedRectangle(r.reduced(4.0f), 8.0f - 1.0f, 0.8f);
            
            // Create recessed button window effect for metallic ComboBoxes
            // Increase top/bottom padding to compensate for styling changes
            auto buttonWindow = r.reduced(6.0f, 16.0f); // More reduction on top/bottom to make interior smaller
            g.setColour(theme.panel.darker(0.3f));
            g.fillRoundedRectangle(buttonWindow, 3.0f);
            
            // Add subtle highlight to top edge of button window
            g.setColour(theme.panel.brighter(0.1f));
            g.drawHorizontalLine(buttonWindow.getY() + 1, buttonWindow.getX(), buttonWindow.getRight());
        }
        else
        {
            // Background: mimic SwitchCell panel (mode cell style)
            g.setColour(theme.panel);
            g.fillRoundedRectangle(r, 5.0f);
        }

        // Border - only draw if not in SimpleSwitchCell (which handles its own border)
        // Check if this ComboBox is wrapped in a SimpleSwitchCell
        if (!parent || !dynamic_cast<SimpleSwitchCell*>(parent))
        {
            g.setColour(theme.sh);
            g.drawRoundedRectangle(r, 5.0f, 1.0f);
        }
        else if (parent && dynamic_cast<SimpleSwitchCell*>(parent) && metallicKind != MetallicKind::None)
        {
            // For metallic ComboBoxes in SimpleSwitchCell, draw KnobCell-style border
            auto border = r.reduced(2.0f);
            g.setColour(theme.accentSecondary.withAlpha(0.85f)); // Match KnobCell accent color
            g.drawRoundedRectangle(border, 8.0f, 1.5f); // Match KnobCell border thickness
        }

        // Arrow - only show if no item is selected or if ComboBox is focused
        bool hasSelection = box.getSelectedId() > 0;
        bool shouldShowArrow = !hasSelection || box.hasKeyboardFocus(true);
        
        if (shouldShowArrow)
        {
            juce::Path p;
            p.addTriangle(r.getCentreX() - 4, r.getCentreY() - 2,
                         r.getCentreX() + 4, r.getCentreY() - 2,
                         r.getCentreX(), r.getCentreY() + 2);
            g.setColour(accent);
            g.fillPath(p);
        }
    }

    void applyTextWrapping(juce::ComboBox& box, juce::Label& label)
    {
        // Check if text contains line breaks (already formatted for two lines)
        juce::String text = label.getText();
        if (text.contains("\n"))
        {
            // Text already has line breaks, just set up formatting
            label.setJustificationType(juce::Justification::centred);
            label.setFont(juce::Font(10.0f)); // Slightly smaller font for two lines
        }
        else if (text.contains(" "))
        {
            // Fallback: split text into two words using spaces as separators
            juce::StringArray words = juce::StringArray::fromTokens(text, " ", "");
            if (words.size() >= 2)
            {
                // Create two-line text with line break
                juce::String twoLineText = words[0] + "\n" + words[1];
                label.setText(twoLineText, juce::dontSendNotification);
            }
            
            // Enable multi-line text for two-word labels
            label.setJustificationType(juce::Justification::centred);
            // Set font size to accommodate two lines
            label.setFont(juce::Font(10.0f)); // Slightly smaller font for two lines
        }
        
        // For metallic ComboBoxes, ensure text color is visible over metallic background
        auto metallicKind = metallicFromProps(box.getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Set text color to be visible over metallic background
            label.setColour(juce::Label::textColourId, juce::Colours::white);
        }
    }

    void positionComboBoxText(juce::ComboBox& box, juce::Label& label)
    {
        // Check if this ComboBox is in a SimpleSwitchCell with metallic properties
        auto* parent = box.getParentComponent();
        auto metallicKind = metallicFromProps(box.getProperties());
        
        if (parent && dynamic_cast<SimpleSwitchCell*>(parent) && metallicKind != MetallicKind::None)
        {
            // For metallic ComboBoxes in SimpleSwitchCell, use full bounds to match KnobCell
            label.setBounds(box.getLocalBounds());
        }
        else
        {
            // Standard ComboBox text positioning with padding
            label.setBounds(box.getLocalBounds().reduced(4));
        }
        
        // Enable text wrapping for two-word labels
        label.setJustificationType(juce::Justification::centred);
        
        // Apply text wrapping logic immediately to prevent flashing
        applyTextWrapping(box, label);
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
            g.setColour(theme.meters.panelDark);
            g.fillRoundedRectangle(r, r.getHeight() * 0.5f);
        }
    }

    void drawKnobLabel(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, const FieldTheme& theme)
    {
        if (text.isEmpty()) return;
        
        // Simple text without background gradient
        g.setColour(theme.text);
        g.setFont(juce::Font(juce::FontOptions(11.0f).withStyle("Bold")));
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
