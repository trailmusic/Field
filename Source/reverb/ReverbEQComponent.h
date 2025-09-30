#pragma once
#include <JuceHeader.h>

class ReverbEQComponent : public juce::Component, private juce::Timer
{
public:
    explicit ReverbEQComponent (juce::AudioProcessorValueTreeState& s) : state (s) { startTimerHz (30); }
    
    ~ReverbEQComponent() override
    {
        // Stop timer before destruction to prevent use-after-free
        stopTimer();
    }
    
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;

private:
    juce::AudioProcessorValueTreeState& state;
    int dragBand = -1; // 0=low, 1=mid, 2=high
    void timerCallback() override { if (isShowing()) repaint(); }
    void visibilityChanged() override { if (isVisible()) startTimerHz(30); else stopTimer(); }
};


