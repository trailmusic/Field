#pragma once

#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"
#include "shared/ui/Design/Layout.h"
#include "ReverbParamIDs.h"
#include "shared/Core/FieldLookAndFeel.h"
#include "shared/Core/FieldMetallic.h"

// ReverbControlsPane: 2x16 flat grid container for Reverb controls.
// Scaffolding-only: initially populated with styled empty KnobCells.
class ReverbControlsPane : public juce::Component
{
public:
    explicit ReverbControlsPane (juce::AudioProcessorValueTreeState& s)
        : apvts (s)
    {
        buildControls();
        applyMetricsToAll();
    }
    
    ~ReverbControlsPane() override
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
            cell->getProperties().set ("caption", cap);
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

        // Row 1 (16 controls)
        makeCell (pre,  preV,  "PRE",       ReverbIDs::preDelayMs);
        makeCell (erL,  erLV,  "ER LVL",    ReverbIDs::erLevelDb);
        makeCell (erD,  erDV,  "ER DEN",    ReverbIDs::erDensityPct);
        makeCell (erW,  erWV,  "ER WID",    ReverbIDs::erWidthPct);
        makeCell (dif,  difV,  "DIFF",      ReverbIDs::diffusionPct);
        makeCell (md,   mdV,   "MOD DEP",   ReverbIDs::modDepthCents);
        makeCell (mr,   mrV,   "MOD RATE",  ReverbIDs::modRateHz);
        makeCell (w,    wV,    "TL WID",    ReverbIDs::widthPct);
        makeCell (erTime, erTimeV, "ER TIME", ReverbIDs::erTimeMs);
        makeCell (erToTail, erToTailV, "ER->T", ReverbIDs::erToTailPct);
        makeCell (density, densityV, "DENS", ReverbIDs::densityPct);
        makeCell (widthStart, widthStartV, "W START", ReverbIDs::widthStartPct);
        makeCell (widthEnd, widthEndV, "W END", ReverbIDs::widthEndPct);
        makeCell (rotStart, rotStartV, "R START", ReverbIDs::rotStartDeg);
        makeCell (rotEnd, rotEndV, "R END", ReverbIDs::rotEndDeg);
        makeCell (outTrim, outTrimV, "TRIM", ReverbIDs::outTrimDb);

        // Row 2 (16 controls)
        makeCell (wet,  wetV,  "WET",       ReverbIDs::wetMix01);
        makeCell (dec,  decV,  "DECAY",     ReverbIDs::decaySec);
        makeCell (size, sizeV, "SIZE",      ReverbIDs::sizePct);
        makeCell (bloom,bloomV,"BLOOM",     ReverbIDs::bloomPct);
        makeCell (distance, distanceV, "DIST", ReverbIDs::distancePct);
        makeCell (duckDepth, duckDepthV, "DUCK", ReverbIDs::duckDepthDb);
        makeCell (duckAtk,   duckAtkV,   "ATT",  ReverbIDs::duckAtkMs);
        makeCell (duckRel,   duckRelV,   "REL",  ReverbIDs::duckRelMs);
        makeCell (duckThr,   duckThrV,   "THR",  ReverbIDs::duckThrDb);
        makeCell (duckRatio, duckRatioV, "RAT",  ReverbIDs::duckRatio);
        makeCell (duckKnee, duckKneeV, "KNEE", ReverbIDs::duckKneeDb);
        makeCell (duckLa, duckLaV, "LOOK", ReverbIDs::duckLaMs);
        makeCell (duckRms, duckRmsV, "RMS", ReverbIDs::duckRmsMs);
        makeCell (duckBandHz, duckBandHzV, "BAND", ReverbIDs::duckBandHz);
        makeCell (duckBandQ, duckBandQV, "Q", ReverbIDs::duckBandQ);
        makeCell (freeze, freezeV, "FREEZE", ReverbIDs::freeze);

        // Grid order (Row 1, then Row 2)
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row 1
        for (int i = 0; i < 16; ++i) push (ownedCells[(size_t) i].get());
        // Row 2
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
            // Use new enum-based metallic system
            setAreaMetallicForCell (*cell, MetallicKind::Reverb);
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

    int knobPx     = 52; // Increased knob size from 50 to 52
    int valuePx    = 14;
    int labelGapPx = 4;
    int colW       = 56;
    int rowH       = 0;

    // Sliders/labels
    juce::Slider pre, erL, erD, erW, dif, md, mr, w,
                 erTime, erToTail, density, widthStart, widthEnd, rotStart, rotEnd, outTrim,
                 wet, dec, size, bloom, distance,
                 duckDepth, duckAtk, duckRel, duckThr, duckRatio,
                 duckKnee, duckLa, duckRms, duckBandHz, duckBandQ,
                 freeze;
    juce::Label  preV, erLV, erDV, erWV, difV, mdV, mrV, wV,
                 erTimeV, erToTailV, densityV, widthStartV, widthEndV, rotStartV, rotEndV, outTrimV,
                 wetV, decV, sizeV, bloomV, distanceV,
                 duckDepthV, duckAtkV, duckRelV, duckThrV, duckRatioV,
                 duckKneeV, duckLaV, duckRmsV, duckBandHzV, duckBandQV,
                 freezeV;
};


