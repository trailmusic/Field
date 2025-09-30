#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/FieldTheme.h"
#include "ThemedIconButton.h"

//==============================================================================
// AuditionButton - Custom button with blinking animation for mono audition
//==============================================================================
class AuditionButton : public ThemedIconButton, public juce::Timer
{
public:
    AuditionButton() : ThemedIconButton(Options{ IconSystem::Mono, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true })
    {
        setButtonText("AUD");
        startTimerHz (6); // ~6 Hz repaint; blink phase derived from time
    }
    
    ~AuditionButton() override { stopTimer(); }
    
    void timerCallback() override { if (isShowing()) repaint(); }
    
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
        auto r = getLocalBounds().toFloat().reduced (2.0f);
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel  = lf ? lf->theme.panel  : juce::Colour (0xFF3A3D45);
        auto accent = lf ? lf->theme.accent : juce::Colour (0xFF2196F3);
        auto sh     = lf ? lf->theme.sh     : juce::Colour (0xFF2A2A2A);
        auto hl     = lf ? lf->theme.hl     : juce::Colour (0xFF4A4A4A);

        // Elevation shadow first
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour (lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour (juce::Colour (0x40000000));
        g.fillRoundedRectangle (r.translated (1.5f, 1.5f), 4.0f);

        const bool on = getToggleState();
        if (on)
        {
            // Blink between two blue tones
            auto now = juce::Time::getMillisecondCounter();
            const bool phase = ((now / 280) % 2) == 0; // ~3.6 Hz blink
            auto bg = phase ? accent : accent.darker (0.35f);
            if (down || over) bg = bg.brighter (0.10f);
            g.setColour (bg);
            g.fillRoundedRectangle (r, 4.0f);
            g.setColour (bg.darker (0.30f));
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }
        else
        {
            // Dark when not engaged (gradient panel)
            juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
            juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (r, 4.0f);
            g.setColour (down ? sh : (over ? hl : sh));
            g.drawRoundedRectangle (r, 4.0f, 1.0f);
        }

        // Text
        auto textCol = on ? juce::Colours::white : (lf ? lf->theme.textMuted : juce::Colour (0xFF888888));
        g.setColour (textCol);
        g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
        g.drawText ("AUD", r, juce::Justification::centred);
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AuditionButton)
};
