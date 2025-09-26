#pragma once
#include <JuceHeader.h>
#include "../Core/PluginProcessor.h"
#include "../Core/FieldLookAndFeel.h"
#include "ControlGridMetrics.h"
#include "Components/KnobCell.h"
#include "SimpleSwitchCell.h"

class PhaseTab : public juce::Component
{
public:
    PhaseTab (MyPluginAudioProcessor& p, juce::LookAndFeel* lnf)
        : proc (p)
    {
        setLookAndFeel (lnf);
        buildControls();
        applyMetricsToAll();
    }
    
    ~PhaseTab() override
    {
        setLookAndFeel (nullptr);
    }
    
    void resized() override;
    void paint (juce::Graphics& g) override;

private:
    void styleKnob (juce::Slider& k);
    void makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid);
    void makeComboCell (juce::ComboBox& c, const juce::String& cap, const char* pid);
    void makeSwitchCell (juce::ToggleButton& t, const juce::String& cap, const char* pid);
    void buildControls();
    void applyMetricsToAll();

    MyPluginAudioProcessor& proc;
    
    // Control components
    juce::ComboBox refSourceCombo, channelModeCombo, captureCombo, alignModeCombo, alignGoalCombo;
    juce::ComboBox unitsCombo, linkCombo, engineCombo, resetCombo;
    juce::ComboBox firLengthCombo, dynamicPhaseCombo, monitorCombo, metricCombo, auditionBlendCombo;
    juce::ToggleButton followXOSwitch, polarityASwitch, polarityBSwitch, commitSwitch;
    juce::ToggleButton phaseRecSwitch, applyOnLoadSwitch;
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
    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PhaseTab)
};