#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"
#include "ThemedIconButton.h"

class HelpButton : public ThemedIconButton
{
public:
    HelpButton() 
        : ThemedIconButton(Options{ 
            IconSystem::Help, 
            false, 
            ThemedIconButton::Style::GradientPanel, 
            3.0f, 
            4.0f, 
            false 
        }) 
    {
        // Set LookAndFeel for metallic styling
        // LookAndFeel will be set by parent component
    }
    
    ~HelpButton() override
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
