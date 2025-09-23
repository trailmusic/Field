#pragma once

#include <JuceHeader.h>
#include "../KnobCell.h"

// BandControlsPane: empty 2x16 grid (placeholders) to reserve consistent controls strip
class BandControlsPane : public juce::Component
{
public:
    BandControlsPane() { buildPlaceholders(); }

    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
    {
        knobPx     = juce::jmax (24, knobDiameterPx);
        valuePx    = juce::jmax (10, valueBandPx);
        labelGapPx = juce::jmax (0,  labelGapPxIn);
        colW       = juce::jmax (knobPx, columnWidthPx);
        for (auto& c : cells) if (c) c->setMetrics (knobPx, valuePx, labelGapPx);
        resized();
    }
    void setRowHeightPx (int px) { rowH = juce::jmax (1, px); resized(); }

    void resized() override
    {
        auto r = getLocalBounds();
        const int cols = 16, rows = 2;
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
        const int minRowH = knobPx + labelGapPx + valuePx;
        const int autoH   = juce::jmax (1, r.getHeight() / rows);
        const int cellH   = juce::jmax (rowH > 0 ? rowH : autoH, minRowH);
        const int totalW = cellW * cols, totalH = cellH * rows;
        const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);
        int idx = 0;
        for (int row = 0; row < rows; ++row)
        {
            for (int col = 0; col < cols; ++col)
            {
                auto* c = cells[(size_t) idx++].get();
                if (c)
                {
                    const int x = r.getX() + xOffset + col * cellW;
                    const int y = r.getY() + yOffset + row * cellH;
                    c->setBounds (x, y, cellW, cellH);
                }
            }
        }
    }

private:
    void buildPlaceholders()
    {
        cells.reserve (32);
        for (int i = 0; i < 32; ++i)
        {
            // Empty caption/label placeholders
            static juce::Slider dummy; static juce::Label v;
            auto cell = std::make_unique<KnobCell> (dummy, v, juce::String());
            addAndMakeVisible (*cell);
            cells.emplace_back (std::move (cell));
        }
    }

    int knobPx = 56, valuePx = 16, labelGapPx = 4, colW = 64, rowH = 0;
    std::vector<std::unique_ptr<KnobCell>> cells;
};


