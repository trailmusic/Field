#pragma once
#include <JuceHeader.h>

//==============================================================================
// VerticalSlider3D - Beautiful 3D vertical slider with metallic treatment
//==============================================================================
class VerticalSlider3D : public juce::Slider
{
public:
    VerticalSlider3D();
    ~VerticalSlider3D() override = default;
    
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    
    void setSliderStyle (SliderStyle newStyle);
    
private:
    void draw3DHandle (juce::Graphics& g, juce::Rectangle<float> handleRect);
    void drawMetallicTrack (juce::Graphics& g, juce::Rectangle<float> trackRect);
    void drawMetallicBackground (juce::Graphics& g, juce::Rectangle<float> backgroundRect);
    void drawMarkers (juce::Graphics& g, juce::Rectangle<float> trackRect);
    
    bool isDragging = false;
    juce::Point<float> lastMousePos;
};
