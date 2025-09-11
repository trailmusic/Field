#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "Design.h"
#include "Layout.h"

// Forward declarations
class KnobCell;
class DoubleKnobCell;
class QuadKnobCell;

/**
 * ControlFactory - Standardized control creation with consistent sizing
 * 
 * This factory ensures all controls follow the same sizing standards:
 * - Single controls: Standard size (64px)
 * - Double controls: Double size (128px) 
 * - Quad controls: Quad size (128px)
 * 
 * All controls use design tokens for consistent spacing and sizing.
 */
class ControlFactory {
public:
    // Standard single controls (all same size)
    static std::unique_ptr<KnobCell> createKnobCell(
        juce::Slider& slider, 
        juce::Label& valueLabel, 
        const juce::String& caption
    );
    
    
    static std::unique_ptr<juce::ComboBox> createComboBox(
        const juce::String& caption
    );
    
    // Special controls (larger sizes)
    static std::unique_ptr<DoubleKnobCell> createDoubleKnobCell(
        juce::Slider& slider1, juce::Label& label1,
        juce::Slider& slider2, juce::Label& label2
    );
    
    static std::unique_ptr<QuadKnobCell> createQuadKnobCell(
        juce::Slider& slider1, juce::Label& label1,
        juce::Slider& slider2, juce::Label& label2,
        juce::Slider& slider3, juce::Label& label3,
        juce::Component& clusterContainer
    );
    
    // Standard sizing constants
    static const int STANDARD_CELL_SIZE = UI::u(8);  // 64px
    static const int DOUBLE_CELL_SIZE = UI::u(16);   // 128px
    static const int QUAD_CELL_SIZE = UI::u(16);     // 128px
    
    // Standard metrics for all controls
    static constexpr int STANDARD_KNOB_SIZE = 48;        // Base knob size
    static constexpr int STANDARD_VALUE_SIZE = 14;       // Value label size
    static const int STANDARD_GAP = UI::dp / 2;      // 4px gap
    
    // Apply standard metrics to a control
    static void applyStandardMetrics(KnobCell* cell);
    static void applyStandardMetrics(DoubleKnobCell* cell);
    static void applyStandardMetrics(QuadKnobCell* cell);
    
private:
    // Helper to get scaled knob size
    static int getScaledKnobSize();
};
