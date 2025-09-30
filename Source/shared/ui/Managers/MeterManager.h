#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Components/VerticalLRMeters.h"
#include "../Components/IOGainMeters.h"
#include "../Components/CorrelationMeter.h"
#include "../Components/ControlContainer.h"

// Forward declaration
class MyPluginAudioProcessorEditor;

//==============================================================================
// MeterManager - Centralized management for all meter components
//==============================================================================
class MeterManager
{
public:
    MeterManager(MyPluginAudioProcessorEditor& editor);
    ~MeterManager() = default;
    
    // Core Methods
    void initializeMeters();
    void layoutMeters(juce::Rectangle<int> area, float s, float sv);
    void cleanupMeters();
    
    // Container Management
    void setMetersContainerBounds(juce::Rectangle<int> bounds);
    juce::Rectangle<int> getMetersContainerBounds() const;
    
    // Meter Access
    VerticalLRMeters& getLRMeters() { return lrMeters; }
    IOGainMeters& getIOGainMeters() { return ioMeters; }
    CorrelationMeter& getCorrelationMeter() { return corrMeter; }
    ControlContainer& getMetersContainer() { return metersContainer; }
    
private:
    MyPluginAudioProcessorEditor& editor;
    
    // Meter Components
    VerticalLRMeters lrMeters;
    IOGainMeters ioMeters;
    CorrelationMeter corrMeter;
    
    // Container
    ControlContainer metersContainer;
};
