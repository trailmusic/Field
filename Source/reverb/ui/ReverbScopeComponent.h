#pragma once
#include <JuceHeader.h>

class ReverbScopeComponent : public juce::Component, private juce::Timer
{
public:
    ReverbScopeComponent (std::function<float()> er, std::function<float()> tail,
                          std::function<float()> duckDb, std::function<float()> widthNow)
        : getEr (std::move(er)), getTail (std::move(tail)), getDuck (std::move(duckDb)), getWidth (std::move(widthNow))
    { startTimerHz (30); }

    void paint (juce::Graphics& g) override;

private:
    std::function<float()> getEr, getTail, getDuck, getWidth;
    void timerCallback() override { if (isShowing()) repaint(); }
};


