#include "RangerWindow.h"
#include "RangerDesigner.h"

RangerWindow::RangerWindow(const juce::String& name)
    : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
    setResizable(true, true);
    setResizeLimits(800, 600, 1600, 1200);
    
    // Create the main designer component
    designer = std::make_unique<RangerDesigner>();
    setContentOwned(designer.get(), true);
    
    // Center the window
    centreWithSize(getWidth(), getHeight());
    
    // Set window icon (placeholder for now)
    setTitleBarTextCentred(false);
}

RangerWindow::~RangerWindow()
{
    designer = nullptr;
}

void RangerWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}
