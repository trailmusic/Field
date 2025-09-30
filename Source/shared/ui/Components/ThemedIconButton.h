#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"

//==============================================================================
// ThemedIconButton - Base class for icon buttons with theme support
//==============================================================================
class ThemedIconButton : public juce::TextButton
{
public:
    enum class Style { SolidAccentWhenOn, GradientPanel };
    struct Options
    {
        IconSystem::IconType icon;
        bool isToggle;
        Style style;
        float corner;
        float glowRadius;
        bool glowWhenOn;
    };

    explicit ThemedIconButton(const Options& opts) : options(opts)
    {
        setClickingTogglesState(options.isToggle);
        setButtonText(""); // Prefer icon-only LNF drawing
    }

    void paintButton(juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto r = getLocalBounds().toFloat().reduced(0.5f, 0.5f);
        drawBackground(g, r, isMouseOver, isButtonDown);
        drawIcon(g, r);
    }

protected:
    Options options;

    void drawBackground (juce::Graphics& g, juce::Rectangle<float> r, bool over, bool down)
    {
        // Check for metallic properties first - if found, delegate to FieldLNF
        auto metallicKind = metallicFromProps(getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Delegate to FieldLNF for metallic buttons
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            {
                lf->drawButtonBackground(g, *this, juce::Colour(), over, down);
                return;
            }
        }
        
        // Fall back to custom rendering for non-metallic buttons
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel = lf ? lf->theme.panel : juce::Colour(0xFF3A3D45);
        auto accent = lf ? lf->theme.accent : juce::Colour(0xFF2196F3);
        // Optional per-button accent override (stored as ARGB int)
        if (getProperties().contains("accentOverrideARGB"))
        {
            auto v = (uint32) (int) getProperties()["accentOverrideARGB"];
            accent = juce::Colour (v);
        }
        auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
        auto hl = lf ? lf->theme.hl : juce::Colour(0xFF4A4A4A);

        const bool invert = (bool) getProperties().getWithDefault ("invertActive", false);
        const bool on = invert ? (! getToggleState()) : getToggleState();

        auto drawGradient = [&] {
            juce::Colour top = panel.brighter(0.10f), bot = panel.darker(0.10f);
            juce::ColourGradient grad(top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
            g.setGradientFill(grad);
            g.fillRoundedRectangle(r, options.corner);
            g.setColour(down ? sh : (over ? hl : sh));
            g.drawRoundedRectangle(r, options.corner, 1.0f);
        };

        if (options.style == Style::SolidAccentWhenOn && on)
        {
            auto bg = down ? accent.darker(0.30f) : (over ? accent.brighter(0.10f) : accent);
            g.setColour(bg);
            g.fillRoundedRectangle(r, options.corner);
            g.setColour(bg.darker(0.30f));
            g.drawRoundedRectangle(r, options.corner, 1.0f);
            if (options.glowWhenOn)
                g.setColour(bg.withAlpha(0.30f)), g.drawRoundedRectangle(r.expanded(1.0f), options.corner+1.0f, 2.0f);
        }
        else
        {
            drawGradient();
        }

        // subtle elevation shadow
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour(lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(r.translated(1.5f, 1.5f), options.corner);
    }

    void drawIcon (juce::Graphics& g, juce::Rectangle<float> r)
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto textMuted = lf ? lf->theme.textMuted : juce::Colour(0xFF888888);
        auto onCol = juce::Colours::white;
        // Optional per-button icon colour override (stored as ARGB int)
        if (getProperties().contains("iconOverrideARGB"))
        {
            auto v = (uint32) (int) getProperties()["iconOverrideARGB"];
            textMuted = juce::Colour (v);
            onCol = juce::Colour (v);
        }
        const bool invert = (bool) getProperties().getWithDefault ("invertActive", false);
        const bool on = invert ? (! getToggleState()) : getToggleState();

        // Custom label rendering if provided
        if (getProperties().contains ("labelText"))
        {
            juce::String label = getProperties()["labelText"].toString();
            auto col = on ? onCol : textMuted;
            g.setColour (col);
            // Fit text nicely inside button
            auto bounds = r; bounds.reduce (2.0f, 2.0f);
            g.setFont (juce::Font (juce::FontOptions (14.0f)).boldened());
            g.drawFittedText (label, bounds.toNearestInt(), juce::Justification::centred, 1);
            return;
        }

        IconSystem::drawIcon(g, options.icon, r, on ? onCol : textMuted);
    }
};
