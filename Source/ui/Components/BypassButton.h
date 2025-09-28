#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"

//==============================================================================
// BypassButton - Header bypass button with theme-based animation
//==============================================================================
class BypassButton : public juce::TextButton, public juce::Timer
{
public:
    BypassButton(FieldLNF& mainLnf) : juce::TextButton(""), bypassLookAndFeel(mainLnf)
    {
        setLookAndFeel(&bypassLookAndFeel);
        setClickingTogglesState(true);
        // Use theme animation FPS for consistent performance
        startTimerHz(mainLnf.theme.animation.animationFps);
    }
    
    ~BypassButton() override
    {
        stopTimer();
        setLookAndFeel(nullptr);
    }
    
    void timerCallback() override
    {
        // Always repaint when visible to drive blink smoothly when toggled
        if (isShowing()) repaint();
    }
    
private:
    class BypassLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        BypassLookAndFeel(FieldLNF& mainLnf) : mainFieldLNF(mainLnf) {}
        
        void drawButtonBackground(juce::Graphics& g, juce::Button& button, const juce::Colour&,
                                bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
        {
            // Check for metallic properties first - if found, delegate to FieldLNF
            auto metallicKind = metallicFromProps (button.getProperties());
            if (metallicKind != MetallicKind::None)
            {
                // Delegate to the main FieldLNF instance
                mainFieldLNF.drawButtonBackground(g, button, juce::Colour(), shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
                return;
            }
            
            // Fall back to custom rendering for non-metallic buttons
            auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
            
            // Read current theme
            juce::Colour accent = juce::Colour(0xFF2196F3); // Hardcoded blue color
            juce::Colour textGrey = juce::Colour(0xFFB8BDC7);
            juce::Colour panel = juce::Colour(0xFF3A3D45);
            FieldLNF* lnf = nullptr;
            {
                const juce::Component* c = &button;
                while (c)
                {
                    if (auto* lf = dynamic_cast<FieldLNF*>(&c->getLookAndFeel()))
                    {
                        lnf = lf;
                        accent = lf->theme.accent;
                        textGrey = lf->theme.textMuted; // theme font grey
                        panel = lf->theme.panel;
                        break;
                    }
                    c = c->getParentComponent();
                }
            }

            juce::Colour baseColour;
            if (button.getToggleState())
            {
                // Bypassed: use theme animation colors and timing
                if (lnf && lnf->theme.animation.enableAnimations)
                {
                    auto now = juce::Time::getMillisecondCounter();
                    const bool phase = ((now / lnf->theme.animation.blinkIntervalMs) % 2) == 0;
                    baseColour = phase ? lnf->theme.animation.bypassBlinkDark : lnf->theme.animation.bypassBlinkBright;
                    g.setColour(baseColour.withAlpha(phase ? lnf->theme.animation.blinkAlphaDark : lnf->theme.animation.blinkAlphaBright));
                }
                else
                {
                    // Fallback to static grey when animations disabled
                    baseColour = textGrey;
                    g.setColour(baseColour.withAlpha(0.20f));
                }
                g.fillRoundedRectangle(bounds.expanded(4.0f), 6.0f);
            }
            else
            {
                baseColour = accent;
            }

            if (shouldDrawButtonAsDown || shouldDrawButtonAsHighlighted)
                baseColour = baseColour.brighter(0.1f);
            
            // shadow
            g.setColour(lnf ? lnf->theme.shadowDark.withAlpha (0.25f) : juce::Colour(0x40000000));
            g.fillRoundedRectangle(bounds.translated(2.0f, 2.0f), 4.0f);
            
            // bg + border
            g.setColour(baseColour);
            g.fillRoundedRectangle(bounds, 4.0f);
            g.setColour(baseColour.darker(0.45f));
            g.drawRoundedRectangle(bounds, 4.0f, 1.0f);
        }
    private:
        FieldLNF& mainFieldLNF;
    };
    
    BypassLookAndFeel bypassLookAndFeel;
};
