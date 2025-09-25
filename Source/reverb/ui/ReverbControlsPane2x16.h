#pragma once

#include <JuceHeader.h>
#include "../../KnobCell.h"
#include "../../Layout.h"
#include "../ReverbParamIDs.h"

// ReverbControlsPane2x16: 2x16 flat grid container for Reverb controls.
// Scaffolding-only: initially populated with styled empty KnobCells.
class ReverbControlsPane2x16 : public juce::Component
{
public:
    explicit ReverbControlsPane2x16 (juce::AudioProcessorValueTreeState& s)
        : apvts (s)
    {
        buildControls();
        applyMetricsToAll();
    }
    
    ~ReverbControlsPane2x16() override
    {
        // Clear parameter attachments before destruction to prevent crashes
        sAtts.clear();
    }

    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
    {
        knobPx     = juce::jmax (24, knobDiameterPx);
        valuePx    = juce::jmax (10, valueBandPx);
        labelGapPx = juce::jmax (0,  labelGapPxIn);
        colW       = juce::jmax (knobPx, columnWidthPx);
        applyMetricsToAll();
        resized();
        repaint();
    }

    void setRowHeightPx (int px)
    {
        rowH = juce::jmax (1, px);
        resized();
        repaint();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        const int cols = 16;
        const int rows = 2;
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
        const int cellH = (rowH > 0 ? rowH : juce::jmax (1, r.getHeight() / rows));
        const int totalW = cellW * cols;
        const int totalH = cellH * rows;
        const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);

        auto place = [&] (int index, int row, int col)
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
    void buildControls()
    {
        auto styleKnob = [] (juce::Slider& k)
        {
            k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            k.setRotaryParameters (juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                   true);
        };

        auto makeCell = [&](juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
        {
            // Safety check: ensure parameter exists before creating attachment
            if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
            {
                // Skip this cell if parameter doesn't exist
                return;
            }
            
            styleKnob (s);
            s.setName (cap);
            auto cell = std::make_unique<KnobCell> (s, v, cap);
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            // Styling: Reverb metallic (burnt orange) + border
            cell->getProperties().set ("reverbMaroonBorder", true);
            cell->getProperties().set ("metallic", true);
            cell->getProperties().set ("reverbMetallic", true);
            addAndMakeVisible (*cell);
            knobCells.emplace_back (cell.get());
            ownedCells.emplace_back (std::move (cell));
            sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));

            // Initialize and live-update value labels
            auto applyLabel = [&]()
            {
                int decimals = 2;
                juce::String id (pid);
                if (id.containsIgnoreCase ("_hz")) decimals = 0;
                else if (id.containsIgnoreCase ("_db")) decimals = 1;
                else if (id.containsIgnoreCase ("_pct") || id.containsIgnoreCase ("_ratio")) decimals = 0;
                else if (id.containsIgnoreCase ("_ms")) decimals = 0;
                else if (id.containsIgnoreCase ("_sec")) decimals = 2;
                v.setText (juce::String (s.getValue(), decimals), juce::dontSendNotification);
            };
            applyLabel();
            s.onValueChange = [&, applyLabel]() { applyLabel(); };
        };

        // Row A first 8 (Enable/Algo/WetOnly/SIZE handled in ReverbTab header area if needed)
        makeCell (pre,  preV,  "PRE",       ReverbIDs::preDelayMs);
        makeCell (erL,  erLV,  "ER LVL",    ReverbIDs::erLevelDb);
        makeCell (erD,  erDV,  "ER DEN",    ReverbIDs::erDensityPct);
        makeCell (erW,  erWV,  "ER WID",    ReverbIDs::erWidthPct);
        makeCell (dif,  difV,  "DIFF",      ReverbIDs::diffusionPct);
        makeCell (md,   mdV,   "MOD DEP",   ReverbIDs::modDepthCents);
        makeCell (mr,   mrV,   "MOD RATE",  ReverbIDs::modRateHz);
        makeCell (hp,   hpV,   "HP",        ReverbIDs::hpfHz);
        // Row A tail
        makeCell (lp,   lpV,   "LP",        ReverbIDs::lpfHz);
        makeCell (tilt, tiltV, "TILT",      ReverbIDs::tiltDb);
        makeCell (eqm,  eqmV,  "EQ MIX",    ReverbIDs::postEqMixPct);
        makeCell (ert,  ertV,  "ER→TAIL",   ReverbIDs::erToTailPct);
        makeCell (dl,   dlV,   "LOW×",      ReverbIDs::dreqLowX);
        makeCell (dm,   dmV,   "MID×",      ReverbIDs::dreqMidX);
        makeCell (dh,   dhV,   "HIGH×",     ReverbIDs::dreqHighX);
        makeCell (w,    wV,    "TL WID",    ReverbIDs::widthPct);

