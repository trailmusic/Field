#pragma once

#include <JuceHeader.h>
#include "../Panes/ImagerPane.h"
#include "../Panes/ImagerControlsPane.h"
#include "../Controls/ControlGridMetrics.h"

class MyPluginAudioProcessor; // fwd

// Composite Imager tab: Imager visuals + 2x16 controls grid
class ImagerTab : public juce::Component
{
public:
    explicit ImagerTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p)
    {
        // CRITICAL: Set our own LookAndFeel first
        setLookAndFeel(lnf);
        
        visuals = std::make_unique<ImagerPane>();
        addAndMakeVisible (*visuals);
        
        // CRITICAL: Pass the LookAndFeel to the ImagerPane
        visuals->setLookAndFeel(lnf);
    }
    // Mirror legacy callbacks so PaneManager integrations continue to work
public:
    std::function<void(const juce::String&, const juce::var&)> onUiChange;
    std::function<void(const juce::String& paramID, float value)> onParamEdit;

    void setOptions (const ImagerPane::Options& o)
    {
        if (visuals) visuals->setOptions (o);
    }
    
    // Public getter for graphics container
    ImagerPane* getImagerPane() const { return visuals.get(); }


    void paint(juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
        auto panel = lf ? lf->theme.meters.panelDark : juce::Colour(0xFF2A2C30);
        auto sh = lf ? lf->theme.sh : juce::Colour(0xFF2A2A2A);
        
        // AB Button styling: Solid panel background with elevation shadow
        const float cr = 8.0f; // Match KnobCell corner radius
        
        // Elevation shadow first (AB button style)
        if (lf) g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
        else g.setColour(juce::Colour(0x40000000));
        g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
        
        // Solid panel background (no aliasing)
        g.setColour(panel);
        g.fillRoundedRectangle(r, cr);
        
        // Border (AB button style)
        g.setColour(sh);
        g.drawRoundedRectangle(r, cr, 1.0f);
        
        // Add 10px top and bottom padding for content
        auto contentR = r.reduced(0, 10.0f);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        // Add 10px top and bottom padding for content
        auto contentR = r.reduced(0, 10);
        // Imager is visuals-only now (no 2x16 controls)
        if (visuals) visuals->setBounds (contentR);
    }

private:
    MyPluginAudioProcessor& proc;
    std::unique_ptr<ImagerPane> visuals;
};


