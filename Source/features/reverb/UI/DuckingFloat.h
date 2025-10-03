#pragma once
#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"

/*
====================================================================================================
 DuckingFloat
 ---------------------------------------------------------------------------------------------------
 Purpose
    Floating "ducking" tool used in Reverb UI — always-expanded module with:
      - Top GR (gain-reduction) meter (painted, not a Slider)
      - Two compact selectors (Mode, Detector)
      - 8 rotary controls displayed via KnobCell (Depth / Threshold / Ratio / Knee / Attack / Release /
        Focus Freq / Focus Q)

 Wiring
    All knobs/combos are bound to APVTS parameters declared in ReverbParamIDs.h:
      - duckMode (choice)
      - duckDetectorSrc (choice)
      - duckDepthDb, duckThrDb, duckRatio, duckKneeDb, duckAtkMs, duckRelMs (floats)
      - duckBandHz, duckBandQ (floats)
    NOTE: duckOn can be used by the parent to grey out this module via setActive(false).

 Look & Feel
    Uses FieldLNF theme via getLookAndFeel(). Component respects theme changes (lookAndFeelChanged()).

 Lifetime
    - No timers here, repaint is driven by value attachments (from APVTS) and external calls to updateGrMeter().
    - APVTS attachments auto-destroy in destructor.

 Integration Tips
    1) Create it with your processor's apvts.
    2) Call updateGrMeter(dB) from your ducking processor code — it will redraw the top meter.
    3) Use setActive()/setGreyedOut() to visually disable the module when ducking is off (duckOn == false).

 Notes
    - This module is "always expanded" by design. expand/collapse code paths are left in place in case
      you later add a toggle, but expanded==true is the default.
    - The meter is fully custom (no JUCE Slider). Scale: 0…-20 dB on a horizontal track.
====================================================================================================
*/

class DuckingFloat : public juce::Component
{
public:
    explicit DuckingFloat(juce::AudioProcessorValueTreeState& apvts);
    ~DuckingFloat() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;

    // State management
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded; }

    // GR meter update (call from your DSP/UI thread-safe bridge as needed)
    void updateGrMeter(float grDb);

    // Visibility control
    void setVisible(bool shouldBeVisible) override;

    // Visual "disabled" states (parent should drive these)
    void setActive(bool active);
    bool isActive() const { return active; }
    void setGreyedOut(bool greyedOut);
    bool isGreyedOut() const { return greyedOut; }

    // LNF passthrough (ensures children pick up external LNF swaps)
    void setLookAndFeel(juce::LookAndFeel* newLookAndFeel);

private:
    // Setup / layout / paint
    void setupComponents();
    void updateLayout();
    void paintCollapsed(juce::Graphics& g);
    void paintExpanded(juce::Graphics& g);
    void paintGrMeter(juce::Graphics& g, juce::Rectangle<float> bounds);

    // UI primitives
    juce::Label grLabel;
    juce::Slider grMeter; // (not used visually; left as legacy member, drawing is custom)

    // Selectors
    juce::ComboBox modeSelector;
    juce::ComboBox detectorSelector;

    // Ducking controls
    juce::Slider depthSlider, thresholdSlider, ratioSlider, kneeSlider;
    juce::Slider attackSlider, releaseSlider;
    juce::Slider bandFreqSlider, bandQSlider;

    // Labels (kept for completeness; hidden in this design)
    juce::Label depthLabel, thresholdLabel, ratioLabel, kneeLabel;
    juce::Label attackLabel, releaseLabel;
    juce::Label bandFreqLabel, bandQLabel;
    juce::Label modeLabel, detectorLabel;

    // Value labels for KnobCells
    juce::Label depthValue, thresholdValue, ratioValue, kneeValue;
    juce::Label attackValue, releaseValue, bandFreqValue, bandQValue;

    // KnobCells (render numeric formatted labels inside the knobs)
    std::unique_ptr<KnobCell> depthKnobCell, thresholdKnobCell, ratioKnobCell, kneeKnobCell;
    std::unique_ptr<KnobCell> attackKnobCell, releaseKnobCell, bandFreqKnobCell, bandQKnobCell;

    // APVTS binding (real wiring happens in setupComponents)
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> detectorAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> depthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> thresholdAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ratioAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> kneeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attackAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> releaseAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bandQAttachment;

    // Backing APVTS (non-owning)
    juce::AudioProcessorValueTreeState& apvtsRef;

    // State
    bool expanded  = true;   // always expanded by default
    bool active    = true;
    bool greyedOut = false;
    float currentGrDb = 0.0f;
    juce::Rectangle<float> grMeterBounds;

    // Constants
    static constexpr float COLLAPSED_HEIGHT   = 40.0f;
    static constexpr float EXPANDED_HEIGHT    = 200.0f;
    static constexpr float PILL_CORNER_RADIUS = 20.0f;
    static constexpr float GR_METER_HEIGHT    = 22.0f;
};