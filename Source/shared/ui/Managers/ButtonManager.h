#pragma once

#include <JuceHeader.h>
#include "../../Core/PluginEditor.h"
#include "../Components/PhaseModeButton.h"
#include "../Components/QualityButton.h"

class ButtonManager
{
public:
    ButtonManager(MyPluginAudioProcessorEditor& editor);
    ~ButtonManager() = default;
    
    // Button initialization
    void initializeButtons();
    void setupButtonCallbacks();
    
    // Button functionality
    void setPhaseMode(int mode);
    void setQualityMode(int mode);
    
    // Getters for buttons
    PhaseModeButton& getPhaseModeButton() { return phaseModeButton; }
    QualityButton& getQualityButton() { return qualityButton; }
    
    // Container for buttons
    juce::Component buttonsContainer;
    
private:
    MyPluginAudioProcessorEditor& editor;
    
    // Button components
    PhaseModeButton phaseModeButton;
    QualityButton qualityButton;
    
    // Helper methods
    void showPhaseModeMenu();
    void showQualityModeMenu();
};