        // Row B core (WET, DECAY, SIZE, BLOOM, DISTANCE, dec XO Lo/Hi)
        makeCell (wet,  wetV,  "WET",       ReverbIDs::wetMix01);
        makeCell (dec,  decV,  "DECAY",     ReverbIDs::decaySec);
        makeCell (size, sizeV, "SIZE",      ReverbIDs::sizePct);
        makeCell (bloom,bloomV,"BLOOM",     ReverbIDs::bloomPct);
        makeCell (distance, distanceV, "DIST", ReverbIDs::distancePct);
        makeCell (xLo,  xLoV, "DEC XO LO", ReverbIDs::dreqXoverLoHz);
        makeCell (xHi,  xHiV, "DEC XO HI", ReverbIDs::dreqXoverHiHz);

        // Ducking cluster (mode + 5)
        makeCell (duckDepth, duckDepthV, "DUCK", ReverbIDs::duckDepthDb);
        makeCell (duckAtk,   duckAtkV,   "ATT",  ReverbIDs::duckAtkMs);
        makeCell (duckRel,   duckRelV,   "REL",  ReverbIDs::duckRelMs);
        makeCell (duckThr,   duckThrV,   "THR",  ReverbIDs::duckThrDb);
        makeCell (duckRatio, duckRatioV, "RAT",  ReverbIDs::duckRatio);

        // Grid order (Row A, then Row B), aligned to DEC-0002 mapping
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row A
        for (int i = 0; i < 16; ++i) push (ownedCells[(size_t) i].get());
        // Row B (WET..RAT then XO HI)
        for (int i = 16; i < ownedCells.size(); ++i) push (ownedCells[(size_t) i].get());

        // Fill blanks up to 32 with styled Reverb blanks
        const int totalNeeded = 32;
        while ((int) ownedCells.size() < totalNeeded)
        {
            // Dummy slider/label (hidden label)
            auto sl = std::make_unique<juce::Slider>();
            auto lb = std::make_unique<juce::Label>(); lb->setVisible (false);
            styleKnob (*sl);
            auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            cell->setShowKnob (false);
            cell->getProperties().set ("metallic", true);
            cell->getProperties().set ("reverbMetallic", true);
            cell->getProperties().set ("reverbMaroonBorder", true);
            addAndMakeVisible (*cell);
            knobCells.emplace_back (cell.get());
            blankSliders.emplace_back (std::move (sl));
            blankLabels.emplace_back (std::move (lb));
            ownedCells.emplace_back (std::move (cell));
        }
        // Append blanks to grid until 32
        for (int i = (int) gridOrder.size(); i < totalNeeded; ++i)
            push (ownedCells[(size_t) i].get());
    }

    void applyMetricsToAll()
    {
        for (auto* c : knobCells)
        {
            if (c == nullptr) continue;
            c->setMetrics (knobPx, valuePx, labelGapPx);
            c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            c->setValueLabelGap (labelGapPx);
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Label>>  values;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;

    int knobPx     = 48;
    int valuePx    = 14;
    int labelGapPx = 4;
    int colW       = 56;
    int rowH       = 0;

    // Sliders/labels
    juce::Slider pre, erL, erD, erW, dif, md, mr, hp, lp, tilt, eqm, ert, dl, dm, dh, w,
                 wet, dec, size, bloom, distance, xLo, xHi,
                 duckDepth, duckAtk, duckRel, duckThr, duckRatio;
    juce::Label  preV, erLV, erDV, erWV, difV, mdV, mrV, hpV, lpV, tiltV, eqmV, ertV, dlV, dmV, dhV, wV,
                 wetV, decV, sizeV, bloomV, distanceV, xLoV, xHiV,
                 duckDepthV, duckAtkV, duckRelV, duckThrV, duckRatioV;
};


