#pragma once
#include <JuceHeader.h>
#include "../../Core/PluginProcessor.h"
#include "../../Core/FieldLookAndFeel.h"

//==============================================================================
// CorrelationMeter - Displays stereo correlation with positive/negative visualization
//==============================================================================
class CorrelationMeter : public juce::Component, public juce::Timer
{
public:
    ~CorrelationMeter() override { stopTimer(); }
    
    CorrelationMeter (MyPluginAudioProcessor& p, FieldLNF& l);
    
    void paint (juce::Graphics& g) override;
    void timerCallback() override;
    void visibilityChanged() override;
    
private:
    MyPluginAudioProcessor& proc;
    FieldLNF& lnf;
};
