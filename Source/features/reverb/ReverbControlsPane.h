#pragma once

#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"
#include "shared/ui/Controls/SimpleSwitchCell.h"
#include "shared/ui/Design/Layout.h"
#include "ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldMetallic.h"

/*
====================================================================================================
 ReverbControlsPane — 2×16 Grid Container for Reverb Controls (Header)
 ---------------------------------------------------------------------------------------------------
 Purpose
    Declares a 2×16 grid UI presenting the main Reverb parameters. Implementation lives in the .cpp.
    Each cell is a KnobCell (rotary) or SimpleSwitchCell (toggle/combo), bound to APVTS.

 Key Behavior
    • Uses FieldLNF + MetallicKind::Reverb for styling; accent borders enabled.
    • Value labels come from the parameter's own getText(...) → host-visible formatting (%, Hz, dB, s…).
    • Defensive: validates parameter IDs before creating attachments.
    • Grid pads to exactly 32 slots with styled blanks (no stray controls).

 Integration
    • Call setCellMetrics(...) and setRowHeightPx(...) to size/align the grid in parent layouts.
    • Only the public API appears here; implementation details are in the .cpp to reduce rebuilds.

 Dev Notes
    • If you want hard assertions on missing parameters during development, see DBG sites in .cpp.
====================================================================================================
*/

class ReverbControlsPane : public juce::Component
{
public:
    explicit ReverbControlsPane (juce::AudioProcessorValueTreeState& s);
    ~ReverbControlsPane() override;

    // Metrics
    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx);
    void setRowHeightPx (int px);

    // JUCE
    void resized() override;
    void lookAndFeelChanged() override;
    void parentHierarchyChanged() override;

private:
    // Build/layout helpers
    void buildControls();
    void applyMetricsToAll();
    void applyLookAndFeelToTree();

    // ===== Members =====
    juce::AudioProcessorValueTreeState& apvts;

    std::vector<juce::Component*> gridOrder;
    int knobPx     = 60;   // default; can be overridden via setCellMetrics()
    int valuePx    = 16;
    int labelGapPx = 4;
    int colW       = 56;
    int rowH       = 0;    // 0 = auto

    // APVTS attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>>    sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>>    btnAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>>  cmbAtts;

    // Knob cells & blanks
    std::vector<KnobCell*>                          knobCells;
    std::vector<std::unique_ptr<KnobCell>>          ownedCells;
    std::vector<std::unique_ptr<juce::Slider>>      blankSliders;
    std::vector<std::unique_ptr<juce::Label>>       blankLabels;

    // Switch cells
    std::vector<SimpleSwitchCell*>                  switchCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>>  ownedSwitches;

    // Controls (2×16 map)
    juce::ToggleButton enabled, killDry, freeze, followWidth, followRot, duckOn;
    juce::ComboBox     dreqApply;

    juce::Slider pre, erL, erD, erW, erTime, erToTail, dif, density, md, mr, w, rotation, size, dec,
                 wet, bloom, distance, shimmerAmt, shimmerInt, gateAmt, dreqXoverLo, dreqXoverHi,
                 followWidthAmt, followRotAmt, outTrim;

    juce::Label  preV, erLV, erDV, erWV, erTimeV, erToTailV, difV, densityV, mdV, mrV, wV, rotationV, sizeV, decV,
                 wetV, bloomV, distanceV, shimmerAmtV, shimmerIntV, gateAmtV, dreqXoverLoV, dreqXoverHiV,
                 followWidthAmtV, followRotAmtV, outTrimV;
};