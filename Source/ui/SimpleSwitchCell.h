#pragma once

#include <JuceHeader.h>
#include "../Core/FieldLookAndFeel.h"

// Minimal reusable switch/host cell with caption and themed panel/border.
class SimpleSwitchCell : public juce::Component
{
public:
    explicit SimpleSwitchCell (juce::Component& childToHost)
        : child (childToHost)
    {
        setOpaque (false);
        caption.setJustificationType (juce::Justification::centred);
        caption.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (caption);
        addAndMakeVisible (child);
    }

    void setCaption (const juce::String& text)
    {
        captionText = text;
        caption.setText (captionText, juce::dontSendNotification);
        repaint();
    }

    void setShowBorder (bool on) { showBorder = on; repaint(); }
    void setReverbMaroon (bool on) { reverbMaroon = on; repaint(); }
    void setDelayTheme (bool on) { delayTheme = on; repaint(); }

    void resized() override
    {
        auto b = getLocalBounds().reduced (6);
        const int capH = captionText.isNotEmpty() ? 14 : 0;
        if (capH > 0)
        {
            caption.setBounds (b.removeFromTop (capH));
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
        }
        child.setBounds (b);
    }

    void paint (juce::Graphics& g) override
    {
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto r = getLocalBounds().toFloat().reduced (3.0f);
        const float rad = 8.0f;
        auto panel = lf ? lf->theme.panel : juce::Colour (0xFF3A3D45);
        auto border = lf ? lf->theme.sh    : juce::Colour (0xFF2A2A2A);
        
        // Check if the child component has metallic properties
        auto metallicKind = metallicFromProps (child.getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // For metallic components, let the button handle its own rendering
            // We need to call the button's drawButtonBackground method
            if (auto* button = dynamic_cast<juce::Button*>(&child))
            {
                // Get the LookAndFeel and call drawButtonBackground
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                {
                    // Save the current graphics state
                    juce::Graphics::ScopedSaveState saveState(g);
                    
                    // Adjust the graphics context to match the cell's bounds
                    // The button should fill the cell's area (minus the reduced margin)
                    auto cellBounds = getLocalBounds().reduced(3);
                    g.setOrigin(cellBounds.getPosition());
                    
                    // Temporarily set the button's bounds to match the cell
                    auto originalBounds = button->getBounds();
                    button->setBounds(cellBounds);
                    
                    // Call the button's drawButtonBackground method
                    lf->drawButtonBackground(g, *button, juce::Colour(), false, false);
                    
                    // Restore the button's original bounds
                    button->setBounds(originalBounds);
                }
            }
            return;
        }
        
        // Non-metallic components use the standard SimpleSwitchCell rendering
        if (delayTheme)
        {
            panel  = panel.brighter (0.10f);
            border = lf ? lf->theme.text : border;
        }
        g.setColour (panel);
        g.fillRoundedRectangle (r, rad);
        if (showBorder)
        {
            g.setColour (reverbMaroon ? juce::Colour (0xFF8E3A2F) : border);
            g.drawRoundedRectangle (r, rad, 1.5f);
        }
    }

private:
    juce::Component& child;
    juce::Label caption;
    juce::String captionText;
    bool showBorder { true };
    bool reverbMaroon { false };
    bool delayTheme { false };
};


