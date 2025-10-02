#include "SliderManager.h"
#include "../../Core/PluginEditor.h"

//==============================================================================
// SliderManager Implementation
//==============================================================================

SliderManager::SliderManager(MyPluginAudioProcessorEditor& editor)
    : editor(editor)
{
    // Initialize sliders container
    rightSlidersContainer.setTitle("");
    rightSlidersContainer.setShowBorder(false);
}

void SliderManager::initializeSliders()
{
    // Configure Input slider
    inputSlider.setRange(-60.0, 12.0, 0.1);
    inputSlider.setValue(0.0);
    inputSlider.setName("input_gain");
    inputSlider.setTextValueSuffix(" dB");
    
    // Configure Output slider
    outputSlider.setRange(-60.0, 12.0, 0.1);
    outputSlider.setValue(0.0);
    outputSlider.setName("output_gain");
    outputSlider.setTextValueSuffix(" dB");
    
    // Configure Mix slider
    mixSlider.setRange(0.0, 100.0, 0.1);
    mixSlider.setValue(100.0);
    mixSlider.setName("mix");
    mixSlider.setTextValueSuffix(" %");
    
    // Add sliders to their container
    rightSlidersContainer.addAndMakeVisible(inputSlider);
    rightSlidersContainer.addAndMakeVisible(outputSlider);
    rightSlidersContainer.addAndMakeVisible(mixSlider);
}

void SliderManager::layoutSliders(juce::Rectangle<int> slidersArea)
{
    // Layout the individual sliders horizontally within the container with gaps
    const int gap = 4; // Closed gap between sliders (was 6, now 4)
    const int totalGaps = gap * 2; // Two gaps between three sliders
    const int sliderWidth = (slidersArea.getWidth() - totalGaps) / 3 + 1; // Add one more unit of width
    
    inputSlider.setBounds(0, 0, sliderWidth, slidersArea.getHeight());
    outputSlider.setBounds(sliderWidth + gap, 0, sliderWidth, slidersArea.getHeight());
    mixSlider.setBounds((sliderWidth + gap) * 2, 0, sliderWidth, slidersArea.getHeight());
}

void SliderManager::setSlidersContainerBounds(juce::Rectangle<int> bounds)
{
    rightSlidersContainer.setBounds(bounds);
}

juce::Rectangle<int> SliderManager::getSlidersContainerBounds() const
{
    return rightSlidersContainer.getBounds();
}

void SliderManager::cleanupSliders()
{
    // Cleanup handled by destructor
}
