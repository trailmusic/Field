#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"
#include "ThemedIconButton.h"

class QualityButton : public ThemedIconButton
{
public:
    QualityButton() 
        : ThemedIconButton(Options{ 
            IconSystem::Options, 
            true, 
            ThemedIconButton::Style::SolidAccentWhenOn, 
            4.0f, 
            4.0f, 
            true 
        }) 
    {
        // Set LookAndFeel for metallic styling
        setLookAndFeel(&FieldLNF::getInstance());
    }
    
    ~QualityButton() override
    {
        setLookAndFeel(nullptr);
    }
    
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
        
        // Fall back to ThemedIconButton rendering for non-metallic buttons
        ThemedIconButton::paintButton(g, over, down);
    }
};
