#pragma once

#include <JuceHeader.h>
#include "RangerDesigner.h"
#include "RangerFilePane.h"
#include "RangerPlotPane.h"
#include "RangerSettingsPane.h"
#include "RangerInstructionsPane.h"

class RangerWindow : public juce::DocumentWindow
{
public:
    RangerWindow(const juce::String& name);
    ~RangerWindow() override;

    void closeButtonPressed() override;
    void resized() override;

private:
    // Main components
        std::unique_ptr<RangerDesigner> designer;
        std::unique_ptr<RangerFilePane> filePane;
        std::unique_ptr<RangerPlotPane> plotPane;
        std::unique_ptr<RangerSettingsPane> settingsPane;
        std::unique_ptr<RangerInstructionsPane> instructionsPane;
    
    // Layout
    juce::Component mainContent;
    juce::TabbedComponent tabbedComponent;
    
    // Menu bar
    juce::MenuBarComponent menuBar;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RangerWindow)
};