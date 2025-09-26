#include "PhaseTab.h"

void PhaseTab::resized()
{
    auto r = getLocalBounds();
    auto m = ControlGridMetrics::compute (r.getWidth(), r.getHeight());
    
        // Apply metrics to all controls
        for (auto* c : knobCells)
        {
            if (!c) continue;
            c->setMetrics (m.knobPx, m.valuePx, m.labelGapPx);
            c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            c->setValueLabelGap (m.labelGapPx);
        }
        
        // SimpleSwitchCell doesn't need metrics, but we can set them if needed
        for (auto* c : switchCells)
        {
            if (!c) continue;
            // SimpleSwitchCell handles its own sizing
        }
    
    // Layout 2x16 grid
    const int cols = 16, rows = 2;
    const int cellW = m.colW;
    const int cellH = m.rowH;
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

void PhaseTab::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Use theme colors like other components
    if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
    {
        g.setColour(lf->theme.panel);
        g.fillRoundedRectangle(bounds, 6.0f);
        
        // Standard accent border treatment
        g.setColour(lf->theme.accent.withAlpha(0.3f));
        g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
    }
    else
    {
        // Fallback for non-theme look and feel
        g.setColour(juce::Colours::darkgrey);
        g.fillRoundedRectangle(bounds, 6.0f);
    }
    
    // Draw title
    g.setColour (juce::Colours::white);
    g.setFont (juce::Font (24.0f, juce::Font::bold));
    g.drawText ("Phase Alignment", getLocalBounds().removeFromTop (50), juce::Justification::centred);
}

void PhaseTab::styleKnob (juce::Slider& k)
{
    k.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    k.setRotaryParameters (juce::MathConstants<float>::pi,
                           juce::MathConstants<float>::pi + juce::MathConstants<float>::twoPi,
                           true);
}

void PhaseTab::makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
{
    // Safety check: ensure parameter exists before creating attachment
    if (pid == nullptr || proc.apvts.getParameter(juce::String(pid)) == nullptr)
    {
        return;
    }
    
    styleKnob (s); s.setName (cap);
    auto cell = std::make_unique<KnobCell> (s, v, cap);
    cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
    cell->setValueLabelGap (labelGapPx);
    // Phase tab styling: metallic blue
    cell->getProperties().set ("metallic", true);
    cell->getProperties().set ("phaseMetallic", true);
    addAndMakeVisible (*cell);
    knobCells.emplace_back (cell.get());
    ownedCells.emplace_back (std::move (cell));
    sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (proc.apvts, pid, s));

    auto applyLabel = [&]()
    {
        int decimals = 2;
        juce::String id (pid);
        if (id.containsIgnoreCase ("_hz")) decimals = 0;
        else if (id.containsIgnoreCase ("_deg")) decimals = 1;
        else if (id.containsIgnoreCase ("_db")) decimals = 1;
        else if (id.containsIgnoreCase ("_ms")) decimals = 2;
        else if (id.containsIgnoreCase ("_q")) decimals = 2;
        v.setText (juce::String (s.getValue(), decimals), juce::dontSendNotification);
    };
    applyLabel();
    s.onValueChange = [&, applyLabel]() { applyLabel(); };
}

void PhaseTab::makeComboCell (juce::ComboBox& c, const juce::String& cap, const char* pid)
{
    if (pid == nullptr || proc.apvts.getParameter(juce::String(pid)) == nullptr)
    {
        return;
    }
    
    c.setName (cap);
    auto cell = std::make_unique<SimpleSwitchCell> (c);
    cell->setCaption (cap);
    cell->getProperties().set ("metallic", true);
    cell->getProperties().set ("phaseMetallic", true);
    addAndMakeVisible (*cell);
    switchCells.emplace_back (cell.get());
    ownedSwitches.emplace_back (std::move (cell));
    comboAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (proc.apvts, pid, c));
}

void PhaseTab::makeSwitchCell (juce::ToggleButton& t, const juce::String& cap, const char* pid)
{
    if (pid == nullptr || proc.apvts.getParameter(juce::String(pid)) == nullptr)
    {
        return;
    }
    
    t.setName (cap);
    auto cell = std::make_unique<SimpleSwitchCell> (t);
    cell->setCaption (cap);
    cell->getProperties().set ("metallic", true);
    cell->getProperties().set ("phaseMetallic", true);
    addAndMakeVisible (*cell);
    switchCells.emplace_back (cell.get());
    ownedSwitches.emplace_back (std::move (cell));
    buttonAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (proc.apvts, pid, t));
}

