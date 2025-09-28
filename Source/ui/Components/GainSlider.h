#pragma once

#include <JuceHeader.h>
#include "UIHelpers.h"

class GainSlider : public juce::Slider
{
public:
    GainSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
    
    void mouseEnter(const juce::MouseEvent&) override { hovered = true; repaint(); }
    void mouseExit(const juce::MouseEvent&) override { hovered = false; repaint(); }
    void mouseDown(const juce::MouseEvent& e) override { active = true; Slider::mouseDown(e); repaint(); }
    void mouseUp(const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e); repaint(); }

    void paint(juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat().reduced(2.0f);
        if (hovered || active) b = b.expanded(2.0f);
        ui::paintRotaryWithLNF(g, *this, b);
    }
    
private:
    bool hovered = false, active = false;
};
