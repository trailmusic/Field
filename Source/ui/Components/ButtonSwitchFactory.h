#pragma once

#include <JuceHeader.h>
#include "ButtonSwitch.h"
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"

//==============================================================================
// ButtonSwitchFactory - Factory for creating consistent Button Switches
//==============================================================================
class ButtonSwitchFactory
{
public:
    static std::unique_ptr<ButtonSwitch> createButtonSwitch(
        const juce::String& buttonText,
        IconSystem::IconType iconType = IconSystem::None,
        MetallicKind metallicKind = MetallicKind::Band,
        FieldLNF* lnf = nullptr)
    {
        auto button = std::make_unique<ButtonSwitch>(buttonText, iconType, metallicKind);
        
        if (lnf)
        {
            button->setLookAndFeel(lnf);
        }
        
        return button;
    }
    
    // Convenience methods for common button types
    static std::unique_ptr<ButtonSwitch> createXYButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("XY", IconSystem::XY, MetallicKind::XY, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createPolarButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("Polar", IconSystem::Polar, MetallicKind::XY, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createHeatButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("Heat", IconSystem::Heat, MetallicKind::XY, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createLearnButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("", IconSystem::Learn, MetallicKind::Band, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createStopButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("", IconSystem::Stop, MetallicKind::Band, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createDynamicButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("DYN", IconSystem::Dynamic, MetallicKind::Band, lnf);
    }
    
    static std::unique_ptr<ButtonSwitch> createSpectralButton(FieldLNF* lnf = nullptr)
    {
        return createButtonSwitch("SPEC", IconSystem::Spectral, MetallicKind::Band, lnf);
    }
};
