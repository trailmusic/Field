#pragma once
#include <JuceHeader.h>
#include "KnobCell.h"

// MiniKnobCell: a smaller variant of KnobCell (~half size)
class MiniKnobCell : public KnobCell
{
public:
    MiniKnobCell (juce::Slider& knob, juce::Label& valueLabel, const juce::String& caption = {})
        : KnobCell (knob, valueLabel, caption)
    {
        // Half-size defaults relative to KnobCell's K=88,V=14,G=4
        setMetrics (44, 12, 3, 0);
        setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        setValueLabelGap (2);
        setShowBorder (true);
        setShowPanel (true);
    }
};


