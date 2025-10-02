#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

class SimpleRangerWindow : public juce::DocumentWindow
{
public:
    SimpleRangerWindow() 
        : DocumentWindow("Field Ranger", juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setResizeLimits(800, 600, 1600, 1200);
        centreWithSize(1000, 700);
        setVisible(true);
    }
    
    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SimpleRangerWindow)
};

class SimpleRangerApplication : public juce::JUCEApplication
{
public:
    SimpleRangerApplication() = default;

    const juce::String getApplicationName() override { return "Field Ranger"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow = std::make_unique<SimpleRangerWindow>();
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
    }

private:
    std::unique_ptr<SimpleRangerWindow> mainWindow;
};

START_JUCE_APPLICATION(SimpleRangerApplication)
