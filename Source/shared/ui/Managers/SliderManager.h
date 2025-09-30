#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../Components/VerticalSlider3D.h"
#include "../Components/ControlContainer.h"

// Forward declaration
class MyPluginAudioProcessorEditor;

//==============================================================================
// SliderManager - Centralized management for all slider components
//==============================================================================
class SliderManager
{
public:
    SliderManager(MyPluginAudioProcessorEditor& editor);
    ~SliderManager() = default;
    
    // Core Methods
    void initializeSliders();
    void layoutSliders(juce::Rectangle<int> area);
    void cleanupSliders();
    
    // Container Management
    void setSlidersContainerBounds(juce::Rectangle<int> bounds);
    juce::Rectangle<int> getSlidersContainerBounds() const;
    
    // Slider Access
    VerticalSlider3D& getInputSlider() { return inputSlider; }
    VerticalSlider3D& getOutputSlider() { return outputSlider; }
    VerticalSlider3D& getMixSlider() { return mixSlider; }
    ControlContainer& getSlidersContainer() { return rightSlidersContainer; }
    
private:
    MyPluginAudioProcessorEditor& editor;
    
    // Slider Components
    VerticalSlider3D inputSlider;
    VerticalSlider3D outputSlider;
    VerticalSlider3D mixSlider;
    
    // Container
    ControlContainer rightSlidersContainer;
};
