#pragma once

#include <JuceHeader.h>
#include "UIHelpers.h"

class PanSlider : public juce::Slider
{
public:
    PanSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
    
    void mouseEnter(const juce::MouseEvent&) override { hovered = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { hovered = false; repaint(); }
    void mouseDown(const juce::MouseEvent& e) override { active = true; Slider::mouseDown(e); repaint(); }
    void mouseUp(const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e); repaint(); }

    void setSplitPercentage(float leftPercent, float rightPercent) 
    { 
        splitLeftPercent = leftPercent; 
        splitRightPercent = rightPercent; 
        repaint(); 
    }
    
    void setLabel(const juce::String& label) 
    { 
        knobLabel = label; 
        repaint(); 
    }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(2.0f);
        if (hovered || active) b = b.expanded(2.0f);
        ui::paintRotaryWithLNF(g, *this, b);
    }
    
private:
    bool hovered = false, active = false;
    float splitLeftPercent = 0.5f, splitRightPercent = 0.5f;
    juce::String knobLabel;
};
