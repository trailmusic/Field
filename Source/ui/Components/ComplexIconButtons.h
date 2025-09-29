#pragma once
#include "ThemedIconButton.h"
#include "Core/FieldLookAndFeel.h"
#include "Core/FieldMetallic.h"
#include "Core/IconSystem.h"

//------------------------------------------------------------------------------
// Complex Icon Button Classes
// These have custom paint methods for specialized behavior
//------------------------------------------------------------------------------

class FullScreenButton : public ThemedIconButton
{
public:
    FullScreenButton() : ThemedIconButton(Options{ IconSystem::FullScreen, true, ThemedIconButton::Style::GradientPanel, 3.0f, 4.0f, false }) {}
    
    void paintButton(juce::Graphics& g, bool over, bool down) override
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
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto iconColor = lf ? lf->theme.textMuted : juce::Colour(0xFF888888);
        auto icon = getToggleState() ? IconSystem::ExitFullScreen : IconSystem::FullScreen;
        IconSystem::drawIcon(g, icon, r.reduced(4.0f), iconColor);
    }
};

class ColorModeButton : public ThemedIconButton
{
public:
    ColorModeButton() : ThemedIconButton(Options{ IconSystem::ColorPalette, true, ThemedIconButton::Style::GradientPanel, 4.0f, 4.0f, false }) {}
    
    void paintButton(juce::Graphics& g, bool over, bool down) override
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
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto iconColor = lf ? lf->theme.accent : juce::Colour(0xFF5AA9E6);
        IconSystem::drawIcon(g, IconSystem::ColorPalette, r.reduced(4.0f), iconColor);
    }
};

class LockButton : public ThemedIconButton
{
public:
    LockButton() : ThemedIconButton(Options{ IconSystem::Lock, true, ThemedIconButton::Style::GradientPanel, 4.0f, 4.0f, false }) {}
    
    void paintButton(juce::Graphics& g, bool over, bool down) override
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
        auto r = getLocalBounds().toFloat().reduced(2.0f);
        drawBackground(g, r, over, down);
        auto icon = getToggleState() ? IconSystem::Lock : IconSystem::Unlock;
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto col = getToggleState() ? (lf ? lf->theme.accent : juce::Colour(0xFF5AA9E6))
                                    : (lf ? lf->theme.textMuted : juce::Colour(0xFF888888));
        IconSystem::drawIcon(g, icon, r.reduced(4.0f), col);
    }
};
