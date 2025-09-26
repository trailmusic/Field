#pragma once

#include <JuceHeader.h>
#include "Components/KnobCell.h"
#include "SimpleSwitchCell.h"
#include "../ui/Layout.h"
#include "../Core/IconSystem.h"

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
        applyMetricsToAll(); resized(); repaint();
    }
    void setRowHeightPx (int px) { rowH = juce::jmax (1, px); resized(); repaint(); }

    void resized() override
    {
        auto r = getLocalBounds();
        const int rows = 2;
        const int minRowH = knobPx + labelGapPx + valuePx;
        const int autoH   = juce::jmax (1, r.getHeight() / rows);
        const int cellH   = juce::jmax (rowH > 0 ? rowH : autoH, minRowH);
        
        // Calculate cell width based on available space and grid layout
        const int availableWidth = r.getWidth();
        const int cellW = juce::jmax (colW, availableWidth / 16); // Base cell width
        
        // Ensure double-wide cells get proper width (minimum 120px for double-wide)
        const int doubleWideWidth = juce::jmax (120, cellW * 2);
        
        // Define grid layout with double-wide cells
        struct GridCell {
            int col;
            int width; // 1 = single, 2 = double
            juce::Component* component;
        };
        
        std::vector<GridCell> gridLayout;
        
        // Row A layout: MONO(1,2), HP(3), LP(4), BASS(5,6), AIR(7,8), TILT(9,10), SCOOP(11,12), Q+QLink(13,14), Shape(15), [1 empty]
        int col = 1;
        if (ownedCells.size() > 0) gridLayout.push_back({col, 2, ownedCells[0].get()}); col += 2; // MONO
        if (ownedCells.size() > 1) gridLayout.push_back({col, 1, ownedCells[1].get()}); col += 1; // HP
        if (ownedCells.size() > 2) gridLayout.push_back({col, 1, ownedCells[2].get()}); col += 1; // LP
        if (ownedCells.size() > 3) gridLayout.push_back({col, 2, ownedCells[3].get()}); col += 2; // BASS
        if (ownedCells.size() > 4) gridLayout.push_back({col, 2, ownedCells[4].get()}); col += 2; // AIR
        if (ownedCells.size() > 5) gridLayout.push_back({col, 2, ownedCells[5].get()}); col += 2; // TILT
        if (ownedCells.size() > 6) gridLayout.push_back({col, 2, ownedCells[6].get()}); col += 2; // SCOOP
        if (ownedCells.size() > 7) gridLayout.push_back({col, 2, ownedCells[7].get()}); col += 2; // Q+QLink
        if (ownedCells.size() > 8) gridLayout.push_back({col, 1, ownedCells[8].get()}); col += 1; // Shape
        
        // Row B layout: Center tools + imaging controls
        int rowBCol = 1;
        for (int i = 10; i < (int)ownedCells.size() && rowBCol <= 16; ++i)
        {
            gridLayout.push_back({rowBCol, 1, ownedCells[i].get()});
            rowBCol++;
        }
        
        // Place components based on grid layout
        for (const auto& cell : gridLayout)
        {
            if (cell.component)
            {
                const int x = r.getX() + (cell.col - 1) * cellW;
                const int y = r.getY() + (cell.component == ownedCells[0].get() || 
                                         cell.component == ownedCells[1].get() || cell.component == ownedCells[2].get() || 
                                         cell.component == ownedCells[3].get() || cell.component == ownedCells[4].get() || 
                                         cell.component == ownedCells[5].get() || cell.component == ownedCells[6].get() || 
                                         cell.component == ownedCells[7].get() || cell.component == ownedCells[8].get() ? 0 : 1) * cellH;
                const int width = (cell.width == 2) ? doubleWideWidth : cell.width * cellW;
                cell.component->setBounds(x, y, width, cellH);
            }
        }
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
        if (metallic) cell->getProperties().set ("metallic", true);
        // Center/XY knobs: give a subtle metallic gradient hint
        cell->getProperties().set ("centerStyle", true);
        // Ensure caption renders via KnobCell paint helper
        cell->getProperties().set ("caption", cap);
        addAndMakeVisible (*cell);
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
    
    // Helper for creating cells with frequency mini sliders (double-wide)
    void makeCellWithFreq (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid,
                          juce::Slider& freqS, juce::Label& freqV, const char* freqPid, bool metallic=false)
    {
        // Safety check: ensure parameters exist before creating attachments
        if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr) return;
        if (freqPid == nullptr || apvts.getParameter(juce::String(freqPid)) == nullptr) return;
        
        styleKnob (s); s.setName (cap);
        
        // Style frequency slider as linear horizontal
        freqS.setSliderStyle (juce::Slider::LinearHorizontal);
        freqS.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        freqS.setMouseDragSensitivity (140);
        freqS.setVelocityBasedMode (false);
        freqS.setSliderSnapsToMousePosition (false);
        freqS.setDoubleClickReturnValue (true, 0.0);
        freqS.getProperties().set ("micro", true);
        
        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        if (metallic) cell->getProperties().set ("metallic", true);
        cell->getProperties().set ("centerStyle", true);
        cell->getProperties().set ("caption", cap);
        
        // Add mini slider with label - place on the right and center vertically
        const int miniHeight = Layout::dp (12, 1.0f); // 12px mini height
        cell->setMiniWithLabel (&freqS, &freqV, miniHeight);
        cell->setMiniPlacementRight (true); // Place on the right side
        cell->setMiniThicknessPx (Layout::dp (8, 1.0f)); // 8px thickness
        
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        
        // Create attachments
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, pid, s));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, freqPid, freqS));
        
        // Initialize value labels
        s.onValueChange = [this, &s, &v]() { v.setText (juce::String (s.getValue(), 2), juce::dontSendNotification); };
        freqS.onValueChange = [this, &freqS, &freqV]() { 
            freqV.setText (juce::String (freqS.getValue(), 0) + "Hz", juce::dontSendNotification); 
        };
        
        v.setInterceptsMouseClicks (false, false);
        v.setJustificationType (juce::Justification::centred);
        v.setText (juce::String (s.getValue(), 2), juce::dontSendNotification);
        
        freqV.setInterceptsMouseClicks (false, false);
        freqV.setJustificationType (juce::Justification::centred);
        freqV.setText (juce::String (freqS.getValue(), 0) + "Hz", juce::dontSendNotification);
    }
    
    // Helper for creating Q link cell with HP/LP Q controls (double-wide)
    void makeQLinkCell (juce::Slider& qS, juce::Label& qV, const juce::String& qCap, const char* qPid,
                       juce::ToggleButton& linkB, const juce::String& linkCap, const char* linkPid,
                       juce::Slider& hpQS, juce::Label& hpQV, const char* hpQPid,
                       juce::Slider& lpQS, juce::Label& lpQV, const char* lpQPid, bool metallic=false)
    {
        // Safety check: ensure parameters exist
        if (qPid == nullptr || apvts.getParameter(juce::String(qPid)) == nullptr) return;
        if (linkPid == nullptr || apvts.getParameter(juce::String(linkPid)) == nullptr) return;
        if (hpQPid == nullptr || apvts.getParameter(juce::String(hpQPid)) == nullptr) return;
        if (lpQPid == nullptr || apvts.getParameter(juce::String(lpQPid)) == nullptr) return;
        
        styleKnob (qS); qS.setName (qCap);
        
        // Style HP/LP Q sliders as linear horizontal
        for (auto* slider : { &hpQS, &lpQS })
        {
            slider->setSliderStyle (juce::Slider::LinearHorizontal);
            slider->setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            slider->setMouseDragSensitivity (140);
            slider->setVelocityBasedMode (false);
            slider->setSliderSnapsToMousePosition (false);
            slider->setDoubleClickReturnValue (true, 0.0);
            slider->getProperties().set ("micro", true);
        }
        
        auto cell = std::make_unique<KnobCell> (qS, qV, qCap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        if (metallic) cell->getProperties().set ("metallic", true);
        cell->getProperties().set ("centerStyle", true);
        cell->getProperties().set ("caption", qCap);
        
        // Add Q link toggle and HP/LP Q sliders as aux components
        std::vector<juce::Component*> auxComponents = { &linkB, &hpQS, &lpQS };
        cell->setAuxComponents (auxComponents, Layout::dp (40, 1.0f)); // 40px aux height
        
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
        ownedCells.emplace_back (std::move (cell));
        
        // Create attachments
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, qPid, qS));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, linkPid, linkB));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, hpQPid, hpQS));
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, lpQPid, lpQS));
        
        // Initialize value labels
        qS.onValueChange = [this, &qS, &qV]() { qV.setText (juce::String (qS.getValue(), 3), juce::dontSendNotification); };
        hpQS.onValueChange = [this, &hpQS, &hpQV]() { hpQV.setText (juce::String (hpQS.getValue(), 3), juce::dontSendNotification); };
        lpQS.onValueChange = [this, &lpQS, &lpQV]() { lpQV.setText (juce::String (lpQS.getValue(), 3), juce::dontSendNotification); };
        
        qV.setInterceptsMouseClicks (false, false);
        qV.setJustificationType (juce::Justification::centred);
        qV.setText (juce::String (qS.getValue(), 3), juce::dontSendNotification);
        
        hpQV.setInterceptsMouseClicks (false, false);
        hpQV.setJustificationType (juce::Justification::centred);
        hpQV.setText (juce::String (hpQS.getValue(), 3), juce::dontSendNotification);
        
        lpQV.setInterceptsMouseClicks (false, false);
        lpQV.setJustificationType (juce::Justification::centred);
        lpQV.setText (juce::String (lpQS.getValue(), 3), juce::dontSendNotification);
        
        // Set up Q link toggle
        linkB.setButtonText (linkCap);
        linkB.setLookAndFeel (&getLookAndFeel());
    }
    
    
    // Helper for creating mono group cell (double-wide with slope switch and audition button)
    void makeMonoGroupCell (juce::Slider& monoS, juce::Label& monoV, const juce::String& monoCap, const char* monoPid,
                           juce::ComboBox& slopeC, const char* slopePid,
                           juce::ToggleButton& audB, const char* audPid, bool metallic=false)
    {
        // Safety check: ensure parameters exist before creating attachments
        if (monoPid == nullptr || apvts.getParameter(juce::String(monoPid)) == nullptr) return;
        if (slopePid == nullptr || apvts.getParameter(juce::String(slopePid)) == nullptr) return;
        if (audPid == nullptr || apvts.getParameter(juce::String(audPid)) == nullptr) return;
        
        styleKnob (monoS); monoS.setName (monoCap);
        
        // Set up slope switch and audition button
        slopeSwitch.setIndex (1); // Default to 12 dB/oct
        slopeSwitch.onChange = [this, slopePid](int idx) {
            if (auto* param = apvts.getParameter (slopePid))
                param->setValueNotifyingHost (idx / 2.0f);
        };
        
        auditionButton.setButtonText ("");
        auditionButton.setToggleState (false, juce::dontSendNotification);
        
        // Create standard KnobCell with auxiliary components (templated approach)
        auto cell = std::make_unique<KnobCell> (monoS, monoV, monoCap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        if (metallic) cell->getProperties().set ("metallic", true);
        cell->getProperties().set ("centerStyle", true);
        cell->getProperties().set ("caption", monoCap);
        
        // Add auxiliary components (slope switch and audition button) to the right side
        std::vector<juce::Component*> auxComponents = { &slopeSwitch, &auditionButton };
        cell->setAuxComponents (auxComponents, 60); // 60px height for aux area
        cell->setAuxWeights ({2.0f, 1.0f}); // Slope switch gets 2x weight, audition button gets 1x
        cell->setMiniPlacementRight (true); // Place auxiliary components on the RIGHT side, not bottom
        // Note: auxAsBars defaults to false, which gives us the natural weighted vertical stack we want
        
        addAndMakeVisible (*cell);
        ownedCells.emplace_back (std::move (cell));
        
        // Create attachments
        sAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, monoPid, monoS));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, slopePid, slopeC));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, audPid, audB));
        
        // Initialize value label
        monoS.onValueChange = [this, &monoS, &monoV]() { 
            monoV.setText (juce::String (monoS.getValue(), 0) + "Hz", juce::dontSendNotification); 
        };
        
        monoV.setInterceptsMouseClicks (false, false);
        monoV.setJustificationType (juce::Justification::centred);
        monoV.setText (juce::String (monoS.getValue(), 0) + "Hz", juce::dontSendNotification);
    }

    void buildControls()
    {
        // Row A: MONO(1,2), HP(3), LP(4), BASS(5,6), AIR(7,8), TILT(9,10), SCOOP(11,12), Q+QLink(13,14), Shape(15), [1 empty]
        const bool Mgrey = true; // all XY controls use grey metallic styling
        
        // MONO group with slope switch and audition button (double-wide, positions 1-2)
        makeMonoGroupCell (monoHz, monoV, "MONO", "mono_hz", monoSlopeChoice, "mono_slope_db_oct", 
                          monoAuditionButton, "mono_audition", Mgrey);
        
        // HP and LP (single-wide, positions 3-4)
        makeCell (hp,     hpV,     "HP",     "hp_hz",          Mgrey);
        makeCell (lp,     lpV,     "LP",     "lp_hz",          Mgrey);
        
        // BASS with frequency mini slider (double-wide, positions 5-6)
        makeCellWithFreq (bass, bassV, "BASS", "bass_db", bassFreq, bassFreqV, "bass_freq", Mgrey);
        
        // AIR with frequency mini slider (double-wide, positions 7-8)
        makeCellWithFreq (air, airV, "AIR", "air_db", airFreq, airFreqV, "air_freq", Mgrey);
        
        // TILT with frequency mini slider (double-wide, positions 9-10)
        makeCellWithFreq (tilt, tiltV, "TILT", "tilt", tiltFreq, tiltFreqV, "tilt_freq", Mgrey);
        
        // SCOOP with frequency mini slider (double-wide, positions 11-12)
        makeCellWithFreq (scoop, scoopV, "SCOOP", "scoop", scoopFreq, scoopFreqV, "scoop_freq", Mgrey);
        
        // Q LINK with HP/LP Q controls (double-wide, positions 13-14)
        makeQLinkCell (q, qV, "Q", "eq_filter_q", qLink, "Q LINK", "eq_q_link", 
                      hpQ, hpQV, "hp_q", lpQ, lpQV, "lp_q", Mgrey);
        
        // SHELF SHAPE (single-wide, position 15)
        makeCell (shelfS, shelfSV, "S",      "eq_shelf_shape", Mgrey);

        // Additional imaging/placement controls moved from Imager to XY
        makeCell (rotation, rotationV, "ROT",      "rotation_deg",     Mgrey);
        makeCell (asym,     asymV,     "ASYM",     "asymmetry",        Mgrey);
        makeCell (shufLo,   shufLoV,   "SHUF LO",  "shuffler_lo_pct",  Mgrey);
        makeCell (shufHi,   shufHiV,   "SHUF HI",  "shuffler_hi_pct",  Mgrey);
        makeCell (shufX,    shufXV,    "SHUF XO",  "shuffler_xover_hz",Mgrey);
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

        // Grid layout is now handled directly in resized() method
        // No need for the old gridOrder system since we use explicit component placement
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
    
    // Frequency mini sliders for BASS, AIR, TILT, SCOOP
    juce::Slider bassFreq, airFreq, tiltFreq, scoopFreq;
    juce::Label  bassFreqV, airFreqV, tiltFreqV, scoopFreqV;
    
    // Individual Q controls for HP/LP
    juce::Slider hpQ, lpQ;
    juce::Label  hpQV, lpQV;
    
    // Mono group controls
    juce::ComboBox monoSlopeChoice;
    juce::ToggleButton monoAuditionButton;
    
    // Slope switch and audition button for mono group
    class MonoSlopeSwitch : public juce::Component
    {
    public:
        MonoSlopeSwitch() = default;
        void setIndex (int idx) { current = juce::jlimit (0, 2, idx); repaint(); if (onChange) onChange (current); }
        int  getIndex () const { return current; }
        std::function<void(int)> onChange;
        void paint (juce::Graphics& g) override
        {
            auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel());
            auto accent = lf ? lf->theme.eq.hp : juce::Colour (0xFF5AA9E6);
            auto panel  = lf ? lf->theme.panel  : juce::Colour (0xFF2A2C30);
            auto sh     = lf ? lf->theme.sh     : juce::Colour (0xFF1A1C20);
            auto text   = lf ? lf->theme.text   : juce::Colours::white;

            auto b = getLocalBounds().toFloat();
            const float spacing = 6.0f;
            const float availableH = juce::jmax (0.0f, b.getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f;

            auto draw = [&](juce::Rectangle<float> r, int idx, const juce::String& lbl)
            {
                const bool on = (current == idx);
                // Elevation shadow like AUD
                if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel())) g.setColour (lf->theme.shadowDark.withAlpha (0.25f)); else g.setColour (juce::Colour (0x40000000));
                g.fillRoundedRectangle (r.translated (1.5f, 1.5f), 6.0f);

                if (on)
                {
                    juce::Colour bg = accent;
                    if (idx == 0) bg = accent.brighter (0.25f);    // 6 dB
                    else if (idx == 2) bg = accent.darker (0.25f); // 24 dB
                    g.setColour (bg);
                    g.fillRoundedRectangle (r, 6.0f);
                    g.setColour (bg.darker (0.30f));
                    g.drawRoundedRectangle (r, 6.0f, 1.0f);
                }
                else
                {
                    // Gradient panel like ThemedIconButton::GradientPanel
                    juce::Colour top = panel.brighter (0.10f), bot = panel.darker (0.10f);
                    juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
                    g.setGradientFill (grad);
                    g.fillRoundedRectangle (r, 6.0f);
                    g.setColour (sh);
                    g.drawRoundedRectangle (r, 6.0f, 1.0f);
                }

                g.setColour (text);
                g.setFont (juce::Font (juce::FontOptions (12.0f).withStyle ("Bold")));
                g.drawText (lbl, r, juce::Justification::centred);
            };

            draw ({ b.getX(), b.getY(),                     b.getWidth(), h },                 0, "6");
            draw ({ b.getX(), b.getY() + h + spacing,       b.getWidth(), h },                 1, "12");
            draw ({ b.getX(), b.getY() + 2*(h + spacing),   b.getWidth(), h },                 2, "24");
        }
        void mouseDown (const juce::MouseEvent& e) override
        {
            const float spacing = 6.0f;
            const float availableH = juce::jmax (0.0f, (float)getHeight() - 2.0f * spacing);
            const float h = availableH / 3.0f; const float y = (float) e.y;
            int idx = (y <= h) ? 0 : (y <= h * 2 + spacing ? 1 : 2);
            if (idx != current) { current = idx; repaint(); if (onChange) onChange (current); }
        }
    private:
        int current { 1 }; // default to 12 dB/oct
    };
    
    MonoSlopeSwitch slopeSwitch;
    juce::ToggleButton auditionButton;
    
    
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


