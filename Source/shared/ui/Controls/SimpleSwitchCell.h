#pragma once

#include <JuceHeader.h>
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldMetallic.h"
#include "shared/Core/IconSystem.h"

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
        
        // Auto-detect and set appropriate icons for ToggleButtons
        if (auto* button = dynamic_cast<juce::ToggleButton*>(&child))
        {
            auto buttonName = button->getName().toLowerCase();
            auto buttonText = button->getButtonText().toLowerCase();
            
            // Set icons based on button name or text
            if (buttonName.contains("enable") || buttonName.contains("on") || buttonText.contains("enable"))
                button->getProperties().set("iconType", (int)IconSystem::Power);
            else if (buttonName.contains("wet") || buttonName.contains("wetonly") || buttonText.contains("wet"))
                button->getProperties().set("iconType", (int)IconSystem::Speaker);
            else if (buttonName.contains("freeze") || buttonText.contains("freeze"))
                button->getProperties().set("iconType", (int)IconSystem::Snowflake);
            else if (buttonName.contains("duck") || buttonText.contains("duck"))
                button->getProperties().set("iconType", (int)IconSystem::Duck);
            else if (buttonName.contains("learn") || buttonText.contains("learn"))
                button->getProperties().set("iconType", (int)IconSystem::Learn);
            else if (buttonName.contains("bypass") || buttonText.contains("bypass"))
                button->getProperties().set("iconType", (int)IconSystem::Bypass);
            else if (buttonName.contains("lock") || buttonText.contains("lock"))
                button->getProperties().set("iconType", (int)IconSystem::Lock);
            else if (buttonName.contains("link") || buttonText.contains("link"))
                button->getProperties().set("iconType", (int)IconSystem::Link);
            else if (buttonName.contains("auto") || buttonText.contains("auto"))
                button->getProperties().set("iconType", (int)IconSystem::Auto);
            else if (buttonName.contains("manual") || buttonText.contains("manual"))
                button->getProperties().set("iconType", (int)IconSystem::Manual);
            else if (buttonName.contains("reset") || buttonText.contains("reset"))
                button->getProperties().set("iconType", (int)IconSystem::Reset);
            else if (buttonName.contains("save") || buttonText.contains("save"))
                button->getProperties().set("iconType", (int)IconSystem::Save);
            else if (buttonName.contains("stop") || buttonText.contains("stop"))
                button->getProperties().set("iconType", (int)IconSystem::Stop);
            else if (buttonName.contains("show") || buttonText.contains("show"))
                button->getProperties().set("iconType", (int)IconSystem::Show);
            else if (buttonName.contains("audition") || buttonText.contains("audition"))
                button->getProperties().set("iconType", (int)IconSystem::Audition);
            else if (buttonName.contains("dynamic") || buttonText.contains("dynamic"))
                button->getProperties().set("iconType", (int)IconSystem::Dynamic);
            else if (buttonName.contains("spectral") || buttonText.contains("spectral"))
                button->getProperties().set("iconType", (int)IconSystem::Spectral);
            else if (buttonName.contains("xy") || buttonText.contains("xy"))
                button->getProperties().set("iconType", (int)IconSystem::XY);
            else if (buttonName.contains("polar") || buttonText.contains("polar"))
                button->getProperties().set("iconType", (int)IconSystem::Polar);
            else if (buttonName.contains("heat") || buttonText.contains("heat"))
                button->getProperties().set("iconType", (int)IconSystem::Heat);
            else if (buttonName.contains("mono") || buttonText.contains("mono"))
                button->getProperties().set("iconType", (int)IconSystem::Mono);
            else if (buttonName.contains("stereo") || buttonText.contains("stereo"))
                button->getProperties().set("iconType", (int)IconSystem::Stereo);
            else if (buttonName.contains("split") || buttonText.contains("split"))
                button->getProperties().set("iconType", (int)IconSystem::Split);
            else if (buttonName.contains("mix") || buttonText.contains("mix"))
                button->getProperties().set("iconType", (int)IconSystem::Mix);
            else if (buttonName.contains("width") || buttonText.contains("width"))
                button->getProperties().set("iconType", (int)IconSystem::Width);
            else if (buttonName.contains("pan") || buttonText.contains("pan"))
                button->getProperties().set("iconType", (int)IconSystem::Pan);
            else if (buttonName.contains("space") || buttonText.contains("space"))
                button->getProperties().set("iconType", (int)IconSystem::Space);
            else if (buttonName.contains("tilt") || buttonText.contains("tilt"))
                button->getProperties().set("iconType", (int)IconSystem::Tilt);
            else if (buttonName.contains("drive") || buttonText.contains("drive"))
                button->getProperties().set("iconType", (int)IconSystem::Drive);
            else if (buttonName.contains("air") || buttonText.contains("air"))
                button->getProperties().set("iconType", (int)IconSystem::Air);
            else if (buttonName.contains("snap") || buttonText.contains("snap"))
                button->getProperties().set("iconType", (int)IconSystem::Snap);
            else if (buttonName.contains("options") || buttonText.contains("options"))
                button->getProperties().set("iconType", (int)IconSystem::Options);
            else if (buttonName.contains("help") || buttonText.contains("help"))
                button->getProperties().set("iconType", (int)IconSystem::Help);
            else if (buttonName.contains("question") || buttonText.contains("question"))
                button->getProperties().set("iconType", (int)IconSystem::QuestionMark);
            else if (buttonName.contains("lightbulb") || buttonText.contains("lightbulb"))
                button->getProperties().set("iconType", (int)IconSystem::Lightbulb);
            else if (buttonName.contains("x") || buttonText.contains("x"))
                button->getProperties().set("iconType", (int)IconSystem::X);
            else if (buttonName.contains("note") || buttonText.contains("note"))
                button->getProperties().set("iconType", (int)IconSystem::Note);
            else if (buttonName.contains("droplet") || buttonText.contains("droplet"))
                button->getProperties().set("iconType", (int)IconSystem::Droplet);
            else if (buttonName.contains("delta") || buttonText.contains("delta"))
                button->getProperties().set("iconType", (int)IconSystem::Delta);
            else if (buttonName.contains("zoom") || buttonText.contains("zoom"))
                button->getProperties().set("iconType", (int)IconSystem::ZoomIn);
            else if (buttonName.contains("fullscreen") || buttonText.contains("fullscreen"))
                button->getProperties().set("iconType", (int)IconSystem::FullScreen);
            else if (buttonName.contains("color") || buttonText.contains("color"))
                button->getProperties().set("iconType", (int)IconSystem::ColorPalette);
            else if (buttonName.contains("cog") || buttonName.contains("gear") || buttonText.contains("cog"))
                button->getProperties().set("iconType", (int)IconSystem::CogWheel);
            else if (buttonName.contains("anchor") || buttonText.contains("anchor"))
                button->getProperties().set("iconType", (int)IconSystem::Anchor);
            else if (buttonName.contains("retrig") || buttonText.contains("retrig"))
                button->getProperties().set("iconType", (int)IconSystem::Retrig);
            else if (buttonName.contains("hp") || buttonText.contains("hp"))
                button->getProperties().set("iconType", (int)IconSystem::HP);
            else if (buttonName.contains("lp") || buttonText.contains("lp"))
                button->getProperties().set("iconType", (int)IconSystem::LP);
            else if (buttonName.contains("commit") || buttonText.contains("commit"))
                button->getProperties().set("iconType", (int)IconSystem::Save);
            else if (buttonName.contains("phase") || buttonName.contains("rec") || buttonText.contains("phase") || buttonText.contains("rec"))
                button->getProperties().set("iconType", (int)IconSystem::Retrig);
            else if (buttonName.contains("apply") || buttonName.contains("load") || buttonText.contains("apply") || buttonText.contains("load"))
                button->getProperties().set("iconType", (int)IconSystem::LeftArrow);
            else if (buttonName.contains("random") || buttonName.contains("seed") || buttonText.contains("random") || buttonText.contains("seed"))
                button->getProperties().set("iconType", (int)IconSystem::Delta);
            else if (buttonName.contains("follow") || buttonText.contains("follow"))
                button->getProperties().set("iconType", (int)IconSystem::Link);
            else if (buttonName.contains("sync") || buttonText.contains("sync"))
                button->getProperties().set("iconType", (int)IconSystem::Link);
            else if (buttonName.contains("wet") || buttonName.contains("wetonly") || buttonText.contains("wet") || buttonText.contains("wet only"))
                button->getProperties().set("iconType", (int)IconSystem::Speaker);
        }
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
                // For metallic buttons, show caption above the button
                auto b = getLocalBounds(); // Use full cell area for metallic buttons
                
                if (capH > 0)
                {
                    caption.setVisible (true);
                    caption.setBounds (b.removeFromTop (capH));
                    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                        caption.setColour (juce::Label::textColourId, lf->theme.textMuted);
                    // Ensure caption is always on top for metallic buttons
                    caption.toFront (false);
                }
                else
                {
                    caption.setVisible (false);
                }
                
                // Give button the remaining space after caption
                child.setBounds (b);
                
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


