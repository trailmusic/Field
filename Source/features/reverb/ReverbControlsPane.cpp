#include "ReverbControlsPane.h"

// ============================== Construction / Destruction ==============================

ReverbControlsPane::ReverbControlsPane (juce::AudioProcessorValueTreeState& s)
    : apvts (s)
{
    buildControls();
    applyMetricsToAll();
    
    // Ensure children inherit the current LNF once they exist
    applyLookAndFeelToTree();
}

ReverbControlsPane::~ReverbControlsPane()
{
    // Clear attachments safely before children/components go away
    sAtts.clear();
    btnAtts.clear();
    cmbAtts.clear();
}

// ============================== Public API ==============================

void ReverbControlsPane::setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
{
    knobPx     = juce::jmax (24, knobDiameterPx);
    valuePx    = juce::jmax (10, valueBandPx);
    labelGapPx = juce::jmax (0,  labelGapPxIn);
    colW       = juce::jmax (knobPx, columnWidthPx);
    applyMetricsToAll();
    resized();
    repaint();
}

void ReverbControlsPane::setRowHeightPx (int px)
{
    rowH = juce::jmax (1, px);
    resized();
    repaint();
}

// ============================== JUCE overrides ==============================

void ReverbControlsPane::resized()
{
    auto r = getLocalBounds();
    const int cols = 16, rows = 2;
    const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
    const int cellH = (rowH > 0 ? rowH : juce::jmax (1, r.getHeight() / rows));
    const int totalW = cellW * cols, totalH = cellH * rows;
    const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
    const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);

    auto place = [&] (int index, int row, int col)
    {
        if ((index < 0) || (index >= (int) gridOrder.size())) return;
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

void ReverbControlsPane::lookAndFeelChanged()
{
    // Parent's LNF changed → push it down the tree
    applyLookAndFeelToTree();
    repaint();
}

void ReverbControlsPane::parentHierarchyChanged()
{
    // Parent reset or LNF flipped higher up → re-apply to children
    applyLookAndFeelToTree();
    repaint();
}

void ReverbControlsPane::applyLookAndFeelToTree()
{
    auto* lf = &getLookAndFeel(); // FieldLNF expected, but we don't hard-require it

    // 1) Knob cells - the parent setLookAndFeel should propagate to children
    for (auto* kc : knobCells)
    {
        if (!kc) continue;
        kc->setLookAndFeel(lf);
    }

    // 2) Switch cells - the parent setLookAndFeel should propagate to children
    for (auto* sc : switchCells)
    {
        if (!sc) continue;
        sc->setLookAndFeel(lf);
    }

    // 3) Standalone controls (in case some are not wrapped yet)
    enabled.setLookAndFeel(lf);
    killDry.setLookAndFeel(lf);
    freeze.setLookAndFeel(lf);
    followWidth.setLookAndFeel(lf);
    followRot.setLookAndFeel(lf);
    duckOn.setLookAndFeel(lf);
    dreqApply.setLookAndFeel(lf);

    for (juce::Slider* s : { &pre,&erL,&erD,&erW,&erTime,&erToTail,&dif,&density,&md,&mr,&w,&rotation,&size,&dec,
                              &wet,&bloom,&distance,&shimmerAmt,&shimmerInt,&gateAmt,&dreqXoverLo,&dreqXoverHi,
                              &followWidthAmt,&followRotAmt,&outTrim })
        if (s) s->setLookAndFeel(lf);

    // 4) Blank cells (the padded 32)
    for (auto& bs : blankSliders) if (bs) bs->setLookAndFeel(lf);
    for (auto& bl : blankLabels)  if (bl) bl->setLookAndFeel(lf);
}

// ============================== Private helpers ==============================

void ReverbControlsPane::buildControls()
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
        if (pid == nullptr)
        {
            DBG("❌ CRITICAL: Null parameter ID in makeCell for: " << cap);
            // jassertfalse;
            return;
        }

        auto* param = apvts.getParameter (juce::String (pid));
        if (param == nullptr)
        {
            DBG("❌ CRITICAL: Param not found in APVTS: " << juce::String(pid) << " for: " << cap);
            // jassertfalse;
            return;
        }

        styleKnob (s);
        s.setName (cap);

        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        setAreaMetallicForCell (*cell, MetallicKind::Reverb);
        cell->setShowBorder (true);
        cell->getProperties().set ("reverbMaroonBorder", true);
        cell->setLookAndFeel (&getLookAndFeel());

        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));

        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));

        // Label text from parameter's formatter → host-visible units/precision
        auto applyLabel = [&, param]()
        {
            juce::String txt;
            if (auto* rp = dynamic_cast<juce::RangedAudioParameter*>(param))
                txt = rp->getText ((float) s.getValue(), 64);
            else
                txt = param->getCurrentValueAsText();

            v.setText (txt, juce::dontSendNotification);
        };
        applyLabel();
        s.onValueChange = [applyLabel]() { applyLabel(); };
    };

    auto makeToggleCell = [&](juce::ToggleButton& b, const juce::String& cap, const char* pid)
    {
        if (pid == nullptr)
        {
            DBG("❌ CRITICAL: Null parameter ID in makeToggleCell for: " << cap);
            // jassertfalse;
            return;
        }
        if (apvts.getParameter (juce::String (pid)) == nullptr)
        {
            DBG("❌ CRITICAL: Param not found in APVTS: " << juce::String(pid) << " for: " << cap);
            // jassertfalse;
            return;
        }

        b.setName (cap);
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            b.setLookAndFeel (lf);

        setAreaMetallicForCell (b, MetallicKind::Reverb);

        auto cell = std::make_unique<SimpleSwitchCell> (b);
        cell->setCaption (cap);
        cell->setShowBorder (true);
        setAreaMetallicForCell (*cell, MetallicKind::Reverb);

        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));

        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid, b));
    };

    auto makeComboCell = [&](juce::ComboBox& c, const juce::String& cap, const char* pid)
    {
        if (pid == nullptr)
        {
            DBG("❌ CRITICAL: Null parameter ID in makeComboCell for: " << cap);
            // jassertfalse;
            return;
        }
        auto* param = apvts.getParameter (juce::String (pid));
        if (param == nullptr)
        {
            DBG("❌ CRITICAL: Param not found in APVTS: " << juce::String(pid) << " for: " << cap);
            // jassertfalse;
            return;
        }

        c.setName (cap);

        // Populate from the AudioParameterChoice (source-of-truth)
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*>(param))
        {
            c.clear();
            const auto& opts = choice->choices;
            for (int i = 0; i < opts.size(); ++i)
                c.addItem (opts[i], i + 1);
        }

        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            c.setLookAndFeel (lf);

        setAreaMetallicForCell (c, MetallicKind::Reverb);

        auto cell = std::make_unique<SimpleSwitchCell> (c);
        cell->setCaption (cap);
        cell->setShowBorder (true);
        setAreaMetallicForCell (*cell, MetallicKind::Reverb);

        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));

        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pid, c));
    };

    // ------- Row 1 -------
    makeToggleCell (enabled, "ENABLE", ReverbParamIDs::enabled);
    makeCell (pre,  preV,  "PRE",       ReverbParamIDs::preDelayMs);
    makeCell (erL,  erLV,  "ER LVL",    ReverbParamIDs::erLevelDb);
    makeCell (erD,  erDV,  "ER DEN",    ReverbParamIDs::erDensityPct);
    makeCell (erW,  erWV,  "ER WID",    ReverbParamIDs::erWidthPct);
    makeCell (erTime, erTimeV, "ER TIME", ReverbParamIDs::erTimeMs);
    makeCell (erToTail, erToTailV, "ER→T", ReverbParamIDs::erToTailPct);
    makeCell (dif,  difV,  "DIFF",      ReverbParamIDs::diffusionPct);
    makeCell (density, densityV, "DENS", ReverbParamIDs::densityPct);
    makeCell (md,   mdV,   "MOD DEP",   ReverbParamIDs::modDepthCents);
    makeCell (mr,   mrV,   "MOD RATE",  ReverbParamIDs::modRateHz);
    makeCell (w,    wV,    "WIDTH",     ReverbParamIDs::widthPct);
    makeCell (rotation, rotationV, "ROT", ReverbParamIDs::rotationDeg);
    makeCell (size, sizeV, "SIZE",      ReverbParamIDs::sizePct);
    makeCell (dec,  decV,  "DECAY",     ReverbParamIDs::decaySec);
    makeToggleCell (killDry, "WET ONLY", ReverbParamIDs::killDry);

    // ------- Row 2 -------
    makeCell (wet,  wetV,  "WET",       ReverbParamIDs::wetMix01);
    makeCell (bloom,bloomV,"BLOOM",     ReverbParamIDs::bloomPct);
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

    // Grid order: Row 1 then Row 2
    auto push = [&](juce::Component* c){ gridOrder.push_back (c); };

    // Row 1
    push (ownedSwitches[0].get()); // ENABLE
    push (ownedCells[0].get());    // PRE
    push (ownedCells[1].get());    // ER LVL
    push (ownedCells[2].get());    // ER DEN
    push (ownedCells[3].get());    // ER WID
    push (ownedCells[4].get());    // ER TIME
    push (ownedCells[5].get());    // ER→T
    push (ownedCells[6].get());    // DIFF
    push (ownedCells[7].get());    // DENS
    push (ownedCells[8].get());    // MOD DEP
    push (ownedCells[9].get());    // MOD RATE
    push (ownedCells[10].get());   // WIDTH
    push (ownedCells[11].get());   // ROT
    push (ownedCells[12].get());   // SIZE
    push (ownedCells[13].get());   // DECAY
    push (ownedSwitches[1].get()); // WET ONLY

    // Row 2
    push (ownedCells[14].get());   // WET
    push (ownedCells[15].get());   // BLOOM
    push (ownedCells[16].get());   // DIST
    push (ownedSwitches[2].get()); // FREEZE
    push (ownedCells[17].get());   // SHIM AMT
    push (ownedCells[18].get());   // SHIM INT
    push (ownedCells[19].get());   // GATE
    push (ownedCells[20].get());   // DR XO LO
    push (ownedCells[21].get());   // DR XO HI
    push (ownedSwitches[3].get()); // EQ APPLY
    push (ownedSwitches[4].get()); // FOLLOW W
    push (ownedCells[22].get());   // W AMT
    push (ownedSwitches[5].get()); // FOLLOW R
    push (ownedCells[23].get());   // R AMT
    push (ownedCells[24].get());   // TRIM
    push (ownedSwitches[6].get()); // DUCK

    // Pad to exactly 32 with blank KnobCells
    const int totalSlots   = 32;
    const int placed       = (int) gridOrder.size();
    const int blanksNeeded = juce::jmax (0, totalSlots - placed);

    for (int i = 0; i < blanksNeeded; ++i)
    {
        auto sl = std::make_unique<juce::Slider>();
        auto lb = std::make_unique<juce::Label>(); lb->setVisible (false);
        styleKnob (*sl);

        auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        cell->setShowKnob (false);
        setAreaMetallicForCell (*cell, MetallicKind::Reverb);
        cell->setShowBorder (true);
        cell->getProperties().set("reverbMaroonBorder", true);

        addAndMakeVisible (*cell);

        knobCells.emplace_back (cell.get());
        blankSliders.emplace_back (std::move (sl));
        blankLabels.emplace_back (std::move (lb));
        ownedCells.emplace_back (std::move (cell));

        gridOrder.push_back (ownedCells.back().get());
    }
}

void ReverbControlsPane::applyMetricsToAll()
{
    for (auto* c : knobCells)
    {
        if (c == nullptr) continue;
        c->setMetrics (knobPx, valuePx, labelGapPx);
        c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        c->setValueLabelGap (labelGapPx);
    }
    for (auto* c : switchCells)
    {
        if (c == nullptr) continue;
        c->setMetrics (knobPx, valuePx, labelGapPx);
    }
}