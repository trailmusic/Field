#include "RangerPlotPane.h"

// Simple plot components for now
class RangerPlotPane::ImpulsePlot : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Impulse Response", getLocalBounds(), juce::Justification::centred);
    }
};

class RangerPlotPane::StepPlot : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Step Response", getLocalBounds(), juce::Justification::centred);
    }
};

class RangerPlotPane::MagnitudePlot : public juce::Component
{
public:
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colour(0xff1a1a1a));
        g.setColour(juce::Colours::white);
        g.setFont(16.0f);
        g.drawText("Magnitude Response", getLocalBounds(), juce::Justification::centred);
    }
};

RangerPlotPane::RangerPlotPane()
{
    // Create plot components
    impulsePlot = std::make_unique<ImpulsePlot>();
    stepPlot = std::make_unique<StepPlot>();
    magnitudePlot = std::make_unique<MagnitudePlot>();
    
    // Add tabs
    plotTabs.addTab("Impulse", juce::Colour(0xff2a2a2a), impulsePlot.get(), false);
    plotTabs.addTab("Step", juce::Colour(0xff2a2a2a), stepPlot.get(), false);
    plotTabs.addTab("Magnitude", juce::Colour(0xff2a2a2a), magnitudePlot.get(), false);
    
    addAndMakeVisible(plotTabs);
}

RangerPlotPane::~RangerPlotPane()
{
    impulsePlot = nullptr;
    stepPlot = nullptr;
    magnitudePlot = nullptr;
}

void RangerPlotPane::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a1a));
}

void RangerPlotPane::resized()
{
    plotTabs.setBounds(getLocalBounds());
}
