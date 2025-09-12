
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
    std::unique_ptr<juce::ComboBox> pannerSelect, path, mode, quant;
    std::unique_ptr<juce::Slider> rate, depth, phase, spread, elevBias, bounce, jitter, holdMs, sens, offsetDeg, frontBias, doppler, motionSend, bassFloor;
    std::unique_ptr<juce::ToggleButton> retrig, anchor, headphoneSafe;
    using Att = juce::AudioProcessorValueTreeState::SliderAttachment;
    using Btn = juce::AudioProcessorValueTreeState::ButtonAttachment;
    using Box = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<Att> aRate, aDepth, aPhase, aSpread, aElevBias, aBounce, aJitter, aHold, aSens, aOffset, aFront, aDoppler, aMotionSend, aBassFloor;
    std::unique_ptr<Btn> aRetrig, aAnchor, aHeadSafe;
    std::unique_ptr<Box> aPanner, aPath, aMode, aQuant;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MotionPanel)
};
}
