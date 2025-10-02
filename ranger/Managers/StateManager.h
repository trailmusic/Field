#pragma once

#include <JuceHeader.h>

class MyPluginAudioProcessorEditor;

class StateManager
{
public:
    explicit StateManager(MyPluginAudioProcessorEditor& editor);
    ~StateManager() = default;

    // State management methods
    void saveCurrentState();
    void loadState(bool loadStateA);
    void toggleABState();
    void copyState(bool copyFromA);
    void pasteState(bool pasteToA);
    void updatePresetDisplay();

    // State access
    bool isStateA() const { return isStateA_; }
    void setStateA(bool state) { isStateA_ = state; }

private:
    MyPluginAudioProcessorEditor& editor;
    
    // State variables
    std::map<juce::String, float> stateA, stateB;
    bool isStateA_ = true;
    std::map<juce::String, float> clipboardState;
    juce::String presetNameA = "Default", presetNameB = "Default";
    
    // Helper methods
    static void applyStateToSlider(juce::Slider& s, float v);
};
