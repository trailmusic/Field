#include "ControlFactory.h"
#include "../KnobCell.h"
#include "../KnobCellDual.h"
#include "../KnobCellQuad.h"

// Forward declarations for nested classes
class DoubleKnobCell;
class QuadKnobCell;

std::unique_ptr<KnobCell> ControlFactory::createKnobCell(
    juce::Slider& slider, 
    juce::Label& valueLabel, 
    const juce::String& caption)
{
    auto cell = std::make_unique<KnobCell>(slider, valueLabel, caption);
    applyStandardMetrics(cell.get());
    return cell;
}


std::unique_ptr<juce::ComboBox> ControlFactory::createComboBox(
    const juce::String& caption)
{
    auto combo = std::make_unique<juce::ComboBox>();
    combo->setName(caption);
    combo->setSize(STANDARD_CELL_SIZE, STANDARD_CELL_SIZE);
    return combo;
}

std::unique_ptr<DoubleKnobCell> ControlFactory::createDoubleKnobCell(
    juce::Slider& slider1, juce::Label& label1,
    juce::Slider& slider2, juce::Label& label2)
{
    auto cell = std::make_unique<DoubleKnobCell>(slider1, label1, slider2, label2);
    applyStandardMetrics(cell.get());
    return cell;
}

std::unique_ptr<QuadKnobCell> ControlFactory::createQuadKnobCell(
    juce::Slider& slider1, juce::Label& label1,
    juce::Slider& slider2, juce::Label& label2,
    juce::Slider& slider3, juce::Label& label3,
    juce::Component& clusterContainer)
{
    auto cell = std::make_unique<QuadKnobCell>(slider1, label1, slider2, label2, slider3, label3, clusterContainer);
    applyStandardMetrics(cell.get());
    return cell;
}

void ControlFactory::applyStandardMetrics(KnobCell* cell)
{
    if (!cell) return;
    
    const int knobSize = getScaledKnobSize();
    cell->setMetrics(knobSize, STANDARD_VALUE_SIZE, STANDARD_GAP);
    cell->setValueLabelMode(KnobCell::ValueLabelMode::Managed);
    cell->setValueLabelGap(STANDARD_GAP);
}


void ControlFactory::applyStandardMetrics(DoubleKnobCell* cell)
{
    if (!cell) return;
    
    const int knobSize = getScaledKnobSize();
    cell->setMetrics(knobSize, STANDARD_VALUE_SIZE, STANDARD_GAP);
}

void ControlFactory::applyStandardMetrics(QuadKnobCell* cell)
{
    if (!cell) return;
    
    const int knobSize = getScaledKnobSize();
    cell->setMetrics(knobSize, STANDARD_VALUE_SIZE, STANDARD_GAP);
}

int ControlFactory::getScaledKnobSize()
{
    // This should match the scaleFactor used in PluginEditor
    // For now, return the standard size
    return STANDARD_KNOB_SIZE;
}
