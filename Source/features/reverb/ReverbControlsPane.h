#pragma once

#include <JuceHeader.h>
#include "shared/ui/Components/KnobCell.h"
#include "shared/ui/Controls/SimpleSwitchCell.h"
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
        btnAtts.clear();
        cmbAtts.clear();
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
            // Apply Reverb metallic styling
            setAreaMetallicForCell (*cell, MetallicKind::Reverb);
            // Enable accent border for KnobCells in 2x16 grid
            cell->setShowBorder (true);
            // Set reverb maroon border property for proper border color
            cell->getProperties().set("reverbMaroonBorder", true);
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

        auto makeToggleCell = [&](juce::ToggleButton& b, const juce::String& cap, const char* pid)
        {
            if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
            {
                return;
            }
            
            b.setName (cap);
            
            // CRITICAL: Assign FieldLNF LookAndFeel to the button BEFORE setting metallic properties
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                b.setLookAndFeel(lf);
            
            // Apply Reverb metallic styling
            setAreaMetallicForCell (b, MetallicKind::Reverb);
            
            // Wrap in SimpleSwitchCell for consistent styling and labels
            auto cell = std::make_unique<SimpleSwitchCell> (b);
            cell->setCaption (cap);
            cell->setShowBorder (true);
            // Apply metallic styling to SimpleSwitchCell itself for border rendering
            setAreaMetallicForCell (*cell, MetallicKind::Reverb);
            addAndMakeVisible (*cell);
            switchCells.emplace_back (cell.get());
            ownedSwitches.emplace_back (std::move (cell));
            btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid, b));
        };

        auto makeComboCell = [&](juce::ComboBox& c, const juce::String& cap, const char* pid)
        {
            if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
            {
                return;
            }
            
            c.setName (cap);
            // Add items for EQ Apply choices
            if (pid == ReverbParamIDs::dreqApply)
            {
                c.addItem ("Pre", 1);
                c.addItem ("Post", 2);
                c.addItem ("ER", 3);
                c.addItem ("Tail", 4);
            }
            
            // CRITICAL: Assign FieldLNF LookAndFeel to the ComboBox BEFORE setting metallic properties
            if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
                c.setLookAndFeel(lf);
            
            // Apply Reverb metallic styling
            setAreaMetallicForCell (c, MetallicKind::Reverb);
            
            // Wrap in SimpleSwitchCell for consistent styling
            auto cell = std::make_unique<SimpleSwitchCell> (c);
            cell->setCaption (cap);
            cell->setShowBorder (true);
            // Apply metallic styling to SimpleSwitchCell itself for border rendering
            setAreaMetallicForCell (*cell, MetallicKind::Reverb);
            addAndMakeVisible (*cell);
            switchCells.emplace_back (cell.get());
            ownedSwitches.emplace_back (std::move (cell));
            cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pid, c));
        };

        // Row 1 (16 controls) - Final 2×16 grid map
        makeToggleCell (enabled, "ENABLE", ReverbParamIDs::enabled);
        makeCell (pre,  preV,  "PRE",       ReverbParamIDs::preDelayMs);
        makeCell (erL,  erLV,  "ER LVL",    ReverbParamIDs::erLevelDb);
        makeCell (erD,  erDV,  "ER DEN",    ReverbParamIDs::erDensityPct);
        makeCell (erW,  erWV,  "ER WID",    ReverbParamIDs::erWidthPct);
        makeCell (erTime, erTimeV, "ER TIME", ReverbParamIDs::erTimeMs);
        makeCell (erToTail, erToTailV, "ER->T", ReverbParamIDs::erToTailPct);
        makeCell (dif,  difV,  "DIFF",      ReverbParamIDs::diffusionPct);
        makeCell (density, densityV, "DENS", ReverbParamIDs::densityPct);
        makeCell (md,   mdV,   "MOD DEP",   ReverbParamIDs::modDepthCents);
        makeCell (mr,   mrV,   "MOD RATE",  ReverbParamIDs::modRateHz);
        makeCell (w,    wV,    "WID",       ReverbParamIDs::widthPct);
        makeCell (rotation, rotationV, "ROT", ReverbParamIDs::rotationDeg);
        makeCell (size, sizeV, "SIZE",      ReverbParamIDs::sizePct);
        makeCell (dec,  decV,  "Dec",       ReverbParamIDs::decaySec);
        makeToggleCell (killDry, "WET ONLY", ReverbParamIDs::killDry);

        // Row 2 (16 controls) - Final 2×16 grid map
        makeCell (wet,  wetV,  "WET",       ReverbParamIDs::wetMix01);
        makeCell (bloom,bloomV,"BLM",        ReverbParamIDs::bloomPct);
        makeCell (distance, distanceV, "DIST", ReverbParamIDs::distancePct);
        makeToggleCell (freeze, "FREEZE", ReverbParamIDs::freeze);
        makeCell (shimmerAmt, shimmerAmtV, "SHIM AMT", ReverbParamIDs::shimmerAmtPct);
        makeCell (shimmerInt, shimmerIntV, "SHIM INT", ReverbParamIDs::shimmerInt);
        makeCell (gateAmt, gateAmtV, "GATE", ReverbParamIDs::gateAmtPct);
        makeCell (dreqXoverLo, dreqXoverLoV, "DR XO LO", ReverbParamIDs::dreqXoverLoHz);
        makeCell (dreqXoverHi, dreqXoverHiV, "DR XO HI", ReverbParamIDs::dreqXoverHiHz);
        makeComboCell (dreqApply, "EQ APPLY", ReverbParamIDs::dreqApply);
        makeToggleCell (followWidth, "FOLLOW W", ReverbParamIDs::followWidth);
        makeCell (followWidthAmt, followWidthAmtV, "W AMT", ReverbParamIDs::followWidthAmt);
        makeToggleCell (followRot, "FOLLOW R", ReverbParamIDs::followRot);
        makeCell (followRotAmt, followRotAmtV, "R AMT", ReverbParamIDs::followRotAmt);
        makeCell (outTrim, outTrimV, "TRIM", ReverbParamIDs::outTrimDb);
        makeToggleCell (duckOn, "DUCK", ReverbParamIDs::duckOn);

        // Grid order (Row 1, then Row 2) - matches Reverb.md documentation
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        
        // Row 1 (16 controls): ENABLE, PRE, ER LVL, ER DEN, ER WID, ER TIME, ER→T, DIFF, DENS, MOD DEP, MOD RATE, WIDTH, ROT, SIZE, DECAY, WET ONLY
        push (ownedSwitches[0].get()); // ENABLE (enabled) - SimpleSwitchCell
        push (ownedCells[0].get());    // PRE (preDelayMs)
        push (ownedCells[1].get());    // ER LVL (erLevelDb)
        push (ownedCells[2].get());    // ER DEN (erDensityPct)
        push (ownedCells[3].get());    // ER WID (erWidthPct)
        push (ownedCells[4].get());    // ER TIME (erTimeMs)
        push (ownedCells[5].get());    // ER→T (erToTailPct)
        push (ownedCells[6].get());    // DIFF (diffusionPct)
        push (ownedCells[7].get());    // DENS (densityPct)
        push (ownedCells[8].get());    // MOD DEP (modDepthCents)
        push (ownedCells[9].get());    // MOD RATE (modRateHz)
        push (ownedCells[10].get());   // WIDTH (widthPct)
        push (ownedCells[11].get());   // ROT (rotationDeg)
        push (ownedCells[12].get());   // SIZE (sizePct)
        push (ownedCells[13].get());   // DECAY (decaySec)
        push (ownedSwitches[1].get()); // WET ONLY (killDry) - SimpleSwitchCell
        
        // Row 2 (16 controls): WET, BLOOM, DIST, FREEZE, SHIM AMT, SHIM INT, GATE, DREQ XO LO, DREQ XO HI, EQ APPLY, FOLLOW W, W AMT, FOLLOW R, R AMT, TRIM, DUCK
        push (ownedCells[14].get());   // WET (wetMix01)
        push (ownedCells[15].get());    // BLOOM (bloomPct)
        push (ownedCells[16].get());    // DIST (distancePct)
        push (ownedSwitches[2].get()); // FREEZE (freeze) - SimpleSwitchCell
        push (ownedCells[17].get());    // SHIM AMT (shimmerAmtPct)
        push (ownedCells[18].get());    // SHIM INT (shimmerInt)
        push (ownedCells[19].get());    // GATE (gateAmtPct)
        push (ownedCells[20].get());    // DREQ XO LO (dreqXoverLoHz)
        push (ownedCells[21].get());    // DREQ XO HI (dreqXoverHiHz)
        push (ownedSwitches[3].get());  // EQ APPLY (dreqApply) - SimpleSwitchCell
        push (ownedSwitches[4].get());  // FOLLOW W (followWidth) - SimpleSwitchCell
        push (ownedCells[22].get());    // W AMT (followWidthAmt)
        push (ownedSwitches[5].get());  // FOLLOW R (followRot) - SimpleSwitchCell
        push (ownedCells[23].get());    // R AMT (followRotAmt)
        push (ownedCells[24].get());    // TRIM (outTrimDb)
        push (ownedSwitches[6].get());  // DUCK (duckOn) - SimpleSwitchCell

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
            // Enable accent border for blank KnobCells in 2x16 grid
            cell->setShowBorder (true);
            // Set reverb maroon border property for proper border color
            cell->getProperties().set("reverbMaroonBorder", true);
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
        
        // Apply metrics to SimpleSwitchCell components for consistent sizing
        for (auto* c : switchCells)
        {
            if (c == nullptr) continue;
            c->setMetrics (knobPx, valuePx, labelGapPx);
        }
    }

    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<juce::Slider>> sliders;
    std::vector<std::unique_ptr<juce::Label>>  values;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<SimpleSwitchCell*> switchCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>> ownedSwitches;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;

    // Metrics - now set via setCellMetrics() from centralized ControlGridMetrics
    int knobPx     = 60; // Default values, overridden by setCellMetrics()
    int valuePx    = 16;
    int labelGapPx = 4;
    int colW       = 56;
    int rowH       = 0;

    // ToggleButtons and ComboBoxes
    juce::ToggleButton enabled, killDry, freeze, followWidth, followRot, duckOn;
    juce::ComboBox dreqApply;
    
    // Sliders/labels - Updated for final 2×16 grid map
    juce::Slider pre, erL, erD, erW, erTime, erToTail, dif, density, md, mr, w, rotation, size, dec,
                 wet, bloom, distance, shimmerAmt, shimmerInt, gateAmt, dreqXoverLo, dreqXoverHi,
                 followWidthAmt, followRotAmt, outTrim;
    juce::Label  preV, erLV, erDV, erWV, erTimeV, erToTailV, difV, densityV, mdV, mrV, wV, rotationV, sizeV, decV,
                 wetV, bloomV, distanceV, shimmerAmtV, shimmerIntV, gateAmtV, dreqXoverLoV, dreqXoverHiV,
                 followWidthAmtV, followRotAmtV, outTrimV;
};


