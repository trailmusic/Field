#include <JuceHeader.h>
#include "RangerWindow.h"

class RangerApplication : public juce::JUCEApplication
{
public:
    RangerApplication() = default;

    const juce::String getApplicationName() override { return "Field Ranger"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        // Set up Field Look & Feel
        fieldLNF = std::make_unique<FieldLNF>();
        juce::LookAndFeel::setDefaultLookAndFeel(fieldLNF.get());
        
        // Create main window
        mainWindow = std::make_unique<RangerWindow>(getApplicationName());
        mainWindow->setVisible(true);
    }

    void shutdown() override
    {
        mainWindow = nullptr;
        fieldLNF = nullptr;
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String& commandLine) override
    {
        // Handle multiple instances if needed
    }

private:
    std::unique_ptr<RangerWindow> mainWindow;
    std::unique_ptr<FieldLNF> fieldLNF;
};

// This macro generates the main() routine that launches the app
START_JUCE_APPLICATION(RangerApplication)
