#pragma once

#include <JuceHeader.h>
#include "../KnobCell.h"
#include "SimpleSwitchCell.h"
#include "../Layout.h"

// XYControlsPane: 2x16 grid for EQ/Center controls shown with the XY visuals
class XYControlsPane : public juce::Component
{
public:
    explicit XYControlsPane (juce::AudioProcessorValueTreeState& s)
        : apvts (s) { buildControls(); applyMetricsToAll(); }

    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
    {
        knobPx     = juce::jmax (24, knobDiameterPx);
        valuePx    = juce::jmax (10, valueBandPx);
        labelGapPx = juce::jmax (0,  labelGapPxIn);
        colW       = juce::jmax (knobPx, columnWidthPx);
        applyMetricsToAll(); resized(); this->juce::Component::repaint();
    }
    void setRowHeightPx (int px) { rowH = juce::jmax (1, px); resized(); this->juce::Component::repaint(); }

    void resized() override
    {
        auto r = this->juce::Component::getLocalBounds();
        const int cols = 16, rows = 2;
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
        const int minRowH = knobPx + labelGapPx + valuePx;
        const int autoH   = juce::jmax (1, r.getHeight() / rows);
        const int cellH   = juce::jmax (rowH > 0 ? rowH : autoH, minRowH);
        const int totalW = cellW * cols, totalH = cellH * rows;
        const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);
        auto place = [&](int index, int row, int col)
        {
            if (index < 0 || index >= (int) gridOrder.size()) return;
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
    void makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid, bool metallic=false)
    {
        styleKnob (s); s.setName (cap);
        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        if (metallic) cell->getProperties().set ("metallic", true);
        // Center/XY knobs: give a subtle metallic gradient hint
        cell->getProperties().set ("centerStyle", true);
        // Ensure caption renders via KnobCell paint helper
        cell->getProperties().set ("caption", cap);
        this->juce::Component::addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));
        s.onValueChange = [this, &s, &v]() { v.setText (juce::String (s.getValue(), 2), juce::dontSendNotification); };
        // Initialize value label immediately
        v.setInterceptsMouseClicks (false, false);
        v.setJustificationType (juce::Justification::centred);
        v.setText (juce::String (s.getValue(), 2), juce::dontSendNotification);
    }
    void makeSwitch (juce::Button& b, const juce::String& cap, const char* pid, bool metallic=false)
    {
        b.setButtonText (cap);
        auto cell = std::make_unique<SimpleSwitchCell> (b);
        cell->setCaption (cap);
        if (metallic) cell->getProperties().set ("metallic", true);
        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid, b));
    }
    void makeCombo (juce::ComboBox& c, const juce::String& cap, const char* pid, bool metallic=false)
    {
        auto cell = std::make_unique<SimpleSwitchCell> (c);
        cell->setCaption (cap);
        if (metallic) cell->getProperties().set ("metallic", true);
        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pid, c));
    }

    void buildControls()
    {
        // Row A: BASS, HP, LP, Q, Q LINK, AIR, TILT, SCOOP, SHELF SHAPE, XO LO, XO HI, [5 empty]
        const bool Mgrey = true; // all XY controls use grey metallic styling
        makeCell (bass,   bassV,   "BASS",   "bass_db",        Mgrey);
        makeCell (hp,     hpV,     "HP",     "hp_hz",          Mgrey);
        makeCell (lp,     lpV,     "LP",     "lp_hz",          Mgrey);
        makeCell (q,      qV,      "Q",      "eq_filter_q",    Mgrey);
        makeSwitch (qLink,         "Q LINK", "eq_q_link",      Mgrey);
        makeCell (air,    airV,    "AIR",    "air_db",         Mgrey);
        makeCell (tilt,   tiltV,   "TILT",   "tilt",           Mgrey);
        makeCell (scoop,  scoopV,  "SCOOP",  "scoop",          Mgrey);
        makeCell (shelfS, shelfSV, "S",      "eq_shelf_shape", Mgrey);
        // Move imaging crossover/placement from Imager to XY
        makeCell (xoLo,   xoLoV,   "XO LO",  "xover_lo_hz",    Mgrey);
        makeCell (xoHi,   xoHiV,   "XO HI",  "xover_hi_hz",    Mgrey);

        // Additional imaging/placement controls moved from Imager to XY
        makeCell (rotation, rotationV, "ROT",      "rotation_deg",     Mgrey);
        makeCell (asym,     asymV,     "ASYM",     "asymmetry",        Mgrey);
        makeCell (shufLo,   shufLoV,   "SHUF LO",  "shuffler_lo_pct",  Mgrey);
        makeCell (shufHi,   shufHiV,   "SHUF HI",  "shuffler_hi_pct",  Mgrey);
        makeCell (shufX,    shufXV,    "SHUF XO",  "shuffler_xover_hz",Mgrey);
        makeCell (monoHz,   monoV,     "MONO",     "mono_hz",          Mgrey);
        makeCell (pan,      panV,      "PAN",      "pan",               Mgrey);
        makeCell (satMix,   satMixV,   "SAT MIX",  "sat_mix",           Mgrey);
        for (int i = 0; i < 5; ++i) gridOrder.push_back (nullptr);

        // Row B: Center tools (metallic)
        const bool M = true;
        makeCell (punchAmt, punchAmtV, "PUNCH",    "center_punch_amt", M);
        makeCombo (punchMode,          "PUNCH MODE","center_punch_mode", M);
        makeSwitch (phaseRecOn,        "PHASE REC", "center_phase_rec_on", M);
        makeCell (phaseAmt, phaseAmtV, "PHASE",    "center_phase_rec_amt", M);
        makeSwitch (centerLockOn,      "CNTR LOCK", "center_lock_on", M);
        makeCell (promDb,   promDbV,   "CNTR",     "center_prom_db", M);
        makeCell (focusLo,  focusLoV,  "LO",       "center_f_lo_hz", M);
        makeCell (focusHi,  focusHiV,  "HI",       "center_f_hi_hz", M);
        for (int i = 0; i < 8; ++i) gridOrder.push_back (nullptr);

        // Compose order explicitly for first row (11 controls + 5 blanks)
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Rebuild first row order matching mapping
        gridOrder.clear();
        push (ownedCells[0].get());  // BASS
        push (ownedCells[1].get());  // HP
        push (ownedCells[2].get());  // LP
        push (ownedCells[3].get());  // Q
        push (ownedSwitches[0].get()); // Q LINK
        push (ownedCells[4].get());  // AIR
        push (ownedCells[5].get());  // TILT
        push (ownedCells[6].get());  // SCOOP
        push (ownedCells[7].get());  // S (Shelf Shape)
        push (ownedCells[8].get());  // XO LO
        push (ownedCells[9].get());  // XO HI
        for (int i = 0; i < 5; ++i) push (nullptr);
        // Second row mapping (center tools first, then imaging controls moved from Imager)
        push (ownedCells[10].get()); // PUNCH AMT
        push (ownedSwitches[1].get()); // PUNCH MODE
        push (ownedSwitches[2].get()); // PHASE REC
        push (ownedCells[11].get()); // PHASE AMT
        push (ownedSwitches[3].get()); // CNTR LOCK
        push (ownedCells[12].get()); // PROM
        push (ownedCells[13].get()); // FOCUS LO
        push (ownedCells[14].get()); // FOCUS HI
        // Append: ROT, ASYM, SHUF LO, SHUF HI, SHUF XO, MONO, PAN, SAT MIX
        push (ownedCells[15].get()); // ROT
        push (ownedCells[16].get()); // ASYM
        push (ownedCells[17].get()); // SHUF LO
        push (ownedCells[18].get()); // SHUF HI
        push (ownedCells[19].get()); // SHUF XO
        push (ownedCells[20].get()); // MONO
        push (ownedCells[21].get()); // PAN
        push (ownedCells[22].get()); // SAT MIX

        // Fill existing nullptr slots with styled XY blanks (metallic grey)
        const int totalNeeded = 32;
        if ((int) gridOrder.size() < totalNeeded) gridOrder.resize (totalNeeded, nullptr);
        for (int i = 0; i < totalNeeded; ++i)
        {
            if (gridOrder[(size_t) i] == nullptr)
            {
                auto sl = std::make_unique<juce::Slider>();
                auto lb = std::make_unique<juce::Label>(); lb->setVisible (false);
                styleKnob (*sl);
                auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
                cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                cell->setValueLabelGap (labelGapPx);
                cell->setShowKnob (false);
                cell->getProperties().set ("metallic", true); // metallic grey default for XY
                addAndMakeVisible (*cell);
                knobCells.emplace_back (cell.get());
                blankSliders.emplace_back (std::move (sl));
                blankLabels.emplace_back (std::move (lb));
                ownedCells.emplace_back (std::move (cell));
                gridOrder[(size_t) i] = ownedCells.back().get();
            }
        }
    }

    void applyMetricsToAll()
    {
        for (auto* c : knobCells)
            if (c) { c->setMetrics (knobPx, valuePx, labelGapPx); c->setValueLabelMode (KnobCell::ValueLabelMode::Managed); c->setValueLabelGap (labelGapPx); }
        for (auto* s : switchCells)
            if (s) s->setShowBorder (true);
    }

    juce::AudioProcessorValueTreeState& apvts;
    // EQ row + imaging controls moved from Imager
    juce::Slider bass, hp, lp, q, air, tilt, scoop, shelfS, xoLo, xoHi;
    juce::Slider rotation, asym, shufLo, shufHi, shufX, monoHz, pan, satMix;
    juce::Label  bassV, hpV, lpV, qV, airV, tiltV, scoopV, shelfSV, xoLoV, xoHiV;
    juce::Label  rotationV, asymV, shufLoV, shufHiV, shufXV, monoV, panV, satMixV;
    juce::ToggleButton qLink;
    // Center row
    juce::Slider punchAmt, phaseAmt, promDb, focusLo, focusHi;
    juce::Label  punchAmtV, phaseAmtV, promDbV, focusLoV, focusHiV;
    juce::ComboBox punchMode;
    juce::ToggleButton phaseRecOn, centerLockOn;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;

    std::vector<KnobCell*> knobCells;
    std::vector<SimpleSwitchCell*> switchCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>> ownedSwitches;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;

    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;
};


