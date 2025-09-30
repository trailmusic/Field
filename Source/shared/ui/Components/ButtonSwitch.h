#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"
#include "../../Core/FieldMetallic.h"
#include "../../Core/IconSystem.h"

//==============================================================================
// ButtonSwitch - Standard button switch with metallic styling and icons
//==============================================================================
class ButtonSwitch : public juce::ToggleButton
{
public:
    ButtonSwitch(const juce::String& buttonText = "", 
                IconSystem::IconType iconType = IconSystem::None,
                MetallicKind metallicKind = MetallicKind::None)
        : juce::ToggleButton(buttonText)
    {
        // Set up the button
        setClickingTogglesState(true);
        setButtonText(buttonText);
        
        // Set icon if provided
        if (iconType != IconSystem::None)
        {
            getProperties().set("iconType", (int)iconType);
        }
        
        // Set metallic properties
        if (metallicKind != MetallicKind::None)
        {
            setAreaMetallicForCell(*this, metallicKind);
        }
    }
    
    void setLookAndFeel(FieldLNF* lnf)
    {
        if (lnf)
        {
            juce::ToggleButton::setLookAndFeel(lnf);
        }
    }
    
    void setIcon(IconSystem::IconType iconType)
    {
        getProperties().set("iconType", (int)iconType);
        repaint();
    }
    
    void setMetallicKind(MetallicKind metallicKind)
    {
        if (metallicKind != MetallicKind::None)
        {
            setAreaMetallicForCell(*this, metallicKind);
        }
        else
        {
            // Remove metallic properties
            getProperties().set("metallic", false);
            getProperties().set("bandMetallic", false);
            getProperties().set("phaseMetallic", false);
            getProperties().set("delayMetallic", false);
            getProperties().set("motionMetallic", false);
            getProperties().set("xyMetallic", false);
            getProperties().set("reverbMetallic", false);
        }
        repaint();
    }
};
