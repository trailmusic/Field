#pragma once

#include <JuceHeader.h>

class MyPluginAudioProcessorEditor;

class CleanupManager
{
public:
    CleanupManager(MyPluginAudioProcessorEditor& editor);
    ~CleanupManager() = default;
    
    // Main cleanup method
    void performCleanup();
    
    // Individual cleanup methods
    void cleanupParameterAttachments();
    void cleanupTimersAndListeners();
    void cleanupAudioCallbacks();
    void cleanupParameterListeners();
    void cleanupUIListeners();
    void cleanupState();
    void cleanupLookAndFeel();
    
private:
    MyPluginAudioProcessorEditor& editor;
};
