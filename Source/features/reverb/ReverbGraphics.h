#pragma once
#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"
#include "ReverbCanvasComponent.h"
#include "ReverbDynEQPane.h"

class ReverbGraphics : public juce::Component
{
public:
    ReverbGraphics (juce::AudioProcessorValueTreeState& s,
                 std::function<float()> getEr,
                 std::function<float()> getTail,
                 std::function<float()> getDuckDb,
                 std::function<float()> getWidthNow);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::AudioProcessorValueTreeState& state;
    // Minimal top bar
    juce::ToggleButton enableBtn, wetOnlyBtn; juce::ComboBox algoBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> enableA, wetOnlyA;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algoA;

    // Top-wide visualization
    std::unique_ptr<ReverbCanvasComponent> canvas;
    // DynEQ editor now handled by ReverbTab

public:

    // Old knob-based EQ controls removed - now handled by DynEQ pane

    // Ducking strip (always visible)
    juce::ComboBox duckMode;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> duckModeA;
    juce::Slider duckDepth, duckThr, duckKnee, duckRatio, duckAtk, duckRel, duckLa, duckRms, duckBandHz, duckBandQ;
    juce::Label  duckDepthV, duckThrV, duckKneeV, duckRatioV, duckAtkV, duckRelV, duckLaV, duckRmsV, duckBandHzV, duckBandQV;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> duckAtts;
};


