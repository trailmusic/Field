#include <JuceHeader.h>
#include "RangerWindow.h"

class FieldRangerApplication : public juce::JUCEApplication
{
public:
    FieldRangerApplication() = default;

    const juce::String getApplicationName() override { return "Field Ranger"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        // Set up the main window
        mainWindow = std::make_unique<RangerWindow>(getApplicationName());
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // Bring existing window to front
        if (mainWindow != nullptr)
            mainWindow->toFront(true);
    }

private:
    std::unique_ptr<RangerWindow> mainWindow;
};

START_JUCE_APPLICATION(FieldRangerApplication)