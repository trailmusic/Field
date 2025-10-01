#pragma once

#include <JuceHeader.h>

// Responsive metrics for 2x16 control grids, avoiding per-tab hardcoding
struct ControlGridMetrics
{
    int knobPx { 60 };     // Default knob size
    int valuePx { 16 };     // Default value label size  
    int labelGapPx { 4 };   // Default gap between knob and label
    int colW { 64 };        // Default column width
    int rowH { 76 };        // Default row height
    int controlsH { 152 };  // rowH * 2

    static ControlGridMetrics compute (int availableWidthPx, int /*availableHeightPx*/)
    {
        ControlGridMetrics m;
        const int cols = 16;
        const int padding = 8; // per-cell inner padding budget
        const int minKnob = 60; // upscale baseline to make controls larger
        const int maxKnob = 120;

        const int col = juce::jmax (1, availableWidthPx / cols);
        m.colW = col;

        // Fill width with the knob minus a small inner padding
        m.knobPx = juce::jlimit (minKnob, maxKnob, col - padding);

        // Derive bands from knob size, clamped to sensible display values
        m.valuePx = juce::jlimit (12, 22, m.knobPx / 3);
        m.labelGapPx = juce::jlimit (3, 8, m.knobPx / 14);

        m.rowH = m.knobPx + m.valuePx + m.labelGapPx;
        m.controlsH = m.rowH * 2;
        return m;
    }
};


