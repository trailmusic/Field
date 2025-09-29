#pragma once

#include <JuceHeader.h>

class MyPluginAudioProcessorEditor;

class PaintManager
{
public:
    PaintManager(MyPluginAudioProcessorEditor& editor);
    ~PaintManager() = default;
    
    // Main paint method
    void paint(juce::Graphics& g);
    
    // Individual paint methods
    void paintBackground(juce::Graphics& g);
    void paintHeader(juce::Graphics& g);
    void paintResizeHandle(juce::Graphics& g);
    void drawHeaderFieldLogo(juce::Graphics& g, juce::Rectangle<float> area);
    
private:
    MyPluginAudioProcessorEditor& editor;
};
