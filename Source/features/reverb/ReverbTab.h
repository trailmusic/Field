#pragma once

#include <JuceHeader.h>
#include "ReverbGraphics.h"
#include "ReverbControlsPane.h"
#include "shared/ui/Controls/ControlGridMetrics.h"
#include "shared/Core/PluginProcessor.h"

// Composite Reverb tab: canvas + 2x16 grid controls
class ReverbTab : public juce::Component
{
public:
    explicit ReverbTab (MyPluginAudioProcessor& p)
        : proc (p)
    {
        // Visuals-only Reverb pane (existing component)
        reverbPanel = std::make_unique<ReverbGraphics>(p, p.apvts,
            [&p]{ return p.getReverbErRms(); },
            [&p]{ return p.getReverbTailRms(); },
            [&p]{ return p.getReverbDuckGrDb(); },
            [&p]{ return p.getReverbWidthNow(); });
        addAndMakeVisible (*reverbPanel);

        // Controls (2x16 grid)
        controls = std::make_unique<ReverbControlsPane>(p.apvts);
        controls->setVisible (true);
        addAndMakeVisible (*controls);
    }
    
    ~ReverbTab() override
    {
        // Add crash logging for debugging
        juce::File f = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("Field_CrashLog.txt");
        f.appendText("ReverbTab Destructor: STARTED\n", false, false, "\n");
        
        // Destroy controls first
        controls.reset();
        f.appendText("ReverbTab Destructor: Controls destroyed\n", false, false, "\n");
        
        // Destroy reverb panel
        reverbPanel.reset();
        f.appendText("ReverbTab Destructor: ReverbPanel destroyed\n", false, false, "\n");
        
        f.appendText("ReverbTab Destructor: COMPLETE\n", false, false, "\n");
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


