#include "RangerDesigner.h"
#include "RangerFilePane.h"
#include "RangerPlotPane.h"
#include "RangerSettingsPane.h"

RangerDesigner::RangerDesigner()
{
    // Create main panels
    filePane = std::make_unique<RangerFilePane>();
    plotPane = std::make_unique<RangerPlotPane>();
    settingsPane = std::make_unique<RangerSettingsPane>();
    
    addAndMakeVisible(*filePane);
    addAndMakeVisible(*plotPane);
    addAndMakeVisible(*settingsPane);
    
    // Set up main layout
    mainLayout.flexDirection = juce::FlexBox::Direction::row;
    mainLayout.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    mainLayout.alignItems = juce::FlexBox::AlignItems::stretch;
    
    // Set up center layout for plot area
    centerLayout.flexDirection = juce::FlexBox::Direction::column;
    centerLayout.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    centerLayout.alignItems = juce::FlexBox::AlignItems::stretch;
}

RangerDesigner::~RangerDesigner()
{
    filePane = nullptr;
    plotPane = nullptr;
    settingsPane = nullptr;
}

void RangerDesigner::paint(juce::Graphics& g)
{
    // Use Field theme background
    g.fillAll(juce::Colour(0xff1a1a1a)); // Dark background
}

void RangerDesigner::resized()
{
    auto bounds = getLocalBounds();
    
    // Left panel: File management (200px width)
    auto leftBounds = bounds.removeFromLeft(200);
    filePane->setBounds(leftBounds);
    
    // Right panel: Settings (250px width)
    auto rightBounds = bounds.removeFromRight(250);
    settingsPane->setBounds(rightBounds);
    
    // Center panel: Plots (remaining space)
    plotPane->setBounds(bounds);
}
