#pragma once

#include <JuceHeader.h>
#include "../../Core/FieldLookAndFeel.h"

class TooltipBubble : public juce::Component
{
public:
    std::function<void(juce::Point<int>)> onMenu;
    
    void setText(juce::String t) 
    { 
        text = std::move(t); 
        repaint(); 
    }
    
    void setAnchor(juce::Rectangle<int> target)
    {
        const int w = juce::jlimit(160, 300, (int)juce::jlimit(160.0f, 300.0f, (float)text.length() * 6.5f));
        const int h = 54; // compact height
        const int x = target.getRight() + 8;
        const int y = juce::jmax(target.getY() - 8, 6);
        setBounds(x, y, w, h);
        infoRect = juce::Rectangle<int>(getWidth() - 20, 6, 14, 14);
    }
    
    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel = lf ? lf->theme.panel : juce::Colour(0xFF2F3136);
        auto border = lf ? lf->theme.hl : juce::Colour(0xFF45484D);
        auto textCol = lf ? lf->theme.text : juce::Colours::white;
        
        // Panel
        juce::ColourGradient grad(panel.brighter(0.06f), r.getX(), r.getY(), panel.darker(0.10f), r.getX(), r.getBottom(), false);
        g.setGradientFill(grad); 
        g.fillRoundedRectangle(r, 6.0f);
        g.setColour(border.withAlpha(0.9f)); 
        g.drawRoundedRectangle(r, 6.0f, 1.0f);
        
        // Text
        g.setColour(textCol.withAlpha(0.92f));
        g.setFont(juce::Font(juce::FontOptions(11.0f)));
        auto textArea = getLocalBounds().reduced(8, 8).withTrimmedRight(22);
        g.drawFittedText(text, textArea, juce::Justification::topLeft, 3);
        
        // Small info/menu glyph (three dots)
        juce::Rectangle<float> dotArea = infoRect.toFloat();
        g.setColour(textCol.withAlpha(0.8f));
        const float cx = dotArea.getCentreX(), cy = dotArea.getCentreY();
        for (int i = -1; i <= 1; ++i) 
            g.fillEllipse(cx + i * 4.0f - 1.25f, cy - 1.25f, 2.5f, 2.5f);
    }
    
    void mouseUp(const juce::MouseEvent& e) override
    {
        if (infoRect.contains(e.getPosition()) && onMenu) 
            onMenu(localPointToGlobal(e.getPosition()));
    }
    
private:
    juce::String text;
    juce::Rectangle<int> infoRect;
};
