#pragma once

#include <JuceHeader.h>
#include "SimpleFieldTheme.h"
#include "RangerDesigner.h"
#include "RangerFilePane.h"
#include "RangerPlotPane.h"
#include "RangerSettingsPane.h"
#include "RangerInstructionsPane.h"
#include "SimpleRangerDocsPane.h"
#include "SimpleRangerAuditionPane.h"
#include "RangerLogo.h"

class RangerWindow : public juce::DocumentWindow
{
public:
    RangerWindow(const juce::String& name);
    ~RangerWindow() override;

    void closeButtonPressed() override;
    void resized() override;

private:
    // Simple Field Theme System
    SimpleFieldTheme currentTheme;
    SimpleThemeVariant currentThemeVariant;
    
    // Main components
    std::unique_ptr<RangerDesigner> designer;
    std::unique_ptr<RangerFilePane> filePane;
    std::unique_ptr<RangerPlotPane> plotPane;
    std::unique_ptr<RangerSettingsPane> settingsPane;
    std::unique_ptr<RangerInstructionsPane> instructionsPane;
    std::unique_ptr<SimpleRangerDocsPane> docsPane;
    std::unique_ptr<SimpleRangerAuditionPane> auditionPane;
    std::unique_ptr<RangerLogo> rangerLogo;
    
    // Layout
    juce::Component mainContent;
    juce::TabbedComponent tabbedComponent;
    
    // Menu bar
    juce::MenuBarComponent menuBar;
    
    // Theme management
    void applyFieldTheme(SimpleThemeVariant variant);
    void setupFieldStyling();
    void cycleTheme();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerWindow)
};