#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/FieldTheme.h"
#include "ThemedIconButton.h"

//==============================================================================
// ABButton - A/B state toggle button for preset management
//==============================================================================
class ABButton : public ThemedIconButton
{
public:
    explicit ABButton (bool isAButton)
    : ThemedIconButton({ IconSystem::ColorPalette /*unused*/, true, ThemedIconButton::Style::SolidAccentWhenOn, 4.0f, 4.0f, true })
    , isA(isAButton) { setButtonText(isA ? "A" : "B"); }

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
        g.setColour(getToggleState() ? juce::Colours::white : juce::Colour(0xFF888888));
        g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
        g.drawText(isA ? "A" : "B", r, juce::Justification::centred);
    }
    
private:
    bool isA;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ABButton)
};
