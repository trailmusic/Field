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
        auto b = getLocalBounds().reduced (3); // Reduced from 6 to 3 for better sizing
        const int capH = captionText.isNotEmpty() ? 14 : 0;
        
        // Check if child has metallic properties - if so, hide caption
        auto metallicKind = metallicFromProps (child.getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // Hide caption for metallic components to avoid double labels
            caption.setVisible (false);
            child.setBounds (b);
            return;
        }
        
        if (capH > 0)
        {
            caption.setVisible (true);
            caption.setBounds (b.removeFromTop (capH));
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
        }
        else
        {
            caption.setVisible (false);
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
            // For metallic components, let them handle their own rendering
            if (auto* button = dynamic_cast<juce::Button*>(&child))
            {
                // Handle metallic buttons - let JUCE handle the complete rendering
                // We just need to ensure they have the right bounds
                auto cellBounds = getLocalBounds().reduced(3);
                button->setBounds(cellBounds);
                
                // Let JUCE handle the complete button rendering (background + text)
                // The FieldLookAndFeel::drawButtonBackground will handle the metallic background
                // and JUCE will automatically call drawButtonText for the text
                return; // Don't draw our own background for metallic buttons
            }
            else if (auto* combo = dynamic_cast<juce::ComboBox*>(&child))
            {
                // Handle metallic ComboBoxes - let them render normally
                // ComboBoxes have their own rendering in FieldLookAndFeel::drawComboBox
                // We just need to ensure they have the right bounds
                auto cellBounds = getLocalBounds().reduced(3);
                combo->setBounds(cellBounds);
                
                // Let the ComboBox render itself with its metallic properties
                // The FieldLookAndFeel::drawComboBox will handle the metallic rendering
                return; // Don't draw our own background for ComboBoxes
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


