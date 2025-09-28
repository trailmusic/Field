#pragma once

#include <JuceHeader.h>
#include "../Core/FieldLookAndFeel.h"
#include "../Core/FieldMetallic.h"

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
    
    /// Set sizing metrics to match KnobCell interface
    void setMetrics (int knobPx, int valuePx, int gapPx)
    {
        // Store metrics for consistent sizing with KnobCell
        K = juce::jmax (16, knobPx);
        V = juce::jmax (0,  valuePx);
        G = juce::jmax (0,  gapPx);
        resized();
        repaint();
    }

    void resized() override
    {
        const int capH = captionText.isNotEmpty() ? V : 0; // Use V (value label height) for caption
        
        // Check if child has metallic properties - show caption for ComboBoxes
        auto metallicKind = metallicFromProps (child.getProperties());
        if (metallicKind != MetallicKind::None)
        {
            // For ComboBoxes, show caption to maintain title and button window format
            if (auto* combo = dynamic_cast<juce::ComboBox*>(&child))
            {
                // Use full cell area for metallic ComboBoxes (no padding)
                auto b = getLocalBounds(); // Use full cell area, no padding
                
                if (capH > 0)
                {
                    caption.setVisible (true);
                    caption.setBounds (b.removeFromTop (capH));
                    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                        caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
                    // Ensure caption is always on top for metallic ComboBoxes
                    caption.toFront (false);
                }
                else
                {
                    caption.setVisible (false);
                }
                // Give ComboBox the remaining space after caption
                child.setBounds (b);
                return;
            }
            else
            {
                // Hide caption for other metallic components (buttons) to avoid double labels
                caption.setVisible (false);
                child.setBounds (getLocalBounds()); // Use full cell area for metallic buttons too
                
                // LookAndFeel assignment is now handled at the source (in control panes)
                return;
            }
        }
        
        // Non-metallic components use standard padding
        auto b = getLocalBounds().reduced(4); // Match KnobCell padding exactly (4px on all sides)
        
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
                
                // LookAndFeel assignment is now handled in resized() method
                
                // Let JUCE handle the complete button rendering (background + text)
                // The FieldLookAndFeel::drawButtonBackground will handle the metallic background
                // and JUCE will automatically call drawButtonText for the text
                // Don't draw our own background - let the button render itself
                return; // Let the button handle its own rendering
            }
            else if (auto* combo = dynamic_cast<juce::ComboBox*>(&child))
            {
                // Handle metallic ComboBoxes - show title and button window
                // For ComboBoxes, we need to draw the frame on top after the ComboBox renders
                auto cellBounds = getLocalBounds().reduced(3);
                combo->setBounds(cellBounds);
                
                // Let ComboBox handle its own metallic rendering and background first
                // Then draw the frame on top to ensure it's visible
                return; // ComboBox will render first, then we'll draw frame on top
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
        
        // For metallic ComboBoxes, draw the frame on top after ComboBox renders
        if (metallicKind != MetallicKind::None && dynamic_cast<juce::ComboBox*>(&child))
        {
            if (showBorder)
            {
                g.setColour (reverbMaroon ? juce::Colour (0xFF8E3A2F) : border);
                g.drawRoundedRectangle (r, rad, 1.5f);
            }
            
            // Caption positioning is handled in resized() method
        }
    }

private:
    juce::Component& child;
    juce::Label caption;
    juce::String captionText;
    bool showBorder { true };
    bool reverbMaroon { false };
    bool delayTheme { false };
    
    // Layout metrics to match KnobCell sizing system
    int K = 88;   // knob diameter (matches KnobCell default)
    int V = 14;   // value label band height
    int G = 4;    // gap between elements
};