void PhaseTab::buildControls()
{
    // Row 1 — Routing, Capture, Time, Safety (16 controls)
    makeComboCell (refSourceCombo, "Ref Source", IDs::phase_ref_source);
    makeComboCell (channelModeCombo, "Channel Mode", IDs::phase_channel_mode);
    makeSwitchCell (followXOSwitch, "Follow XO", IDs::phase_follow_xo);
    makeComboCell (captureCombo, "Capture", IDs::phase_capture_len);
    makeComboCell (alignModeCombo, "Align Mode", IDs::phase_align_mode);
    makeComboCell (alignGoalCombo, "Align Goal", IDs::phase_align_goal);
    makeSwitchCell (polarityASwitch, "Polarity A", IDs::phase_polarity_a);
    makeSwitchCell (polarityBSwitch, "Polarity B", IDs::phase_polarity_b);
    makeCell (delayCoarseKnob, delayCoarseLabel, "Delay Coarse", IDs::phase_delay_ms_coarse);
    makeCell (delayFineKnob, delayFineLabel, "Delay Fine", IDs::phase_delay_ms_fine);
    makeComboCell (unitsCombo, "Units", IDs::phase_delay_units);
    makeComboCell (linkCombo, "Link", IDs::phase_link_mode);
    makeComboCell (engineCombo, "Engine", IDs::phase_engine);
    makeCell (latencyKnob, latencyLabel, "Latency", IDs::phase_latency_ro);
    makeComboCell (resetCombo, "Reset", IDs::phase_reset_cmd);
    makeSwitchCell (commitSwitch, "Commit", IDs::phase_commit_cmd);
    
    // Row 2 — Per-Band Phase (16 controls)
    makeCell (xoLowKnob, xoLowLabel, "XO Low", IDs::phase_xo_lo_hz);
    makeCell (xoHighKnob, xoHighLabel, "XO High", IDs::phase_xo_hi_hz);
    makeCell (lowAPKnob, lowAPLabel, "Low AP°", IDs::phase_lo_ap_deg);
    makeCell (lowQKnob, lowQLabel, "Low Q", IDs::phase_lo_q);
    makeCell (midAPKnob, midAPLabel, "Mid AP°", IDs::phase_mid_ap_deg);
    makeCell (midQKnob, midQLabel, "Mid Q", IDs::phase_mid_q);
    makeCell (highAPKnob, highAPLabel, "High AP°", IDs::phase_hi_ap_deg);
    makeCell (highQKnob, highQLabel, "High Q", IDs::phase_hi_q);
    makeComboCell (firLengthCombo, "FIR Length", IDs::phase_fir_len);
    makeComboCell (dynamicPhaseCombo, "Dynamic", IDs::phase_dynamic_mode);
    makeComboCell (monitorCombo, "Monitor", IDs::phase_monitor_mode);
    makeComboCell (metricCombo, "Metric", IDs::phase_metric_mode);
    makeComboCell (auditionBlendCombo, "Audition", IDs::phase_audition_blend);
    makeCell (trimKnob, trimLabel, "Trim", IDs::phase_trim_db);
    makeSwitchCell (phaseRecSwitch, "Phase Rec", IDs::phase_rec_enable);
    makeSwitchCell (applyOnLoadSwitch, "Apply @ Load", IDs::phase_apply_on_load);

        // Build grid order - mix of KnobCell and SimpleSwitchCell
        auto push = [&](juce::Component* c){ gridOrder.push_back (c); };
        
        // Row 1: Routing, Capture, Time, Safety
        push (ownedSwitches[0].get());  // Ref Source (ComboBox)
        push (ownedSwitches[1].get());  // Channel Mode (ComboBox)
        push (ownedSwitches[2].get());  // Follow XO (ToggleButton)
        push (ownedSwitches[3].get());  // Capture (ComboBox)
        push (ownedSwitches[4].get());  // Align Mode (ComboBox)
        push (ownedSwitches[5].get());  // Align Goal (ComboBox)
        push (ownedSwitches[6].get());  // Polarity A (ToggleButton)
        push (ownedSwitches[7].get());  // Polarity B (ToggleButton)
        push (ownedCells[0].get());     // Delay Coarse (Slider)
        push (ownedCells[1].get());     // Delay Fine (Slider)
        push (ownedSwitches[8].get());  // Units (ComboBox)
        push (ownedSwitches[9].get());  // Link (ComboBox)
        push (ownedSwitches[10].get()); // Engine (ComboBox)
        push (ownedCells[2].get());     // Latency (Slider)
        push (ownedSwitches[11].get()); // Reset (ComboBox)
        push (ownedSwitches[12].get()); // Commit (ToggleButton)
        
        // Row 2: Per-Band Phase
        push (ownedCells[3].get());     // XO Low (Slider)
        push (ownedCells[4].get());     // XO High (Slider)
        push (ownedCells[5].get());     // Low AP° (Slider)
        push (ownedCells[6].get());     // Low Q (Slider)
        push (ownedCells[7].get());     // Mid AP° (Slider)
        push (ownedCells[8].get());     // Mid Q (Slider)
        push (ownedCells[9].get());     // High AP° (Slider)
        push (ownedCells[10].get());    // High Q (Slider)
        push (ownedSwitches[13].get()); // FIR Length (ComboBox)
        push (ownedSwitches[14].get()); // Dynamic (ComboBox)
        push (ownedSwitches[15].get()); // Monitor (ComboBox)
        push (ownedSwitches[16].get()); // Metric (ComboBox)
        push (ownedSwitches[17].get()); // Audition (ComboBox)
        push (ownedCells[11].get());    // Trim (Slider)
        push (ownedSwitches[18].get()); // Phase Rec (ToggleButton)
        push (ownedSwitches[19].get()); // Apply @ Load (ToggleButton)
}

void PhaseTab::applyMetricsToAll()
{
    for (auto* c : knobCells)
    {
        if (!c) continue;
        c->setMetrics (knobPx, valuePx, labelGapPx);
        c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        c->setValueLabelGap (labelGapPx);
    }
    
    // SimpleSwitchCell doesn't need metrics
    for (auto* c : switchCells)
    {
        if (!c) continue;
        // SimpleSwitchCell handles its own sizing
    }
}
