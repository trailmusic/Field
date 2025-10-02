#include "RangerWindow.h"

RangerWindow::RangerWindow(const juce::String& name)
    : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop),
      currentThemeVariant(SimpleThemeVariant::Ocean)
{
    // Apply Ocean theme by default
    applyFieldTheme(SimpleThemeVariant::Ocean);
    
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
        // docsPane = std::make_unique<RangerDocsPane>();
        rangerLogo = std::make_unique<RangerLogo>();
    
        // Set up tabbed component with Field styling
        tabbedComponent.addTab("Designer", currentTheme.panel, designer.get(), false);
        tabbedComponent.addTab("Files", currentTheme.panel, filePane.get(), false);
        tabbedComponent.addTab("Plots", currentTheme.panel, plotPane.get(), false);
        tabbedComponent.addTab("Settings", currentTheme.panel, settingsPane.get(), false);
        tabbedComponent.addTab("Instructions", currentTheme.panel, instructionsPane.get(), false);
        // tabbedComponent.addTab("Docs", currentTheme.panel, docsPane.get(), false);
    
    // Add components to main content
    addAndMakeVisible(mainContent);
    mainContent.addAndMakeVisible(tabbedComponent);
    
    // Add Field Ranger logo
    addAndMakeVisible(*rangerLogo);
    
    // Set up menu bar
    addAndMakeVisible(menuBar);
    
    // Set up Field styling
    setupFieldStyling();
    
    setVisible(true);
}

RangerWindow::~RangerWindow()
{
    designer = nullptr;
    filePane = nullptr;
    plotPane = nullptr;
    settingsPane = nullptr;
    instructionsPane = nullptr;
    // docsPane = nullptr;
    rangerLogo = nullptr;
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
    
    // Field Ranger logo at top right
    if (rangerLogo)
    {
        auto logoBounds = bounds.removeFromTop(80).removeFromRight(300);
        rangerLogo->setBounds(logoBounds);
    }
    
    // Main content fills the rest
    mainContent.setBounds(bounds);
    tabbedComponent.setBounds(mainContent.getLocalBounds());
}

void RangerWindow::applyFieldTheme(SimpleThemeVariant variant)
{
    // Apply the selected theme
    SimpleThemeManager::applyTheme(currentTheme, variant);
    currentThemeVariant = variant;
    
    // Update window colors using the theme
    setColour(juce::ResizableWindow::backgroundColourId, currentTheme.base);
    
    // Update logo theme
    if (rangerLogo)
    {
        rangerLogo->setTheme(currentTheme);
    }
    
    // Update tab colors
    for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
    {
        tabbedComponent.setTabBackgroundColour(i, currentTheme.panel);
    }
    
    // Repaint all components
    repaint();
}

void RangerWindow::setupFieldStyling()
{
    // Set up Field-style tabbed component
    tabbedComponent.setTabBarDepth(40);
    tabbedComponent.setOutline(0);
    
    // Apply Field styling to all tabs
    for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
    {
        tabbedComponent.setTabBackgroundColour(i, currentTheme.panel);
    }
}

void RangerWindow::cycleTheme()
{
    // Cycle through all 5 Field themes
    int nextTheme = (static_cast<int>(currentThemeVariant) + 1) % 5;
    applyFieldTheme(static_cast<SimpleThemeVariant>(nextTheme));
}