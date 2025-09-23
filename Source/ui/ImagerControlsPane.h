#pragma once

#include <JuceHeader.h>
#include "../KnobCell.h"
#include "../Layout.h"

// ImagerControlsPane: 2x16 grid for imaging/placement controls
class ImagerControlsPane : public juce::Component
{
public:
    explicit ImagerControlsPane (juce::AudioProcessorValueTreeState& s)
        : apvts (s) { buildControls(); applyMetricsToAll(); }

    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
    {
        knobPx     = juce::jmax (24, knobDiameterPx);
        valuePx    = juce::jmax (10, valueBandPx);
        labelGapPx = juce::jmax (0,  labelGapPxIn);
        colW       = juce::jmax (knobPx, columnWidthPx);
        applyMetricsToAll(); resized();
    }
    void setRowHeightPx (int px) { rowH = juce::jmax (1, px); resized(); }

    void resized() override
    {
        auto r = getLocalBounds();
        const int cols = 16, rows = 2;
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
        const int cellH = (rowH > 0 ? rowH : juce::jmax (1, r.getHeight() / rows));
        const int totalW = cellW * cols, totalH = cellH * rows;
        const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);
        auto place = [&](int index, int row, int col)
        {
            if (index < 0 || index >= gridOrder.size()) return;
            if (auto* c = gridOrder[(size_t) index])
            {
                const int x = r.getX() + xOffset + (col - 1) * cellW;
                const int y = r.getY() + yOffset + (row - 1) * cellH;
                c->setBounds (x, y, cellW, cellH);
            }
        };
        int idx = 0;
        for (int row = 1; row <= rows; ++row)
            for (int col = 1; col <= cols; ++col)
                place (idx++, row, col);
    }

private:
    void styleKnob (juce::Slider& k)
    {
        k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        k.setRotaryParameters (juce::MathConstants<float>::pi,
                               juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                               true);
    }
    void makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
    {
        styleKnob (s); s.setName (cap);
        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));
    }

    void buildControls()
    {
        // Imaging width only (other imaging/placement controls moved to XY)
        makeCell (widthLo, widthLoV, "WIDTH LO", "width_lo");
        makeCell (widthMid, widthMidV, "WIDTH MID", "width_mid");
        makeCell (widthHi, widthHiV, "WIDTH HI", "width_hi");

        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row A: width lo/mid/hi, [13 empty]
        push (ownedCells[0].get()); // WIDTH LO
        push (ownedCells[1].get()); // WIDTH MID
        push (ownedCells[2].get()); // WIDTH HI
        for (int i = 0; i < 13; ++i) gridOrder.push_back (nullptr);

        // Row B: [16 empty]
        for (int i = 0; i < 16; ++i) gridOrder.push_back (nullptr);
    }

    void applyMetricsToAll()
    {
        for (auto* c : knobCells)
        {
            if (!c) continue;
            c->setMetrics (knobPx, valuePx, labelGapPx);
            c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            c->setValueLabelGap (labelGapPx);
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    // Sliders/labels (width only; other imaging controls moved to XY)
    juce::Slider widthLo, widthMid, widthHi;
    juce::Label  widthLoV, widthMidV, widthHiV;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<juce::Component*> gridOrder;
    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;
};


