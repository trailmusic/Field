#pragma once

#include <JuceHeader.h>
#include "Components/KnobCell.h"
#include "../ui/Layout.h"

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
        // Safety check: ensure parameter exists before creating attachment
        if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
        {
            // Skip this cell if parameter doesn't exist
            return;
        }
        
        styleKnob (s); s.setName (cap);
        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        // Band pane styling: metallic blue
        cell->getProperties().set ("metallic", true);
        cell->getProperties().set ("bandMetallic", true);
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));

        // Initialize and live-update label with sensible precision
        auto applyLabel = [&]()
        {
            int decimals = 2;
            juce::String id (pid);
            if (id.containsIgnoreCase ("_hz")) decimals = 0;
            else if (id.equalsIgnoreCase ("width")) decimals = 2;
            else if (id.containsIgnoreCase ("_db")) decimals = 1;
            else if (id.containsIgnoreCase ("_pct")) decimals = 0;
            v.setText (juce::String (s.getValue(), decimals), juce::dontSendNotification);
        };
        applyLabel();
        s.onValueChange = [&, applyLabel]() { applyLabel(); };
    }

    void buildControls()
    {
        // Imaging widths for Band: Global + per-band
        makeCell (width,    widthV,    "WIDTH",     "width");
        makeCell (widthLo,  widthLoV,  "WIDTH LO",  "width_lo");
        makeCell (widthMid, widthMidV, "WIDTH MID", "width_mid");
        makeCell (widthHi,  widthHiV,  "WIDTH HI",  "width_hi");

        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row A: WIDTH, WIDTH LO, WIDTH MID, WIDTH HI, [12 empty]
        push (ownedCells[0].get()); // WIDTH
        push (ownedCells[1].get()); // WIDTH LO
        push (ownedCells[2].get()); // WIDTH MID
        push (ownedCells[3].get()); // WIDTH HI
        for (int i = 0; i < 12; ++i) gridOrder.push_back (nullptr);

        // Row B: [16 empty]
        for (int i = 0; i < 16; ++i) gridOrder.push_back (nullptr);

        // Fill blanks up to 32 with styled Band blanks (metallic blue)
        const int totalNeeded = 32;
        while ((int) ownedCells.size() < totalNeeded)
        {
            auto sl = std::make_unique<juce::Slider>();
            auto lb = std::make_unique<juce::Label>(); lb->setVisible (false);
            styleKnob (*sl);
            auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            cell->setShowKnob (false);
            cell->getProperties().set ("metallic", true);
            cell->getProperties().set ("bandMetallic", true); // metallic blue mapping in KnobCell
            addAndMakeVisible (*cell);
            knobCells.emplace_back (cell.get());
            blankSliders.emplace_back (std::move (sl));
            blankLabels.emplace_back (std::move (lb));
            ownedCells.emplace_back (std::move (cell));
        }
        for (int i = (int) gridOrder.size(); i < totalNeeded; ++i)
            push (ownedCells[(size_t) i].get());
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
    // Sliders/labels (Band): master width + per-band widths
    juce::Slider width, widthLo, widthMid, widthHi;
    juce::Label  widthV, widthLoV, widthMidV, widthHiV;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;
    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;
};


