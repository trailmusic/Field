#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/ui/Components/UIHelpers.h"

//------------------------------------------------------------------------------
// Pan Slider
// Custom rotary slider with pan visualization and split mode support
//------------------------------------------------------------------------------

class PanSlider : public juce::Slider
{
public:
    PanSlider() : Slider(RotaryHorizontalVerticalDrag, NoTextBox) {}
    
    void mouseEnter (const juce::MouseEvent&) override { hovered = true; repaint(); }
    void mouseExit  (const juce::MouseEvent&) override { hovered = false; repaint(); }
    void mouseDown  (const juce::MouseEvent& e) override { active = true;  Slider::mouseDown(e); repaint(); }
    void mouseUp    (const juce::MouseEvent& e) override { active = false; Slider::mouseUp(e);   repaint(); }

    void setSplitPercentage(float leftPercent, float rightPercent) { splitLeftPercent = leftPercent; splitRightPercent = rightPercent; repaint(); }
    void setLabel(const juce::String& label) { knobLabel = label; repaint(); }
    void setOverlayEnabled (bool enabled) { overlayEnabled = enabled; repaint(); }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat().reduced(2.0f);
        if (hovered || active) bounds = bounds.expanded(2.0f);
        
        // base rotary
        ui::paintRotaryWithLNF(g, *this, bounds);
        
        if (overlayEnabled)
        {
            // current pan indicator arc
            const float normalizedValue = (getValue() + 1.0f) * 0.5f; // -1..1 -> 0..1
            const float borderThickness = 3.0f;
            juce::Path valueBorder;
            const float valueAngle = juce::jmap(normalizedValue, 0.0f, 1.0f, 
                                        juce::MathConstants<float>::pi, 
                                        juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
            valueBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                             juce::MathConstants<float>::pi, valueAngle, true);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour(lf->theme.accent.withAlpha(0.8f));
            else
                g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
            g.strokePath(valueBorder, juce::PathStrokeType(borderThickness));
        }
        
        // split arcs (L: blue, R: red)
        if (overlayEnabled && splitLeftPercent >= 0.0f && splitRightPercent >= 0.0f)
        {
            const float borderThickness = 3.0f;

            juce::Path leftBorder;
            const float leftAngle = juce::jmap(splitLeftPercent, 0.0f, 100.0f, 
                                       juce::MathConstants<float>::pi, 
                                       juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
            leftBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                             juce::MathConstants<float>::pi, leftAngle, true);
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour(lf->theme.accent.withAlpha(0.8f));
            else
                g.setColour(juce::Colours::lightblue.withAlpha(0.8f));
            g.strokePath(leftBorder, juce::PathStrokeType(borderThickness));
            
            juce::Path rightBorder;
            const float rightAngle = juce::jmap(splitRightPercent, 0.0f, 100.0f, 
                                        juce::MathConstants<float>::pi, 
                                        juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi);
            rightBorder.addArc(bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(),
                              leftAngle, rightAngle, true);
            g.setColour(juce::Colour(0xFFFF6B6B).withAlpha(0.8f));
            g.strokePath(rightBorder, juce::PathStrokeType(borderThickness));
        }
        
        if (knobLabel.isNotEmpty())
        {
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                g.setColour(lf->theme.text);
            else
                g.setColour(juce::Colours::ghostwhite);
            g.setFont(juce::Font(juce::FontOptions(14.0f).withStyle("Bold")));
            g.drawText(knobLabel, bounds, juce::Justification::centred);
        }
    }
    
private:
    float splitLeftPercent = -1.0f;  // -1 = not in split mode
    float splitRightPercent = -1.0f;
    bool hovered = false, active = false, overlayEnabled = false;
    juce::String knobLabel;
};