#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "MotionIDs.h"
#include "../ui/Design.h"
namespace motion {
class MotionPanel : public juce::Component, private juce::Timer {
public:
    MotionPanel(juce::AudioProcessorValueTreeState& s, juce::UndoManager* um = nullptr);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    void timerCallback() override { repaint(orbBounds); }
    juce::Rectangle<int> orbBounds; float demoTheta = 0.0f;
    juce::AudioProcessorValueTreeState& state;
    // No controls - they are handled by the 5x4 grid in Group 2 panel
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MotionPanel)
};
}