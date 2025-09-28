#pragma once

#include <JuceHeader.h>
#include "MotionIDs.h"
#include "MotionSlot.h"
#include "../ui/Components/KnobCell.h"
#include "../ui/SimpleSwitchCell.h"
#include "../ui/Layout.h"
#include "../Core/FieldLookAndFeel.h"
#include "../Core/FieldMetallic.h"

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
        // Assign FieldLookAndFeel to get custom tick rendering
        if (auto* lf = dynamic_cast<FieldLNF*>(&getLookAndFeel()))
            k.setLookAndFeel(lf);
    }
    void makeCell (juce::Slider& s, juce::Label& v, const juce::String& cap, const char* pid)
    {
        styleKnob (s); s.setName (cap);
        auto cell = std::make_unique<KnobCell> (s, v, cap);
        cell->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap (labelGapPx);
        // Motion metallic styling - use proper enum-based system
        setAreaMetallicForCell (*cell, MetallicKind::Motion);
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

    void makeMotionButtonCell (juce::ToggleButton& t, const juce::String& cap, const char* pid)
    {
        if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
        {
            return;
        }
        
        t.setName (cap);
        // Apply Motion metallic styling to the actual button, not the wrapper
        setAreaMetallicForCell (t, MetallicKind::Motion);
        auto cell = std::make_unique<SimpleSwitchCell> (t);
        cell->setCaption (cap);
        cell->setShowBorder (true);
        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));
        btnAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, pid, t));
    }
    
    void makeMotionComboCell (juce::ComboBox& c, const juce::String& cap, const char* pid)
    {
        if (pid == nullptr || apvts.getParameter(juce::String(pid)) == nullptr)
        {
            return;
        }
        
        c.setName (cap);
        // Apply Motion metallic styling to the actual combo, not the wrapper
        setAreaMetallicForCell (c, MetallicKind::Motion);
        auto cell = std::make_unique<SimpleSwitchCell> (c);
        cell->setCaption (cap);
        cell->setShowBorder (true);
        addAndMakeVisible (*cell);
        switchCells.emplace_back (cell.get());
        ownedSwitches.emplace_back (std::move (cell));
        cmbAtts.push_back (std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, pid, c));
    }

    void buildControls()
    {
        using namespace MotionSlot;
        
        // Build all 32 controls using canonical registry
        for (int slot = 1; slot <= 32; ++slot) {
            const auto& param = getSlot(slot);
            
            switch (param.type) {
                case kButton:
                    createButton(slot, param);
                    break;
                case kComboBox:
                    createComboBox(slot, param);
                    break;
                case kKnob:
                    createKnob(slot, param);
                    break;
            }
        }
    }
    
    void createButton(int slot, const MotionSlot::ParamRef& param)
    {
        auto button = std::make_unique<juce::ToggleButton>();
        button->setName(param.name);
        setAreaMetallicForCell(*button, MetallicKind::Motion);
        
        auto cell = std::make_unique<SimpleSwitchCell>(*button);
        cell->setCaption(param.name);
        cell->setShowBorder(true);
        addAndMakeVisible(*cell);
        
        switchCells.emplace_back(cell.get());
        ownedSwitches.emplace_back(std::move(cell));
        ownedButtons.emplace_back(std::move(button));
        btnAtts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, param.id, *ownedButtons.back()));
        gridOrder.push_back(switchCells.back());
    }
    
    void createComboBox(int slot, const MotionSlot::ParamRef& param)
    {
        auto combo = std::make_unique<juce::ComboBox>();
        combo->setName(param.name);
        
        // Add items based on parameter type
        if (slot == MotionSlot::kPanner) {
            combo->addItemList(motion::choiceListPanner(), 1);
        } else if (slot == MotionSlot::kPath) {
            combo->addItemList(motion::choiceListPath(), 1);
        } else if (slot == MotionSlot::kMode) {
            combo->addItemList(motion::choiceListMode(), 1);
        } else if (slot == MotionSlot::kQuant) {
            combo->addItemList(motion::choiceListQuant(), 1);
        }
        
        // Apply metallic properties to the ComboBox itself, not the wrapper
        combo->getProperties().set("metallic", true);
        combo->getProperties().set("motionMetallic", true);
        
        auto cell = std::make_unique<SimpleSwitchCell>(*combo);
        cell->setCaption(param.name);
        cell->setShowBorder(true);
        addAndMakeVisible(*cell);
        
        switchCells.emplace_back(cell.get());
        ownedSwitches.emplace_back(std::move(cell));
        ownedComboBoxes.emplace_back(std::move(combo));
        cmbAtts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, param.id, *ownedComboBoxes.back()));
        gridOrder.push_back(switchCells.back());
    }
    
    void createKnob(int slot, const MotionSlot::ParamRef& param)
    {
        auto slider = std::make_unique<juce::Slider>();
        auto label = std::make_unique<juce::Label>();
        
        styleKnob(*slider);
        slider->setName(param.name);
        slider->setRange(param.min, param.max, 0.01f);
        slider->setValue(param.defaultVal);
        
        auto cell = std::make_unique<KnobCell>(*slider, *label, param.name);
        cell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
        cell->setValueLabelGap(labelGapPx);
        setAreaMetallicForCell(*cell, MetallicKind::Motion);
        cell->getProperties().set ("caption", param.name);
        addAndMakeVisible(*cell);
        
        knobCells.emplace_back(cell.get());
        ownedCells.emplace_back(std::move(cell));
        ownedSliders.emplace_back(std::move(slider));
        ownedLabels.emplace_back(std::move(label));
        sAtts.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, param.id, *ownedSliders.back()));
        gridOrder.push_back(ownedCells.back().get());
    }
    
    void createPlaceholder(int slot, const char* name)
    {
        auto dummySlider = std::make_unique<juce::Slider>();
        auto dummyLabel = std::make_unique<juce::Label>();
        auto placeholder = std::make_unique<KnobCell>(*dummySlider, *dummyLabel, "---");
        placeholder->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
        placeholder->setValueLabelGap(labelGapPx);
        setAreaMetallicForCell(*placeholder, MetallicKind::Motion);
        placeholder->setEnabled(false);
        addAndMakeVisible(*placeholder);
        
        placeholderCells.emplace_back(placeholder.get());
        ownedPlaceholders.emplace_back(std::move(placeholder));
        ownedDummySliders.emplace_back(std::move(dummySlider));
        ownedDummyLabels.emplace_back(std::move(dummyLabel));
        gridOrder.push_back(ownedPlaceholders.back().get());
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
        for (auto* c : placeholderCells)
        {
            if (!c) continue;
            c->setMetrics (knobPx, valuePx, labelGapPx);
            c->setValueLabelMode (KnobCell::ValueLabelMode::Managed);
            c->setValueLabelGap (labelGapPx);
        }
        // Switch cells don't need metrics - they handle their own layout
    }

    juce::AudioProcessorValueTreeState& apvts;

    // Attachment vectors
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> sAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> cmbAtts;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>> btnAtts;

    // Component storage
    std::vector<KnobCell*> knobCells;
    std::vector<std::unique_ptr<KnobCell>> ownedCells;
    std::vector<KnobCell*> placeholderCells;
    std::vector<std::unique_ptr<KnobCell>> ownedPlaceholders;
    std::vector<std::unique_ptr<juce::Slider>> ownedDummySliders;
    std::vector<std::unique_ptr<juce::Label>> ownedDummyLabels;
    std::vector<SimpleSwitchCell*> switchCells;
    std::vector<std::unique_ptr<SimpleSwitchCell>> ownedSwitches;
    std::vector<std::unique_ptr<juce::ToggleButton>> ownedButtons;
    std::vector<std::unique_ptr<juce::ComboBox>> ownedComboBoxes;
    std::vector<std::unique_ptr<juce::Slider>> ownedSliders;
    std::vector<std::unique_ptr<juce::Label>> ownedLabels;
    std::vector<juce::Component*> gridOrder;

    int knobPx = 52, valuePx = 14, labelGapPx = 4, colW = 56, rowH = 0; // Increased knob size from 50 to 52
};


