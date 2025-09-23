#pragma once

#include <JuceHeader.h>
#include "../../KnobCell.h"
#include "../../Layout.h"

// DelayControlsPane: 2x16 flat grid container for Delay controls.
// Scaffolding-only: initially populated with styled empty KnobCells.
class DelayControlsPane : public juce::Component
{
public:
    explicit DelayControlsPane (juce::AudioProcessorValueTreeState& state)
        : apvts (state)
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

        // Create switches and combos first
        addAndMakeVisible (delayEnabled);   delayEnabled.setComponentID ("delayEnabled");
        addAndMakeVisible (delaySync);
        addAndMakeVisible (delayKillDry);
        addAndMakeVisible (delayFreeze);
        addAndMakeVisible (delayPingpong);
        addAndMakeVisible (delayDuckPost);
        addAndMakeVisible (delayMode);
        addAndMakeVisible (delayGridFlavor);
        addAndMakeVisible (delayDuckSource);
        addAndMakeVisible (delayFilterType);

        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_enabled",      delayEnabled));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "delay_mode",       delayMode));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_sync",         delaySync));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "delay_grid_flavor", delayGridFlavor));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_kill_dry",    delayKillDry));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_freeze",      delayFreeze));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_pingpong",    delayPingpong));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "delay_duck_source", delayDuckSource));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, "delay_duck_post",    delayDuckPost));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, "delay_filter_type", delayFilterType));

        // Knobs + cells
        auto makeCell = [&](juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
        {
            styleKnob (s);
            s.setName (cap);
            auto cell = std::make_unique<KnobCell> (s, v, cap);
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            // Styling: Delay metallic (light yellowish-green) + border
            cell->getProperties().set ("delayThemeBorderTextGrey", true);
            cell->getProperties().set ("metallic", true);
            cell->getProperties().set ("delayMetallic", true);
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
                else if (id.containsIgnoreCase ("_pct")) decimals = 0;
                else if (id.containsIgnoreCase ("_ms")) decimals = 0;
                else if (id.containsIgnoreCase ("_sec")) decimals = 2;
                v.setText (juce::String (s.getValue(), decimals), juce::dontSendNotification);
            };
            applyLabel();
            s.onValueChange = [&, applyLabel]() { applyLabel(); };
        };

        // Row A tail (TIME..PRE)
        makeCell (delayTime,      delayTimeValue,      "TIME",  "delay_time_ms");
        makeCell (delayFeedback,  delayFeedbackValue,  "FB",    "delay_feedback_pct");
        makeCell (delayWet,       delayWetValue,       "WET",   "delay_wet");
        makeCell (delayModRate,   delayModRateValue,   "RATE",  "delay_mod_rate_hz");
        makeCell (delayModDepth,  delayModDepthValue,  "DEPTH", "delay_mod_depth_ms");
        makeCell (delaySpread,    delaySpreadValue,    "SPREAD","delay_crossfeed_pct");
        makeCell (delayWidth,     delayWidthValue,     "WIDTH", "delay_width");
        makeCell (delayPreDelay,  delayPreDelayValue,  "PRE",   "delay_pre_delay_ms");

        // Row B head (SAT..JITTER)
        makeCell (delaySat,       delaySatValue,       "SAT",   "delay_sat");
        makeCell (delayDiffusion, delayDiffusionValue, "DIFF",  "delay_diffusion");
        makeCell (delayDiffuseSize, delayDiffuseSizeValue, "SIZE", "delay_diffuse_size_ms");
        makeCell (delayHp,        delayHpValue,        "HP",    "delay_hp_hz");
        makeCell (delayLp,        delayLpValue,        "LP",    "delay_lp_hz");
        makeCell (delayTilt,      delayTiltValue,      "TILT",  "delay_tilt_db");
        makeCell (delayWowflutter,delayWowflutterValue,"WOW",   "delay_wowflutter");
        makeCell (delayJitter,    delayJitterValue,    "JITTER","delay_jitter_pct");

        // Row B tail (Duck cluster)
        makeCell (delayDuckThreshold, delayDuckThresholdValue, "THR",  "delay_duck_threshold_db");
        makeCell (delayDuckDepth,     delayDuckDepthValue,     "DEPTH","delay_duck_depth");
        makeCell (delayDuckAttack,    delayDuckAttackValue,    "ATT",  "delay_duck_attack_ms");
        makeCell (delayDuckRelease,   delayDuckReleaseValue,   "REL",  "delay_duck_release_ms");
        makeCell (delayDuckLookahead, delayDuckLookaheadValue, "LA",   "delay_duck_lookahead_ms");
        makeCell (delayDuckRatio,     delayDuckRatioValue,     "RAT",  "delay_duck_ratio");

        // Compose 2x16 grid order (Row A 1..16, Row B 17..32)
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        // Row A (switch/combos then knobs)
        push (&delayEnabled);
        push (&delayMode);
        push (&delaySync);
        push (&delayGridFlavor);
        push (&delayPingpong);
        push (&delayFreeze);
        push (&delayFilterType);
        push (&delayKillDry);
        push (ownedCells[0].get()); // TIME
        push (ownedCells[1].get()); // FB
        push (ownedCells[2].get()); // WET
        push (ownedCells[3].get()); // RATE
        push (ownedCells[4].get()); // DEPTH
        push (ownedCells[5].get()); // SPREAD
        push (ownedCells[6].get()); // WIDTH
        push (ownedCells[7].get()); // PRE
        // Row B
        push (ownedCells[8].get()); // SAT
        push (ownedCells[9].get()); // DIFF
        push (ownedCells[10].get()); // SIZE
        push (ownedCells[11].get()); // HP
        push (ownedCells[12].get()); // LP
        push (ownedCells[13].get()); // TILT
        push (ownedCells[14].get()); // WOW
        push (ownedCells[15].get()); // JITTER
        push (&delayDuckSource);
        push (&delayDuckPost);
        push (ownedCells[16].get()); // THR
        push (ownedCells[17].get()); // DEPTH
        push (ownedCells[18].get()); // ATT
        push (ownedCells[19].get()); // REL
        push (ownedCells[20].get()); // LA
        push (ownedCells[21].get()); // RAT

        // Fill blanks up to 32 with styled Delay blanks
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
            cell->getProperties().set ("delayMetallic", true);
            cell->getProperties().set ("delayThemeBorderTextGrey", true);
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
            if (c == nullptr) continue;
            c->setMetrics (knobPx, valuePx, labelGapPx);
            c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            c->setValueLabelGap (labelGapPx);
        }
    }

    juce::AudioProcessorValueTreeState& apvts;

    // Controls
    juce::ToggleButton delayEnabled, delaySync, delayKillDry, delayFreeze, delayPingpong, delayDuckPost;
    juce::ComboBox delayMode, delayTimeDiv, delayDuckSource, delayGridFlavor, delayFilterType;

    // Sliders + labels
    juce::Slider delayTime, delayFeedback, delayWet, delaySpread, delayWidth, delayModRate, delayModDepth, delayWowflutter, delayJitter, delayPreDelay;
    juce::Slider delayHp, delayLp, delayTilt, delaySat, delayDiffusion, delayDiffuseSize;
    juce::Slider delayDuckDepth, delayDuckAttack, delayDuckRelease, delayDuckThreshold, delayDuckRatio, delayDuckLookahead;
    juce::Label delayTimeValue, delayFeedbackValue, delayWetValue, delaySpreadValue, delayWidthValue, delayModRateValue, delayModDepthValue, delayWowflutterValue, delayJitterValue, delayPreDelayValue;
    juce::Label delayHpValue, delayLpValue, delayTiltValue, delaySatValue, delayDiffusionValue, delayDiffuseSizeValue;
    juce::Label delayDuckDepthValue, delayDuckAttackValue, delayDuckReleaseValue, delayDuckThresholdValue, delayDuckRatioValue, delayDuckLookaheadValue;

    // Attachments
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;

    // Knob cells
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;

    // Metrics
    int knobPx     = 48;
    int valuePx    = 14;
    int labelGapPx = 4;
    int colW       = 56;
    int rowH       = 0;
};


