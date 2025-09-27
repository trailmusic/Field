#pragma once

#include <JuceHeader.h>
#include "Components/KnobCell.h"
#include "Components/KnobCellWithAux.h"
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
        const int cols = 16;
        const int rows = 2;
        const int cellW = (colW > 0 ? colW : juce::jmax (1, r.getWidth() / cols));
        const int cellH = (rowH > 0 ? rowH : juce::jmax (1, r.getHeight() / rows));
        const int totalW = cellW * cols;
        const int totalH = cellH * rows;
        const int xOffset = (r.getWidth()  > totalW ? (r.getWidth()  - totalW) / 2 : 0);
        const int yOffset = (r.getHeight() > totalH ? (r.getHeight() - totalH) / 2 : 0);

        auto place = [&] (int index, int row, int col, int width = 1)
        {
            if (index < 0 || index >= ownedCells.size()) return;
            if (auto* c = ownedCells[(size_t) index].get())
            {
                const int x = r.getX() + xOffset + (col - 1) * cellW;
                const int y = r.getY() + yOffset + (row - 1) * cellH;
                const int w = width * cellW;
                c->setBounds (x, y, w, cellH);
            }
        };

        // Row A layout: MONO(2), HP(1), BASS(2), TILT(2), SCOOP(2), AIR(2), LP(1), Q+QLink(2), S(1), BLANK(1)
        int idx = 0;
        place(idx++, 1, 1, 2);  // MONO (double-wide)
        place(idx++, 1, 3, 1);  // HP
        place(idx++, 1, 4, 2);  // BASS (double-wide)
        place(idx++, 1, 6, 2);  // TILT (double-wide)
        place(idx++, 1, 8, 2);  // SCOOP (double-wide)
        place(idx++, 1, 10, 2); // AIR (double-wide)
        place(idx++, 1, 12, 1); // LP
        place(idx++, 1, 13, 2); // Q+QLink (double-wide)
        place(idx++, 1, 15, 1); // S
        // Slot 16 is intentionally left blank - no control placed there
        
        // Row B layout: Specific slot assignments for center processing controls
        // Slots 17-20: ROT, ASYM, PAN, SAT MIX (imaging controls)
        place(idx++, 2, 1, 1);  // ROT (slot 17)
        place(idx++, 2, 2, 1);  // ASYM (slot 18)
        place(idx++, 2, 3, 1);  // PAN (slot 19)
        place(idx++, 2, 4, 1);  // SAT MIX (slot 20)
        
        // Slots 21-27: 7 blank placeholders (already created in buildControls)
        for (int col = 5; col <= 11 && idx < (int)ownedCells.size(); ++col)
        {
            place(idx++, 2, col, 1);
        }
        
        // Slots 28-30: PUNCH, CNTR, LO, HI (center processing controls)
        place(idx++, 2, 12, 1); // PUNCH (slot 28)
        place(idx++, 2, 13, 1); // CNTR (slot 29)
        place(idx++, 2, 14, 1); // LO (slot 30)
        place(idx++, 2, 15, 1); // HI (slot 31)
        
        // Slot 32: Final blank placeholder
        if (idx < (int)ownedCells.size())
        {
            place(idx++, 2, 16, 1); // BLANK (slot 32)
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
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
        }
        // XY knobs now use proper metallic system - no centerStyle needed
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
        // Apply metallic properties to the actual button, not the wrapper
        if (metallic) {
            setAreaMetallicForCell (b, MetallicKind::XY);
        }
        auto cell = std::make_unique<SimpleSwitchCell> (b);
        cell->setCaption (cap);
        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid, b));
    }
    void makeCombo (juce::ComboBox& c, const juce::String& cap, const char* pid, bool metallic=false)
    {
        // Apply metallic properties to the actual combo, not the wrapper
        if (metallic) {
            setAreaMetallicForCell (c, MetallicKind::XY);
        }
        auto cell = std::make_unique<SimpleSwitchCell> (c);
        cell->setCaption (cap);
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
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
        }
        // XY knobs now use proper metallic system - no centerStyle needed
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
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
        }
        // XY knobs now use proper metallic system - no centerStyle needed
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
    
    // Helper for creating BASS cell with frequency mini slider using KnobCellWithAux
    void makeBassCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid,
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
        
        // Create KnobCellWithAux with frequency mini slider as aux component
        std::vector<juce::Component*> auxComponents = { &freqS };
        std::vector<float> auxWeights = { 1.0f }; // Single aux component gets full weight
        auto cell = std::make_unique<KnobCellWithAux> (s, v, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
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
    
    // Helper for creating AIR cell with frequency mini slider using KnobCellWithAux
    void makeAirCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid,
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
        
        // Create KnobCellWithAux with frequency mini slider as aux component
        std::vector<juce::Component*> auxComponents = { &freqS };
        std::vector<float> auxWeights = { 1.0f }; // Single aux component gets full weight
        auto cell = std::make_unique<KnobCellWithAux> (s, v, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
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
    
    // Helper for creating TILT cell with frequency mini slider using KnobCellWithAux
    void makeTiltCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid,
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
        
        // Create KnobCellWithAux with frequency mini slider as aux component
        std::vector<juce::Component*> auxComponents = { &freqS };
        std::vector<float> auxWeights = { 1.0f }; // Single aux component gets full weight
        auto cell = std::make_unique<KnobCellWithAux> (s, v, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
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
    
    // Helper for creating SCOOP cell with frequency mini slider using KnobCellWithAux
    void makeScoopCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid,
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
        
        // Create KnobCellWithAux with frequency mini slider as aux component
        std::vector<juce::Component*> auxComponents = { &freqS };
        std::vector<float> auxWeights = { 1.0f }; // Single aux component gets full weight
        auto cell = std::make_unique<KnobCellWithAux> (s, v, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
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
    
    // Helper for creating Q cell with Q link toggle and HP/LP Q sliders using KnobCellWithAux
    void makeQCell (juce::Slider& qS, juce::Label& qV, const juce::String& qCap, const char* qPid,
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
        
        // Create KnobCellWithAux with Q link toggle and HP/LP Q sliders as aux components
        std::vector<juce::Component*> auxComponents = { &linkB, &hpQS, &lpQS };
        std::vector<float> auxWeights = { 1.0f, 1.0f, 1.0f }; // Equal weights for all aux components
        auto cell = std::make_unique<KnobCellWithAux> (qS, qV, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
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
        
        monoS.setName (monoCap);
        
        // Style the mono slider like other knobs
        styleKnob (monoS);
        
        // Set up slope switch and audition button
        slopeSwitch.setIndex (1); // Default to 12 dB/oct
        slopeSwitch.onChange = [this, slopePid](int idx) {
            if (auto* param = apvts.getParameter (slopePid))
                param->setValueNotifyingHost (idx / 2.0f);
        };
        
        auditionButton.setButtonText ("");
        auditionButton.setToggleState (false, juce::dontSendNotification);
        
        // Create KnobCellWithAux template for double-wide with auxiliary components
        std::vector<juce::Component*> auxComponents = { &slopeSwitch, &auditionButton };
        std::vector<float> auxWeights = { 2.0f, 1.0f }; // Slope switch gets 2x weight, audition button gets 1x
        auto cell = std::make_unique<KnobCellWithAux> (monoS, monoV, auxComponents, auxWeights);
        
        // Set metrics to match other cells
        cell->setMetrics (knobPx, valuePx, labelGapPx);
        cell->setAuxHeight (Layout::dp (40, 1.0f)); // Responsive aux height like other cells
        
        // Apply metallic styling like other cells
        if (metallic) {
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            // For KnobCellWithAux, also apply metallic to aux components
            for (auto* auxComp : cell->getAuxComponents()) {
                if (auxComp) {
                    setAreaMetallic (*auxComp, MetallicKind::XY);
                }
            }
        }
        
        addAndMakeVisible (*cell);
        knobCells.emplace_back (cell.get());
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
        // Row A: MONO(1,2), HP(3), BASS(4,5), TILT(6,7), SCOOP(8,9), AIR(10,11), LP(12), Q+QLink(13,14), S(15), [1 empty]
        const bool Mgrey = true; // all XY controls use grey metallic styling
        
        // MONO group with slope switch and audition button (double-wide, positions 1-2)
        makeMonoGroupCell (monoHz, monoV, "MONO", "mono_hz", monoSlopeChoice, "mono_slope_db_oct", 
                          monoAuditionButton, "mono_audition", Mgrey);
        
        // HP (single-wide, position 3)
        makeCell (hp,     hpV,     "HP",     "hp_hz",          Mgrey);
        
        // BASS with frequency mini slider (double-wide, positions 4-5) - using KnobCellWithAux
        makeBassCell (bass, bassV, "BASS", "bass_db", bassFreq, bassFreqV, "bass_freq", Mgrey);
        
        // TILT with frequency mini slider (double-wide, positions 6-7) - using KnobCellWithAux
        makeTiltCell (tilt, tiltV, "TILT", "tilt", tiltFreq, tiltFreqV, "tilt_freq", Mgrey);
        
        // SCOOP with frequency mini slider (double-wide, positions 8-9) - using KnobCellWithAux
        makeScoopCell (scoop, scoopV, "SCOOP", "scoop", scoopFreq, scoopFreqV, "scoop_freq", Mgrey);
        
        // AIR with frequency mini slider (double-wide, positions 10-11) - using KnobCellWithAux
        makeAirCell (air, airV, "AIR", "air_db", airFreq, airFreqV, "air_freq", Mgrey);
        
        // LP (single-wide, position 12)
        makeCell (lp,     lpV,     "LP",     "lp_hz",          Mgrey);
        
        // Q LINK with HP/LP Q controls (double-wide, positions 13-14) - using KnobCellWithAux
        makeQCell (q, qV, "Q", "eq_filter_q", qLink, "Q LINK", "eq_q_link", 
                  hpQ, hpQV, "hp_q", lpQ, lpQV, "lp_q", Mgrey);
        
        // SHELF SHAPE (single-wide, position 15)
        makeCell (shelfS, shelfSV, "S",      "eq_shelf_shape", Mgrey);
        

        // Additional imaging/placement controls moved from Imager to XY
        makeCell (rotation, rotationV, "ROT",      "rotation_deg",     Mgrey);
        makeCell (asym,     asymV,     "ASYM",     "asymmetry",        Mgrey);
        makeCell (pan,      panV,      "PAN",      "pan",               Mgrey);
        makeCell (satMix,   satMixV,   "SAT MIX",  "sat_mix",           Mgrey);
        // Create blank placeholders for Row A (7 blanks)
        for (int i = 0; i < 7; ++i) {
            auto sl = std::make_unique<juce::Slider>();
            auto lb = std::make_unique<juce::Label>(); 
            lb->setVisible (false);
            styleKnob (*sl);
            auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            cell->setShowKnob (false);
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            addAndMakeVisible (*cell);
            knobCells.emplace_back (cell.get());
            blankSliders.emplace_back (std::move (sl));
            blankLabels.emplace_back (std::move (lb));
            ownedCells.emplace_back (std::move (cell));
        }

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
        
        // Create blank placeholders for Row B (8 blanks)
        for (int i = 0; i < 8; ++i) {
            auto sl = std::make_unique<juce::Slider>();
            auto lb = std::make_unique<juce::Label>(); 
            lb->setVisible (false);
            styleKnob (*sl);
            auto cell = std::make_unique<KnobCell> (*sl, *lb, juce::String());
            cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            cell->setValueLabelGap (labelGapPx);
            cell->setShowKnob (false);
            setAreaMetallicForCell (*cell, MetallicKind::XY);
            addAndMakeVisible (*cell);
            knobCells.emplace_back (cell.get());
            blankSliders.emplace_back (std::move (sl));
            blankLabels.emplace_back (std::move (lb));
            ownedCells.emplace_back (std::move (cell));
        }

        // Grid layout is now handled directly in resized() method
        // No need for the old gridOrder system since we use explicit component placement
    }

    void applyMetricsToAll()
    {
        for (auto* c : knobCells)
        {
            if (c) 
            {
                // Handle KnobCell
                if (auto* knobCell = dynamic_cast<KnobCell*>(c))
                {
                    knobCell->setMetrics (knobPx, valuePx, labelGapPx);
                    knobCell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
                    knobCell->setValueLabelGap (labelGapPx);
                }
                // Handle KnobCellWithAux
                else if (auto* knobCellWithAux = dynamic_cast<KnobCellWithAux*>(c))
                {
                    knobCellWithAux->setMetrics (knobPx, valuePx, labelGapPx);
                }
            }
        }
        for (auto* s : switchCells)
            if (s) s->setShowBorder (true);
    }

    juce::AudioProcessorValueTreeState& apvts;
    // EQ row + imaging controls moved from Imager
    juce::Slider bass, hp, lp, q, air, tilt, scoop, shelfS, mix;
    juce::Slider rotation, asym, monoHz, pan, satMix;
    juce::Label  bassV, hpV, lpV, qV, airV, tiltV, scoopV, shelfSV, mixV;
    juce::Label  rotationV, asymV, monoV, panV, satMixV;
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

    std::vector<juce::Component*> knobCells;
    std::vector<SimpleSwitchCell*> switchCells;
    std::vector<std::unique_ptr<juce::Component>> ownedCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>> ownedSwitches;
    std::vector<juce::Component*> gridOrder;
    std::vector<std::unique_ptr<juce::Slider>> blankSliders;
    std::vector<std::unique_ptr<juce::Label>>  blankLabels;

    int knobPx = 48, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0;
};


