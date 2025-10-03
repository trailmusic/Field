#pragma once
#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"

/*
====================================================================================================
 DecayRateFloat
 ---------------------------------------------------------------------------------------------------
 Purpose
    Floating "decay-rate" tool used in Reverb UI — always-expanded module with:
      - 8 rotary controls for decay-rate parameters (Lo Mult / Hi Mult / Mid Db / Mid Freq / Mid Q / Tilt Db / Smoothing / Mode)
      - No GR meter (unlike DuckingFloat)
      - No selectors (unlike DuckingFloat)
      - Always visible (no greyed out state)

 Wiring
    All knobs are bound to APVTS parameters declared in ReverbParamIDs.h:
      - decayLoMult, decayHiMult, decayMidDb, decayMidFreqHz, decayMidQ (floats)
      - decayTiltDb, decaySmoothing, decayMode (floats)

 Look & Feel
    Uses FieldLNF theme via getLookAndFeel(). Component respects theme changes (lookAndFeelChanged()).

 Lifetime
    - No timers here, repaint is driven by value attachments (from APVTS).
    - APVTS attachments auto-destroy in destructor.

 Integration Tips
    1) Create it with your processor's apvts.
    2) Use setActive() to visually enable/disable the module.
    3) No greyed out state needed - visibility controlled by parent.

 Notes
    - This module is "always expanded" by design.
    - Matches DuckingFloat styling but without GR meter or selectors.
    - Uses same compact padding system as DuckingFloat.
====================================================================================================
*/

class DecayRateFloat : public juce::Component
{
public:
    explicit DecayRateFloat(juce::AudioProcessorValueTreeState& apvts);
    ~DecayRateFloat() override;

    void resized() override;
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;

    // State management
    void setExpanded(bool expanded);
    bool isExpanded() const { return expanded; }

    // Visibility control
    void setVisible(bool shouldBeVisible) override;

    // Visual "disabled" states (parent should drive these)
    void setActive(bool active);
    bool isActive() const { return active; }

    // LNF passthrough (ensures children pick up external LNF swaps)
    void setLookAndFeel(juce::LookAndFeel* newLookAndFeel);

private:
    // Setup / layout / paint
    void setupComponents();
    void updateLayout();
    void paintCollapsed(juce::Graphics& g);
    void paintExpanded(juce::Graphics& g);

    // Decay-rate controls
    juce::Slider loMultSlider, hiMultSlider, midDbSlider, midFreqSlider;
    juce::Slider midQSlider, tiltDbSlider, smoothingSlider, modeSlider;

    // Labels (kept for completeness; hidden in this design)
    juce::Label loMultLabel, hiMultLabel, midDbLabel, midFreqLabel;
    juce::Label midQLabel, tiltDbLabel, smoothingLabel, modeLabel;

    // Value labels for KnobCells
    juce::Label loMultValue, hiMultValue, midDbValue, midFreqValue;
    juce::Label midQValue, tiltDbValue, smoothingValue, modeValue;

    // KnobCells (render numeric formatted labels inside the knobs)
    std::unique_ptr<KnobCell> loMultKnobCell, hiMultKnobCell, midDbKnobCell, midFreqKnobCell;
    std::unique_ptr<KnobCell> midQKnobCell, tiltDbKnobCell, smoothingKnobCell, modeKnobCell;

    // APVTS binding (real wiring happens in setupComponents)
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loMultAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiMultAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midDbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midQAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tiltDbAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> smoothingAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modeAttachment;

    // Backing APVTS (non-owning)
    juce::AudioProcessorValueTreeState& apvtsRef;

    // State
    bool expanded  = true;   // always expanded by default
    bool active    = true;

    // Constants
    static constexpr float COLLAPSED_HEIGHT   = 40.0f;
    static constexpr float EXPANDED_HEIGHT    = 100.0f;  // Same as DuckingFloat
    static constexpr float PILL_CORNER_RADIUS = 20.0f;
};
