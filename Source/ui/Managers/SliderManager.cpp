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
    // Add sliders to their container
    rightSlidersContainer.addAndMakeVisible(inputSlider);
    rightSlidersContainer.addAndMakeVisible(outputSlider);
    rightSlidersContainer.addAndMakeVisible(mixSlider);
}

void SliderManager::layoutSliders(juce::Rectangle<int> slidersArea)
{
    // Layout the individual sliders horizontally within the container
    const int sliderWidth = slidersArea.getWidth() / 3;
    inputSlider.setBounds(0, 0, sliderWidth, slidersArea.getHeight());
    outputSlider.setBounds(sliderWidth, 0, sliderWidth, slidersArea.getHeight());
    mixSlider.setBounds(sliderWidth * 2, 0, sliderWidth, slidersArea.getHeight());
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
