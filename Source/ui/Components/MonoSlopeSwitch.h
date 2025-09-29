#pragma once

#include <JuceHeader.h>

class MonoSlopeSwitch : public juce::Component
{
public:
    MonoSlopeSwitch() = default;
    void setIndex (int idx) { current = juce::jlimit (0, 2, idx); repaint(); if (onChange) onChange (current); }
    int  getIndex () const { return current; }
    std::function<void(int)> onChange;
    
    void paint (juce::Graphics& g) override;
    void mouseDown (const juce::MouseEvent& e) override;
    
private:
    int current { 1 }; // default to 12 dB/oct
};
