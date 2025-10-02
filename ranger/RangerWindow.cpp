#include "RangerWindow.h"

RangerWindow::RangerWindow(const juce::String& name)
    : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(1000, 700, 2000, 1400);
    centreWithSize(1200, 800);
    
        // Create main components
        designer = std::make_unique<RangerDesigner>();
        filePane = std::make_unique<RangerFilePane>();
        plotPane = std::make_unique<RangerPlotPane>();
        settingsPane = std::make_unique<RangerSettingsPane>();
        instructionsPane = std::make_unique<RangerInstructionsPane>();
    
        // Set up tabbed component
        tabbedComponent.addTab("Designer", juce::Colour(0xff2d2d2d), designer.get(), false);
        tabbedComponent.addTab("Files", juce::Colour(0xff2d2d2d), filePane.get(), false);
        tabbedComponent.addTab("Plots", juce::Colour(0xff2d2d2d), plotPane.get(), false);
        tabbedComponent.addTab("Settings", juce::Colour(0xff2d2d2d), settingsPane.get(), false);
        tabbedComponent.addTab("Instructions", juce::Colour(0xff2d2d2d), instructionsPane.get(), false);
    
    // Add components to main content
    addAndMakeVisible(mainContent);
    mainContent.addAndMakeVisible(tabbedComponent);
    
    // Set up menu bar
    addAndMakeVisible(menuBar);
    
    setVisible(true);
}

RangerWindow::~RangerWindow()
{
        designer = nullptr;
        filePane = nullptr;
        plotPane = nullptr;
        settingsPane = nullptr;
        instructionsPane = nullptr;
}

void RangerWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}

void RangerWindow::resized()
{
    auto bounds = getLocalBounds();
    
    // Menu bar at top
    menuBar.setBounds(bounds.removeFromTop(25));
    
    // Main content fills the rest
    mainContent.setBounds(bounds);
    tabbedComponent.setBounds(mainContent.getLocalBounds());
}