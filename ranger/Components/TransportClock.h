#pragma once

#include <JuceHeader.h>
#include "../../Core/PluginProcessor.h"

class TransportClock : public juce::Component, private juce::Timer
{
public:
    TransportClock(MyPluginAudioProcessor& processor);
    ~TransportClock() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;

    // Clock formatting helper
    static inline juce::String formatClock(double seconds)
    {
        if (seconds < 0.0) seconds = 0.0;
        int mm = (int)(seconds / 60.0);
        double ss = seconds - (mm * 60.0);
        return juce::String::formatted("%02d:%06.3f", mm, ss);
    }

private:
    void timerCallback() override;
    void updateClockDisplay();

    // UI smoother for smooth clock updates
    struct UISmoother {
        double shownSec = 0.0;
        void snapTo(double t) { shownSec = t; }
        double stepToward(double targetSec, double frameDtSec, double maxRate = 5.0) // sec/sec
        {
            const double delta = targetSec - shownSec;
            const double maxStep = maxRate * frameDtSec;
            const double step = juce::jlimit(-maxStep, maxStep, delta);
            shownSec += step;
            return shownSec;
        }
    };

    MyPluginAudioProcessor& proc;
    juce::Label clockLabel;
    
    // Clock state
    MyPluginAudioProcessor::ClockSnapshot lastClock{};
    UISmoother clockSmoother;
    juce::uint32 lastClockUpdateMs = 0;
    
    // Constants
    static constexpr int UPDATE_HZ = 30;
};
