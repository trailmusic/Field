#pragma once
#include <JuceHeader.h>

class DecayCurveComponent : public juce::Component, private juce::Timer
{
public:
    DecayCurveComponent (juce::AudioProcessorValueTreeState& s,
                         const juce::String& lowId, const juce::String& midId, const juce::String& highId)
        : state (s), lowParamId (lowId), midParamId (midId), highParamId (highId) { startTimerHz (30); }

    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& state;
    juce::String lowParamId, midParamId, highParamId;
    int dragIdx = -1;
    void timerCallback() override { repaint(); }
};


