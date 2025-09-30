#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldMetallic.h"

//------------------------------------------------------------------------------
// Preset Arrow Button
// Custom button with half-circle motif for preset navigation
//------------------------------------------------------------------------------

class PresetArrowButton : public juce::TextButton
{
public:
    explicit PresetArrowButton(bool isLeft) : juce::TextButton(""), leftArrow(isLeft) {}

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), isMouseOver, isButtonDown);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);

        // shadow
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour(lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(bounds.translated(1.5f, 1.5f), 3.0f);

        // panel gradient
        juce::Colour base = juce::Colour(0xFF3A3D45);
        juce::Colour top  = base.brighter(0.10f);
        juce::Colour bot  = base.darker(0.10f);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            base = lf->theme.panel; top = base.brighter(0.10f); bot = base.darker(0.10f);
        }
        juce::ColourGradient grad(top, bounds.getX(), bounds.getY(), bot, bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill(grad);
        g.fillRoundedRectangle(bounds, 3.0f);

        // border
        auto borderColor = juce::Colour(0xFF2A2A2A);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            borderColor = isButtonDown ? lf->theme.sh : (isMouseOver ? lf->theme.hl : lf->theme.sh);
        g.setColour(borderColor);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        // half-circle motif
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto accent = lf ? lf->theme.accent : juce::Colour(0xFF2196F3);
        auto defaultColor = lf ? lf->theme.text : juce::Colour(0xFFF0F2F5);

        auto c = bounds.getCentre();
        float r = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.25f;
        juce::Rectangle<float> circle (c.x - r, c.y - r, 2*r, 2*r);

        if (leftArrow)
        {
            g.setColour(accent);       g.fillEllipse(circle.getX(), circle.getY(), circle.getWidth(), circle.getHeight() * 0.5f);       // top
            g.setColour(defaultColor); g.fillEllipse(circle.getX(), circle.getCentreY(), circle.getWidth(), circle.getHeight() * 0.5f); // bottom
        }
        else
        {
            g.setColour(defaultColor); g.fillEllipse(circle.getX(), circle.getY(), circle.getWidth(), circle.getHeight() * 0.5f);       // top
            g.setColour(accent);       g.fillEllipse(circle.getX(), circle.getCentreY(), circle.getWidth(), circle.getHeight() * 0.5f); // bottom
        }

        g.setColour(juce::Colour(0xFF2A2A2A));
        g.drawEllipse(circle, 1.0f);
    }

private:
    bool leftArrow;
};
