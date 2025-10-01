#pragma once

#include <JuceHeader.h>
#include "ReverbGraphics.h"
#include "ReverbControlsPane.h"
#include "shared/ui/Controls/ControlGridMetrics.h"
#include "shared/Core/PluginProcessor.h"

// DEBUG: Test container with yellow border
class TestContainer : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Fill with bright yellow
        g.setColour(juce::Colour(0xFFFFFF00));
        g.fillRoundedRectangle(bounds, 8.0f);
        
        // Add red border
        g.setColour(juce::Colour(0xFFFF0000));
        g.drawRoundedRectangle(bounds, 8.0f, 3.0f);
        
        // Add text
        g.setColour(juce::Colour(0xFF000000));
        g.setFont(16.0f);
        g.drawText("TEST CONTAINER", bounds, juce::Justification::centred);
    }
};

// Composite Reverb tab: canvas + 2x16 grid controls
class ReverbTab : public juce::Component
{
public:
    explicit ReverbTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        // DEBUG: Log ReverbTab creation
        DBG("🔧 ReverbTab constructor called - creating ReverbTab");
        
        // Visuals-only Reverb pane (existing component)
        reverbPanel = std::make_unique<ReverbGraphics>(p, p.apvts,
            [&p]{ return p.getReverbErRms(); },
            [&p]{ return p.getReverbTailRms(); },
            [&p]{ return p.getReverbDuckGrDb(); },
            [&p]{ return p.getReverbWidthNow(); });
        addAndMakeVisible (*reverbPanel);
        DBG("🔧 ReverbGraphics created and added to ReverbTab");

        // Controls (2x16 grid)
        controls = std::make_unique<ReverbControlsPane>(p.apvts);
        controls->setVisible (true);
        addAndMakeVisible (*controls);
        DBG("🔧 ReverbControlsPane created and added to ReverbTab");
    }
    
    ~ReverbTab() override
    {
        // Clean destructor - no logging needed
        controls.reset();
        reverbPanel.reset();
    }

    void setControlsVisible (bool on)
    {
        if (controls)
            controls->setVisible (on);
        resized();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        DBG("🔧 ReverbTab::resized() called - bounds: " << r.toString());
        
        auto m = ControlGridMetrics::compute (r.getWidth(), r.getHeight());
        if (controls) { controls->setCellMetrics (m.knobPx, m.valuePx, m.labelGapPx, m.colW); controls->setRowHeightPx (m.rowH); }
        auto controlsArea = r.removeFromBottom (m.controlsH);
        
        if (reverbPanel) reverbPanel->setBounds (r);
        if (controls && controls->isVisible()) controls->setBounds (controlsArea);
    }
    
    void lookAndFeelChanged() override
    {
        // Forward theme changes to ReverbGraphics
        if (reverbPanel) {
            reverbPanel->lookAndFeelChanged();
        }
    }
    
    // Public getter for graphics container
    ReverbGraphics* getReverbCanvas() const { return reverbPanel.get(); }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<ReverbGraphics> reverbPanel;
    std::unique_ptr<ReverbControlsPane> controls;
};


