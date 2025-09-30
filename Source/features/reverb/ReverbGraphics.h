#pragma once
#include <JuceHeader.h>

class ReverbGraphics : public juce::Component
{
public:
    ReverbGraphics (juce::AudioProcessorValueTreeState& s,
                 std::function<float()> getEr,
                 std::function<float()> getTail,
                 std::function<float()> getDuckDb,
                 std::function<float()> getWidthNow);

    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    juce::AudioProcessorValueTreeState& state;
};


