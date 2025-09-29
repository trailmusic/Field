#pragma once
#include <JuceHeader.h>
#include "Core/PluginProcessor.h"
#include "Core/FieldLookAndFeel.h"
#include "Core/FieldMetallic.h"
#include "../Controls/ControlGridMetrics.h"
#include "../Components/KnobCell.h"
#include "../Components/ButtonSwitch.h"
#include "../Controls/SimpleSwitchCell.h"

// Phase Visual Container - placeholder for phase alignment visuals
class PhaseVisualContainer : public juce::Component
{
public:
    PhaseVisualContainer() = default;
    
    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat();
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
        {
            // AB Button styling: Solid panel background with elevation shadow
            const float cr = 8.0f; // Match KnobCell corner radius
            
            // Elevation shadow first (AB button style)
            g.setColour(lf->theme.shadowDark.withAlpha(0.25f));
            g.fillRoundedRectangle(r.translated(1.5f, 1.5f), cr);
            
            // Solid panel background (no aliasing)
            g.setColour(lf->theme.meters.panelDark);
            g.fillRoundedRectangle(r, cr);
            
            // Border (AB button style)
            g.setColour(lf->theme.sh);
            g.drawRoundedRectangle(r, cr, 1.0f);
            
            // Add 10px top and bottom padding for content
            auto contentR = r.reduced(0, 10.0f);
            
            // Placeholder text
            g.setColour (lf->theme.textMuted);
            g.setFont (14.0f);
            g.drawText ("Phase Alignment Visuals", contentR, juce::Justification::centred);
        }
        else
        {
            g.fillAll (juce::Colours::darkgrey);
            g.setColour (juce::Colours::white.withAlpha (0.3f));
            g.drawRoundedRectangle (r, 8.0f, 1.0f);
            g.setColour (juce::Colours::white);
            g.setFont (14.0f);
            g.drawText ("Phase Alignment Visuals", r, juce::Justification::centred);
        }
    }
};

class PhaseTab : public juce::Component
{
public:
    PhaseTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p)
    {
        setLookAndFeel (lnf);
        
        // Create phase visual container (placeholder for now)
        phaseVisualContainer = std::make_unique<PhaseVisualContainer>();
        addAndMakeVisible (*phaseVisualContainer);
        
        buildControls();
        applyMetricsToAll();
    }
    
    // Public getter for graphics container
    PhaseVisualContainer* getPhaseVisualContainer() const { return phaseVisualContainer.get(); }
    
    ~PhaseTab() override
    {
        setLookAndFeel (nullptr);
    }
    
    void resized() override;

private:
    void styleKnob (juce::Slider& k);
    void makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid);
    void makeComboCell (juce::ComboBox& c, const juce::String& cap, const char* pid);
    void makeSwitchCell (ButtonSwitch& t, const juce::String& cap, const char* pid);
    void buildControls();
    void applyMetricsToAll();

    MyPluginAudioProcessor& proc;
    
    // Visual container (same size as Band tab)
    std::unique_ptr<PhaseVisualContainer> phaseVisualContainer;
    
    // Control components
    juce::ComboBox refSourceCombo, channelModeCombo, captureCombo, alignModeCombo, alignGoalCombo;
    juce::ComboBox unitsCombo, linkCombo, engineCombo, resetCombo;
    juce::ComboBox firLengthCombo, dynamicPhaseCombo, monitorCombo, metricCombo, auditionBlendCombo;
    ButtonSwitch followXOSwitch, polarityASwitch, polarityBSwitch, commitSwitch;
    ButtonSwitch phaseRecSwitch, applyOnLoadSwitch;
    juce::Slider delayCoarseKnob, delayFineKnob, latencyKnob;
    juce::Slider xoLowKnob, xoHighKnob, lowAPKnob, lowQKnob, midAPKnob, midQKnob;
    juce::Slider highAPKnob, highQKnob, trimKnob;
    juce::Label delayCoarseLabel, delayFineLabel, latencyLabel;
    juce::Label xoLowLabel, xoHighLabel, lowAPLabel, lowQLabel, midAPLabel, midQLabel;
    juce::Label highAPLabel, highQLabel, trimLabel;
    
    // Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> comboAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> buttonAtts;
    
    // Control grid system
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<SimpleSwitchCell*> switchCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>> ownedSwitches;
    std::vector<juce::Component*> gridOrder;
    int knobPx = 52, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0; // Increased knob size from 50 to 52

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseTab)
};