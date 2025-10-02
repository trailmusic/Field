#pragma once

#include <JuceHeader.h>

class RangerPlotPane : public juce::Component
{
public:
    RangerPlotPane();
    ~RangerPlotPane() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // Plot tabs
    juce::TabbedComponent plotTabs;
    
    // Plot components
    class ImpulsePlot;
    class StepPlot;
    class MagnitudePlot;
    
    std::unique_ptr<ImpulsePlot> impulsePlot;
    std::unique_ptr<StepPlot> stepPlot;
    std::unique_ptr<MagnitudePlot> magnitudePlot;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerPlotPane)
};
