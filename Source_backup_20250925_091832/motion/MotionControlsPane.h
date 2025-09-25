#pragma once

#include <JuceHeader.h>
#include "MotionIDs.h"
#include "../KnobCell.h"
#include "../Layout.h"

// MotionControlsPane: 2x16 grid for Motion controls (24 + blanks)
class MotionControlsPane : public juce::Component
{
public:
    explicit MotionControlsPane (juce::AudioProcessorValueTreeState& s)
        : apvts (s)
    {
        buildControls();
        applyMetricsToAll();
    }

    void setCellMetrics (int knobDiameterPx, int valueBandPx, int labelGapPxIn, int columnWidthPx)
    {
        knobPx     = juce::jmax (24, knobDiameterPx);
        valuePx    = juce::jmax (10, valueBandPx);
        labelGapPx = juce::jmax (0,  labelGapPxIn);
        colW       = juce::jmax (knobPx, columnWidthPx);
        applyMetricsToAll();
        resized();
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
        // Motion metallic styling
        cell->getProperties().set ("metallic", true);
        cell->getProperties().set ("motionPurpleBorder", true);
        cell->getProperties().set ("caption", cap);
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));
        // Initialize value label
        v.setInterceptsMouseClicks (false, false);
        v.setJustificationType (juce::Justification::centred);
        v.setText (juce::String (s.getValue(), 2), juce::dontSendNotification);
    }

    void buildControls()
    {
        using namespace motion;
        using namespace motion::id;

        // Combos/buttons
        addAndMakeVisible (panner);   panner.addItemList (choiceListPanner(), 1);  cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, panner_select, panner));
        addAndMakeVisible (path);     path.addItemList (choiceListPath(), 1);      cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, p1_path, path));
        addAndMakeVisible (mode);     mode.addItemList (choiceListMode(), 1);      cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, p1_mode, mode));
        addAndMakeVisible (quant);    quant.addItemList (choiceListQuant(), 1);    cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, p1_quantize_div, quant));
        addAndMakeVisible (enableBtn); btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, enable, enableBtn));
        addAndMakeVisible (retrigBtn); btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, p1_retrig, retrigBtn));
        addAndMakeVisible (anchorBtn); btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, anchor_enable, anchorBtn));

        // Knobs
        makeCell (rate,   rateV,   "RATE",   p1_rate_hz);
        makeCell (depth,  depthV,  "DEPTH",  p1_depth_pct);
        makeCell (phase,  phaseV,  "PHASE",  p1_phase_deg);
        makeCell (spread, spreadV, "SPREAD", p1_spread_pct);
        makeCell (elev,   elevV,   "ELEV",   p1_elev_bias);
        makeCell (bounce, bounceV, "BOUNCE", p1_shape_bounce);
        makeCell (jitter, jitterV, "JITTER", p1_jitter_amt);
        makeCell (swing,  swingV,  "SWING",  p1_swing_pct);
        makeCell (hold,   holdV,   "HOLD",   p1_hold_ms);
        makeCell (sens,   sensV,   "SENS",   p1_sens);
        makeCell (inertia,inertiaV,"INERTIA", p1_inertia_ms);
        makeCell (front,  frontV,  "FRONT",  p1_front_bias);
        makeCell (doppler,dopplerV,"DOPPLER",p1_doppler_amt);
        makeCell (send,   sendV,   "SEND",   p1_motion_send);
        makeCell (bass,   bassV,   "BASS",   bass_floor_hz);
        makeCell (occl,   occlV,   "OCCL",   occlusion);

        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row A (16): Enable, Panner, Path, Rate, Depth, Phase, Spread, ELEV, Bounce, Jitter, Quant, Swing, Mode, Retrig, Hold, Sens
        push (&enableBtn); push (&panner); push (&path);
        push (ownedCells[0].get()); push (ownedCells[1].get()); push (ownedCells[2].get()); push (ownedCells[3].get());
        push (ownedCells[4].get()); push (ownedCells[5].get()); push (ownedCells[6].get());
        push (&quant); push (ownedCells[7].get()); push (&mode); push (&retrigBtn); push (ownedCells[8].get()); push (ownedCells[9].get());
        // Row B (next 16): Offset(not separate), Inertia, Front, Doppler, Send, Anchor, Bass, Occl + 8 blanks
        push (ownedCells[10].get()); // Inertia
        push (ownedCells[11].get()); // Front
        push (ownedCells[12].get()); // Doppler
        push (ownedCells[13].get()); // Send
        push (&anchorBtn);
        push (ownedCells[14].get()); // Bass
        push (ownedCells[15].get()); // Occl
        // Fill remaining with nullptrs (styled blanks can be added later if needed)
        for (int i = 0; i < 9; ++i) gridOrder.push_back (nullptr);
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

    // Controls
    juce::ComboBox panner, path, mode, quant;
    juce::ToggleButton enableBtn, retrigBtn, anchorBtn;

    // Knobs + labels (subset mapped to P1 + globals)
    juce::Slider rate, depth, phase, spread, elev, bounce, jitter, swing, hold, sens, inertia, front, doppler, send, bass, occl;
    juce::Label  rateV, depthV, phaseV, spreadV, elevV, bounceV, jitterV, swingV, holdV, sensV, inertiaV, frontV, dopplerV, sendV, bassV, occlV;

    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;

    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<juce::Component*> gridOrder;

    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;
};


