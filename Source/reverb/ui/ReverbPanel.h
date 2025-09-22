#pragma once
#include <JuceHeader.h>
#include "../../KnobCell.h"
#include "DecayCurveComponent.h"
#include "ReverbEQComponent.h"
#include "ReverbScopeComponent.h"
#include "ReverbCanvasComponent.h"

class ReverbPanel : public juce::Component
{
public:
    ReverbPanel (juce::AudioProcessorValueTreeState& s,
                 std::function<float()> getEr,
                 std::function<float()> getTail,
                 std::function<float()> getDuckDb,
                 std::function<float()> getWidthNow);

    void resized() override;

private:
    juce::AudioProcessorValueTreeState& state;
    // Minimal top bar
    juce::ToggleButton enableBtn, wetOnlyBtn; juce::ComboBox algoBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableA, wetOnlyA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoA;

    // Small visuals column
    std::unique_ptr<ReverbCanvasComponent> canvas; // top-wide visualization
    std::unique_ptr<DecayCurveComponent>   decayCurve; // optional keep
    std::unique_ptr<ReverbEQComponent>     eqGraph;    // optional keep
    std::unique_ptr<ReverbScopeComponent>  scope;      // optional keep

    // Core 5×4 grid controls (sliders + value labels)
    juce::Slider preDelay, erLvl, erTime, erDens, erWidth;
    juce::Slider decay, dens, diff, modDepth, modRate;
    juce::Slider hpf, lpf, tilt, postEqMix, erToTail;
    juce::Slider dreqL, dreqM, dreqH, width, wet;
    juce::Label  preDelayV, erLvlV, erTimeV, erDensV, erWidthV,
                 decayV, densV, diffV, modDepthV, modRateV,
                 hpfV, lpfV, tiltV, postEqMixV, erToTailV,
                 dreqLV, dreqMV, dreqHV, widthV, wetV;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayA, erLvlA, erTimeA, erDensA, erWidthA,
        decayA, densA, diffA, modDepthA, modRateA,
        hpfA, lpfA, tiltA, postEqMixA, erToTailA,
        dreqLA, dreqMA, dreqHA, widthA, wetA;

    // Ducking strip (always visible)
    juce::ComboBox duckMode;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> duckModeA;
    juce::Slider duckDepth, duckThr, duckKnee, duckRatio, duckAtk, duckRel, duckLa, duckRms, duckBandHz, duckBandQ;
    juce::Label  duckDepthV, duckThrV, duckKneeV, duckRatioV, duckAtkV, duckRelV, duckLaV, duckRmsV, duckBandHzV, duckBandQV;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> duckAtts;
};


