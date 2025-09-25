#pragma once
#include <JuceHeader.h>
#include "../../ui/Components/KnobCell.h"
#include "../../ui/Layout.h"
#include "../ReverbParamIDs.h"

class ReverbControlsPanel : public juce::Component
{
public:
    explicit ReverbControlsPanel (juce::AudioProcessorValueTreeState& s) : state (s)
    {
        auto styleKnob = [] (juce::Slider& k)
        {
            k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            k.setRotaryParameters (juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                                   true);
        };
        auto attach = [&](std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& a,
                          const char* id, juce::Slider& slider, juce::Label& value, const juce::String& caption)
        {
            styleKnob (slider);
            slider.setName (caption); // FieldLNF draws knob name from Slider::getName()
            auto cell = std::make_unique<KnobCell> (slider, value, caption);
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cells.add (cell.get());
            addAndMakeVisible (*cell);
            cellPtrs.emplace_back (std::move (cell));
            a = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (state, id, slider);

            // Initialize and live-update the value label (simple formatting for now)
            auto fmt = [&value](double v)
            {
                value.setText (juce::String (v, 1), juce::dontSendNotification);
            };
            fmt (slider.getValue());
            slider.onValueChange = [&, fmt]() mutable { fmt (slider.getValue()); };
        };

        // Row 1
        attach (preA,  ReverbIDs::preDelayMs,   pre,  preV,  "PRE");
        attach (erLA,  ReverbIDs::erLevelDb,    erL,  erLV,  "ER LVL");
        attach (erTA,  ReverbIDs::erTimeMs,     erT,  erTV,  "ER TIME");
        attach (erDA,  ReverbIDs::erDensityPct, erD,  erDV,  "ER DEN");
        attach (erWA,  ReverbIDs::erWidthPct,   erW,  erWV,  "ER WID");
        // Row 2
        attach (decA,  ReverbIDs::decaySec,     dec,  decV,  "DECAY");
        attach (denA,  ReverbIDs::densityPct,   den,  denV,  "DENS");
        attach (difA,  ReverbIDs::diffusionPct, dif,  difV,  "DIFF");
        attach (mdA,   ReverbIDs::modDepthCents, md, mdV,   "MOD DEP");
        attach (mrA,   ReverbIDs::modRateHz,    mr,   mrV,   "MOD RATE");
        // Row 3
        attach (hpA,   ReverbIDs::hpfHz,        hp,   hpV,   "HP");
        attach (lpA,   ReverbIDs::lpfHz,        lp,   lpV,   "LP");
        attach (tiltA, ReverbIDs::tiltDb,       tilt, tiltV, "TILT");
        attach (eqmA,  ReverbIDs::postEqMixPct, eqm,  eqmV,  "EQ MIX");
        attach (ertA,  ReverbIDs::erToTailPct,  ert,  ertV,  "ER→TAIL");
        // Row 4
        attach (dlA,   ReverbIDs::dreqLowX,     dl,   dlV,   "LOW×");
        attach (dmA,   ReverbIDs::dreqMidX,     dm,   dmV,   "MID×");
        attach (dhA,   ReverbIDs::dreqHighX,    dh,   dhV,   "HIGH×");
        attach (wA,    ReverbIDs::widthPct,     w,    wV,    "TL WID");
        attach (wetA,  ReverbIDs::wetMix01,     wet,  wetV,  "WET");

        // Initial sizing (will be overridden by setCellMetrics from parent)
        for (auto* c : cells)
        {
            c->setMetrics (knobPx, valuePx, labelGap);
            c->setValueLabelGap (labelGap);
        }
    }

    // Expose flat list of cells so parent can grid-layout them without this container
    void collectCells (juce::Array<KnobCell*>& out) const
    {
        out.clearQuick();
        for (auto* c : cells)
            if (c != nullptr) out.add (c);
    }

    // Adopt Delay group's exact metrics/column width so visuals match 1:1
    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPx, int columnWidthPx)
    {
        knobPx   = juce::jmax (24, knobDiameterPx);
        valuePx  = juce::jmax (10, valueBandPx);
        labelGap = juce::jmax (0,  labelGapPx);
        colW     = juce::jmax (knobPx, columnWidthPx);

        for (auto* c : cells)
        {
            if (c != nullptr)
            {
                c->setMetrics (knobPx, valuePx, labelGap);
                c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                c->setValueLabelGap (labelGap);
            }
        }
        resized();
        repaint();
    }

    // Ensure row height exactly matches Delay/Motion containerHeight (avoid div rounding)
    void setRowHeightPx (int px)
    {
        rowH = juce::jmax (1, px);
        resized();
        repaint();
    }

    void resized() override
    {
        auto r = getLocalBounds();
        // Match Delay: fixed column width provided by parent; row height is uniform 1/4 of panel
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / 5));
        const int cellH = (rowH > 0 ? rowH : juce::jmax (1, r.getHeight() / 4));
        // Enforce exact width for 5 columns; center within panel if wider
        const int totalW = cellW * 5;
        const int xOffset = (r.getWidth() > totalW ? (r.getWidth() - totalW) / 2 : 0);
        // Center vertically if panel is taller than 4 rows
        const int totalH = cellH * 4;
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);
        auto place = [&](KnobCell* c, int row, int col)
        {
            if (!c) return;
            const int x = r.getX() + xOffset + (col - 1) * cellW;
            const int y = r.getY() + yOffset + (row - 1) * cellH;
            c->setBounds (x, y, cellW, cellH);
        };
        int i = 0;
        for (int row = 1; row <= 4; ++row)
            for (int col = 1; col <= 5; ++col)
                place (cells[i++], row, col);
    }

private:
    juce::AudioProcessorValueTreeState& state;

    // KnobCells in row-major order 4x5
    juce::OwnedArray<KnobCell> cells;
    std::vector<std::unique_ptr<KnobCell>> cellPtrs;
    // Metrics (overridden by parent via setCellMetrics)
    int knobPx  = 48;    // knob diameter
    int valuePx = 14;    // value label band height
    int labelGap = 4;    // gap under knob before value label
    int colW    = 56;    // column width (single cell width)
    int rowH    = 0;     // exact row height in px (containerHeight); 0 = derive from bounds

    // Sliders + value labels
    juce::Slider pre, erL, erT, erD, erW, dec, den, dif, md, mr, hp, lp, tilt, eqm, ert, dl, dm, dh, w, wet;
    juce::Label  preV, erLV, erTV, erDV, erWV, decV, denV, difV, mdV, mrV, hpV, lpV, tiltV, eqmV, ertV, dlV, dmV, dhV, wV, wetV;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preA, erLA, erTA, erDA, erWA,
        decA, denA, difA, mdA, mrA, hpA, lpA, tiltA, eqmA, ertA, dlA, dmA, dhA, wA, wetA;
};


